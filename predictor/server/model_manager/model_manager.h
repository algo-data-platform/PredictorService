#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include "predictor/if/gen-cpp2/PredictorService.h"
#include "folly/concurrency/AtomicSharedPtr.h"
#include "folly/Synchronized.h"
#include "folly/executors/CPUThreadPoolExecutor.h"
#include "folly/executors/FutureExecutor.h"

namespace predictor {

typedef std::pair<std::string, PredictResponse> ResultPair;

class ModelManager {
 public:
  ModelManager() = default;
  ~ModelManager() = default;
  ModelManager(const ModelManager&) = delete;
  ModelManager(ModelManager&&) = delete;
  ModelManager& operator=(const ModelManager&) = delete;
  ModelManager& operator=(ModelManager&&) = delete;

  // param: model location path
  bool init(const std::string& model_path);
  /**
   * param: model_file: file name with suffix of .json
   * param: is_full_path  file is or is not full path
   */
  bool load_model(const std::string& model_file,
                  bool is_full_path = false,
                  const std::string &model_name = "model_name_not_set",
                  const std::string &business_line = "business_line_not_set");
  void load_all_models();
  void predict_multithread(PredictResponses* predict_responses,
                           std::unique_ptr<PredictRequests> predict_requests_ptr);
  void multiPredict(MultiPredictResponse& multi_predict_response,  // NOLINT
                    std::unique_ptr<MultiPredictRequest> multi_predict_request);
  void calculateVector(CalculateVectorResponses* calculate_vector_responses,
                       std::unique_ptr<CalculateVectorRequests> calculate_vector_requests_ptr);
  bool shouldDowngradeModel(const std::string& model_full_name) const;
  void calculateBatchVector(CalculateBatchVectorResponses* calculate_batch_vector_responses,
                            std::unique_ptr<CalculateBatchVectorRequests> calculate_batch_vector_requests_ptr);
  std::shared_ptr<std::map<std::string, int>> get_model_downgrade_percent_map() const;
  void set_model_downgrade_percent_map(std::shared_ptr<std::map<std::string, int>> tmp_map_ptr);
  bool setModelBusinessLine(const std::string &model_name, const std::string &business_line);
  void setThreadPool(int max_thread_num);

 private:
  std::string model_path_;
  std::shared_ptr<folly::FutureExecutor<folly::CPUThreadPoolExecutor>> cpu_thread_pool_;
  // map<model_name, downgrade_percent>
  std::shared_ptr<std::map<std::string, int>> model_downgrade_percent_map_ptr_;

  ResultPair singlePredict(const std::string &model_full_name, const PredictRequest &predict_request) const;
  CalculateBatchVectorResponse singleCalculateBatchVector(const CalculateBatchVectorRequest &batch_request) const;
};

}  // namespace predictor
