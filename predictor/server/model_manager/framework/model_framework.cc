#include "predictor/server/model_manager/framework/model_framework.h"
#include "predictor/util/predictor_util.h"

namespace predictor {
bool ModelFramework::load(const ModelConfig &model_config,
                          const std::string &model_package_dir,
                          const std::string &business_line) {
  // find model factory
  const std::string &model_full_name = util::getModelFullName(model_config);
  auto factory = common::FactoryRegistry<ModelFactory>::Find(model_config.model_class);
  if (!factory) {
    util::logAndMetricError(model_config.model_class + "-invalid_model_class");
    return false;
  }
  // create model ptr
  std::shared_ptr<Model> model = factory->body();
  if (!model->load(model_config, model_package_dir, business_line)) {
    util::logAndMetricError(model_full_name + "-model_load_fail");
    return false;
  }
  // save to ModelMap
  models_.withWLock([&model_full_name, model](auto& wlock) {
    wlock[model_full_name] = model;
  });
  return true;
}

bool ModelFramework::predict(PredictResponse* response,
                             const PredictRequest& request,
                             const std::string& model_full_name) const {
  // get model
  std::shared_ptr<Model> model_ptr = nullptr;
  models_.withRLock([&model_full_name, &model_ptr](auto& rlock) {
    auto iter = rlock.find(model_full_name);
    if (iter != rlock.end()) {
      model_ptr = iter->second;
    }
  });

  if (model_ptr != nullptr) {
    // per model time consuming
    const MetricTagsMap model_tag_map{{TAG_MODEL, model_ptr->model_full_name_},
                                      {TAG_BUSINESS_LINE, model_ptr->business_line_}};
    metrics::Timer model_timer(util::buildTimers(MODEL_CONSUMING, model_tag_map));
    // per channel time consuming
    const MetricTagsMap channel_tag_map{{TAG_CHANNEL, request.channel.empty() ? DEFAULT_CHANNEL_NAME : request.channel},
                                        {TAG_BUSINESS_LINE, model_ptr->business_line_}};
    metrics::Timer channel_timer(util::buildTimers(CHANNEL_TIME_CONSUMING, channel_tag_map));
    if (!model_ptr->predict(response, request)) {
      return false;
    }
  } else {
    // log error
    FB_LOG_EVERY_MS(ERROR, 2000) << "Invalid model_full_name=" << model_full_name;
    return false;
  }
  return true;
}

bool ModelFramework::calculateVector(CalculateVectorResponse* response,
                                     const CalculateVectorRequest& request,
                                     const std::string& model_full_name) const {
  // get model
  std::shared_ptr<Model> model_ptr = nullptr;
  models_.withRLock([&model_full_name, &model_ptr](auto& rlock) {
    auto iter = rlock.find(model_full_name);
    if (iter != rlock.end()) {
      model_ptr = iter->second;
    }
  });

  if (model_ptr == nullptr) {
    // log error
    FB_LOG_EVERY_MS(ERROR, 2000) << "Invalid model_full_name=" << model_full_name;
    return false;
  }

  // per model time consuming
  const MetricTagsMap model_tag_map{{TAG_MODEL, model_ptr->model_full_name_},
                                    {TAG_BUSINESS_LINE, model_ptr->business_line_}};
  metrics::Timer model_timer(util::buildTimers(MODEL_CONSUMING, model_tag_map));
  // per channel time consuming
  const MetricTagsMap channel_tag_map{{TAG_CHANNEL, request.channel.empty() ? DEFAULT_CHANNEL_NAME : request.channel},
                                      {TAG_BUSINESS_LINE, model_ptr->business_line_}};
  metrics::Timer channel_timer(util::buildTimers(CHANNEL_TIME_CONSUMING, channel_tag_map));
  if (!model_ptr->calculateVector(response, request)) {
    return false;
  }

  return true;
}

bool ModelFramework::calculateBatchVector(CalculateBatchVectorResponse* batch_response,
                                          const CalculateBatchVectorRequest& batch_request,
                                          const std::string& model_full_name) const {
  // get model
  std::shared_ptr<Model> model_ptr = nullptr;
  models_.withRLock([&model_full_name, &model_ptr](auto& rlock) {
    auto iter = rlock.find(model_full_name);
    if (iter != rlock.end()) {
      model_ptr = iter->second;
    }
  });

  if (model_ptr == nullptr) {
    // log error
    FB_LOG_EVERY_MS(ERROR, 2000) << "Invalid model_full_name=" << model_full_name;
    return false;
  }

  // per model time consuming
  const MetricTagsMap model_tag_map {
    {TAG_MODEL, model_ptr->model_full_name_},
    {TAG_BUSINESS_LINE, model_ptr->business_line_}
  };
  metrics::Timer model_timer(util::buildTimers(MODEL_CONSUMING, model_tag_map));
  // per channel time consuming
  const MetricTagsMap channel_tag_map {
    {TAG_CHANNEL, batch_request.channel.empty() ? DEFAULT_CHANNEL_NAME : batch_request.channel},
    {TAG_BUSINESS_LINE, model_ptr->business_line_}
  };
  metrics::Timer channel_timer(util::buildTimers(CHANNEL_TIME_CONSUMING, channel_tag_map));
  if (!model_ptr->calculateBatchVector(batch_response, batch_request)) {
    return false;
  }

  return true;
}

const std::shared_ptr<ModelFrameworkFactory> ModelFramework::getModelFramework(const std::string& model_full_name,
                                                                      const std::string& req_id) {
  std::vector<std::string> splitted_name;
  folly::split('_', model_full_name, splitted_name);
  if (splitted_name.size() < 3) {
    util::logAndMetricError(model_full_name + "-invalid_model_name", req_id);
    return nullptr;
  }
  std::string& framework_name = splitted_name[0];
  const auto& factory = common::FactoryRegistry<ModelFrameworkFactory>::Find(framework_name);
  return factory;
}

}  // namespace predictor

namespace common {
COMMON_REGISTRY_ENABLE(predictor::ModelFrameworkFactory);
}  // namespace common
