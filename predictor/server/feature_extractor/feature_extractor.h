#pragma once

#include <string>
#include "common/factory_registry.h"
#include "thirdparty/rapidjson/document.h"
#include "feature_master/if/gen-cpp2/feature_master_types.h"
#include "feature_master/parameter/parameter_utils.h"
#include "feature_master/parameter/parameter_extractor.h"
#include <any>
#include "common/util.h"


namespace predictor {

template<class T>
T* getPtrFromAny(const std::any& a) {
  T *ptr;

  try {
    ptr = std::any_cast<T*>(a);
  } catch (std::bad_any_cast& e) {
    ptr = nullptr;
  }

  return ptr;
}

/*
 *  BASE CLASS OF ALL FEATURE_EXTRACTOR
 */
class FeatureExtractor {
 public:
  virtual ~FeatureExtractor() = default;

  virtual bool init(const std::string& path_prefix, const rapidjson::Document& document) = 0;

  //  抽取公共特征
  virtual bool extractCommonFeatures(const std::any& extracted_features_ptr,
                                     const std::vector<feature_master::Feature>& features) { return true; }

  //  抽取物料特征
  virtual bool extractItemFeatures(const std::any& extracted_features_ptr,
                                   const std::vector<feature_master::Feature>& item_features,
                                   const std::vector<feature_master::Feature>& common_features) = 0;
};

// registry to get the feature_extractor
struct FeatureExtractorFactory
    : public common::FunctionRegEntryBase<FeatureExtractorFactory,
        std::function<std::shared_ptr<FeatureExtractor>()> > {};

#define REGISTER_EXTRACTOR(ClassName, Name)                                    \
  COMMON_REGISTRY_REGISTER(::predictor::FeatureExtractorFactory, FeatureExtractorFactory, Name) \
      .set_body([]() { return std::shared_ptr<FeatureExtractor>(new ClassName()); })

}  // namespace predictor
