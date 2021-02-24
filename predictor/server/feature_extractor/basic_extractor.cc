#include "basic_extractor.h"

namespace predictor {

using FeatureMasterMap = std::unordered_map<std::string, const feature_master::Feature*>;

bool BasicExtractor::extractItemFeatures(const std::any& extracted_features_ptr,
                          const std::vector<feature_master::Feature>& item_features,
                          const std::vector<feature_master::Feature>& common_features) {
  auto fea_map = getPtrFromAny<FeatureMasterMap>(extracted_features_ptr);
  if (!fea_map) {
    FB_LOG_EVERY_MS(ERROR, 2000) << "Invalid extracted_feature type!";
    return false;
  }

  for (const auto& common_feature : common_features) {
    const std::string& feature_name = common_feature.get_feature_name();
    fea_map->emplace(feature_name, &common_feature);
  }
  for (const auto& item_feature : item_features) {
    const std::string& feature_name = item_feature.get_feature_name();
    fea_map->emplace(feature_name, &item_feature);
  }

  return true;
}

REGISTER_EXTRACTOR(BasicExtractor, basic_extractor).describe("This is a basic feature extractor.");
}  // namespace predictor
