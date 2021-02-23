#pragma once

#include <string>
#include "predictor/if/gen-cpp2/predictor_config_types.tcc"
#include "folly/GLog.h"
#include "common/file/file_util.h"
#include "common/serialize_util.h"

namespace predictor {
namespace util {
template <class T>
bool initFromFile(T *config, const std::string &config_filename) {
  if (config_filename.empty()) {
    return false;
  }
  std::string json_file_content;
  if (!common::FileUtil::read_file_str(&json_file_content, config_filename)) {
    return false;
  }
  if (!common::deserializeThriftObjFromSimpleJson(config, json_file_content)) {
    return false;
  }
  LOG(INFO) << "Successfuly inited Config from file=" << config_filename;
  return true;
}
std::string getModelFullName(const ModelConfig &config);
}  // namespace util
}  // namespace predictor
