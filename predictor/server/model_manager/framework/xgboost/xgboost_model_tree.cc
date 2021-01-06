#include <fstream>
#include <sys/types.h>
#include <folly/ScopeGuard.h>

#include "predictor/server/model_manager/framework/xgboost/xgboost_model_tree.h"
#include "predictor/server/model_manager/framework/fea_extract/xgboost_example/xgboost_example_extract.h"
#include "predictor/util/predictor_constants.h"
#include "predictor/util/predictor_util.h"
#include "glog/logging.h"
#include "common/util.h"

namespace predictor {

namespace {
constexpr char LOG_CATEGORY[] = "xgboost_model_tree.cc: ";
using PredsMap = std::map<int64_t, std::map<std::string, double>>;
}  // namespace

XGBoostModelTree::XGBoostModelTree() {}

bool XGBoostModelTree::init(const std::string& path_prefix, const rapidjson::Document& document) {
  if (!document.HasMember(COMMON_MODEL_FILE)) {
    LOG(ERROR) << "config file is missing model file!";
    return false;
  }
  std::string path = path_prefix + "/";
  bool load_ret = xgboost_predictor_.load(path + document[COMMON_MODEL_FILE].GetString());
  if (!load_ret) {
    LOG(ERROR) << "xgboost load model failed!";
    return false;
  }

  std::string fea_lib_name = document[FEALIB_CONFIG].GetString();
  if (fea_lib_name == FEALIB_XGBOOST_EXAMPLE) {
    fea_extractor_ = std::make_shared<XGBoostExample_Extract>();
  }
  if (!fea_extractor_) {
    LOG(ERROR) << "fea extractor : " << fea_lib_name << " create fail";
    return false;
  }
  if (!fea_extractor_->init(path_prefix, document)) {
    LOG(ERROR) << "fea extractor : " << fea_lib_name << " init fail";
    return false;
  }

  return true;
}

bool XGBoostModelTree::predict(PredictResponse* predict_response, const PredictRequest& request) {
  std::map<int64_t, feature_master::PredictResults> results_map;

  std::vector<std::vector<std::pair<uint64_t, double>>> rows;
  std::vector<int64_t> itemid_vector;

  const auto& common_features = request.get_common_features().get_features();
  for (const auto& predict_request : request.get_item_features()) {
    const auto& features = predict_request.second.get_features();
    std::vector<std::pair<uint64_t, double>> features_row;
    bool ret_getFea = fea_extractor_->extractXGboostParameters(&features_row, features, common_features);
    if (!ret_getFea) {
      LOG(ERROR) << "request id=" << request.get_req_id()
          << " model name=" << model_full_name_
          << " item id=" << predict_request.first << " getItemFea error";
      continue;
    }
    rows.push_back(std::move(features_row));
    itemid_vector.push_back(predict_request.first);
  }
  // Comput ctr ysing the extracted feature values and the model data.
  std::vector<double> xgboost_preds;
  bool predictor_ret = xgboost_predictor_.predict(rows, &xgboost_preds);
  if (!predictor_ret) {
    LOG(ERROR) << "xgboost predict function error";
    return false;
  }
  if (xgboost_preds.size() != itemid_vector.size()) {
    LOG(ERROR) << "xgboost preds != item size";
    return false;
  }
  for (uint64_t i = 0; i < xgboost_preds.size(); ++i) {
    double &ctr = xgboost_preds[i];
    std::map<std::string, double> preds;
    preds["ctr"] = ctr;  // question: is ctr the only label?
    feature_master::PredictResults predict_results;
    predict_results.set_preds(std::move(preds));
    results_map[itemid_vector[i]] = std::move(predict_results);
  }
  predict_response->set_req_id(request.get_req_id());
  predict_response->set_results_map(std::move(results_map));
  return true;
}

REGISTER_MODEL(XGBoostModelTree, tree).describe("This is tree model of xgboost.");

}  // namespace predictor
