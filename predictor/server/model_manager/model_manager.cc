#include "predictor/server/model_manager/model_manager.h"
#include "common/file/file_util.h"
#include "common/util.h"
#include "folly/Synchronized.h"
#include "folly/Random.h"
#include "predictor/server/model_manager/framework/model_framework.h"
#include "predictor/util/predictor_util.h"
#include "predictor/util/predictor_util.h"
#include "thirdparty/rapidjson/document.h"
#include "predictor/global_resource/resource_manager.h"
#include <dirent.h>
#include <fstream>
#include <sstream>
#include <sys/stat.h>

DECLARE_int64(request_cpu_thread_num);
DECLARE_bool(monitor_thread_task_stats);

namespace predictor {
namespace {
constexpr char LOG_CATEGORY[] = "model_manager.cc";
PredictResponse makeDownGradeResponse(const std::string &model_full_name, const PredictRequest &predict_request) {
  PredictResponse predict_response;
  predict_response.req_id = predict_request.get_req_id();
  // downgrade log and metrics
  const MetricTagsMap downgrade_tags_map{{TAG_MODEL, model_full_name}};
  metrics::Metrics::getInstance()->buildMeter(SERVER_NAME, DOWNGRADE, downgrade_tags_map)->mark();
  FB_LOG_EVERY_MS(INFO, 2000) << "hit downgrade strategy, return empty response, model=" << model_full_name;
  return predict_response;
}
void markUnknownError(const std::string &req_id) {
  const MetricTagsMap err_tag{{TAG_CATEGORY, "predict_unknown_error"}};
  metrics::Metrics::getInstance()->buildMeter(SERVER_NAME, ERROR, err_tag)->mark();
  FB_LOG_EVERY_MS(ERROR, 2000) << "ModelManager unknown error, request id: " << req_id;
}
}  // namespace

bool ModelManager::init(const std::string& model_path) {
  model_path_ = model_path;

  // set up cpu thread pool
  if (0 == FLAGS_request_cpu_thread_num) {
    unsigned core_num = std::thread::hardware_concurrency();
    FLAGS_request_cpu_thread_num = core_num > 0 ? core_num : 16;
  }
  auto threadFactory = std::make_shared<folly::NamedThreadFactory>("request_thread_pool");
  cpu_thread_pool_ = std::make_shared<folly::FutureExecutor<folly::CPUThreadPoolExecutor>>(
      std::make_pair(FLAGS_request_cpu_thread_num /*max thread num*/,
      1 /*min thread num*/),
      threadFactory);

  if (FLAGS_monitor_thread_task_stats) {
    util::monitorThreadTaskStats(cpu_thread_pool_.get(), REQUEST_TASK_CONSUMING);
  }

  return true;
}

void ModelManager::setThreadPool(int max_thread_num) {
  cpu_thread_pool_->setNumThreads(max_thread_num);
  LOG(INFO) << "set request_cpu_thread_num=" << max_thread_num;
}

void ModelManager::load_all_models() {
  std::vector<std::string> path_vec;
  common::FileUtil::list_dir(&path_vec, model_path_);
  for (auto& file : path_vec) {
    if (common::FileUtil::is_file_with_suffix(file, CONFIG_MODEL_FILE_SUFFIX)) {
      LOG(INFO) << "processing config file=" << file;
      if (!load_model(file, true)) {
        LOG(ERROR) << "failed to load model for config file=" << file;
      }
    }
  }
}

bool ModelManager::load_model(const std::string& model_file, bool is_full_path,
                              const std::string &model_name, const std::string &business_line) {
  const std::string full_model_file = is_full_path ? model_file : common::pathJoin(model_path_, model_file);

  std::string config_json;
  if (!common::FileUtil::read_file_str(&config_json, full_model_file)) {
    LOG(ERROR) << "failed to translate file to json for config file=" << full_model_file;
    return false;
  }

  rapidjson::Document document;
  document.Parse(config_json.c_str());

  if (!document.HasMember(CONFIG_FRAMEWORK_NAME)) {
    LOG(ERROR) << "Has no framework, file=" << full_model_file;
    return false;
  }

  std::string framework_name = document[CONFIG_FRAMEWORK_NAME].GetString();
  std::shared_ptr<predictor::ModelFrameworkFactory> factory =
      common::FactoryRegistry<predictor::ModelFrameworkFactory>::Find(framework_name);
  if (factory == nullptr) {
    LOG(ERROR) << "ModelFrameworkFactory has no framework=" << framework_name;
    return false;
  }

  std::shared_ptr<predictor::ModelFramework> framework = factory->body;
  try {
    auto path_prefix = is_full_path ? common::FileUtil::extract_dir(full_model_file) : model_path_;
    if (!framework->load(path_prefix, config_json, model_name, business_line)) {
      LOG(ERROR) << "Failed to create model=" << framework_name;
      google::FlushLogFiles(google::INFO);
      google::FlushLogFiles(google::ERROR);
      return false;
    }
  } catch (const std::exception &e) {
    LOG(ERROR) << "Failed to create model framework=" << framework_name << ". Error=" << e.what();
    return false;
  }

  return true;
}

void ModelManager::predict_multithread(PredictResponses* predict_responses,
                                       std::unique_ptr<PredictRequests> predict_requests_ptr) {
  std::vector<folly::Future<ResultPair> > fut_responses;
  std::vector<PredictResponse> responses;
  for (const auto& predict_request : predict_requests_ptr->get_reqs()) {
    const std::string& model_full_name = predict_request.get_model_name();
    if (shouldDowngradeModel(model_full_name)) {
      responses.emplace_back(makeDownGradeResponse(model_full_name, predict_request));
      continue;
    }
    // async predict
    fut_responses.emplace_back(cpu_thread_pool_->addFuture([this, &model_full_name, &predict_request](){
      return this->singlePredict(model_full_name, predict_request);
    }));
  }

  folly::collectAll(fut_responses)
      .then([&responses](const std::vector<folly::Try<ResultPair> >& lists) {
        for (auto& v : lists) {
          if (v.hasException()) {
            markUnknownError(v.value().second.get_req_id());
            continue;
          }
          responses.emplace_back(std::move(v.value().second));
        }
      })
      .get();

  predict_responses->set_resps(responses);
}


void ModelManager::multiPredict(MultiPredictResponse& multi_predict_response,
                                std::unique_ptr<MultiPredictRequest> multi_predict_request) {
  std::vector<folly::Future<ResultPair> > fut_results;
  std::map<std::string, PredictResponse> model_responses;

  multi_predict_response.set_req_id(multi_predict_request->get_req_id());
  const auto &predict_request = multi_predict_request->get_single_request();
  for (const auto& model_full_name : multi_predict_request->get_model_names()) {
    if (shouldDowngradeModel(model_full_name)) {
      model_responses.emplace(model_full_name, makeDownGradeResponse(model_full_name, predict_request));
      continue;
    }
    // async predict
    fut_results.emplace_back(cpu_thread_pool_->addFuture([this, &model_full_name, &predict_request](){
      return this->singlePredict(model_full_name, predict_request);
    }));
  }

  folly::collectAll(fut_results)
      .then([&model_responses](const std::vector<folly::Try<ResultPair> >& lists) {
        for (auto& v : lists) {
          if (v.hasException()) {
            markUnknownError(v.value().second.get_req_id());
            continue;
          }
          model_responses.emplace(std::move(v.value()));
        }
      })
      .get();

  multi_predict_response.set_model_responses(std::move(model_responses));
  multi_predict_response.set_return_code(0);
  return;
}

void ModelManager::calculateVector(CalculateVectorResponses* calculate_vector_responses,
                                   std::unique_ptr<CalculateVectorRequests> calculate_vector_requests_ptr) {
  std::vector<folly::Future<CalculateVectorResponse>> fut_responses;
  for (const auto& calculate_vector_request : calculate_vector_requests_ptr->get_reqs()) {
    auto single_model_calculate_vector = [&calculate_vector_request]() mutable {
      const std::string& model_full_name = calculate_vector_request.get_model_name();
      FB_LOG_EVERY_MS(INFO, 2000) << "calculateVector: model_full_name=" << model_full_name
                                  << ", req_id=" << calculate_vector_request.get_req_id();
      CalculateVectorResponse calculate_vector_response;
      calculate_vector_response.req_id = calculate_vector_request.get_req_id();

      auto log_error = [&calculate_vector_response, &calculate_vector_request](const std::string& err_category) {
        calculate_vector_response.set_return_code(RC_ERROR);
        const MetricTagsMap err_tag{{TAG_CATEGORY, err_category}};
        auto err_meter = metrics::Metrics::getInstance()->buildMeter(SERVER_NAME, ERROR, err_tag);
        err_meter->mark();
        FB_LOG_EVERY_MS(ERROR, 2000) << "ModelManager::calculateVector error: " << err_category
                                     << ", req_id: " << calculate_vector_request.get_req_id();
      };

      std::vector<std::string> tmp;
      folly::split('_', model_full_name, tmp);
      if (tmp.size() < 3) {
        log_error(model_full_name + "-invalid_model_name");
        return calculate_vector_response;
      }
      std::string& framework_name = tmp[0];
      std::shared_ptr<predictor::ModelFrameworkFactory> factory =
          common::FactoryRegistry<predictor::ModelFrameworkFactory>::Find(framework_name);
      if (factory == nullptr) {
        log_error(model_full_name + "-invalid_framework");
        return calculate_vector_response;
      }

      // metrics
      {
        // common feature size (calculate_vector_request does not have item features)
        util::markHistogram(COMMON_FEATURE_NUM_COUNT, TAG_MODEL, model_full_name,
                            calculate_vector_request.get_features().get_features().size());
        // item-level qps, calculateVector interface currently carries only 1 item
        const MetricTagsMap item_tags_map{{TAG_MODEL, model_full_name}};
        metrics::Metrics::getInstance()->buildMeter(SERVER_NAME, ITEM_CONSUMING, item_tags_map)->mark();
      }

      std::shared_ptr<predictor::ModelFramework> framework = factory->body;
      if (!framework->calculateVector(&calculate_vector_response, calculate_vector_request, model_full_name)) {
        log_error(model_full_name + "-calculateVector_error");
        return calculate_vector_response;
      }

      calculate_vector_response.set_return_code(RC_SUCCESS);
      return calculate_vector_response;
    };

    fut_responses.emplace_back(cpu_thread_pool_->addFuture(single_model_calculate_vector));
  }

  std::vector<CalculateVectorResponse> responses;
  folly::collectAll(fut_responses)
      .then([&responses](const std::vector<folly::Try<CalculateVectorResponse>>& lists) {
        for (auto& v : lists) {
          if (v.hasException()) {
            const MetricTagsMap err_tag{{TAG_CATEGORY, "calculateVector_unknown_error"}};
            auto err_meter = metrics::Metrics::getInstance()->buildMeter(SERVER_NAME, ERROR, err_tag);
            err_meter->mark();
            FB_LOG_EVERY_MS(ERROR, 2000) << "ModelManager::calculateVector unknown error, request id: "
                                         << v.value().get_req_id();
            continue;
          }
          responses.emplace_back(std::move(v.value()));
        }
      })
      .get();

  calculate_vector_responses->set_resps(std::move(responses));
}

bool ModelManager::shouldDowngradeModel(const std::string& model_full_name) const {
  if (!model_downgrade_percent_map_ptr_) {
    return false;
  }
  int downgrade_percent = 0;
  auto it = model_downgrade_percent_map_ptr_->find(model_full_name);
  if (it == model_downgrade_percent_map_ptr_->end()) {
    return false;
  } else {
    downgrade_percent = it->second;
  }
  if (downgrade_percent == 0) {
    return false;
  } else if (downgrade_percent == 100) {
    return true;
  }
  int expand_index = folly::Random::secureRand32(1, 100);
  return expand_index <= downgrade_percent;
}

std::shared_ptr<std::map<std::string, int>> ModelManager::get_model_downgrade_percent_map() const {
  return model_downgrade_percent_map_ptr_;
}

void ModelManager::set_model_downgrade_percent_map(std::shared_ptr<std::map<std::string, int>> tmp_map_ptr) {
  model_downgrade_percent_map_ptr_ = tmp_map_ptr;
}

ResultPair ModelManager::singlePredict(const std::string &model_full_name,
                                       const PredictRequest &predict_request) const {
  const std::string& req_id = predict_request.get_req_id();
  PredictResponse predict_response;
  predict_response.req_id = req_id;  // set req id in response regardless

  FB_LOG_EVERY_MS(INFO, 2000) << "model_full_name=" << model_full_name << ", req_id=" << req_id;
  const int item_num = predict_request.get_item_features().size();
  if (0 == item_num) {  // not considered as an error
    FB_LOG_EVERY_MS(INFO, 2000) << "Got empty-item request for model = " << model_full_name
                                << ", req_id=" << req_id;
    return {model_full_name, predict_response};
  }

  const auto factory = predictor::ModelFramework::getModelFramework(model_full_name, req_id);
  if (!factory) {
    util::logAndMetricError(model_full_name + "-invalid_framework", req_id);
    return {model_full_name, predict_response};
  }

  // metrics
  {
    // model-level num of items
    util::markHistogram(MODEL_NUM_COUNT, TAG_MODEL, model_full_name, item_num);
    // channel-level num of items
    util::markHistogram(CHANNEL_NUM_COUNT, TAG_CHANNEL,
                        predict_request.channel.empty() ? DEFAULT_CHANNEL_NAME : predict_request.channel,
                        item_num);
    // common feature size
    util::markHistogram(COMMON_FEATURE_NUM_COUNT, TAG_MODEL, model_full_name,
                        predict_request.get_common_features().get_features().size());
    // item feature size, only report if there is at least one item
    if (item_num > 0) {
      util::markHistogram(ITEM_FEATURE_NUM_COUNT, TAG_MODEL, model_full_name,
                          predict_request.get_item_features().begin()->second.get_features().size());
    }
    // item-level qps
    const MetricTagsMap item_tags_map{{TAG_MODEL, model_full_name}};
    metrics::Metrics::getInstance()->buildMeter(SERVER_NAME, ITEM_CONSUMING, item_tags_map)->mark(item_num);
  }

  // start predicting
  std::shared_ptr<predictor::ModelFramework> framework = factory->body;
  if (!framework->predict(&predict_response, predict_request, model_full_name)) {
    util::logAndMetricError(model_full_name + "-predict_error", req_id);
    return {model_full_name, predict_response};
  }

  const int result_map_size = predict_response.get_results_map().size();
  if (result_map_size > 0) {
    // avg ctr metrics
    double sum_ctr = 0;
    for (const auto &result : predict_response.get_results_map()) {
      const auto &iter = result.second.preds.find("ctr");
      if (iter != result.second.preds.end()) {
        sum_ctr += iter->second;
      }
    }
    util::markHistogram(AVG_CTR, TAG_MODEL, model_full_name, sum_ctr / result_map_size);
  }

  if (result_map_size != item_num) {
    util::logAndMetricError(model_full_name + "-missing_response", req_id);
    return {model_full_name, predict_response};
  }

  return {model_full_name, predict_response};
}

bool ModelManager::setModelBusinessLine(const std::string &model_full_name, const std::string &business_line) {
  // parse framework
  const auto& framework_factory = predictor::ModelFramework::getModelFramework(model_full_name);
  if (!framework_factory) {
    util::logAndMetricError(model_full_name + "-invalid_framework");
    return false;
  }
  // set model attribute
  framework_factory->body->models_.withWLock([&](auto& wlock) {
    auto iter = wlock.find(model_full_name);
    if (iter != wlock.end()) {
      iter->second.load()->business_line_ = business_line;
    }
  });
  return true;
}


void ModelManager::calculateBatchVector(CalculateBatchVectorResponses* calculate_batch_vector_responses,
                                std::unique_ptr<CalculateBatchVectorRequests> calculate_batch_vector_requests_ptr) {
  std::vector<folly::Future<CalculateBatchVectorResponse> > fut_responses;

  for (const auto &batch_request : calculate_batch_vector_requests_ptr->get_reqs()) {
    // async calculate
    fut_responses.emplace_back(cpu_thread_pool_->addFuture([this, &batch_request](){
      return this->singleCalculateBatchVector(batch_request);
    }));
  }

  std::vector<CalculateBatchVectorResponse> responses;
  folly::collectAll(fut_responses)
      .then([&responses](const std::vector<folly::Try<CalculateBatchVectorResponse>>& lists) {
        for (auto& v : lists) {
          if (v.hasException()) {
            util::logAndMetricError("calculateBatchVector_unknown_error", v.value().get_req_id());
          }
          // clients should validate the response
          responses.emplace_back(std::move(v.value()));
        }
      })
      .get();

  calculate_batch_vector_responses->set_resps(std::move(responses));
  return;
}


CalculateBatchVectorResponse ModelManager::singleCalculateBatchVector(
                             const CalculateBatchVectorRequest &batch_request) const {
  const std::string &req_id = batch_request.get_req_id();
  const std::string &model_full_name = batch_request.get_model_name();
  CalculateBatchVectorResponse batch_response;
  batch_response.req_id = req_id;  // set req id in response regardless
  batch_response.set_return_code(RC_SUCCESS);

  FB_LOG_EVERY_MS(INFO, 2000) << "model_full_name=" << model_full_name << ", req_id=" << req_id;
  const int item_num = batch_request.get_features_map().size();
  if (0 == item_num) {
    FB_LOG_EVERY_MS(INFO, 2000) << "Got empty request for model = " << model_full_name
                                << ", req_id=" << req_id;
    return batch_response;
  }

  const auto factory = predictor::ModelFramework::getModelFramework(model_full_name, req_id);
  if (!factory) {
    util::logAndMetricError(model_full_name + "-invalid_framework", req_id);
    batch_response.set_return_code(RC_ERROR);
    return batch_response;
  }

  // metrics
  {
    // model-level num of ads
    util::markHistogram(MODEL_NUM_COUNT, TAG_MODEL, model_full_name, item_num);
    // channel-level num of ads
    util::markHistogram(CHANNEL_NUM_COUNT, TAG_CHANNEL,
                        batch_request.channel.empty() ? DEFAULT_CHANNEL_NAME : batch_request.channel,
                        item_num);
    // item-level qps
    const MetricTagsMap item_tags_map{{TAG_MODEL, model_full_name}};
    util::markMeter(ITEM_CONSUMING, item_tags_map, item_num);
  }

  std::shared_ptr<predictor::ModelFramework> framework = factory->body;
  if (!framework->calculateBatchVector(&batch_response, batch_request, model_full_name)) {
    util::logAndMetricError(model_full_name + "-calculate_batch_vector_error", req_id);
    batch_response.set_return_code(RC_ERROR);
    return batch_response;
  }

  const int vector_map_size = batch_response.get_vector_map().size();
  if (vector_map_size != item_num) {
    util::logAndMetricError(model_full_name + "-missing_response", req_id);
    batch_response.set_return_code(RC_ERROR);
  }

  return batch_response;
}

}  // namespace predictor
