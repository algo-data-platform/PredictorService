#pragma once
#include <string>
#include <atomic>

#include "predictor/if/gen-cpp2/PredictorService.h"
#include "predictor/server/model_manager/model_manager.h"
#include "predictor/server/router/predictor_router.h"
#include "predictor/server/kafka_producer.h"

namespace predictor {

class PredictorService : virtual public PredictorServiceSvIf {
 public:
  PredictorService() = default;
  ~PredictorService() = default;

  void predict(PredictResponses& predict_responses, std::unique_ptr<PredictRequests> predict_requests_ptr) override;
  void multiPredict(MultiPredictResponse& multi_predict_response,
                    std::unique_ptr<MultiPredictRequest> multi_predict_request) override;
  void calculateVector(CalculateVectorResponses& calculate_vector_responses,
                       std::unique_ptr<CalculateVectorRequests> calculate_vector_requests_ptr) override;
  bool init(const std::string& model_path);
  bool load_model(const std::string& model_file,
                  bool is_full_path = false,
                  const std::string &model_name = "model_name_not_set",
                  const std::string &business_line = "business_line_not_set");
  std::shared_ptr<std::map<std::string, int>> get_model_downgrade_percent_map() const;
  void set_model_downgrade_percent_map(std::shared_ptr<std::map<std::string, int>> tmp_map_ptr);
  template<typename T> void send_req_example_snapshot(const T& req);

  ModelManager& getModelManager() { return model_manager_; }

 private:
  ModelManager model_manager_;
  PredictorRouter predictor_router_;
  KafkaProducer example_snapshot_producer_;
  std::atomic_ullong example_snapshot_atomic_count_;
};

}  // namespace predictor
