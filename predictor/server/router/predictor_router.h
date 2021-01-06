#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include "predictor/if/gen-cpp2/PredictorService.h"

namespace predictor {

class PredictorRouter {
 public:
  PredictorRouter() = default;
  ~PredictorRouter() = default;
  PredictorRouter(const PredictorRouter&) = delete;
  PredictorRouter(PredictorRouter&&) = delete;
  PredictorRouter& operator=(const PredictorRouter&) = delete;
  PredictorRouter& operator=(PredictorRouter&&) = delete;

  // init PredictorRouter
  static bool init();

  // interface
  void predict(PredictResponses* predict_responses,
               std::unique_ptr<PredictRequests> predict_requests_ptr);
  void multiPredict(MultiPredictResponse* multi_predict_response,
                    std::unique_ptr<MultiPredictRequest> multi_predict_request);
  void calculateVector(CalculateVectorResponses* calculate_vector_responses,
                       std::unique_ptr<CalculateVectorRequests> calculate_vector_requests_ptr);

 private:
  void predictSimpleForward(PredictResponses* predict_responses,
               std::unique_ptr<PredictRequests> predict_requests_ptr);
  void multiPredictSimpleForward(MultiPredictResponse* multi_predict_response,
                          std::unique_ptr<MultiPredictRequest> multi_predict_request);
  void calculateVectorSimpleForward(CalculateVectorResponses* calculate_vector_responses,
                       std::unique_ptr<CalculateVectorRequests> calculate_vector_requests_ptr);
  void multiPredictSplitRequest(MultiPredictResponse* multi_predict_response,
                          std::unique_ptr<MultiPredictRequest> multi_predict_request);
  static bool InitPredictorConsulFallback();
};

}  // namespace predictor
