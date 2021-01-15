#pragma once

#include "predictor/util/predictor_constants.h"
#include "predictor_client_sdk.h"
#include "service_router/http.h"
#include "service_router/thrift.h"
#include "predictor/if/gen-cpp2/predictor_types.tcc"

namespace predictor {
//
// utility functions
//
static std::string getRequestType(const std::map<std::string, std::string>* req_context_ptr) {
if (!req_context_ptr || req_context_ptr->find(REQUEST_TYPE) == req_context_ptr->end()) {
    return PREDICT_REQUEST;
  } else {
    return req_context_ptr->find(REQUEST_TYPE)->second;
  }
}

template<typename T>
static service_router::ClientOption makeServiceRouterClientOptionImpl(const T &rhs) {
  service_router::ClientOption option;

  // set protocol, service_name, timeouts
  option.setProtocol(service_router::ServerProtocol::THRIFT);
  if (rhs.use_predictor_static_list) {
    option.setServiceName(rhs.predictor_service_name + "_static");
  } else {
    option.setServiceName(rhs.predictor_service_name);
  }
  option.setTimeoutMs(rhs.connection_timeout);
  option.setMaxConnPerServer(rhs.max_conn_per_server);

  // set load balance method
  service_router::LoadBalanceMethod load_method = service_router::LoadBalanceMethod::RANDOM;
  auto predictor_client_method = service_router::stringToLoadBalanceMethod(rhs.predictor_load_balance_method);
  if (!predictor_client_method) {
    FB_LOG_EVERY_MS(ERROR, 2000) << "Specified predictor client load balance method is invalid, set to default=random";
  } else {
    load_method = *predictor_client_method;
  }
  option.setLoadBalance(load_method);

  // set local first config
  service_router::BalanceLocalFirstConfig local_first_config;
  local_first_config.setLocalIp(rhs.local_ip);
  local_first_config.setDiffRange(rhs.ip_diff_range);
  option.setLocalFirstConfig(local_first_config);

  return option;
}

static service_router::ClientOption makeServiceRouterClientOption(const predictor::PredictorClientOption &rhs) {
  service_router::ClientOption option = makeServiceRouterClientOptionImpl(rhs);
  service_router::ServerAddress target_server_address;
  target_server_address.setHost(rhs.server_ip);
  target_server_address.setPort(rhs.server_port);
  option.setTargetServerAddress(target_server_address);
  return option;
}

static service_router::ClientOption makeServiceRouterClientOption(const predictor::RequestOption &rhs) {
  return makeServiceRouterClientOptionImpl(rhs);
}

template <class RequestType>
static std::string getReqIds(const std::vector<RequestType> &reqs) {
  std::string req_ids;
  for (const auto& req : reqs) {
    req_ids += req.get_req_id() + " ";
  }
  if (!reqs.empty()) {
    req_ids.pop_back();
  }
  return req_ids;
}

//
// Converting
//
static predictor::RequestOption makeRequestOption(const PredictorClientOption &predictor_client_option) {
  predictor::RequestOption request_option;
  request_option.local_ip                       = predictor_client_option.local_ip;
  request_option.connection_timeout             = predictor_client_option.connection_timeout;
  request_option.request_timeout                = predictor_client_option.request_timeout;
  request_option.ip_diff_range                  = predictor_client_option.ip_diff_range;
  request_option.max_conn_per_server            = predictor_client_option.max_conn_per_server;
  request_option.predictor_service_name         = predictor_client_option.predictor_service_name;
  request_option.http_port                      = predictor_client_option.http_port;
  request_option.predictor_load_balance_method  = predictor_client_option.predictor_load_balance_method;
  request_option.use_predictor_static_list      = predictor_client_option.use_predictor_static_list;
  return request_option;
}

//
// Metrics
//
// model level meter
static void markModelMeter(const std::string &metric_name, const std::string &model_name,
                           int num = 1, const std::string& request_type = PREDICT_REQUEST) {
  const MetricTagsMap tags_map{{TAG_MODEL, model_name}, {REQUEST_TYPE, request_type}};
  metrics::Metrics::getInstance()->buildMeter(SDK_MODULE_NAME, metric_name, tags_map, MINUTES)->mark(num);
}
// service level meter
static void markServiceMeter(const std::string &metric_name, const std::string &service_name,
                             int num = 1, const std::string& request_type = PREDICT_REQUEST) {
  const MetricTagsMap tags_map{{TAG_SERVICE, service_name}, {REQUEST_TYPE, request_type}};
  metrics::Metrics::getInstance()->buildMeter(SDK_MODULE_NAME, metric_name, tags_map, MINUTES)->mark(num);
}
// channel level meter
static void markChannelMeter(const std::string &metric_name, const std::string &channel_name,
                             int num = 1, const std::string& request_type = PREDICT_REQUEST) {
  const MetricTagsMap tags_map{{TAG_CHANNEL, channel_name}, {REQUEST_TYPE, request_type}};
  metrics::Metrics::getInstance()->buildMeter(SDK_MODULE_NAME, metric_name, tags_map, MINUTES)->mark(num);
}
// combined model/service/channel
static void markMeters(const std::string &metric_name, const std::vector<std::string> &model_names,
                       const std::string &channel_name, const std::string &service_name,
                       const std::string &request_type = PREDICT_REQUEST) {
  for (const auto& model_name : model_names) {
    markModelMeter(metric_name, model_name, 1, request_type);
  }
  markChannelMeter(metric_name, channel_name, model_names.size(), request_type);
  markServiceMeter(metric_name, service_name, model_names.size(), request_type);
}

template <class RequestType>
static void markMeters(const std::string &metric_name, const std::vector<RequestType> &reqs,
                       const std::string &service_name, const std::string &request_type = PREDICT_REQUEST) {
  for (const auto& req : reqs) {
    markModelMeter(metric_name, req.get_model_name(), 1, request_type);
    markChannelMeter(metric_name, req.get_channel(), 1, request_type);
  }
  markServiceMeter(metric_name, service_name, reqs.size(), request_type);
}

static void markMeters(const std::string &metric_name,
                       const std::vector<std::pair<std::string, std::string>> &channel_model_names,
                       const std::string &service_name, const std::string &request_type = PREDICT_REQUEST) {
  for (const auto& channel_model : channel_model_names) {
    markChannelMeter(metric_name, channel_model.first, 1, request_type);
    markModelMeter(metric_name, channel_model.second, 1, request_type);
  }
  markServiceMeter(metric_name, service_name, channel_model_names.size(), request_type);
}

//
// below functions currently only used by universal api
//
// async request metrics
static void markModelAsyncRequestMetrics(const std::string &model_name, int num = 1,
                                  const std::string& request_type = PREDICT_REQUEST) {
  const MetricTagsMap tags_map{{TAG_MODEL, model_name}, {REQUEST_TYPE, request_type}};
  metrics::Metrics::getInstance()->buildMeter(SDK_MODULE_NAME, ASYNC_REQ_METER, tags_map, MINUTES)->mark(num);
}
static void markServiceAsyncRequestMetrics(const std::string &service_name, int num = 1,
                                    const std::string& request_type = PREDICT_REQUEST) {
  const MetricTagsMap tags_map{{METER_TAG_SERVICE, service_name}, {REQUEST_TYPE, request_type}};
  metrics::Metrics::getInstance()->buildMeter(SDK_MODULE_NAME, ASYNC_REQ_METER, tags_map, MINUTES)->mark(num);
}
static void markChannelAsyncRequestMetrics(const std::string &channel_name, int num = 1,
                                    const std::string& request_type = PREDICT_REQUEST) {
  const MetricTagsMap tags_map{{METER_TAG_CHANNEL, channel_name}, {REQUEST_TYPE, request_type}};
  metrics::Metrics::getInstance()->buildMeter(SDK_MODULE_NAME, ASYNC_REQ_METER, tags_map, MINUTES)->mark(num);
}

// async error metrics
static void markModelAsyncErrorMetrics(const std::string &model_name, int num = 1,
                                const std::string& request_type = PREDICT_REQUEST) {
  const MetricTagsMap tags_map{{TAG_MODEL, model_name}, {REQUEST_TYPE, request_type}};
  metrics::Metrics::getInstance()->buildMeter(SDK_MODULE_NAME, ASYNC_ERROR_METER, tags_map, MINUTES)->mark(num);
}
static void markServiceAsyncErrorMetrics(const std::string &service_name, int num = 1,
                                  const std::string& request_type = PREDICT_REQUEST) {
  const MetricTagsMap tags_map{{METER_TAG_SERVICE, service_name}, {REQUEST_TYPE, request_type}};
  metrics::Metrics::getInstance()->buildMeter(SDK_MODULE_NAME, ASYNC_ERROR_METER, tags_map, MINUTES)->mark(num);
}
static void markChannelAsyncErrorMetrics(const std::string &channel_name, int num = 1,
                                  const std::string& request_type = PREDICT_REQUEST) {
  const MetricTagsMap tags_map{{METER_TAG_CHANNEL, channel_name}, {REQUEST_TYPE, request_type}};
  metrics::Metrics::getInstance()->buildMeter(SDK_MODULE_NAME, ASYNC_ERROR_METER, tags_map, MINUTES)->mark(num);
}

// async future get exception metrics
static void markModelAsyncGetExceptionMetrics(const std::string &model_name, int num = 1,
                                       const std::string& request_type = PREDICT_REQUEST) {
  const MetricTagsMap tags_map{{TAG_MODEL, model_name}, {REQUEST_TYPE, request_type}};
  metrics::Metrics::getInstance()->buildMeter(SDK_MODULE_NAME, ASYNC_GET_EXCEPTION_METER, tags_map, MINUTES)
    ->mark(num);
}
static void markServiceAsyncGetExceptionMetrics(const std::string &service_name, int num = 1,
                                         const std::string& request_type = PREDICT_REQUEST) {
  const MetricTagsMap tags_map{{METER_TAG_SERVICE, service_name}, {REQUEST_TYPE, request_type}};
  metrics::Metrics::getInstance()->buildMeter(SDK_MODULE_NAME, ASYNC_GET_EXCEPTION_METER, tags_map, MINUTES)
    ->mark(num);
}
static void markChannelAsyncGetExceptionMetrics(const std::string &channel_name, int num = 1,
                                         const std::string& request_type = PREDICT_REQUEST) {
  const MetricTagsMap tags_map{{METER_TAG_CHANNEL, channel_name}, {REQUEST_TYPE, request_type}};
  metrics::Metrics::getInstance()->buildMeter(SDK_MODULE_NAME, ASYNC_GET_EXCEPTION_METER, tags_map, MINUTES)
    ->mark(num);
}

// async get empty response metrics
static void markModelAsyncGetEmptyResponseMetrics(const std::string &model_name, int num = 1,
                                           const std::string& request_type = PREDICT_REQUEST) {
  const MetricTagsMap tags_map{{TAG_MODEL, model_name}, {REQUEST_TYPE, request_type}};
  metrics::Metrics::getInstance()->buildMeter(SDK_MODULE_NAME, ASYNC_GET_EMPTY_RESP_METER, tags_map, MINUTES)
    ->mark(num);
}
static void markServiceAsyncGetEmptyResponseMetrics(const std::string &service_name, int num = 1,
                                             const std::string& request_type = PREDICT_REQUEST) {
  const MetricTagsMap tags_map{{METER_TAG_SERVICE, service_name}, {REQUEST_TYPE, request_type}};
  metrics::Metrics::getInstance()->buildMeter(SDK_MODULE_NAME, ASYNC_GET_EMPTY_RESP_METER, tags_map, MINUTES)
    ->mark(num);
}
static void markChannelAsyncGetEmptyResponseMetrics(const std::string &channel_name, int num = 1,
                                             const std::string& request_type = PREDICT_REQUEST) {
  const MetricTagsMap tags_map{{METER_TAG_CHANNEL, channel_name}, {REQUEST_TYPE, request_type}};
  metrics::Metrics::getInstance()->buildMeter(SDK_MODULE_NAME, ASYNC_GET_EMPTY_RESP_METER, tags_map, MINUTES)
    ->mark(num);
}

// sync request metrics
static void markModelSyncRequestMetrics(const std::string &model_name, int num = 1,
                                 const std::string& request_type = PREDICT_REQUEST) {
  const MetricTagsMap request_meter_tag{{TAG_MODEL, model_name}, {REQUEST_TYPE, request_type}};
  metrics::Metrics::getInstance()->buildMeter(SDK_MODULE_NAME, SYNC_REQ_METER, request_meter_tag, MINUTES)
    ->mark(num);
}
static void markServiceSyncRequestMetrics(const std::string &service_name, int num = 1,
                                   const std::string& request_type = PREDICT_REQUEST) {
  const MetricTagsMap request_meter_tag{{METER_TAG_SERVICE, service_name}, {REQUEST_TYPE, request_type}};
  metrics::Metrics::getInstance()->buildMeter(SDK_MODULE_NAME, SYNC_REQ_METER, request_meter_tag, MINUTES)
    ->mark(num);
}
static void markChannelSyncRequestMetrics(const std::string &channel_name, int num = 1,
                                   const std::string& request_type = PREDICT_REQUEST) {
  const MetricTagsMap request_meter_tag{{METER_TAG_CHANNEL, channel_name}, {REQUEST_TYPE, request_type}};
  metrics::Metrics::getInstance()->buildMeter(SDK_MODULE_NAME, SYNC_REQ_METER, request_meter_tag, MINUTES)
    ->mark(num);
}

// sync request error metrics
static void markModelSyncRequestErrorMetrics(const std::string &model_name, int num = 1,
                                      const std::string& request_type = PREDICT_REQUEST) {
  const MetricTagsMap err_tag{{TAG_MODEL, model_name}, {REQUEST_TYPE, request_type}};
  metrics::Metrics::getInstance()->buildMeter(SDK_MODULE_NAME, SYNC_PREDICT_ERROR, err_tag, MINUTES)->mark(num);
}
static void markServiceSyncRequestErrorMetrics(const std::string &service_name, int num = 1,
                                        const std::string& request_type = PREDICT_REQUEST) {
  const MetricTagsMap err_tag{{METER_TAG_SERVICE, service_name}, {REQUEST_TYPE, request_type}};
  metrics::Metrics::getInstance()->buildMeter(SDK_MODULE_NAME, SYNC_PREDICT_ERROR, err_tag, MINUTES)->mark(num);
}
static void markChannelSyncRequestErrorMetrics(const std::string &channel_name, int num = 1,
                                        const std::string& request_type = PREDICT_REQUEST) {
  const MetricTagsMap err_tag{{METER_TAG_CHANNEL, channel_name}, {REQUEST_TYPE, request_type}};
  metrics::Metrics::getInstance()->buildMeter(SDK_MODULE_NAME, SYNC_PREDICT_ERROR, err_tag, MINUTES)->mark(num);
}

// sync request empty metrics
static void markModelSyncRequestEmptyMetrics(const std::string &model_name, int num = 1,
                                      const std::string& request_type = PREDICT_REQUEST) {
  const MetricTagsMap err_tag{{TAG_MODEL, model_name}, {REQUEST_TYPE, request_type}};
  metrics::Metrics::getInstance()->buildMeter(SDK_MODULE_NAME, SYNC_PREDICT_EMPTY, err_tag, MINUTES)->mark(num);
}
static void markServiceSyncRequestEmptyMetrics(const std::string &service_name, int num = 1,
                                        const std::string& request_type = PREDICT_REQUEST) {
  const MetricTagsMap err_tag{{METER_TAG_SERVICE, service_name}, {REQUEST_TYPE, request_type}};
  metrics::Metrics::getInstance()->buildMeter(SDK_MODULE_NAME, SYNC_PREDICT_EMPTY, err_tag, MINUTES)->mark(num);
}
static void markChannelSyncRequestEmptyMetrics(const std::string &channel_name, int num = 1,
                                        const std::string& request_type = PREDICT_REQUEST) {
  const MetricTagsMap err_tag{{METER_TAG_CHANNEL, channel_name}, {REQUEST_TYPE, request_type}};
  metrics::Metrics::getInstance()->buildMeter(SDK_MODULE_NAME, SYNC_PREDICT_EMPTY, err_tag, MINUTES)->mark(num);
}
static metrics::Timers* buildTimers(const std::string &category, const std::string& request_type = PREDICT_REQUEST) {
  MetricTagsMap timer_tags_map{{TAG_CATEGORY, category}, {REQUEST_TYPE, request_type}};
  return metrics::Metrics::getInstance()->buildTimers(SDK_MODULE_NAME, ASYNC_TIME_CONSUMING,
                                                      TIMER_BUCKET_SCALE, TIMER_MIN, TIMER_MAX,
                                                      timer_tags_map,
                                                      LEVELS, MINUTES, PERCENTILES).get();
}
}  // namespace predictor
