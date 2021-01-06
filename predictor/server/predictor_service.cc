#include "predictor/server/predictor_service.h"

#include "common/metrics/metrics.h"
#include "common/serialize_util.h"
#include "common/util.h"
#include "feature_master/if/gen-cpp2/feature_master_types.tcc"
#include "predictor/if/gen-cpp2/predictor_types.tcc"
#include "predictor/util/predictor_util.h"
#include "predictor/util/predictor_constants.h"
#include <iostream>
#include "predictor/server/resource_manager.h"
#include "predictor/server/fea_extract_snapshot.h"
#include "predictor/util/predictor_util.h"

DEFINE_int32(example_snapshot_frequency, 0, "predictor example_snapshot copy 1/n");
DEFINE_int32(fea_extract_snapshot_frequency, 0, "predictor example_snapshot copy 1/n");
DEFINE_int64(concurrency_cpu_thread_num, 16, "num of concurrency threads for large num of items");
DEFINE_int64(request_cpu_thread_num, 0,
             "num of concurrency threads for processing requests. cpu core number will be applied if set to 0");
DEFINE_int64(min_items_per_thread, 20, "minimum num of items assigned to cpu thread");
DEFINE_string(work_mode, predictor::SERVER_MODE, "work mode of server: ServerMode or RouterMode");
DECLARE_string(example_snapshot_producer_config);
DECLARE_string(fea_extract_snapshot_producer_config);
DECLARE_string(host);
DECLARE_int32(use_model_service);

namespace predictor {

bool PredictorService::init(const std::string& model_path) {
  // heavy_tasks_thread_pool_ is needed by model manager. therefore we must initialize it before using it.
  ResourceMgr::initHeavyTasksThreadPool();
  // init model_manager
  if (!model_manager_.init(model_path)) {
    return false;
  }

  if (!FLAGS_use_model_service) {
    model_manager_.load_all_models();
  }

  if (FLAGS_example_snapshot_frequency > 0) {
    example_snapshot_atomic_count_ = 0;
    if (example_snapshot_producer_.initialize(FLAGS_example_snapshot_producer_config) != Status::OK) {
      util::logAndMetricError("example_snapshot_producer_init_error");
      FLAGS_example_snapshot_frequency = 0;
    }
  }

  if (FLAGS_fea_extract_snapshot_frequency > 0) {
    if (!ResourceMgr::initFeaExtractKafkaProducer(FLAGS_fea_extract_snapshot_producer_config)) {
      util::logAndMetricError("fea_extract_snapshot_producer_config_init_error");
      FLAGS_fea_extract_snapshot_frequency = 0;
    }
  }

  return true;
}

bool PredictorService::load_model(const std::string& model_file, bool is_full_path,
                                  const std::string &model_name, const std::string &business_line) {
  return model_manager_.load_model(model_file, is_full_path, model_name, business_line);
}

void PredictorService::predict(PredictResponses& predict_responses,
                               std::unique_ptr<PredictRequests> predict_requests_ptr) {
  if (FLAGS_work_mode == predictor::ROUTER_MODE) {
    predictor_router_.predict(&predict_responses, std::move(predict_requests_ptr));
    return;
  }
  const MetricTagsMap tag_map{{TAG_REQ_TYPE, REQ_TYPE_PREDICT}};
  metrics::Timer timer(util::buildTimers(TIME_CONSUMING, tag_map));

  FB_LOG_EVERY_MS(INFO, 2000) << "calling predictor" << std::endl;

  send_req_example_snapshot(*predict_requests_ptr);

  if (predict_requests_ptr->get_reqs().size() <= 0) {
    FB_LOG_EVERY_MS(ERROR, 2000) << "empty predict_requests! return";
    return;
  }

  // original request feature extract snapshot
  if (FLAGS_fea_extract_snapshot_frequency > 0) {
    for (const auto& predict_request : predict_requests_ptr->get_reqs()) {
      if (shouldSnapshot(predict_request.get_req_id())) {
        ResourceMgr::getHeavyTasksThreadPool()->addFuture([predict_request]() {
                                                            originalFeatureExtractSnapshot(predict_request);});
      }
    }
  }

  // real predict
  model_manager_.predict_multithread(&predict_responses, std::move(predict_requests_ptr));
}

void PredictorService::multiPredict(MultiPredictResponse& multi_predict_response,
                                    std::unique_ptr<MultiPredictRequest> multi_predict_request) {
  if (FLAGS_work_mode == predictor::ROUTER_MODE) {
    predictor_router_.multiPredict(&multi_predict_response, std::move(multi_predict_request));
    return;
  }
  
  const MetricTagsMap tag_map{{TAG_REQ_TYPE, REQ_TYPE_MULTI_PREDICT}};
  metrics::Timer timer(util::buildTimers(TIME_CONSUMING, tag_map));

  send_req_example_snapshot(*multi_predict_request);
  multi_predict_request->single_request.req_id = multi_predict_request->get_req_id();
  // original request feature extract snapshot
  if (FLAGS_fea_extract_snapshot_frequency > 0) {
    const auto& single_request = multi_predict_request->get_single_request();
    if (shouldSnapshot(single_request.get_req_id())) {
      ResourceMgr::getHeavyTasksThreadPool()->addFuture([single_request]() {
                                                          originalFeatureExtractSnapshot(single_request);});
    }
  }

  FB_LOG_EVERY_MS(INFO, 2000) << "calling predictor multiPredict" << std::endl;
  model_manager_.multiPredict(multi_predict_response, std::move(multi_predict_request));
  return;
}

void PredictorService::calculateVector(CalculateVectorResponses& calculate_vector_responses,
                                       std::unique_ptr<CalculateVectorRequests> calculate_vector_requests_ptr) {
  if (FLAGS_work_mode == predictor::ROUTER_MODE) {
    predictor_router_.calculateVector(&calculate_vector_responses, std::move(calculate_vector_requests_ptr));
    return;
  }

  const MetricTagsMap tag_map{{TAG_REQ_TYPE, REQ_TYPE_CALCULATE_VECTOR}};
  metrics::Timer timer(util::buildTimers(TIME_CONSUMING, tag_map));

  model_manager_.calculateVector(&calculate_vector_responses, std::move(calculate_vector_requests_ptr));
}

std::shared_ptr<std::map<std::string, int>> PredictorService::get_model_downgrade_percent_map() const {
  return model_manager_.get_model_downgrade_percent_map();
}

void PredictorService::set_model_downgrade_percent_map(std::shared_ptr<std::map<std::string, int>> tmp_map_ptr) {
  model_manager_.set_model_downgrade_percent_map(tmp_map_ptr);
}

template<typename T> void PredictorService::send_req_example_snapshot(const T& req) {
  if (FLAGS_example_snapshot_frequency > 0 &&
      ++example_snapshot_atomic_count_ % FLAGS_example_snapshot_frequency == 0) {
    std::string serialize_str;
    if (!common::serializeThriftObjToCompact(&serialize_str, req)) {
      LOG(ERROR) << "Err: serializeThriftObjToJson failed!";
    } else {
      // librdkafka async send
      example_snapshot_producer_.sendKafka(serialize_str);
    }
  }
}

}  // namespace predictor
