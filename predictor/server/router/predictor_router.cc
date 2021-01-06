#include "predictor/server/router/predictor_router.h"
#include "common/metrics/metrics.h"
#include "predictor/util/predictor_constants.h"
#include "predictor/util/predictor_util.h"
#include "predictor/if/gen-cpp2/PredictorServiceAsyncClient.h"
#include "predictor/if/gen-cpp2/predictor_types.tcc"
#include "sdk/predictor/predictor_client_sdk.h"
#include "common/file/file_util.h"
#include "common/util.h"
#include "service_router/router.h"
#include "predictor/server/resource_manager.h"

DEFINE_string(predictor_static_list_dir, "/data0/vad/algo_service/data/predictor_static_ip_list/", "predictor static list dir"); // NOLINT
DEFINE_string(predictor_service_list_file, "predictor_service_list", "predictor service list file name");
DEFINE_int32(predictor_static_pull_interval, 10, "predictor static ip list update interval");
DECLARE_int64(heavy_tasks_thread_num);

namespace predictor {
namespace {
constexpr char LOG_CATEGORY[] = "router.cc";
}  // namespace

bool PredictorRouter::init() {
  bool ret = InitPredictorConsulFallback();
  if (!ret) {
    util::logAndMetricError("predictor_router_init_error");
  } else {
    LOG(INFO) << "predictor_router init successfully";
  }
  return ret;
}

void PredictorRouter::predict(PredictResponses* predict_responses,
                     std::unique_ptr<PredictRequests> predict_requests_ptr) {
  auto context_ptr = predict_requests_ptr->get_context();
  if (context_ptr == nullptr) {
    predictSimpleForward(predict_responses, std::move(predict_requests_ptr));
    return;
  }
  const auto& router_op_iter = context_ptr->find("router_op");
  if (router_op_iter->second == "SimpleForward") {
    predictSimpleForward(predict_responses, std::move(predict_requests_ptr));
  }
}

void PredictorRouter::multiPredict(MultiPredictResponse* multi_predict_response,
                          std::unique_ptr<MultiPredictRequest> multi_predict_request) {
  auto context_ptr = multi_predict_request->get_context();
  if (context_ptr == nullptr) {
    multiPredictSimpleForward(multi_predict_response, std::move(multi_predict_request));
    return;
  }
  const auto& router_op_iter = context_ptr->find("router_op");
  if (router_op_iter->second == "SimpleForward") {
    multiPredictSimpleForward(multi_predict_response, std::move(multi_predict_request));
  } else if (router_op_iter->second == "SplitRequest") {
    multiPredictSplitRequest(multi_predict_response, std::move(multi_predict_request));
  }
}

void PredictorRouter::calculateVector(CalculateVectorResponses* calculate_vector_responses,
                             std::unique_ptr<CalculateVectorRequests> calculate_vector_requests_ptr) {
  FB_LOG_EVERY_MS(INFO, 2000) << "To be implemented...";
}

bool PredictorRouter::InitPredictorConsulFallback() {
  auto router = service_router::Router::getInstance();
  const std::string predictor_static_list_file_path = common::pathJoin(FLAGS_predictor_static_list_dir,
                                                                       FLAGS_predictor_service_list_file);
  std::vector<std::string> all_lines;
  auto ret = common::FileUtil::read_file_all_lines(&all_lines, predictor_static_list_file_path);
  if (!ret) {
    util::logAndMetricError("open_predictor_service_list_file_failed");
    return false;
  }
  for (auto& service : all_lines) {
    if (service.empty()) {
      continue;
    }
    const std::string service_static_ip_list_file = common::pathJoin(FLAGS_predictor_static_list_dir,
                                                                     "predictor_" + service + "_static_ip_list");
    const std::string service_name_static = service + "_static";
    auto registry = std::make_shared<service_router::FileRegistry>(FLAGS_predictor_static_pull_interval);
    registry->registerServersFromFile(service_name_static, service_static_ip_list_file);
    router->setServiceRegistry(service_name_static, registry);
    router->waitUntilDiscover(service_name_static, service_router::FLAGS_discover_wait_milliseconds);
  }
  std::vector<std::string> registry_names;
  router->getAllRegistryName(&registry_names);
  for (const auto& registry_name : registry_names) {
    LOG(INFO) << "service_router registered service:" << registry_name;
  }
  return true;
}

void PredictorRouter::multiPredictSimpleForward(MultiPredictResponse* multi_predict_response,
                          std::unique_ptr<MultiPredictRequest> multi_predict_request) {
  auto& request_option = multi_predict_request->request_option;
  const auto& model_names = multi_predict_request->get_model_names();
  if (model_names.empty()) {
    FB_LOG_EVERY_MS(ERROR, 2000) << "model_names is empty";
    return;
  }
  {
    auto global_model_service_map_locked = ResourceMgr::global_model_service_map_.rlock();
    // there is only one service for multiPredict request models
    auto iter = global_model_service_map_locked->find(model_names[0]);
    if (iter != global_model_service_map_locked->end()) {
      request_option.predictor_service_name = iter->second;
    }
  }
  if (request_option.predictor_service_name == PREDICTOR_ROUTER_SERVICE_NAME) {
    util::logAndMetricError("forward_multi_predict_request_error-invalid_model:" + model_names[0]);
    return;
  }
  const MetricTagsMap timer_predict_tag{{TAG_CATEGORY, request_option.predictor_service_name}};
  metrics::Timer timer_predict(util::buildTimers(FORWARD_MULTI_PREDICT_TIME_CONSUMING, timer_predict_tag));

  FB_LOG_EVERY_MS(INFO, 2000) << "forwarding to service name:" << request_option.predictor_service_name;
  std::unique_ptr<predictor::MultiPredictResponseFuture> rpc_fu;
  {
    const MetricTagsMap timer_rpc_tag{{TAG_CATEGORY, "rpc_" + request_option.predictor_service_name}};
    metrics::Timer timer_rpc(util::buildTimers(FORWARD_MULTI_PREDICT_TIME_CONSUMING, timer_rpc_tag));
    const bool rc = predictor::PredictorClientSDK::future_multi_predict(&rpc_fu, *multi_predict_request);
    if (!rc) {
      FB_LOG_EVERY_MS(ERROR, 2000) << "callMultiPredictServiceAsync Failed!";
      return;
    }
    if (!rpc_fu) {
      FB_LOG_EVERY_MS(ERROR, 2000) << "rpc_fu ptr is NULL!";
      return;
    }
  }
  {
    const MetricTagsMap timer_get_tag{{TAG_CATEGORY, "get_" + request_option.predictor_service_name}};
    metrics::Timer timer_get(util::buildTimers(FORWARD_MULTI_PREDICT_TIME_CONSUMING, timer_get_tag));
    try {
      *multi_predict_response = rpc_fu->get();
    } catch(const std::exception& e) {
      std::string err_msg = std::string(typeid(e).name()) + ":" + std::string(e.what());
      FB_LOG_EVERY_MS(ERROR, 2000) << "rpc_fu.get(): caught exception, detail:" << err_msg;
    }
  }
}


void PredictorRouter::predictSimpleForward(PredictResponses* predict_responses,
               std::unique_ptr<PredictRequests> predict_requests_ptr) {
  FB_LOG_EVERY_MS(INFO, 2000) << "To be implemented...";
}


void PredictorRouter::calculateVectorSimpleForward(CalculateVectorResponses* calculate_vector_responses,
                       std::unique_ptr<CalculateVectorRequests> calculate_vector_requests_ptr) {
  FB_LOG_EVERY_MS(INFO, 2000) << "To be implemented...";
}


void PredictorRouter::multiPredictSplitRequest(MultiPredictResponse* multi_predict_response,
                          std::unique_ptr<MultiPredictRequest> multi_predict_request) {
  FB_LOG_EVERY_MS(INFO, 2000) << "To be implemented...";
}
}  // namespace predictor
