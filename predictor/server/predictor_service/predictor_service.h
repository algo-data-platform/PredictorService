#pragma once
#include <string>
#include <atomic>

#include "predictor/if/gen-cpp2/PredictorService.h"
#include "predictor/server/model_manager/model_manager.h"
#include "predictor/server/router/predictor_router.h"
#include "predictor/global_resource/kafka_producer.h"

namespace predictor {

class PredictorService : virtual public PredictorServiceSvIf {
 public:
  PredictorService() = default;
  ~PredictorService() = default;

  explicit PredictorService(std::shared_ptr<ModelManager> model_manager) : model_manager_(model_manager) {}
  void predict(PredictResponses& predict_responses, std::unique_ptr<PredictRequests> predict_requests_ptr) override;
  void multiPredict(MultiPredictResponse& multi_predict_response,
                    std::unique_ptr<MultiPredictRequest> multi_predict_request) override;
  void calculateVector(CalculateVectorResponses& calculate_vector_responses,
                       std::unique_ptr<CalculateVectorRequests> calculate_vector_requests_ptr) override;
  void calculateBatchVector(CalculateBatchVectorResponses& calculate_batch_vector_responses,
                       std::unique_ptr<CalculateBatchVectorRequests> calculate_batch_vector_requests_ptr) override;
  bool init();
  template<typename T> void send_req_example_snapshot(const T& req);

 private:
  std::shared_ptr<ModelManager> model_manager_;
  PredictorRouter predictor_router_;
  KafkaProducer example_snapshot_producer_;
  std::atomic_ullong example_snapshot_atomic_count_;
};

}  // namespace predictor
