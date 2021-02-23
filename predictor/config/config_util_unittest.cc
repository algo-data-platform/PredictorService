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

// ModelConfig
TEST_F(ConfigTest, ModelConfig_init_valid) {
  std::string config_filename = "config/testdata/ModelConfig_valid.json";
  ModelConfig model_config;
  bool ret = util::initFromFile(&model_config, config_filename);
  EXPECT_TRUE(ret);
  EXPECT_EQ(model_config.model_framework, "tf");
  EXPECT_EQ(model_config.model_class,     "estimator_multi");
  EXPECT_EQ(model_config.feature_class,   "xfea_extractor");
  EXPECT_EQ(model_config.version,         "v0");
  EXPECT_EQ(model_config.model_name,      "cpl_ctr");
  EXPECT_EQ(model_config.additional,      "incr");
  EXPECT_EQ(model_config.model_file,      "./model.txt");
  EXPECT_EQ(util::getModelFullName(model_config), "tf_estimator_multi_xfea_extractor_v0_cpl_ctr_incr");
}

TEST_F(ConfigTest, ModelConfig_init_optional_field) {
  std::string config_filename = "config/testdata/ModelConfig_optional.json";
  ModelConfig model_config;
  bool ret = util::initFromFile(&model_config, config_filename);
  EXPECT_TRUE(ret);
  EXPECT_EQ(model_config.model_framework, "tf");
  EXPECT_EQ(model_config.model_class,     "estimator_multi");
  EXPECT_EQ(model_config.feature_class,   "xfea_extractor");
  EXPECT_EQ(model_config.version,         "v0");
  EXPECT_EQ(model_config.model_name,      "cpl_ctr");
  EXPECT_EQ(model_config.additional,      "NA");
  EXPECT_EQ(model_config.model_file,      "");
  EXPECT_EQ(util::getModelFullName(model_config), "tf_estimator_multi_xfea_extractor_v0_cpl_ctr_NA");
}

TEST_F(ConfigTest, ModelConfig_init_no_such_file) {
  std::string config_filename = "config/testdata/no_such_file.json";
  ModelConfig model_config;
  bool ret = util::initFromFile(&model_config, config_filename);
  EXPECT_FALSE(ret);
  EXPECT_EQ(model_config.model_framework, "");
  EXPECT_EQ(model_config.model_class,     "");
  EXPECT_EQ(model_config.feature_class,   "");
  EXPECT_EQ(model_config.version,         "");
  EXPECT_EQ(model_config.model_name,      "");
  EXPECT_EQ(model_config.additional,      "NA");
  EXPECT_EQ(model_config.model_file,      "");
  EXPECT_EQ(util::getModelFullName(model_config), "");
}

TEST_F(ConfigTest, ModelConfig_init_missing_field) {
  std::string config_filename = "config/testdata/ModelConfig_missing_field.json";
  ModelConfig model_config;
  bool ret = util::initFromFile(&model_config, config_filename);
  EXPECT_FALSE(ret);
  EXPECT_EQ(model_config.model_framework, "tf");
  EXPECT_EQ(model_config.model_class,     "");
  EXPECT_EQ(model_config.feature_class,   "xfea_extractor");
  EXPECT_EQ(model_config.version,         "v0");
  EXPECT_EQ(model_config.model_name,      "cpl_ctr");
  EXPECT_EQ(model_config.additional,      "incr");
  EXPECT_EQ(model_config.model_file,      "");
  EXPECT_EQ(util::getModelFullName(model_config), "tf__xfea_extractor_v0_cpl_ctr_incr");
}

TEST_F(ConfigTest, ModelConfig_init_invalid_json) {
  std::string config_filename = "config/testdata/ModelConfig_invalid_json.json";
  ModelConfig model_config;
  bool ret = util::initFromFile(&model_config, config_filename);
  EXPECT_FALSE(ret);
  EXPECT_EQ(model_config.model_framework, "");
  EXPECT_EQ(model_config.model_class,     "");
  EXPECT_EQ(model_config.feature_class,   "");
  EXPECT_EQ(model_config.version,         "");
  EXPECT_EQ(model_config.model_name,      "");
  EXPECT_EQ(model_config.additional,      "NA");
  EXPECT_EQ(model_config.model_file,      "");
  EXPECT_EQ(util::getModelFullName(model_config), "");
}

TEST_F(ConfigTest, ModelConfig_init_field_not_string) {
  std::string config_filename = "config/testdata/ModelConfig_field_not_string.json";
  ModelConfig model_config;
  bool ret = util::initFromFile(&model_config, config_filename);

  EXPECT_FALSE(ret);
  EXPECT_EQ(model_config.model_framework, "tf");
  EXPECT_EQ(model_config.model_class,     "estimator_multi");
  EXPECT_EQ(model_config.feature_class,   "");
  EXPECT_EQ(model_config.version,         "");
  EXPECT_EQ(model_config.model_name,      "");
  EXPECT_EQ(model_config.additional,      "NA");
  EXPECT_EQ(model_config.model_file,      "");
}

// TfConfig
TEST_F(ConfigTest, TfConfig_init_valid) {
  std::string config_filename = "config/testdata/TfConfig_valid.json";
  TfConfig tf_config;
  bool ret = util::initFromFile(&tf_config, config_filename);
  EXPECT_TRUE(ret);
  EXPECT_EQ(tf_config.export_dir, "tf_model");
  std::vector<std::string> tags = {"serve"};
  EXPECT_EQ(tf_config.tags, tags);
  EXPECT_EQ(tf_config.signature_def, "serving_default");
  std::vector<std::string> output_tensor_names = {
    "rank/query/fully_connected/BiasAdd", "rank/bias/fully_connected_1/BiasAdd"
  };
  EXPECT_EQ(tf_config.output_tensor_names, output_tensor_names);
  EXPECT_EQ(tf_config.model_timestamp, "20210113140845");
  EXPECT_EQ(tf_config.negative_sampling_ratio, 1.0);
}

TEST_F(ConfigTest, TfConfig_init_optional) {
  std::string config_filename = "config/testdata/TfConfig_optional.json";
  TfConfig tf_config;
  bool ret = util::initFromFile(&tf_config, config_filename);
  EXPECT_TRUE(ret);
  EXPECT_EQ(tf_config.export_dir, "tf_model");
  std::vector<std::string> tags = {"serve"};
  EXPECT_EQ(tf_config.tags, tags);
  EXPECT_EQ(tf_config.signature_def, "serving_default");
    std::vector<std::string> output_tensor_names = {
    "rank/query/fully_connected/BiasAdd", "rank/bias/fully_connected_1/BiasAdd"
  };
  EXPECT_EQ(tf_config.output_tensor_names, output_tensor_names);
  EXPECT_EQ(tf_config.model_timestamp, "");
  EXPECT_EQ(tf_config.negative_sampling_ratio, 1.0);
}
}  // namespace predictor
