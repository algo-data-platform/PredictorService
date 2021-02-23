#pragma once

#include "feature_extractor.h"

namespace predictor {

class BasicExtractor : public FeatureExtractor {
 public:
  BasicExtractor() = default;
  NO_COPY_NO_MOVE(BasicExtractor);
  bool init(const std::string& model_package_dir) override { return true; }
  bool extractItemFeatures(const std::any& extracted_features_ptr,
                           const std::vector<feature_master::Feature>& item_features,
                           const std::vector<feature_master::Feature>& common_features) override;
};

}  //  namespace predictor
