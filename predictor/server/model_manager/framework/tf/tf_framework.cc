#include "predictor/server/model_manager/framework/tf/tf_framework.h"
#include "predictor/util/predictor_util.h"

namespace predictor {

bool TFFramework::calculateVector(CalculateVectorResponse* calculate_vector_response,
                                  const CalculateVectorRequest& calculate_vector_request,
                                  const std::string& model_full_name) {
  // get model
  std::shared_ptr<Model> model_ptr = nullptr;
  models_.withRLock([&model_full_name, &model_ptr](auto& rlock) {
    auto iter = rlock.find(model_full_name);
    if (iter != rlock.end()) {
      model_ptr = iter->second.load();
    }
  });
  if (!model_ptr) {
    LOG(ERROR) << "failed to find model object, model_full_name=" << model_full_name;
    return false;
  }

  // per model time consuming
  const MetricTagsMap model_tag_map{
    {TAG_MODEL, model_ptr->model_full_name_},
    {TAG_BUSINESS_LINE, model_ptr->business_line_}
  };
  metrics::Timer model_timer(util::buildTimers(MODEL_CONSUMING, model_tag_map));
  // per channel time consuming
  const MetricTagsMap channel_tag_map{
    {TAG_CHANNEL, calculate_vector_request.channel.empty() ? DEFAULT_CHANNEL_NAME
                                                                 : calculate_vector_request.channel},
    {TAG_BUSINESS_LINE, model_ptr->business_line_}
  };
  metrics::Timer channel_timer(util::buildTimers(CHANNEL_TIME_CONSUMING, channel_tag_map));
  return model_ptr->calculateVector(calculate_vector_response, calculate_vector_request);
}

REGISTER_FRAMEWORK(TFFramework, tf).describe("This is a tf framework.");

}  // namespace predictor
