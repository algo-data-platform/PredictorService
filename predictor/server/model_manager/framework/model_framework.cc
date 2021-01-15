#include "predictor/server/model_manager/framework/model_framework.h"

namespace predictor {

    bool ModelFramework::load(const std::string& path_prefix, const std::string& config_file,
                    const std::string &model_name, const std::string &business_line) {
    rapidjson::Document document;
    document.Parse(config_file.c_str());

    if (!document.HasMember(CONFIG_MODEL_NAME) || !document.HasMember(CONFIG_FULL_NAME)) {
      // log error
      return false;
    }
    std::string full_name = document[CONFIG_FULL_NAME].GetString();

    std::shared_ptr<predictor::ModelFactory> factory =
        common::FactoryRegistry<predictor::ModelFactory>::Find(document[CONFIG_MODEL_NAME].GetString());
    if (factory == nullptr) {
      // log error
      return false;
    }

    std::shared_ptr<predictor::Model> model = factory->body();
    if (!model->init(path_prefix, document)) {
      // log error
      return false;
    }
    model->model_full_name_ = full_name;
    model->business_line_ = business_line;

    try {
      models_.withWLock([&full_name, model](auto& wlock) {
        wlock[full_name] = model;
      });
    }
    catch (std::exception e) {
      LOG(ERROR) << "Failed to create modle=" << full_name << ". Error=" << e.what();
      return false;
    }

    return true;
  }

  bool ModelFramework::predict(PredictResponse* predict_response, const PredictRequest& predict_request,
                       const std::string& model_full_name) const {
    // get model
    std::shared_ptr<predictor::Model> model_ptr = nullptr;
    models_.withRLock([&model_full_name, &model_ptr](auto& rlock) {
      auto iter = rlock.find(model_full_name);
      if (iter != rlock.end()) {
        model_ptr = iter->second;
      }
    });

    if (model_ptr != nullptr) {
      // per model time consuming
      const MetricTagsMap model_tag_map{
        {TAG_MODEL, model_ptr->model_full_name_},
        {TAG_BUSINESS_LINE, model_ptr->business_line_}
      };
      metrics::Timer model_timer(util::buildTimers(MODEL_CONSUMING, model_tag_map));
      // per channel time consuming
      const MetricTagsMap channel_tag_map{
        {TAG_CHANNEL, predict_request.channel.empty() ? DEFAULT_CHANNEL_NAME : predict_request.channel},
        {TAG_BUSINESS_LINE, model_ptr->business_line_}
      };
      metrics::Timer channel_timer(util::buildTimers(CHANNEL_TIME_CONSUMING, channel_tag_map));
      if (!model_ptr->predict(predict_response, predict_request)) {
        return false;
      }
    } else {
      // log error
      FB_LOG_EVERY_MS(ERROR, 2000) << "Invalid model_full_name=" << model_full_name;
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

  bool ModelFramework::calculateBatchVector(CalculateBatchVectorResponse* batch_response,
                                            const CalculateBatchVectorRequest& batch_request,
                                            const std::string& model_full_name) const {
    // get model
    std::shared_ptr<predictor::Model> model_ptr = nullptr;
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
}  // namespace predictor

namespace common {
COMMON_REGISTRY_ENABLE(predictor::ModelFrameworkFactory);
}  // namespace common
