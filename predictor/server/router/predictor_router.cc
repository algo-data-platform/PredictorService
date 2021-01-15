#include "predictor/server/router/predictor_router.h"
#include "common/metrics/metrics.h"
#include "predictor/util/predictor_constants.h"
#include "predictor/util/predictor_util.h"
#include "predictor/if/gen-cpp2/PredictorServiceAsyncClient.h"
#include "predictor/if/gen-cpp2/predictor_types.tcc"
#include "common/file/file_util.h"
#include "common/util.h"
#include "service_router/router.h"
#include "predictor/global_resource/resource_manager.h"
#include "common/util.h"

DEFINE_string(predictor_static_list_dir, "/data0/vad/algo_service/data/predictor_static_ip_list/", "predictor static list dir"); // NOLINT
DEFINE_string(predictor_service_list_file, "predictor_service_list", "predictor service list file name");
DEFINE_int32(predictor_static_pull_interval, 10, "predictor static ip list update interval");
DECLARE_int64(heavy_tasks_thread_num);
DEFINE_int64(min_items_per_slice, 1, "min items per slice");
DEFINE_int64(max_items_per_slice, 50, "max items per slice");

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
  const auto& router_op_iter = context_ptr->find(ROUTER_OP);
  if (router_op_iter->second == SIMPLE_FORWARD) {
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
  const auto& router_op_iter = context_ptr->find(ROUTER_OP);
  if (router_op_iter->second == SIMPLE_FORWARD) {
    multiPredictSimpleForward(multi_predict_response, std::move(multi_predict_request));
  } else if (router_op_iter->second == SPLIT_REQUEST) {
    multiPredictSplitRequest(multi_predict_response, std::move(multi_predict_request));
  }
}

void PredictorRouter::calculateVector(CalculateVectorResponses* calculate_vector_responses,
                             std::unique_ptr<CalculateVectorRequests> calculate_vector_requests_ptr) {
  FB_LOG_EVERY_MS(INFO, 2000) << "To be implemented...";
}

void PredictorRouter::calculateBatchVector(CalculateBatchVectorResponses* calculate_batch_vector_responses,
                             std::unique_ptr<CalculateBatchVectorRequests> calculate_batch_vector_requests_ptr) {
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
  if (!setRequestOptionPredictorServiceName(&request_option, model_names)) {
    return;
  }
  // just send request for feature_record request
  const auto req_context = multi_predict_request->get_single_request().get_context();
  if (req_context != nullptr) {
    const auto& iter = req_context->find(REQUEST_TYPE);
    if (iter != req_context->end() && iter->second == FEATURE_RECORD_REQUEST) {
      sendFeatureRecordRequest(multi_predict_request);
      return;
    }
  }
  const MetricTagsMap timer_predict_tag{{TAG_CATEGORY, request_option.predictor_service_name}};
  metrics::Timer timer_predict(util::buildTimers(FORWARD_MULTI_PREDICT_TIME_CONSUMING, timer_predict_tag));

  FB_LOG_EVERY_MS(INFO, 2000) << "forwarding to service name:" << request_option.predictor_service_name;
  std::unique_ptr<MultiPredictResponseFuture> rpc_fu;
  {
    const MetricTagsMap timer_rpc_tag{{TAG_CATEGORY, "rpc_" + request_option.predictor_service_name}};
    metrics::Timer timer_rpc(util::buildTimers(FORWARD_MULTI_PREDICT_TIME_CONSUMING, timer_rpc_tag));
    const bool rc = PredictorClientSDK::future_multi_predict(&rpc_fu, *multi_predict_request);
    if (!rc) {
      FB_LOG_EVERY_MS(ERROR, 2000) << "call future_multi_predict Failed!";
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
  auto& request_option = multi_predict_request->request_option;
  const auto& model_names = multi_predict_request->get_model_names();
  if (!setRequestOptionPredictorServiceName(&request_option, model_names)) {
    return;
  }
  // just send request for feature_record request
  const auto req_context = multi_predict_request->get_single_request().get_context();
  if (req_context != nullptr) {
    const auto& iter = req_context->find(REQUEST_TYPE);
    if (iter != req_context->end() && iter->second == FEATURE_RECORD_REQUEST) {
      sendFeatureRecordRequest(multi_predict_request);
      return;
    }
  }
  const MetricTagsMap timer_predict_tag{{TAG_CATEGORY, "total_" + request_option.predictor_service_name}};
  metrics::Timer timer_predict(util::buildTimers(SPLIT_MULTI_PREDICT_TIME_CONSUMING, timer_predict_tag));

  const auto& multi_predict_req = *multi_predict_request;

  std::vector<common::Range> range_vec;
  const auto& items_size = multi_predict_req.single_request.get_item_features().size();
  if (FLAGS_max_items_per_slice <= 0 ||
      !common::split_slice(&range_vec, items_size, FLAGS_min_items_per_slice,
                           items_size/FLAGS_max_items_per_slice + 1)) {
    FB_LOG_EVERY_MS(ERROR, 2000) << "splitMultiPredictRequest split_slice catch wrong input";
    // fallback to multiPredictSimpleForward when split slice fail
    multiPredictSimpleForward(multi_predict_response, std::move(multi_predict_request));
    return;
  }

  std::vector<folly::Future<bool>> results;
  folly::Synchronized<std::vector<std::unique_ptr<MultiPredictResponseFuture>>> slice_response_futs;
  for (const auto& range : range_vec) {
    auto splitAndSendMultiPredictRequest = [this, range, &multi_predict_req, &slice_response_futs]() {
      const auto& single_request = multi_predict_req.single_request;
      MultiPredictRequest multi_predict_req_split;
      multi_predict_req_split.set_req_id(multi_predict_req.get_req_id());
      multi_predict_req_split.set_model_names(multi_predict_req.get_model_names());
      multi_predict_req_split.set_request_option(multi_predict_req.get_request_option());
      multi_predict_req_split.set_context(*multi_predict_req.get_context());
      auto& single_req_split = multi_predict_req_split.single_request;
      single_req_split.set_common_features(single_request.get_common_features());

      auto item_features_iter = std::next(single_request.get_item_features().begin(), range.begin);
      for (unsigned i = range.begin; i < range.end; ++i) {
        single_req_split.item_features.emplace(*item_features_iter);
        if (++item_features_iter == single_request.get_item_features().end()) {
          break;
        }
      }
      std::unique_ptr<MultiPredictResponseFuture> rpc_fu;
      const auto& request_option = multi_predict_req_split.get_request_option();
      {
        const MetricTagsMap timer_rpc_tag{{TAG_CATEGORY, "rpc_" + request_option.predictor_service_name}};
        metrics::Timer timer_rpc(util::buildTimers(SPLIT_MULTI_PREDICT_TIME_CONSUMING, timer_rpc_tag));
        const bool rc = PredictorClientSDK::future_multi_predict(&rpc_fu, multi_predict_req_split);
        if (!rc) {
          FB_LOG_EVERY_MS(ERROR, 2000) << "call future_multi_predict Failed!";
          return false;
        }
        if (!rpc_fu) {
          FB_LOG_EVERY_MS(ERROR, 2000) << "rpc_fu ptr is NULL!";
          return false;
        } else {
          slice_response_futs.wlock()->emplace_back(std::move(rpc_fu));
        }
      }
      return true;
    };
    results.emplace_back(ResourceMgr::getHeavyTasksThreadPool()->addFuture(splitAndSendMultiPredictRequest));
  }

  folly::collectAll(results)
    .then([](const std::vector<folly::Try<bool>>& lists) {
        for (auto& v : lists) {
          if (v.hasException()) {
            FB_LOG_EVERY_MS(ERROR, 2000)  << "splitMultiPredictRequest got exception";
          }
          if (!v.value()) {
            FB_LOG_EVERY_MS(ERROR, 2000)  << "splitMultiPredictRequest failed";
          }
        }
      })
    .get();
  getAndCombineMultiPredictResponse(multi_predict_response, request_option, *slice_response_futs.rlock());
  const auto req_item_num = multi_predict_request->get_single_request().get_item_features().size();
  if (req_item_num == 0) {
    util::logAndMetricError("item_features_empty", multi_predict_request->get_req_id());
    return;
  }
  for (const auto& model_name : multi_predict_request->get_model_names()) {
    const auto model_resp_item_num = multi_predict_response->model_responses[model_name].results_map.size();
    util::markHistogram(ROUTER_ITEM_RESP_RATIO, TAG_MODEL, model_name,
                        model_resp_item_num * 1.0 / req_item_num);
  }
}

bool PredictorRouter::setRequestOptionPredictorServiceName(RequestOption* request_option,
                                                           const std::vector<std::string>& model_names) {
  if (model_names.empty()) {
    FB_LOG_EVERY_MS(ERROR, 2000) << "model_names is empty";
    return false;
  }
  const auto& model_name = model_names[0];
  if (!request_option) {
    util::logAndMetricError("request_option_is_null");
    return false;
  }
  {
    auto global_model_service_map = ResourceMgr::global_model_service_map_.load();
    if (global_model_service_map == nullptr) {
      util::logAndMetricError("forward_multi_predict_request_error-null_global_model_service_map");
      return false;
    }
    // there is only one service for multiPredict request models
    auto iter = global_model_service_map->find(model_name);
    if (iter != global_model_service_map->end()) {
      request_option->predictor_service_name = iter->second;
    }
  }
  if (request_option->predictor_service_name == PREDICTOR_ROUTER_SERVICE_NAME) {
    util::logAndMetricError("forward_multi_predict_request_error-invalid_model:" + model_name);
    return false;
  }
  return true;
}

void PredictorRouter::sendFeatureRecordRequest(const std::unique_ptr<MultiPredictRequest>& multi_predict_request) {
  std::unique_ptr<MultiPredictResponseFuture> rpc_fu;
  const bool rc = PredictorClientSDK::future_multi_predict(&rpc_fu, *multi_predict_request);
  if (!rc) {
    FB_LOG_EVERY_MS(ERROR, 2000) << "call future_multi_predict Failed!";
  }
  if (!rpc_fu) {
    FB_LOG_EVERY_MS(ERROR, 2000) << "rpc_fu ptr is NULL!";
  }
}

void PredictorRouter::getAndCombineMultiPredictResponse(MultiPredictResponse* combined_response,
                                                            const RequestOption& request_option,
             const std::vector<std::unique_ptr<MultiPredictResponseFuture>>& slice_reponse_futs) {
  const MetricTagsMap timer_get_tag{{TAG_CATEGORY, "get_" + request_option.predictor_service_name}};
  metrics::Timer timer_get(util::buildTimers(SPLIT_MULTI_PREDICT_TIME_CONSUMING, timer_get_tag));
  for (auto& slice_resp_fut : slice_reponse_futs) {
    try {
      const auto& slice_resp = slice_resp_fut->get();
      if (slice_resp.model_responses.empty()) {
        FB_LOG_EVERY_MS(ERROR, 2000) << "splitMultiPredictRequest rpc_fu.get():got empty response";
        continue;
      }
      combineMultiPredictResponse(combined_response, slice_resp);
    } catch(const std::exception& e) {
      std::string err_msg = std::string(typeid(e).name()) + ":" + std::string(e.what());
      FB_LOG_EVERY_MS(ERROR, 2000) << "splitMultiPredictRequest rpc_fu.get(): caught exception, err_msg:" << err_msg;
    }
  }
}

void PredictorRouter::combineMultiPredictResponse(MultiPredictResponse* combined_response,
                                               const MultiPredictResponse& slice_response) {
  combined_response->set_req_id(slice_response.get_req_id());
  for (const auto& model_resp : slice_response.model_responses) {
    const auto& model_name = model_resp.first;
    const auto& model_results_map = model_resp.second.results_map;
    auto& model_predict_response = (combined_response->model_responses)[model_name];
    model_predict_response.req_id = model_resp.second.req_id;
    model_predict_response.results_map.insert(model_results_map.begin(), model_results_map.end());
  }
}
}  // namespace predictor
