#pragma once

#include <vector>
#include <thirdparty/rapidjson/document.h>
#include "feature_master/if/gen-cpp2/feature_master_types.h"
#include "feature_master/parameter/parameter_utils.h"
#include "feature_master/parameter/parameter_extractor.h"
#include "predictor/util/predictor_util.h"

namespace predictor {

class FeaExtractInterface {
 public:
  FeaExtractInterface() {}
  virtual ~FeaExtractInterface() {}
  virtual bool init(const std::string& path_prefix, const rapidjson::Document& document) = 0;
  virtual bool getCommonFea(std::vector<std::string>* extract_features,
                            const std::vector<feature_master::Feature>& features) = 0;
  virtual bool getItemFea(std::vector<std::string>* extract_features,
                          const std::vector<feature_master::Feature>& item_features,
                          const std::vector<feature_master::Feature>& common_features) = 0;
  virtual bool getCrossFea(std::vector<std::string>* parameters, std::vector<std::string>* common_features,
                           std::vector<std::string>* item_features) = 0;

  // for some feature extraction libs, the interface could be as simple as this
  virtual bool extractParameters(std::map<int64_t, std::vector<std::string>>* parameters,
                                 const std::vector<feature_master::Feature>& common_features,
                                 const std::map<int64_t, std::vector<feature_master::Feature>>& item_features);

  virtual bool extractXGboostParameters(std::vector<std::pair<uint64_t, double>>* parameters,
                                        const std::vector<feature_master::Feature>& common_features,
                                        const std::vector<feature_master::Feature>& item_features);

  // for feature_master feature_extract
  virtual bool extractFeatureMasterParameters(feature_master::Parameters* extract_features,
                                              const feature_master::Features& item_features,
                                              const feature_master::Features& common_features);

  std::string fea_extract_type_;
  const std::string& getFeaExtractType() {return fea_extract_type_;}

 protected:
  using FeatureMap = std::map<std::string, const feature_master::Feature*>;

  // index input feature vector into feature_map
  bool indexFeature(FeatureMap& feature_map, const std::vector<feature_master::Feature>& features);  // NOLINT

  void transform_int64(int64_t* key, const std::string& name, const FeatureMap& feature_map,
                       int64_t default_value = -1);
  void transform_int(int32_t* key, const std::string& name, const FeatureMap& feature_map, int32_t default_value = -1);
  void transform_float(double* key, const std::string& name, const FeatureMap& feature_map,
                       double default_value = -1.0);
  void transform_string(std::string* key, const std::string& name, const FeatureMap& feature_map);
  void transform_bool(bool* key, const std::string& name, const FeatureMap& feature_map, bool default_value = false);
};
}  //  namespace predictor
