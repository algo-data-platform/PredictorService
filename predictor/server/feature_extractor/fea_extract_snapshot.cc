#include "fea_extract_snapshot.h"
#include "predictor/server/model_manager/framework/model_framework.h"
#include "predictor/util/predictor_util.h"
DECLARE_int32(fea_extract_snapshot_frequency);

namespace predictor {
void originalFeatureSnapshot(const PredictRequest& request) {
  // original common features
  const std::vector<feature_master::Feature>& common_feature_list = request.get_common_features().get_features();
  std::string original_common_features = "";
  generateOriginalFeatures(&original_common_features, common_feature_list);

  // original item features
  std::string original_item_features_list = "";
  for (auto item_features_iter = request.get_item_features().begin();
        item_features_iter != request.get_item_features().end(); item_features_iter++) {
    const auto& item_features = item_features_iter->second.get_features();
    std::string original_item_features = "";
    generateOriginalFeatures(&original_item_features, item_features);
    original_item_features = "AdId:" + std::to_string(item_features_iter->first) + CTRL_A + original_item_features;
    original_item_features_list += original_item_features + CTRL_B;
  }
  if (!original_item_features_list.empty())
    original_item_features_list.pop_back();

  std::string msg_original_features = "ReqId:" + request.get_req_id() +
                                  "\tType:OriginalFeatures\t" + original_common_features +
                                  "\t" + original_item_features_list;
  ResourceMgr::fea_extract_snapshot_producer_->sendKafka(msg_original_features);
}

void originalFeatureSnapshot(const CalculateVectorRequest& request) {
  // original features
  const std::vector<feature_master::Feature>& feature_list = request.get_features().get_features();
  std::string original_features = "";
  generateOriginalFeatures(&original_features, feature_list);

  std::string msg_original_features = "ReqId:" + request.get_req_id() +
                                  "\tType:OriginalFeatures\t" + original_features;
  ResourceMgr::fea_extract_snapshot_producer_->sendKafka(msg_original_features);
}

void predictRespSnapshot(const PredictResponse& response, const std::string& model_name) {
  // predict results
  const std::map<int64_t, feature_master::PredictResults>& results_map = response.get_results_map();
  std::string results = "";
  for (const auto& ad : results_map) {
    std::string result = "";
    for (auto it = ad.second.preds.begin(); it != ad.second.preds.end(); it++) {
      result += it->first + ":" + std::to_string(it->second) + "|";
    }
    if (!result.empty())
      result.pop_back();
    results += "AdId:" + std::to_string(ad.first) + CTRL_A + result + CTRL_B;
  }
  if (!results.empty())
    results.pop_back();
  std::string msg_predict_results = "ReqId:" + response.get_req_id() + "\tModelName:" + model_name
                                    + "\tType:PredictResults\t" + results;
  ResourceMgr::fea_extract_snapshot_producer_->sendKafka(msg_predict_results);
}

bool shouldSnapshot(const std::string& req_id) {
  auto hash_req = CityHash64(req_id.c_str(), req_id.size());
  uint32_t seed = hash_req & 0xFFFFFFFF;
  if (rand_r(&seed) % FLAGS_fea_extract_snapshot_frequency == 0) {
    return true;
  }
  return false;
}

void generateOriginalFeatures(std::string* original_features,
                              const std::vector<feature_master::Feature>& feature_list) {
  for (const auto& feature : feature_list) {
    std::string value_list_str = "";
    if (feature.get_feature_type() == feature_master::FeatureType::STRING_LIST) {
      const auto& value_list = feature.get_string_values();
      for (const auto& i : value_list) {
        value_list_str += i + ";";
      }
    } else if (feature.get_feature_type() == feature_master::FeatureType::FLOAT_LIST) {
      const auto& value_list = feature.get_float_values();
      for (const auto& i : value_list) {
        value_list_str += std::to_string(i) + ";";
      }
    } else if (feature.get_feature_type() == feature_master::FeatureType::INT64_LIST) {
      const auto& value_list = feature.get_int64_values();
      for (const auto& i : value_list) {
        value_list_str += std::to_string(i) + ";";
      }
    }
    if (!value_list_str.empty())
      value_list_str.pop_back();
    *original_features += feature.get_feature_name() + ":" + value_list_str + "|";
  }
  if (!original_features->empty())
    original_features->pop_back();
}
}  // namespace predictor
