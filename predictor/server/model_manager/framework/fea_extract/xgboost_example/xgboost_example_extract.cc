#include "xgboost_example_extract.h"
#include "predictor/util/predictor_util.h"
#include "common/util.h"
#include <iostream>
#include <fstream>
namespace predictor {

bool XGBoostExample_Extract::readEncodeConfig(const std::string &filepath) {
  std::ifstream input(filepath.c_str());
  if (!input) {
    return false;
  }

  uint64_t index;
  std::string name;
  while (input >> name >> index) {
    fea_encode_.insert(std::pair<std::string, uint64_t>(name, index));
  }

  input.close();
  return true;
}
bool XGBoostExample_Extract::init(const std::string& path_prefix, const rapidjson::Document& document) {
  if (!document.HasMember(COMMON_FEALIB_PATH)) {
    LOG(ERROR) << "config error : " << COMMON_FEALIB_PATH;
    return false;
  }
  const std::string configfile = common::pathJoin(path_prefix, document[COMMON_FEALIB_PATH].GetString());
  if (!readEncodeConfig(configfile)) {
    LOG(ERROR) << "readEncodeConfig error";
    return false;
  }

  return true;
}

bool XGBoostExample_Extract::getCommonFea(std::vector<std::string> *extract_features,
                   const std::vector<feature_master::Feature>& common_features) {
  return true;
}

bool XGBoostExample_Extract::getItemFea(std::vector<std::string> *extract_features,
                   const std::vector<feature_master::Feature>& item_features,
                   const std::vector<feature_master::Feature>& common_features) {
  return true;
}

bool XGBoostExample_Extract::getCrossFea(std::vector<std::string> *features, std::vector<std::string>* common_features,
         std::vector<std::string>* item_features) {
    return true;
}

bool XGBoostExample_Extract::extractXGboostParameters(std::vector<std::pair<uint64_t, double>>* parameters,
                          const std::vector<feature_master::Feature>& common_features,
                          const std::vector<feature_master::Feature>& item_features) {
  getFeatures(parameters, common_features);
  getFeatures(parameters, item_features);
  return true;
}

void XGBoostExample_Extract::getFeatures(std::vector<std::pair<uint64_t, double>>* result,
                                          const std::vector<feature_master::Feature> &features) {
  for (const auto& feature : features) {
    // Fetch raw feature value.
    const std::string& feature_name = feature.get_feature_name();
    auto code_iter = fea_encode_.find(feature_name);
    if (code_iter == fea_encode_.end()) {
      continue;
    }
    switch (feature.get_feature_type()) {
      case feature_master::FeatureType::FLOAT_LIST: {
        const auto& float_values = feature.get_float_values();
        if (float_values.size() != 1) {
          LOG(ERROR) << "feature : " << feature_name << " value size error!";
          continue;
        }
        result->push_back(std::move(std::make_pair(code_iter->second, float_values[0])));
      }
      default: { continue; }
    }
  }
  return;
}

}  // namespace predictor
