#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include "predictor/if/gen-cpp2/PredictorService.h"
#include "sdk/predictor/predictor_client_sdk.h"

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
  void calculateBatchVector(CalculateBatchVectorResponses* calculate_batch_vector_responses,
                            std::unique_ptr<CalculateBatchVectorRequests> calculate_batch_vector_requests_ptr);

  void combineMultiPredictResponse(MultiPredictResponse* combined_response, const MultiPredictResponse& slice_response);

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
  bool setRequestOptionPredictorServiceName(RequestOption* request_option, const std::vector<std::string>& model_name);
  void sendFeatureRecordRequest(const std::unique_ptr<MultiPredictRequest>& multi_predict_request);
  void getAndCombineMultiPredictResponse(MultiPredictResponse* combined_response, const RequestOption& request_option,
                                   const std::vector<std::unique_ptr<MultiPredictResponseFuture>>& slice_reponse_futs);
};

}  // namespace predictor
