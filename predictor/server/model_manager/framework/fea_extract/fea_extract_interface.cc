#include "fea_extract_interface.h"

namespace predictor {

bool FeaExtractInterface::extractParameters(
    std::map<int64_t, std::vector<std::string>> *parameters,
    const std::vector<feature_master::Feature> &common_features,
    const std::map<int64_t, std::vector<feature_master::Feature>> &item_features) {
  if (!parameters) {
    LOG(ERROR) << "paramters is nullptr";
    return false;
  }

  // extract common features
  std::vector<std::string> common_parameters;
  if (!getCommonFea(&common_parameters, common_features)) {
    LOG(ERROR) << "failed to get common parameters";
    return false;
  }

  // extract item features
  for (const auto &kv : item_features) {
    std::vector<std::string> item_parameters;
    if (!getItemFea(&item_parameters, kv.second, common_features)) {
      LOG(ERROR) << "failed to get item parameters";
      return false;
    }

    std::vector<std::string> parameter_list;
    if (!getCrossFea(&parameter_list, &common_parameters, &item_parameters)) {
      LOG(ERROR) << "failed to get cross parameters";
      return false;
    }

    (*parameters)[kv.first] = std::move(parameter_list);
  }

  return true;
}

bool FeaExtractInterface::extractFeatureMasterParameters(feature_master::Parameters *extract_features,
                                                         const feature_master::Features &item_features,
                                                         const feature_master::Features &common_features) {
  return false;
}

bool FeaExtractInterface::indexFeature(FeatureMap &feature_map,
                                       const std::vector<feature_master::Feature> &features) {  // NOLINT
  for (auto &feature : features) {
    // Fetch raw feature value.
    feature_map[feature.get_feature_name()] = &feature;
  }
  return true;
}

void FeaExtractInterface::transform_int64(int64_t *key, const std::string &name, const FeatureMap &feature_map,
                                          int64_t default_value) {
  *key = default_value;
  auto it = feature_map.find(name);
  if (it != feature_map.end()) {
    const feature_master::Feature *feature = it->second;
    const auto &int64_values = feature->get_int64_values();
    if (int64_values.size() != 1) {
      LOG(ERROR) << "Wrong size of input feature list" << name;
      return;
    }
    if (feature->get_feature_type() == feature_master::FeatureType::INT64_LIST) {
      *key = int64_values[0];
    } else {
      LOG(ERROR) << "feature name : " << name << "type error";
    }
  }
}

void FeaExtractInterface::transform_int(int32_t *key, const std::string &name, const FeatureMap &feature_map,
                                        int32_t default_value) {
  *key = default_value;
  auto it = feature_map.find(name);
  if (it != feature_map.end()) {
    const feature_master::Feature *feature = it->second;
    const auto &int64_values = feature->get_int64_values();
    if (int64_values.size() != 1) {
      LOG(ERROR) << "Wrong size of input feature list" << name;
      return;
    }
    if (feature->get_feature_type() == feature_master::FeatureType::INT64_LIST) {
      *key = int64_values[0];
    } else {
      LOG(ERROR) << "feature name : " << name << "type error";
    }
  }
}

void FeaExtractInterface::transform_float(double *key, const std::string &name, const FeatureMap &feature_map,
                                          double default_value) {
  *key = default_value;
  auto it = feature_map.find(name);
  if (it != feature_map.end()) {
    const feature_master::Feature *feature = it->second;
    const auto &float_values = feature->get_float_values();
    if (float_values.size() != 1) {
      LOG(ERROR) << "Wrong size of input feature list" << name;
      return;
    }
    if (feature->get_feature_type() == feature_master::FeatureType::FLOAT_LIST) {
      *key = float_values[0];
    } else {
      LOG(ERROR) << "feature name : " << name << "type error";
    }
  }
}

void FeaExtractInterface::transform_string(std::string *key, const std::string &name, const FeatureMap &feature_map) {
  *key = "";
  auto it = feature_map.find(name);
  if (it != feature_map.end()) {
    const feature_master::Feature *feature = it->second;
    const auto &string_values = feature->get_string_values();
    if (string_values.size() != 1) {
      LOG(ERROR) << "Wrong size of input feature list: " << name;
      return;
    }
    if (feature->get_feature_type() == feature_master::FeatureType::STRING_LIST) {
      *key = string_values[0];
    } else {
      LOG(ERROR) << "feature name : " << name << "type error";
    }
  }
}

void FeaExtractInterface::transform_bool(bool *key, const std::string &name, const FeatureMap &feature_map,
                                         bool default_value) {
  *key = default_value;
  auto it = feature_map.find(name);
  if (it != feature_map.end()) {
    const feature_master::Feature *feature = it->second;
    const auto &int64_values = feature->get_int64_values();
    if (int64_values.size() != 1) {
      LOG(ERROR) << "Wrong size of input feature list" << name;
      return;
    }
    if (feature->get_feature_type() == feature_master::FeatureType::INT64_LIST) {
      *key = int64_values[0] == 1 ? true : false;
    } else {
      LOG(ERROR) << "feature name : " << name << "type error";
    }
  }
}

bool FeaExtractInterface::extractXGboostParameters(std::vector<std::pair<uint64_t, double>> *parameters,
                                                   const std::vector<feature_master::Feature> &common_features,
                                                   const std::vector<feature_master::Feature> &item_features) {
  return false;
}

}  // namespace predictor
