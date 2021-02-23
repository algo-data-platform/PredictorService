#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include "predictor/if/gen-cpp2/PredictorService.h"
#include "folly/concurrency/AtomicSharedPtr.h"
#include "folly/Synchronized.h"
#include "folly/executors/CPUThreadPoolExecutor.h"
#include "folly/executors/FutureExecutor.h"
#include "predictor/config/config_util.h"
#include "common/util.h"

namespace predictor {

typedef std::pair<std::string, PredictResponse> ResultPair;

class ModelManager {
 public:
  ModelManager() = default;
  ~ModelManager() = default;
  NO_COPY_NO_MOVE(ModelManager);

  bool init();

  // Model Load APIs
  bool loadModel(const ModelConfig &model_config,
                 const std::string &model_package_dir,
                 const std::string &business_line = "business_line_not_set") const;

  // Predict APIs
  void predict_multithread(PredictResponses* predict_responses,
                           std::unique_ptr<PredictRequests> predict_requests_ptr);
  void multiPredict(MultiPredictResponse& multi_predict_response,  // NOLINT
                    std::unique_ptr<MultiPredictRequest> multi_predict_request);

  // CalculateVector APIs
  void calculateVector(CalculateVectorResponses* calculate_vector_responses,
                       std::unique_ptr<CalculateVectorRequests> calculate_vector_requests_ptr);
  void calculateBatchVector(CalculateBatchVectorResponses* calculate_batch_vector_responses,
                            std::unique_ptr<CalculateBatchVectorRequests> calculate_batch_vector_requests_ptr);

  // Control APIs
  bool shouldDowngradeModel(const std::string& model_full_name) const;
  std::shared_ptr<std::map<std::string, int> > get_model_downgrade_percent_map() const;
  void set_model_downgrade_percent_map(std::shared_ptr<std::map<std::string, int>> tmp_map_ptr);
  bool setModelBusinessLine(const std::string &model_name, const std::string &business_line);
  void setThreadPool(int max_thread_num);

 private:
  // Implementations
  ResultPair singlePredict(const std::string &model_full_name, const PredictRequest &predict_request) const;
  CalculateVectorResponse singleCalculateVector(const CalculateVectorRequest &request) const;
  CalculateBatchVectorResponse singleCalculateBatchVector(const CalculateBatchVectorRequest &batch_request) const;

  // Member Variables
  std::shared_ptr<folly::FutureExecutor<folly::CPUThreadPoolExecutor>> cpu_thread_pool_;
  std::shared_ptr<std::map<std::string, int>> model_downgrade_percent_map_ptr_;  // model_name -> downgrade_percent
};

}  // namespace predictor
