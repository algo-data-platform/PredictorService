#pragma once

#include <string>
#include "common/factory_registry.h"
#include "predictor/if/gen-cpp2/PredictorService.h"
#include "thirdparty/rapidjson/document.h"
#include "predictor/server/model_manager/framework/fea_extract/fea_extract_interface.h"
#include "predictor/server/feature_extractor/feature_extractor.h"
#include "predictor/util/predictor_util.h"
#include "predictor/util/predictor_constants.h"
#include "predictor/config/config_util.h"
#include "predictor/global_resource/resource_manager.h"

DECLARE_int64(min_items_per_thread);
DECLARE_int64(feature_extract_tasks_num);

namespace predictor {

/*
 *  BASE CLASS OF ALL MODELS
 */
class Model {
 public:
  //
  // constructors
  //
  virtual ~Model() = default;

  //
  // api functions
  //
  virtual bool init(const std::string& path_prefix, const rapidjson::Document& document) { return true; }  // old api
  virtual bool load(const ModelConfig &model_config,
                    const std::string &model_package_dir,
                    const std::string &business_line);  // new api
  virtual bool loadModelFile() { return true; }
  virtual bool initFeatureExtractor();
  virtual bool predict(PredictResponse* predict_response, const PredictRequest& predict_request) { return false; }
  virtual bool calculateVector(CalculateVectorResponse* calculate_vector_response,
                               const CalculateVectorRequest& calculate_vector_request) { return false; }
  virtual bool calculateBatchVector(CalculateBatchVectorResponse* batch_response,
                                    const CalculateBatchVectorRequest& batch_request) { return false; }

 protected:
  template <class FeatureType>
  void extractFeaturesInParallel(std::vector<std::pair<int64_t, FeatureType>> *extracted_features,
                                 const PredictRequest& request,
                                 folly::FutureExecutor<folly::CPUThreadPoolExecutor>* cpu_thread_pool =
                                 ResourceMgr::getHeavyTasksThreadPool(),
                                 int min_item_per_task = FLAGS_min_items_per_thread,
                                 int max_tasks_num = FLAGS_feature_extract_tasks_num) {
    if (!extracted_features || !cpu_thread_pool ||
        min_item_per_task < 1 || max_tasks_num < 1) {
      FB_LOG_EVERY_MS(ERROR, 2000) << "invalid parameters";
      return;
    }

    std::vector<common::Range> range_vec;
    common::split_slice(&range_vec, request.get_item_features().size(), min_item_per_task, max_tasks_num);

    std::vector<folly::Future<bool>> results;
    std::mutex lock;
    for (const auto& range : range_vec) {
      auto extractFeatures = [this, range, &request, extracted_features, &lock]() mutable {
        const std::vector<feature_master::Feature>& common_feature_list = request.get_common_features().get_features();
        auto item_features_iter = std::next(request.get_item_features().begin(), range.begin);
        for (unsigned i = range.begin; i < range.end; ++i) {
          const auto& item_features = item_features_iter->second.get_features();
          int64_t adid = item_features_iter->first;
          FeatureType extracted_feature;
          if (!this->feature_extractor_->extractItemFeatures(&extracted_feature, item_features, common_feature_list)) {
            FB_LOG_EVERY_MS(ERROR, 2000) << "extractItemFeatures failed for adid " << adid;
          } else {
            std::lock_guard<std::mutex> lg(lock);
            extracted_features->emplace_back(adid, std::move(extracted_feature));
          }
          if (item_features_iter != request.get_item_features().end()) {
            ++item_features_iter;
          }
        }
        return true;
      };
      results.emplace_back(cpu_thread_pool->addFuture(extractFeatures));
    }

    folly::collectAll(results)
        .then([](const std::vector<folly::Try<bool>>& lists) {
          for (auto& v : lists) {
            if (v.hasException()) {
              FB_LOG_EVERY_MS(ERROR, 2000) << "extractFeatures got exception";
            }
            if (!v.value()) {
              FB_LOG_EVERY_MS(ERROR, 2000) << "extractFeatures failed";
            }
          }
        })
        .get();
  }


 public:
  //
  // member variables
  //
  ModelConfig model_config_;
  std::string model_package_dir_;
  std::string model_full_name_;
  std::string business_line_;
  std::shared_ptr<FeaExtractInterface> fea_extractor_;  // to be deprecated
  std::shared_ptr<FeatureExtractor> feature_extractor_;
};

// registry to get the models
struct ModelFactory
    : public common::FunctionRegEntryBase<ModelFactory, std::function<std::shared_ptr<Model>()> > {};

#define REGISTER_MODEL(ClassName, Name)                                    \
  COMMON_REGISTRY_REGISTER(::predictor::ModelFactory, ModelFactory, Name) \
      .set_body([]() { return std::shared_ptr<Model>(new ClassName()); })

}  // namespace predictor
