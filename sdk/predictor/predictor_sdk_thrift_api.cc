#include "predictor_client_sdk.h"
#include "predictor_sdk_util.h"
#include "common/serialize_util.h"
#include "folly/GLog.h"
#include "folly/Random.h"
#include "folly/init/Init.h"
#include "gflags/gflags.h"
#include "gflags/gflags_declare.h"
#include "feature_master/if/gen-cpp2/feature_master_types.tcc"
#include "predictor/if/gen-cpp2/predictor_types.tcc"
#include "predictor/if/gen-cpp2/PredictorService.h"
#include "predictor/if/gen-cpp2/PredictorServiceAsyncClient.h"
#include "service_router/http.h"
#include "service_router/thrift.h"
#include <dirent.h>
#include <sstream>
#include <sys/types.h>

namespace service_router {
DECLARE_string(router_consul_addresses);
DECLARE_int32(thrift_timeout_retry);
DECLARE_int32(thrift_connection_retry);
}  // namespace service_router

namespace predictor {
std::vector<PredictResponse> PredictResponsesThriftFutureImpl::get() {
  // blocks until the async call is done
  predictor::PredictResponses predict_responses;
  try {
    predict_responses = fut_ptr->get();
  } catch (const std::exception& e) {
    std::stringstream err_msg;
    err_msg << typeid(e).name() << ":" << e.what() << " req_ids: " << req_ids;
    LOG(ERROR) << SDK_MODULE_NAME << " fut_ptr->get() (thrift future_predict) caught exception: " << err_msg.str();
    markMeters(ASYNC_GET_EXCEPTION_METER, channel_model_names, service_name);
    return {};
  }
  // pack response
  const std::vector<PredictResponse>& responses = predict_responses.get_resps();
  if (responses.empty()) {
    LOG(ERROR) << SDK_MODULE_NAME << " Got back empty responses (thrift future_predict). req_ids: " << req_ids;
    markMeters(ASYNC_GET_EMPTY_RESP_METER, channel_model_names, service_name);
    return {};
  }
  return responses;
}

predictor::MultiPredictResponse MultiPredictResponseFutureImpl::get() {
  // blocks until the async call is done
  predictor::MultiPredictResponse predict_response;
  try {
    predict_response = fut_ptr->get();
  } catch (const std::exception& e) {
    std::stringstream err_msg;
    err_msg << typeid(e).name() << ":" << e.what() << " req_id: " << req_id;
    LOG(ERROR) << SDK_MODULE_NAME << " fut_ptr->get() (multi_predict) caught exception: " << err_msg.str();
    markMeters(ASYNC_GET_EXCEPTION_METER, model_names, channel, service_name, request_type);
    return {};
  }
  if (predict_response.get_model_responses().empty()) {
    LOG(ERROR) << SDK_MODULE_NAME << " Got back empty responses (multi_predict). request id: " << req_id;
    markMeters(ASYNC_GET_EMPTY_RESP_METER, model_names, channel, service_name, request_type);
    return {};
  }
  return predict_response;
}

std::vector<CalculateVectorResponse> CalculateVectorResponsesThriftFuture::get() {
  // blocks until the async call is done
  predictor::CalculateVectorResponses calculate_vector_responses;
  try {
    calculate_vector_responses = fut_ptr->get();
  } catch (const std::exception& e) {
    std::stringstream err_msg;
    err_msg << typeid(e).name() << ":" << e.what();
    LOG(ERROR) << SDK_MODULE_NAME << " calculate_vector fut_ptr->get() (thrift) caught exception: " << err_msg.str();
    markMeters(ASYNC_GET_EXCEPTION_METER, channel_model_names, service_name);
    return {};
  }
  // pack response
  const std::vector<CalculateVectorResponse>& responses = calculate_vector_responses.get_resps();
  if (responses.empty()) {
    LOG(ERROR) << SDK_MODULE_NAME << " calculate_vector Got back empty responses (thrift). req_ids: " << req_ids;
    markMeters(ASYNC_GET_EMPTY_RESP_METER, channel_model_names, service_name);
    return {};
  }
  return responses;
}

namespace client_util {
// make thrift feature from name and value
feature_master::Feature makeFeature(const std::string& name, int64_t value) {
  feature_master::Feature feature;
  feature.set_feature_name(name);
  feature.set_feature_type(feature_master::FeatureType::INT64_LIST);
  feature.set_int64_values(std::vector<int64_t>{value});
  return feature;
}
feature_master::Feature makeFeature(const std::string& name, double value) {
  feature_master::Feature feature;
  feature.set_feature_name(name);
  feature.set_feature_type(feature_master::FeatureType::FLOAT_LIST);
  feature.set_float_values(std::vector<double>{value});
  return feature;
}
feature_master::Feature makeFeature(const std::string& name, const std::string& value) {
  feature_master::Feature feature;
  feature.set_feature_name(name);
  feature.set_feature_type(feature_master::FeatureType::STRING_LIST);
  feature.set_string_values(std::vector<std::string>{value});
  return feature;
}
feature_master::Feature makeFeature(const std::string& name, const std::vector<int64_t>& value_vec) {
  feature_master::Feature feature;
  feature.set_feature_name(name);
  feature.set_feature_type(feature_master::FeatureType::INT64_LIST);
  feature.set_int64_values(value_vec);
  return feature;
}
feature_master::Feature makeFeature(const std::string& name, const std::vector<double>& value_vec) {
  feature_master::Feature feature;
  feature.set_feature_name(name);
  feature.set_feature_type(feature_master::FeatureType::FLOAT_LIST);
  feature.set_float_values(value_vec);
  return feature;
}
feature_master::Feature makeFeature(const std::string& name, const std::vector<std::string>& value_vec) {
  feature_master::Feature feature;
  feature.set_feature_name(name);
  feature.set_feature_type(feature_master::FeatureType::STRING_LIST);
  feature.set_string_values(value_vec);
  return feature;
}
}  // namespace client_util

// sync interface
bool PredictorClientSDK::predict(std::vector<PredictResponse>* responses,
                                 const PredictRequests& requests) {
  const auto& request_option = requests.get_request_option();
  const auto &reqs = requests.get_reqs();
  markMeters(SYNC_REQ_METER, reqs, request_option.predictor_service_name);
  predictor::PredictResponses predict_responses;
  // client predict
  const bool rc = service_router::thriftServiceCall<predictor::PredictorServiceAsyncClient>(
    makeServiceRouterClientOption(request_option),
    [&requests, &predict_responses, &request_option](auto client) {
      apache::thrift::RpcOptions rpc_options;
      rpc_options.setTimeout(std::chrono::milliseconds(request_option.request_timeout));
      try {
        client->sync_predict(rpc_options, predict_responses, requests);
      } catch (predictor::PredictException& ex) {
        FB_LOG_EVERY_MS(ERROR, 2000) << ex.get_message();
        throw ex;
      }
  });
  if (!rc) {
    LOG(ERROR) << SDK_MODULE_NAME << " Failed calling predictor service (thrift predict). req_ids: "
               << getReqIds(reqs);
    markMeters(SYNC_PREDICT_ERROR, reqs, request_option.predictor_service_name);
    return false;
  }
  // pack response
  if (predict_responses.get_resps().empty()) {
    LOG(ERROR) << SDK_MODULE_NAME << " Got back empty responses (thrift predict). req_ids: "
               << getReqIds(reqs);
    markMeters(SYNC_PREDICT_EMPTY, reqs, request_option.predictor_service_name);
  } else {
    *responses = std::move(predict_responses.get_resps());
  }
  return true;
}

// async interface
bool PredictorClientSDK::future_predict(std::unique_ptr<PredictResponsesThriftFuture>* response_future,
                                        const PredictRequests& requests) {
  const auto& request_option = requests.get_request_option();
  const auto &reqs = requests.get_reqs();
  markMeters(ASYNC_REQ_METER, reqs, request_option.predictor_service_name);
  // client predict
  const bool rc = service_router::thriftServiceCall<predictor::PredictorServiceAsyncClient>(
    makeServiceRouterClientOption(request_option),
    [&requests, &response_future, &request_option, &reqs](auto client) {
      apache::thrift::RpcOptions rpc_options;
      rpc_options.setTimeout(std::chrono::milliseconds(request_option.request_timeout));
      try {
        auto fut_impl = std::make_unique<PredictResponsesThriftFutureImpl>();
        fut_impl->fut_ptr = std::move(std::make_unique<folly::Future<predictor::PredictResponses>>(
            client->future_predict(rpc_options, requests)));
        fut_impl->req_ids = getReqIds(reqs);
        fut_impl->service_name = request_option.predictor_service_name;
        for (const auto& req : reqs) {
          fut_impl->channel_model_names.emplace_back(req.get_channel(), req.get_model_name());
        }
        *response_future = std::move(fut_impl);
      } catch (predictor::PredictException& ex) {
        FB_LOG_EVERY_MS(ERROR, 2000) << ex.get_message();
        throw ex;
      }
  });
  if (!rc) {
    LOG(ERROR) << "Failed calling predictor service (thrift future_predict). req_ids: " << getReqIds(reqs);
    markMeters(ASYNC_ERROR_METER, reqs, request_option.predictor_service_name);
    return false;
  }

  return true;
}

// multi_predict interface
bool PredictorClientSDK::multi_predict(MultiPredictResponse* response,
                                       const MultiPredictRequest& request) {
  const auto &request_option = request.get_request_option();
  markMeters(ASYNC_REQ_METER, request.get_model_names(), request.get_single_request().get_channel(),
             request_option.predictor_service_name);
  // client predict
  const bool rc = service_router::thriftServiceCall<predictor::PredictorServiceAsyncClient>(
    makeServiceRouterClientOption(request_option),
    [&request, &response, &request_option](auto client) {
      apache::thrift::RpcOptions rpc_options;
      rpc_options.setTimeout(std::chrono::milliseconds(request_option.request_timeout));
      try {
        client->sync_multiPredict(rpc_options, *response, request);
      } catch (predictor::PredictException& ex) {
        FB_LOG_EVERY_MS(ERROR, 2000) << ex.get_message();
        throw ex;
      }
  });
  if (!rc) {
    LOG(ERROR) << SDK_MODULE_NAME << " Failed calling predictor service (multi_predict). request id: "
               << request.get_req_id();
    markMeters(SYNC_PREDICT_ERROR, request.get_model_names(), request.get_single_request().get_channel(),
               request_option.predictor_service_name);
    return false;
  }
  // pack response
  if (response->get_model_responses().empty()) {  // empty response considered call success
    LOG(ERROR) << SDK_MODULE_NAME << " Got back empty responses (multi_predict). request id: "
               << request.get_req_id();
    markMeters(SYNC_PREDICT_EMPTY, request.get_model_names(), request.get_single_request().get_channel(),
               request_option.predictor_service_name);
  }
  return true;
}

// future_multi_predict interface
bool PredictorClientSDK::future_multi_predict(std::unique_ptr<MultiPredictResponseFuture>* response_future,
                                              const MultiPredictRequest &request) {
  const std::string request_type = getRequestType(request.single_request.get_context());
  const auto &request_option = request.get_request_option();
  markMeters(ASYNC_REQ_METER, request.get_model_names(), request.get_single_request().get_channel(),
             request_option.predictor_service_name, request_type);

  // client predict
  const bool rc = service_router::thriftServiceCall<predictor::PredictorServiceAsyncClient>(
    makeServiceRouterClientOption(request_option),
    [&request, &response_future, &request_option, &request_type](auto client) {
      apache::thrift::RpcOptions rpc_options;
      rpc_options.setTimeout(std::chrono::milliseconds(request_option.request_timeout));
      try {
        auto fut_impl = std::make_unique<MultiPredictResponseFutureImpl>();
        fut_impl->fut_ptr = std::move(std::make_unique<folly::Future<MultiPredictResponse>>(
          client->future_multiPredict(rpc_options, request)));
        fut_impl->req_id = request.get_req_id();
        fut_impl->service_name = request_option.predictor_service_name;
        fut_impl->request_type = request_type;
        fut_impl->model_names = request.get_model_names();
        fut_impl->channel = request.get_single_request().get_channel();
        *response_future = std::move(fut_impl);
      } catch (predictor::PredictException& ex) {
        FB_LOG_EVERY_MS(ERROR, 2000) << "caught PredictException: " << ex.get_message();
        throw ex;
      }
  });
  if (!rc) {
    LOG(ERROR) << "Failed calling predictor: api=multi_predict req_id=" << request.get_req_id();
    markMeters(ASYNC_ERROR_METER, request.get_model_names(), request.get_single_request().get_channel(),
               request_option.predictor_service_name, request_type);
    return false;
  }
  return true;
}

bool PredictorClientSDK::future_calculate_vector(
    std::unique_ptr<CalculateVectorResponsesThriftFuture>* response_future,
    const CalculateVectorRequests& requests) {
  const auto &reqs = requests.get_reqs();
  const auto &request_option = requests.get_request_option();
  markMeters(ASYNC_REQ_METER, reqs, request_option.predictor_service_name);

  // client predict
  const bool rc = service_router::thriftServiceCall<predictor::PredictorServiceAsyncClient>(
    makeServiceRouterClientOption(request_option),
    [&requests, &response_future, &request_option, &reqs](auto client) {
      apache::thrift::RpcOptions rpc_options;
      rpc_options.setTimeout(std::chrono::milliseconds(request_option.request_timeout));
      try {
        auto fut_impl = std::make_unique<CalculateVectorResponsesThriftFuture>();
        fut_impl->fut_ptr = std::move(std::make_unique<folly::Future<predictor::CalculateVectorResponses>>(
            client->future_calculateVector(rpc_options, requests)));
        fut_impl->req_ids = getReqIds(reqs);
        fut_impl->service_name = request_option.predictor_service_name;
        for (const auto& req : reqs) {
          fut_impl->channel_model_names.emplace_back(req.get_channel(), req.get_model_name());
        }
        *response_future = std::move(fut_impl);
      } catch (predictor::CalculateVectorException& ex) {
        FB_LOG_EVERY_MS(ERROR, 2000) << ex.get_message();
        throw ex;
      }
  });
  if (!rc) {
    LOG(ERROR) << "Failed calling predictor service calculateVector (async thrift). req_ids: " << getReqIds(reqs);
    markMeters(ASYNC_ERROR_METER, reqs, request_option.predictor_service_name);
    return false;
  }
  return true;
}
}  //  namespace predictor
