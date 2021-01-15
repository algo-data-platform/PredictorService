#include "predictor/server/feature_extractor/fea_extract_snapshot.h"
#include "predictor/server/model_manager/framework/model_framework.h"
#include "predictor/util/predictor_util.h"
DECLARE_int32(fea_extract_snapshot_frequency);

namespace predictor {

void originalFeatureExtractSnapshot(const PredictRequest& request) {
  // original common features
  const std::vector<feature_master::Feature>& common_feature_list = request.get_common_features().get_features();
  std::string ori_common_features = "";
  for (const auto& feature : common_feature_list) {
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
    ori_common_features += feature.get_feature_name() + ":" + value_list_str + "|";
  }
  if (!ori_common_features.empty())
    ori_common_features.pop_back();

  // original item features
  std::string ori_item_features_list = "";
  for (auto item_features_iter = request.get_item_features().begin();
        item_features_iter != request.get_item_features().end(); item_features_iter++) {
    const auto& item_features = item_features_iter->second.get_features();
    std::string ori_item_features = "";
    for (const auto& feature : item_features) {
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
      ori_item_features += feature.get_feature_name() + ":" + value_list_str + "|";
    }
    if (!ori_item_features.empty())
      ori_item_features.pop_back();
    ori_item_features = "AdId:" + std::to_string(item_features_iter->first) + CTRL_A + ori_item_features;
    ori_item_features_list += ori_item_features + CTRL_B;
  }
  if (!ori_item_features_list.empty())
    ori_item_features_list.pop_back();

  std::string msg_ori_features = "ReqId:" + request.get_req_id() +
                                  "\tType:OriginalFeatures\t" + ori_common_features +
                                  "\t" + ori_item_features_list;
  ResourceMgr::fea_extract_snapshot_producer_->sendKafka(msg_ori_features);
}

bool shouldSnapshot(const std::string& req_id) {
  auto hash_req = CityHash64(req_id.c_str(), req_id.size());
  uint32_t seed = hash_req & 0xFFFFFFFF;
  if (rand_r(&seed) % FLAGS_fea_extract_snapshot_frequency == 0) {
    return true;
  }
  return false;
}

}  // namespace predictor

