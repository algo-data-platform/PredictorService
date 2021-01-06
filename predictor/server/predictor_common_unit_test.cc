#include "glog/logging.h"
#include "gtest/gtest.h"

#include "predictor/util/predictor_util.h"
namespace predictor {

class PredictorCommonTest : public ::testing::Test {
 protected:
  void SetUp() override { google::InitGoogleLogging("PredictorCommonTest"); }
  void TearDown() override { google::ShutdownGoogleLogging(); }
};

TEST_F(PredictorCommonTest, Sanity) { EXPECT_EQ(1, 1); }

TEST_F(PredictorCommonTest, restoreFromNegativeSampling) {
  double origin = 0.3;
  double negative_sampling_ratio = 1;
  double ret = util::restoreFromNegativeSampling(origin, negative_sampling_ratio);
  EXPECT_EQ(ret, origin);

  negative_sampling_ratio = 0.1;
  ret = util::restoreFromNegativeSampling(origin, negative_sampling_ratio);
  EXPECT_DOUBLE_EQ(ret, 0.041095890410958909);
}

}  // namespace predictor
