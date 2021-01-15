#include "gtest/gtest.h"
#include "glog/logging.h"

#include "predictor/config/config_util.h"

namespace predictor {
class ConfigTest : public ::testing::Test {
 protected:
  void SetUp() override { google::InitGoogleLogging("ConfigTest"); }
  void TearDown() override { google::ShutdownGoogleLogging(); }
};

TEST_F(ConfigTest, Sanity) { EXPECT_EQ(1, 1); }

TEST_F(ConfigTest, ModelConfig_init_good) {
  std::string config_filename = "config/testdata/good_model_config.json";
  ModelConfig model_config;
  bool ret = initFromFile(&model_config, config_filename);
  EXPECT_TRUE(ret);
  EXPECT_EQ(model_config.model_framework, "tf");
  EXPECT_EQ(model_config.model_class,     "estimator_multi");
  EXPECT_EQ(model_config.feature_class,   "xfea_extractor");
  EXPECT_EQ(model_config.version,         "v0");
  EXPECT_EQ(model_config.model_name,      "cpl_ctr");
  EXPECT_EQ(model_config.additional,      "incr");
  EXPECT_EQ(model_config.model_file,      "");
  EXPECT_EQ(getModelFullName(model_config), "tf_estimator_multi_xfea_extractor_v0_cpl_ctr_incr");
}

TEST_F(ConfigTest, ModelConfig_init_no_such_file) {
  std::string config_filename = "config/testdata/no_such_file.json";
  ModelConfig model_config;
  bool ret = initFromFile(&model_config, config_filename);
  EXPECT_FALSE(ret);
  EXPECT_EQ(model_config.model_framework, "");
  EXPECT_EQ(model_config.model_class,     "");
  EXPECT_EQ(model_config.feature_class,   "");
  EXPECT_EQ(model_config.version,         "");
  EXPECT_EQ(model_config.model_name,      "");
  EXPECT_EQ(model_config.additional,      "");
  EXPECT_EQ(model_config.model_file,      "");
  EXPECT_EQ(getModelFullName(model_config), "");
}

TEST_F(ConfigTest, ModelConfig_init_invalid_json) {
  std::string config_filename = "config/testdata/invalid_json_model_config.json";
  ModelConfig model_config;
  bool ret = initFromFile(&model_config, config_filename);
  EXPECT_FALSE(ret);
  EXPECT_EQ(model_config.model_framework, "");
  EXPECT_EQ(model_config.model_class,     "");
  EXPECT_EQ(model_config.feature_class,   "");
  EXPECT_EQ(model_config.version,         "");
  EXPECT_EQ(model_config.model_name,      "");
  EXPECT_EQ(model_config.additional,      "");
  EXPECT_EQ(model_config.model_file,      "");
  EXPECT_EQ(getModelFullName(model_config), "");
}

TEST_F(ConfigTest, ModelConfig_init_field_not_string) {
  std::string config_filename = "config/testdata/field_not_string_model_config.json";
  ModelConfig model_config;
  bool ret = initFromFile(&model_config, config_filename);

  EXPECT_FALSE(ret);
  EXPECT_EQ(model_config.model_framework, "tf");
  EXPECT_EQ(model_config.model_class,     "estimator_multi");
  EXPECT_EQ(model_config.feature_class,   "");
  EXPECT_EQ(model_config.version,         "");
  EXPECT_EQ(model_config.model_name,      "");
  EXPECT_EQ(model_config.additional,      "");
  EXPECT_EQ(model_config.model_file,      "");
}
}  // namespace predictor
