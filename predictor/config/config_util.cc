#include "predictor/config/config_util.h"

#include "folly/GLog.h"
#include "common/file/file_util.h"
#include "common/serialize_util.h"

namespace {
constexpr char LOG_CATEGORY[] = "config_util.cc";

constexpr char NAME_DELIMITER[] = "_";
constexpr char FAIL_PREFIX[] = "Failed to init ModelConfig: ";
}  // namespace

namespace predictor {
bool initFromFile(ModelConfig *config, const std::string &config_filename) {
  if (config_filename.empty()) {
    LOG(ERROR) << FAIL_PREFIX << "config_filename is empty.";
    return false;
  }

  std::string json_file_content;
  if (!common::FileUtil::read_file_str(&json_file_content, config_filename)) {
    LOG(ERROR) << FAIL_PREFIX << "failed to read file content from config_filename=" << config_filename;
    return false;
  }

  if (!common::deserializeThriftObjFromSimpleJson(config, json_file_content)) {
    LOG(ERROR) << FAIL_PREFIX << "failed to deserialize json to obejct"
               << ", config_filename=" << config_filename
               << ", json_file_content=" << json_file_content;
    return false;
  }

  LOG(INFO) << "Successfuly inited ModelConfig from file=" << config_filename;
  return true;
}

std::string getModelFullName(const ModelConfig &config) {
  if (config.model_framework.empty()) {
    return "";
  }

  return config.model_framework + NAME_DELIMITER +
         config.model_class     + NAME_DELIMITER +
         config.feature_class   + NAME_DELIMITER +
         config.version         + NAME_DELIMITER +
         config.model_name      + NAME_DELIMITER +
         config.additional;
}
}  // namespace predictor
