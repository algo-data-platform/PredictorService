#include "predictor/server/model_manager/framework/model.h"
#include "predictor/server/feature_extractor/feature_extractor.h"
#include "predictor/util/predictor_util.h"


namespace common {
COMMON_REGISTRY_ENABLE(predictor::ModelFactory);
}  // namespace common

namespace predictor {
bool Model::load(const ModelConfig &model_config,
                 const std::string &model_package_dir,
                 const std::string &business_line) {
  model_full_name_   = util::getModelFullName(model_config);
  model_config_      = model_config;
  model_package_dir_  = model_package_dir;
  business_line_     = business_line;

  if (!initFeatureExtractor()) {
    return false;
  }
  if (!loadModelFile()) {
    return false;
  }
  return true;
}

bool Model::initFeatureExtractor() {
  auto factory = common::FactoryRegistry<FeatureExtractorFactory>::Find(model_config_.feature_class);
  if (!factory) {
    util::logAndMetricError(model_config_.feature_class + "-invalid_feature_class");
    return false;
  }
  feature_extractor_ = factory->body();
  if (!feature_extractor_->init(model_package_dir_)) {
    util::logAndMetricError(model_config_.feature_class + "-feature_class_init_fail");
    return false;
  }
  return true;
}
}  // namespace predictor
