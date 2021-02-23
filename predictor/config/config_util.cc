#include "predictor/config/config_util.h"

namespace {
constexpr char NAME_DELIMITER[] = "_";
}  // namespace

namespace predictor {
namespace util {
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
}  // namespace util
}  // namespace predictor
