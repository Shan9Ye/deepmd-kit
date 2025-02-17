#include <gtest/gtest.h>

#include <algorithm>
#include <cmath>
#include <fstream>
#include <vector>

#include "deepmd.hpp"
#include "test_utils.h"

template <class VALUETYPE>
class TestInferDeepPotModeDevi : public ::testing::Test {
 protected:
  std::vector<VALUETYPE> coord = {12.83, 2.56, 2.18, 12.09, 2.87, 2.74,
                                  00.25, 3.32, 1.68, 3.36,  3.00, 1.81,
                                  3.51,  2.51, 2.60, 4.27,  3.22, 1.56};
  std::vector<int> atype = {0, 1, 1, 0, 1, 1};
  std::vector<VALUETYPE> box = {13., 0., 0., 0., 13., 0., 0., 0., 13.};
  int natoms;

  deepmd::hpp::DeepPot dp0;
  deepmd::hpp::DeepPot dp1;
  deepmd::hpp::DeepPotModelDevi dp_md;

  void SetUp() override {
    {
      std::string file_name = "../../tests/infer/deeppot.pbtxt";
      deepmd::hpp::convert_pbtxt_to_pb("../../tests/infer/deeppot.pbtxt",
                                       "deeppot.pb");
      dp0.init("deeppot.pb");
    }
    {
      std::string file_name = "../../tests/infer/deeppot-1.pbtxt";
      deepmd::hpp::convert_pbtxt_to_pb("../../tests/infer/deeppot-1.pbtxt",
                                       "deeppot-1.pb");
      dp1.init("deeppot-1.pb");
    }
    dp_md.init(std::vector<std::string>({"deeppot.pb", "deeppot-1.pb"}));
  };

  void TearDown() override {
    remove("deeppot.pb");
    remove("deeppot-1.pb");
  };
};

TYPED_TEST_SUITE(TestInferDeepPotModeDevi, ValueTypes);

template <class VALUETYPE>
class TestInferDeepPotModeDeviPython : public ::testing::Test {
 protected:
  std::vector<VALUETYPE> coord = {
      4.170220047025740423e-02, 7.203244934421580703e-02,
      1.000114374817344942e-01, 4.053881673400336005e+00,
      4.191945144032948461e-02, 6.852195003967595510e-02,
      1.130233257263184132e+00, 1.467558908171130543e-02,
      1.092338594768797883e-01, 1.862602113776709242e-02,
      1.134556072704304919e+00, 1.396767474230670159e-01,
      5.120445224973151355e+00, 8.781174363909455272e-02,
      2.738759319792616331e-03, 4.067046751017840300e+00,
      1.141730480236712753e+00, 5.586898284457517128e-02,
  };
  std::vector<int> atype = {0, 0, 1, 1, 1, 1};
  std::vector<VALUETYPE> box = {20., 0., 0., 0., 20., 0., 0., 0., 20.};
  int natoms;
  std::vector<VALUETYPE> expected_md_f = {0.509504727653, 0.458424067748,
                                          0.481978258466};  // max min avg
  std::vector<VALUETYPE> expected_md_v = {0.167004837423, 0.00041822790564,
                                          0.0804864867641};  // max min avg

  deepmd::hpp::DeepPot dp0;
  deepmd::hpp::DeepPot dp1;
  deepmd::hpp::DeepPotModelDevi dp_md;

  void SetUp() override {
    {
      std::string file_name = "../../tests/infer/deeppot.pbtxt";
      deepmd::hpp::convert_pbtxt_to_pb("../../tests/infer/deeppot.pbtxt",
                                       "deeppot.pb");
      dp0.init("deeppot.pb");
    }
    {
      std::string file_name = "../../tests/infer/deeppot-1.pbtxt";
      deepmd::hpp::convert_pbtxt_to_pb("../../tests/infer/deeppot-1.pbtxt",
                                       "deeppot-1.pb");
      dp1.init("deeppot-1.pb");
    }
    dp_md.init(std::vector<std::string>({"deeppot.pb", "deeppot-1.pb"}));
  };

  void TearDown() override {
    remove("deeppot.pb");
    remove("deeppot-1.pb");
  };
};

TYPED_TEST_SUITE(TestInferDeepPotModeDeviPython, ValueTypes);

TYPED_TEST(TestInferDeepPotModeDevi, attrs) {
  using VALUETYPE = TypeParam;
  std::vector<VALUETYPE>& coord = this->coord;
  std::vector<int>& atype = this->atype;
  std::vector<VALUETYPE>& box = this->box;
  int& natoms = this->natoms;
  deepmd::hpp::DeepPot& dp0 = this->dp0;
  deepmd::hpp::DeepPot& dp1 = this->dp1;
  deepmd::hpp::DeepPotModelDevi& dp_md = this->dp_md;
  EXPECT_EQ(dp0.cutoff(), dp_md.cutoff());
  EXPECT_EQ(dp0.numb_types(), dp_md.numb_types());
  // EXPECT_EQ(dp0.dim_fparam(), dp_md.dim_fparam());
  // EXPECT_EQ(dp0.dim_aparam(), dp_md.dim_aparam());
  EXPECT_EQ(dp1.cutoff(), dp_md.cutoff());
  EXPECT_EQ(dp1.numb_types(), dp_md.numb_types());
  // EXPECT_EQ(dp1.dim_fparam(), dp_md.dim_fparam());
  // EXPECT_EQ(dp1.dim_aparam(), dp_md.dim_aparam());
}

TYPED_TEST(TestInferDeepPotModeDevi, cpu_lmp_list) {
  using VALUETYPE = TypeParam;
  std::vector<VALUETYPE>& coord = this->coord;
  std::vector<int>& atype = this->atype;
  std::vector<VALUETYPE>& box = this->box;
  int& natoms = this->natoms;
  deepmd::hpp::DeepPot& dp0 = this->dp0;
  deepmd::hpp::DeepPot& dp1 = this->dp1;
  deepmd::hpp::DeepPotModelDevi& dp_md = this->dp_md;
  float rc = dp_md.cutoff();
  int nloc = coord.size() / 3;
  std::vector<VALUETYPE> coord_cpy;
  std::vector<int> atype_cpy, mapping;
  std::vector<std::vector<int> > nlist_data;
  _build_nlist<VALUETYPE>(nlist_data, coord_cpy, atype_cpy, mapping, coord,
                          atype, box, rc);
  int nall = coord_cpy.size() / 3;
  std::vector<int> ilist(nloc), numneigh(nloc);
  std::vector<int*> firstneigh(nloc);
  deepmd::hpp::InputNlist inlist(nloc, &ilist[0], &numneigh[0], &firstneigh[0]);
  deepmd::hpp::convert_nlist(inlist, nlist_data);

  int nmodel = 2;
  std::vector<double> edir(nmodel), emd;
  std::vector<std::vector<VALUETYPE> > fdir_(nmodel), fdir(nmodel),
      vdir(nmodel), fmd_, fmd(nmodel), vmd;
  dp0.compute(edir[0], fdir_[0], vdir[0], coord_cpy, atype_cpy, box,
              nall - nloc, inlist, 0);
  dp1.compute(edir[1], fdir_[1], vdir[1], coord_cpy, atype_cpy, box,
              nall - nloc, inlist, 0);
  dp_md.compute(emd, fmd_, vmd, coord_cpy, atype_cpy, box, nall - nloc, inlist,
                0);
  for (int kk = 0; kk < nmodel; ++kk) {
    _fold_back<VALUETYPE>(fdir[kk], fdir_[kk], mapping, nloc, nall, 3);
    _fold_back<VALUETYPE>(fmd[kk], fmd_[kk], mapping, nloc, nall, 3);
  }

  EXPECT_EQ(edir.size(), emd.size());
  EXPECT_EQ(fdir.size(), fmd.size());
  EXPECT_EQ(vdir.size(), vmd.size());
  for (int kk = 0; kk < nmodel; ++kk) {
    EXPECT_EQ(fdir[kk].size(), fmd[kk].size());
    EXPECT_EQ(vdir[kk].size(), vmd[kk].size());
  }
  for (int kk = 0; kk < nmodel; ++kk) {
    EXPECT_LT(fabs(edir[kk] - emd[kk]), EPSILON);
    for (int ii = 0; ii < fdir[0].size(); ++ii) {
      EXPECT_LT(fabs(fdir[kk][ii] - fmd[kk][ii]), EPSILON);
    }
    for (int ii = 0; ii < vdir[0].size(); ++ii) {
      EXPECT_LT(fabs(vdir[kk][ii] - vmd[kk][ii]), EPSILON);
    }
  }
}

TYPED_TEST(TestInferDeepPotModeDevi, cpu_lmp_list_atomic) {
  using VALUETYPE = TypeParam;
  std::vector<VALUETYPE>& coord = this->coord;
  std::vector<int>& atype = this->atype;
  std::vector<VALUETYPE>& box = this->box;
  int& natoms = this->natoms;
  deepmd::hpp::DeepPot& dp0 = this->dp0;
  deepmd::hpp::DeepPot& dp1 = this->dp1;
  deepmd::hpp::DeepPotModelDevi& dp_md = this->dp_md;
  float rc = dp_md.cutoff();
  int nloc = coord.size() / 3;
  std::vector<VALUETYPE> coord_cpy;
  std::vector<int> atype_cpy, mapping;
  std::vector<std::vector<int> > nlist_data;
  _build_nlist<VALUETYPE>(nlist_data, coord_cpy, atype_cpy, mapping, coord,
                          atype, box, rc);
  int nall = coord_cpy.size() / 3;
  std::vector<int> ilist(nloc), numneigh(nloc);
  std::vector<int*> firstneigh(nloc);
  deepmd::hpp::InputNlist inlist(nloc, &ilist[0], &numneigh[0], &firstneigh[0]);
  deepmd::hpp::convert_nlist(inlist, nlist_data);

  int nmodel = 2;
  std::vector<double> edir(nmodel), emd;
  std::vector<std::vector<VALUETYPE> > fdir_(nmodel), fdir(nmodel),
      vdir(nmodel), fmd_, fmd(nmodel), vmd, aedir(nmodel), aemd, avdir(nmodel),
      avdir_(nmodel), avmd(nmodel), avmd_;
  dp0.compute(edir[0], fdir_[0], vdir[0], aedir[0], avdir_[0], coord_cpy,
              atype_cpy, box, nall - nloc, inlist, 0);
  dp1.compute(edir[1], fdir_[1], vdir[1], aedir[1], avdir_[1], coord_cpy,
              atype_cpy, box, nall - nloc, inlist, 0);
  dp_md.compute(emd, fmd_, vmd, aemd, avmd_, coord_cpy, atype_cpy, box,
                nall - nloc, inlist, 0);
  for (int kk = 0; kk < nmodel; ++kk) {
    _fold_back<VALUETYPE>(fdir[kk], fdir_[kk], mapping, nloc, nall, 3);
    _fold_back<VALUETYPE>(fmd[kk], fmd_[kk], mapping, nloc, nall, 3);
    _fold_back<VALUETYPE>(avdir[kk], avdir_[kk], mapping, nloc, nall, 9);
    _fold_back<VALUETYPE>(avmd[kk], avmd_[kk], mapping, nloc, nall, 9);
  }

  EXPECT_EQ(edir.size(), emd.size());
  EXPECT_EQ(fdir.size(), fmd.size());
  EXPECT_EQ(vdir.size(), vmd.size());
  EXPECT_EQ(aedir.size(), aemd.size());
  EXPECT_EQ(avdir.size(), avmd.size());
  for (int kk = 0; kk < nmodel; ++kk) {
    EXPECT_EQ(fdir[kk].size(), fmd[kk].size());
    EXPECT_EQ(vdir[kk].size(), vmd[kk].size());
    EXPECT_EQ(aedir[kk].size(), aemd[kk].size());
    EXPECT_EQ(avdir[kk].size(), avmd[kk].size());
  }
  for (int kk = 0; kk < nmodel; ++kk) {
    EXPECT_LT(fabs(edir[kk] - emd[kk]), EPSILON);
    for (int ii = 0; ii < fdir[0].size(); ++ii) {
      EXPECT_LT(fabs(fdir[kk][ii] - fmd[kk][ii]), EPSILON);
    }
    for (int ii = 0; ii < vdir[0].size(); ++ii) {
      EXPECT_LT(fabs(vdir[kk][ii] - vmd[kk][ii]), EPSILON);
    }
    for (int ii = 0; ii < aedir[0].size(); ++ii) {
      EXPECT_LT(fabs(aedir[kk][ii] - aemd[kk][ii]), EPSILON);
    }
    for (int ii = 0; ii < avdir[0].size(); ++ii) {
      EXPECT_LT(fabs(avdir[kk][ii] - avmd[kk][ii]), EPSILON);
    }
  }
}

template <class VALUETYPE>
inline VALUETYPE mymax(const std::vector<VALUETYPE>& xx) {
  VALUETYPE ret = 0;
  for (int ii = 0; ii < xx.size(); ++ii) {
    if (xx[ii] > ret) {
      ret = xx[ii];
    }
  }
  return ret;
};
template <class VALUETYPE>
inline VALUETYPE mymin(const std::vector<VALUETYPE>& xx) {
  VALUETYPE ret = 1e10;
  for (int ii = 0; ii < xx.size(); ++ii) {
    if (xx[ii] < ret) {
      ret = xx[ii];
    }
  }
  return ret;
};
template <class VALUETYPE>
inline VALUETYPE myavg(const std::vector<VALUETYPE>& xx) {
  VALUETYPE ret = 0;
  for (int ii = 0; ii < xx.size(); ++ii) {
    ret += xx[ii];
  }
  return (ret / xx.size());
};
template <class VALUETYPE>
inline VALUETYPE mystd(const std::vector<VALUETYPE>& xx) {
  VALUETYPE ret = 0;
  for (int ii = 0; ii < xx.size(); ++ii) {
    ret += xx[ii] * xx[ii];
  }
  return sqrt(ret / xx.size());
};
