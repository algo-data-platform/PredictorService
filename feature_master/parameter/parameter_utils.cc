#include "feature_master/parameter/parameter_utils.h"

#include <fstream>
#include <sstream>

#include "common/util.h"

namespace {

//  constexpr char LOG_CATEGORY[] = "parameter_utils.cc";

}  // namespace

namespace feature_master {

std::unique_ptr<feature_master::ParameterExtractorManager> createParameterExtractorManager(
    const std::string& config_file_name) {
  std::ifstream fin(config_file_name);
  std::stringstream buffer;
  buffer << fin.rdbuf();
  auto feature_extractor_json_str = buffer.str();
  return std::make_unique<feature_master::ParameterExtractorManager>(feature_extractor_json_str);
}

}  // namespace feature_master
