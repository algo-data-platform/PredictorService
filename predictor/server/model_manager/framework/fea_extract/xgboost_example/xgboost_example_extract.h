#pragma once
#include "predictor/server/model_manager/framework/fea_extract/fea_extract_interface.h"
#include "folly/Function.h"

namespace predictor {
class XGBoostExample_Extract : public FeaExtractInterface {
 public:
  XGBoostExample_Extract() {fea_extract_type_ = FEALIB_XGBOOST_EXAMPLE;}
  bool init(const std::string& path_prefix, const rapidjson::Document& document) override;
  bool getCommonFea(std::vector<std::string> *extract_features,
                   const std::vector<feature_master::Feature>& features) override;
  bool getItemFea(std::vector<std::string> *extract_features,
                   const std::vector<feature_master::Feature>& item_features,
                   const std::vector<feature_master::Feature>& common_features) override;
  bool getCrossFea(std::vector<std::string> *weight, std::vector<std::string>* common_features,
         std::vector<std::string>* item_features) override;

  bool extractXGboostParameters(std::vector<std::pair<uint64_t, double>>* parameters,
                          const std::vector<feature_master::Feature>& common_features,
                          const std::vector<feature_master::Feature>& item_features);

 private:
  void getFeatures(std::vector<std::pair<uint64_t, double>>* result,
                    const std::vector<feature_master::Feature> &features);
  bool readEncodeConfig(const std::string &filepath);

 private:
  std::map<std::string, uint64_t> fea_encode_;
};
}  //  namespace predictor
