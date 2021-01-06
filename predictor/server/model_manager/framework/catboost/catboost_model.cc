#include "predictor/server/model_manager/framework/catboost/catboost_model.h"
#include "feature_master/if/gen-cpp2/feature_master_types.h"
#include "common/util.h"
#include "predictor/util/predictor_util.h"
#include "predictor/util/predictor_constants.h"
#include "predictor/server/resource_manager.h"
#include <folly/ScopeGuard.h>

DECLARE_int64(min_items_per_thread);
DECLARE_int64(heavy_tasks_thread_num);
DECLARE_int64(feature_extract_tasks_num);

namespace predictor {
bool CatboostModel::init(const std::string& path_prefix, const rapidjson::Document& document) {
  if (!document.HasMember(COMMON_MODEL_FILE)) {
    LOG(ERROR) << "cannot find key=" << COMMON_MODEL_FILE << "in config file=" << path_prefix;
    return false;
  }
  catboost_calcer_.InitFromFile(common::pathJoin(path_prefix, document[COMMON_MODEL_FILE].GetString()));
  return true;
}

bool CatboostModel::predict(PredictResponse* response, const PredictRequest& request) {
  response->set_req_id(request.get_req_id());  // set req_id in response regardless
  if (request.get_item_features().empty()) {
    LOG(INFO) << "request item_features are empty, nothing to do, model_full_name=" << model_full_name_
              << ", req_id=" << request.get_req_id();
    return true;
  }

  std::vector<common::Range> range_vec;
  common::split_slice(&range_vec, request.get_item_features().size(),
                    FLAGS_min_items_per_thread, FLAGS_feature_extract_tasks_num);
  std::vector<folly::Future<PredRes> > future_results;
  for (const auto& range : range_vec) {
    auto range_predict = [this, range, &request]() mutable {
      PredRes range_res;
      auto item_features_iter = std::next(request.get_item_features().begin(), range.begin);
      for (unsigned i = range.begin; i < range.end; ++i) {
        const auto &item_features = item_features_iter->second.get_features();
        SCOPE_EXIT {
          if (item_features_iter != request.get_item_features().end()) {
            ++item_features_iter;
          }
        };
        if (item_features.size() != 2 ||
            item_features[0].feature_type != feature_master::FeatureType::FLOAT_LIST ||
            item_features[1].feature_type != feature_master::FeatureType::STRING_LIST) {
          FB_LOG_EVERY_MS(ERROR, 2000)
            << "item_features.size() != 2 OR item_features[0] is not float OR item_features[1] is not string"
            << ", model_full_name=" << model_full_name_
            << ", req_id=" << request.get_req_id();
          continue;
        }

        const auto &adid = item_features_iter->first;
        // Calc() only takes in float vectors, we must convert double features to float here
        const std::vector<double> &double_features = item_features[0].float_values;
        std::vector<float> float_features(double_features.begin(), double_features.end());
        const auto &cat_features = item_features[1].string_values;
        std::map<std::string, double> preds;
        try {
          preds["ctr"] = catboost_calcer_.Calc(float_features, cat_features);  // Calc() can throw
        } catch(const std::exception &e) {
          FB_LOG_EVERY_MS(ERROR, 2000) << "catboost_calcer_.Calc() threw exception=" << e.what()
            << ", model_full_name=" << model_full_name_ << ", req_id=" << request.get_req_id();
          continue;
        }
        range_res.emplace(adid, std::move(preds));
      }
      return range_res;
    };
    future_results.emplace_back(ResourceMgr::getHeavyTasksThreadPool()->addFuture(range_predict));
  }

  // sync wait for all results
  std::map<int64_t, feature_master::PredictResults> results_map;
  folly::collectAll(future_results)
      .then([&results_map](const std::vector<folly::Try<PredRes> >& list) {
        for (auto& v : list) {
          if (v.hasException()) {
            LOG(ERROR) << "range_predict got exception";
            continue;
          }
          for (auto& t : v.value()) {
            feature_master::PredictResults preds;
            preds.set_preds(t.second);
            results_map.emplace(t.first, std::move(preds));
          }
        }
      })
      .get();

  response->set_results_map(std::move(results_map));
  return true;
}
REGISTER_MODEL(CatboostModel, catboost).describe("This is catboost model of catboost framework.");
};  // namespace predictor
