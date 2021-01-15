#include "predictor_client_sdk.h"
#include "predictor_sdk_util.h"
#include "common/serialize_util.h"
#include "folly/GLog.h"
#include "folly/Random.h"
#include "folly/init/Init.h"
#include "gflags/gflags.h"
#include "gflags/gflags_declare.h"
#include "service_router/http.h"
#include "service_router/thrift.h"
#include "feature_master/if/gen-cpp2/feature_master_types.tcc"
#include "predictor/if/gen-cpp2/predictor_types.tcc"
#include "predictor/if/gen-cpp2/PredictorService.h"
#include "predictor/if/gen-cpp2/PredictorServiceAsyncClient.h"
#include <dirent.h>
#include <sstream>
#include <sys/types.h>

namespace service_router {
DECLARE_string(router_consul_addresses);
DECLARE_int32(thrift_timeout_retry);
DECLARE_int32(thrift_connection_retry);
}  // namespace service_router

namespace predictor {

namespace {

void packClientResponses(std::vector<PredictClientResponse>* client_response_list_ptr,
                         const std::vector<PredictResponse>& responses) {
  if (!client_response_list_ptr) {
    LOG(ERROR) << "invalid client_response_list_ptr";
    return;
  }
  std::vector<PredictClientResponse>& client_response_list = *client_response_list_ptr;
  for (const auto& response : responses) {
    const std::map<int64_t, feature_master::PredictResults>& results_map = response.get_results_map();
    PredictClientResponse client_response;
    client_response.req_id = response.req_id;
    for (auto result : results_map) {
      VLOG(DEBUG_LEVEL) << result.first << ":" << result.second.preds["ctr"];
      client_response.item_results[result.first] = result.second.preds["ctr"];
      auto iter = result.second.preds.find("corrected_ctr");
      if (result.second.preds.end() != iter) {
        client_response.corrected_ctr_map[result.first] = iter->second;
      }
    }
    client_response_list.emplace_back(std::move(client_response));
  }
}

}  // namespace

feature_master::Feature featureParse(const PredictFeature& predict_feature) {
  feature_master::Feature master_feature;
  master_feature.set_feature_name(predict_feature.feature_name);
  master_feature.set_feature_type((feature_master::FeatureType)predict_feature.feature_type);
  switch (predict_feature.feature_type) {
    case INT_TYPE: {
      master_feature.set_int64_values(predict_feature.int64_values);
      break;
    }
    case FLOAT_TYPE: {
      master_feature.set_float_values(predict_feature.float_values);
      break;
    }
    case STRING_TYPE: {
      master_feature.set_string_values(predict_feature.string_values);
      break;
    }
  }
  return master_feature;
}

PredictRequest PredictClientRequest::toPredictRequest() const {
  // transform client req to predictor req
  predictor::PredictRequest request;
  request.set_req_id(req_id);
  request.set_model_name(model_name);
  request.set_channel(channel);
  for (const auto& it : common_feature) {
    request.common_features.features.emplace_back(std::move(featureParse(it)));
  }
  for (const auto& item : item_features) {
    feature_master::Features features;
    for (const auto& feature_item : item.second) {
      features.features.emplace_back(std::move(featureParse(feature_item)));
    }
    request.item_features[item.first] = std::move(features);
  }

  return request;
}

std::vector<PredictClientResponse> PredictResponsesFutureImpl::get() {
  // blocks until the async call is done
  predictor::PredictResponses predict_responses;
  try {
    predict_responses = fut_ptr->get();
  } catch (const std::exception& e) {
    std::stringstream err_msg;
    err_msg << typeid(e).name() << ":" << e.what();
    LOG(ERROR) << SDK_MODULE_NAME << " fut_ptr->get() caught exception: " << err_msg.str();
    for (const auto& channel_model_name : channel_model_names) {
      markChannelAsyncGetExceptionMetrics(channel_model_name.first);
      markModelAsyncGetExceptionMetrics(channel_model_name.second);
    }
    markServiceAsyncGetExceptionMetrics(service_name, channel_model_names.size());
    return {};
  }
  // pack response
  const std::vector<PredictResponse>& responses = predict_responses.get_resps();
  if (responses.empty()) {
    LOG(ERROR) << SDK_MODULE_NAME << " Got back empty responses. request ids: " << req_ids;
    for (const auto& channel_model_name : channel_model_names) {
      markChannelAsyncGetEmptyResponseMetrics(channel_model_name.first);
      markModelAsyncGetEmptyResponseMetrics(channel_model_name.second);
    }
    markServiceAsyncGetEmptyResponseMetrics(service_name, channel_model_names.size());
    return {};
  }
  std::vector<PredictClientResponse> client_response_list;
  packClientResponses(&client_response_list, responses);
  return client_response_list;
}

namespace client_util {
PredictFeature parse(const std::string& name, int64_t value) {
  PredictFeature feature;
  feature.feature_name = name;
  feature.feature_type = INT_TYPE;
  feature.int64_values.push_back(value);
  return feature;
}
PredictFeature parse(const std::string& name, double value) {
  PredictFeature feature;
  feature.feature_name = name;
  feature.feature_type = FLOAT_TYPE;
  feature.float_values.push_back(value);
  return feature;
}
PredictFeature parse(const std::string& name, const std::string& value) {
  PredictFeature feature;
  feature.feature_name = name;
  feature.feature_type = STRING_TYPE;
  feature.string_values.push_back(value);
  return feature;
}
PredictFeature parse(const std::string& name, const std::vector<int64_t>& value_vec) {
  PredictFeature feature;
  feature.feature_name = name;
  feature.feature_type = INT_TYPE;
  feature.int64_values = value_vec;
  return feature;
}
PredictFeature parse(const std::string& name, const std::vector<double>& value_vec) {
  PredictFeature feature;
  feature.feature_name = name;
  feature.feature_type = FLOAT_TYPE;
  feature.float_values = value_vec;
  return feature;
}
PredictFeature parse(const std::string& name, const std::vector<std::string>& value_vec) {
  PredictFeature feature;
  feature.feature_name = name;
  feature.feature_type = STRING_TYPE;
  feature.string_values = value_vec;
  return feature;
}
}  // namespace client_util

// sync interface
bool PredictorClientSDK::predict(std::vector<PredictClientResponse>* client_response_list,
                                 const std::vector<PredictClientRequest>& client_request_list,
                                 const PredictorClientOption &predictor_client_option) {
  predictor::PredictResponses predict_responses;
  predictor::PredictRequests predict_requests;

  std::vector<predictor::PredictRequest> request_list;
  for (const auto& client_request : client_request_list) {
    request_list.emplace_back(std::move(client_request.toPredictRequest()));
    markModelSyncRequestMetrics(client_request.model_name);
    markChannelSyncRequestMetrics(client_request.channel);
  }
  markServiceSyncRequestMetrics(predictor_client_option.predictor_service_name, request_list.size());
  predict_requests.set_reqs(std::move(request_list));
  predict_requests.set_request_option(std::move(makeRequestOption(predictor_client_option)));

  // client predict
  int request_timeout = predictor_client_option.request_timeout;
  service_router::ClientOption option = makeServiceRouterClientOption(predictor_client_option);
  if (!service_router::thriftServiceCall<predictor::PredictorServiceAsyncClient>(
          option, [&predict_requests, &predict_responses, request_timeout](auto client) {
            apache::thrift::RpcOptions rpc_options;
            rpc_options.setTimeout(std::chrono::milliseconds(request_timeout));
            // thriftServiceCall will catch all exceptions related to transport and connection, and everything else.
            // Here we just want to catch logical error, add a log, and throw it again.
            try {
              client->sync_predict(rpc_options, predict_responses, predict_requests);
            } catch (predictor::PredictException& ex) {
              FB_LOG_EVERY_MS(ERROR, 2000) << ex.get_message();
              throw ex;
            }
          })) {
    std::stringstream req_ids;
    for (const auto& req : client_request_list) {
      req_ids << req.req_id << " ";
    }
    LOG(ERROR) << SDK_MODULE_NAME
      << " Failed calling predictor service (sync with option). request ids: " << req_ids.str();

    for (const auto& client_request : client_request_list) {
      markModelSyncRequestErrorMetrics(client_request.model_name);
      markChannelSyncRequestErrorMetrics(client_request.channel);
    }
    markServiceSyncRequestErrorMetrics(predictor_client_option.predictor_service_name, client_request_list.size());
    return false;
  }
  // pack response
  const std::vector<PredictResponse>& responses = predict_responses.get_resps();
  std::stringstream req_ids;
  if (responses.empty()) {
    for (const auto& req : client_request_list) {
      req_ids << req.req_id << " ";
      markModelSyncRequestEmptyMetrics(req.model_name);
      markChannelSyncRequestEmptyMetrics(req.channel);
    }
    markServiceSyncRequestEmptyMetrics(predictor_client_option.predictor_service_name, client_request_list.size());
    LOG(ERROR) << SDK_MODULE_NAME << " Got back empty responses. request ids: " << req_ids.str();
  } else {
    packClientResponses(client_response_list, responses);
  }

  return true;
}

// async interface
bool PredictorClientSDK::future_predict(std::unique_ptr<PredictResponsesFuture>* predictor_responses_future,
                                        const std::vector<PredictClientRequest>& client_request_list,
                                        const PredictorClientOption &predictor_client_option) {
  predictor::PredictRequests predict_requests;

  std::stringstream req_ids_ss;
  std::vector<predictor::PredictRequest> request_list;
  for (const auto& client_request : client_request_list) {
    req_ids_ss << client_request.req_id << " ";
    request_list.emplace_back(std::move(client_request.toPredictRequest()));
    markModelAsyncRequestMetrics(client_request.model_name);
    markModelAsyncRequestMetrics(client_request.channel);
  }
  markServiceAsyncRequestMetrics(predictor_client_option.predictor_service_name, client_request_list.size());
  predict_requests.set_reqs(std::move(request_list));
  predict_requests.set_request_option(std::move(makeRequestOption(predictor_client_option)));
  const std::string req_ids = req_ids_ss.str();
  const std::string &service_name = predictor_client_option.predictor_service_name;

  // client predict
  int request_timeout = predictor_client_option.request_timeout;
  service_router::ClientOption option = makeServiceRouterClientOption(predictor_client_option);
  if (!service_router::thriftServiceCall<predictor::PredictorServiceAsyncClient>(
         option,
         [&predict_requests, &predictor_responses_future, &req_ids, request_timeout,
          &service_name](auto client) {
            apache::thrift::RpcOptions rpc_options;
            rpc_options.setTimeout(std::chrono::milliseconds(request_timeout));
            // thriftServiceCall will catch all exceptions related to transport and connection, and everything else.
            // Here we just want to catch logical error, add a log, and throw it again.
            try {
              auto fut_impl = std::make_unique<PredictResponsesFutureImpl>();
              fut_impl->fut_ptr = std::move(std::make_unique<folly::Future<predictor::PredictResponses>>(
                  client->future_predict(rpc_options, predict_requests)));
              fut_impl->req_ids = req_ids;
              fut_impl->service_name = service_name;
              for (const auto& predict_request : predict_requests.get_reqs()) {
                fut_impl->channel_model_names.emplace_back(std::make_pair(predict_request.get_channel(),
                                                                           predict_request.get_model_name()));
              }
              *predictor_responses_future = std::move(fut_impl);
            } catch (predictor::PredictException& ex) {
              FB_LOG_EVERY_MS(ERROR, 2000) << ex.get_message();
              throw ex;
            }
          })) {
    LOG(ERROR) << "Failed calling predictor service (batch async with option). request ids: " << req_ids;
    for (const auto& client_request : client_request_list) {
      markModelAsyncErrorMetrics(client_request.model_name);
      markChannelAsyncErrorMetrics(client_request.channel);
    }
    markServiceAsyncErrorMetrics(predictor_client_option.predictor_service_name, client_request_list.size());
    return false;
  }

  return true;
}

CalculateVectorRequest CalculateVectorClientRequest::toCalculateVectorRequest() const {
  // transform client req to predictor req
  predictor::CalculateVectorRequest request;
  request.set_req_id(req_id);
  request.set_model_name(model_name);
  request.set_channel(channel);
  request.set_output_names(output_names);
  for (const auto& it : features) {
    request.features.features.emplace_back(std::move(featureParse(it)));
  }

  return request;
}

CalculateBatchVectorRequest CalculateBatchVectorClientRequest::toCalculateBatchVectorRequest() const {
  // transform client req to predictor req
  predictor::CalculateBatchVectorRequest request;
  request.set_req_id(req_id);
  request.set_model_name(model_name);
  request.set_channel(channel);
  request.set_output_names(output_names);
  for (const auto &item_features : feature_map) {
    feature_master::Features features;
    features.features.reserve(item_features.second.size());
    for (const auto &it : item_features.second) {
      features.features.push_back(featureParse(it));
    }
    request.features_map.emplace(item_features.first, std::move(features));
  }

  return request;
}

template <class ClientResponseType, class ResponseType>
void packCalculaterClientResponses(std::vector<ClientResponseType>* client_response_list_ptr,
                         const std::vector<ResponseType>& responses) {
  if (!client_response_list_ptr) {
    LOG(ERROR) << "invalid client_response_list_ptr";
    return;
  }
  std::vector<ClientResponseType>& client_response_list = *client_response_list_ptr;
  client_response_list.reserve(responses.size());
  for (const auto& response : responses) {
    ClientResponseType client_response;
    client_response.return_code = response.return_code;
    client_response.model_timestamp = response.model_timestamp;
    client_response.req_id = response.get_req_id();
    client_response.vector_map = std::move(response.get_vector_map());
    client_response_list.emplace_back(std::move(client_response));
  }
}

// calculate_vector() interface (sync) but takes in a PredictClientOption
bool PredictorClientSDK::calculate_vector(std::vector<CalculateVectorClientResponse>* client_response_list,
                                 const std::vector<CalculateVectorClientRequest>& client_request_list,
                                 const PredictorClientOption &predictor_client_option) {
  predictor::CalculateVectorResponses calculate_vector_responses;
  predictor::CalculateVectorRequests calculate_vector_requests;

  std::vector<predictor::CalculateVectorRequest> request_list;
  for (const auto& client_request : client_request_list) {
    request_list.emplace_back(std::move(client_request.toCalculateVectorRequest()));
    markModelSyncRequestMetrics(client_request.model_name);
    markChannelSyncRequestMetrics(client_request.channel);
  }
  markServiceSyncRequestMetrics(predictor_client_option.predictor_service_name, request_list.size());
  calculate_vector_requests.set_reqs(std::move(request_list));

  // client predict
  int request_timeout = predictor_client_option.request_timeout;
  service_router::ClientOption option = makeServiceRouterClientOption(predictor_client_option);
  if (!service_router::thriftServiceCall<predictor::PredictorServiceAsyncClient>(
          option, [&calculate_vector_requests, &calculate_vector_responses, request_timeout](auto client) {
            apache::thrift::RpcOptions rpc_options;
            rpc_options.setTimeout(std::chrono::milliseconds(request_timeout));
            // thriftServiceCall will catch all exceptions related to transport and connection, and everything else.
            // Here we just want to catch logical error, add a log, and throw it again.
            try {
              client->sync_calculateVector(rpc_options, calculate_vector_responses, calculate_vector_requests);
            } catch (predictor::CalculateVectorException& ex) {
              FB_LOG_EVERY_MS(ERROR, 2000) << ex.get_message();
              throw ex;
            }
          })) {
    std::stringstream req_ids;
    for (const auto& req : client_request_list) {
      req_ids << req.req_id << " ";
    }
    FB_LOG_EVERY_MS(ERROR, 2000) << SDK_MODULE_NAME
      << " Failed calling predictor service (sync with option). request ids: " << req_ids.str();

    for (const auto& client_request : client_request_list) {
      markModelSyncRequestErrorMetrics(client_request.model_name);
      markChannelSyncRequestErrorMetrics(client_request.channel);
    }
    markServiceSyncRequestErrorMetrics(predictor_client_option.predictor_service_name, client_request_list.size());
    return false;
  }
  // pack response
  const std::vector<CalculateVectorResponse>& responses = calculate_vector_responses.get_resps();
  std::stringstream req_ids;
  if (responses.empty()) {
    for (const auto& req : client_request_list) {
      req_ids << req.req_id << " ";
      markModelSyncRequestEmptyMetrics(req.model_name);
      markChannelSyncRequestEmptyMetrics(req.channel);
    }
    markServiceSyncRequestEmptyMetrics(predictor_client_option.predictor_service_name, client_request_list.size());
    FB_LOG_EVERY_MS(ERROR, 2000) << SDK_MODULE_NAME << " Got back empty responses. request ids: " << req_ids.str();
  } else {
    packCalculaterClientResponses(client_response_list, responses);
  }

  return true;
}

// calculate_batch_vector() interface (sync)
bool PredictorClientSDK::calculate_batch_vector(std::vector<CalculateBatchVectorClientResponse>* client_response_list,
                                 const std::vector<CalculateBatchVectorClientRequest>& client_request_list,
                                 const PredictorClientOption &predictor_client_option) {
  predictor::CalculateBatchVectorResponses batch_responses;
  predictor::CalculateBatchVectorRequests batch_requests;

  std::vector<predictor::CalculateBatchVectorRequest> request_list;
  for (const auto& client_request : client_request_list) {
    request_list.emplace_back(std::move(client_request.toCalculateBatchVectorRequest()));
    markModelSyncRequestMetrics(client_request.model_name);
    markChannelSyncRequestMetrics(client_request.channel);
  }
  markServiceSyncRequestMetrics(predictor_client_option.predictor_service_name, request_list.size());
  batch_requests.set_reqs(std::move(request_list));

  // client predict
  int request_timeout = predictor_client_option.request_timeout;
  service_router::ClientOption option = makeServiceRouterClientOption(predictor_client_option);
  const bool rc = service_router::thriftServiceCall<predictor::PredictorServiceAsyncClient>(
          option, [&batch_requests, &batch_responses, request_timeout](auto client) {
            apache::thrift::RpcOptions rpc_options;
            rpc_options.setTimeout(std::chrono::milliseconds(request_timeout));
            // thriftServiceCall will catch all exceptions related to transport and connection, and everything else.
            // Here we just want to catch logical error, add a log, and throw it again.
            try {
              client->sync_calculateBatchVector(rpc_options, batch_responses, batch_requests);
            } catch (predictor::CalculateVectorException& ex) {
              FB_LOG_EVERY_MS(ERROR, 2000) << ex.get_message();
              throw ex;
            }
          });
  if (!rc) {
    std::stringstream req_ids;
    for (const auto& req : client_request_list) {
      req_ids << req.req_id << " ";
    }
    FB_LOG_EVERY_MS(ERROR, 2000) << SDK_MODULE_NAME
      << " Failed calling predictor service (sync with option). request ids: " << req_ids.str();

    for (const auto& client_request : client_request_list) {
      markModelSyncRequestErrorMetrics(client_request.model_name);
      markChannelSyncRequestErrorMetrics(client_request.channel);
    }
    markServiceSyncRequestErrorMetrics(predictor_client_option.predictor_service_name, client_request_list.size());
    return false;
  }
  // pack response
  const std::vector<CalculateBatchVectorResponse>& responses = batch_responses.get_resps();
  std::stringstream req_ids;
  if (responses.empty()) {
    for (const auto& req : client_request_list) {
      req_ids << req.req_id << " ";
      markModelSyncRequestEmptyMetrics(req.model_name);
      markChannelSyncRequestEmptyMetrics(req.channel);
    }
    markServiceSyncRequestEmptyMetrics(predictor_client_option.predictor_service_name, client_request_list.size());
    FB_LOG_EVERY_MS(ERROR, 2000) << SDK_MODULE_NAME << " Got back empty responses. request ids: " << req_ids.str();
  } else {
    packCalculaterClientResponses(client_response_list, responses);
  }

  return true;
}
}  //  namespace predictor
