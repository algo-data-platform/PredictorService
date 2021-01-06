#pragma once

#include "common/factory_registry.h"
#include "predictor/if/gen-cpp2/PredictorService.h"
#include "predictor/util/predictor_util.h"
#include "predictor/util/predictor_constants.h"
#include "thirdparty/rapidjson/document.h"
#include "predictor/util/predictor_util.h"
#include "predictor/server/model_manager/framework/model.h"
#include "folly/concurrency/AtomicSharedPtr.h"

namespace predictor {

using ModelMap = folly::Synchronized<std::unordered_map<std::string, folly::atomic_shared_ptr<Model>>>;
class ModelFrameworkFactory;

class ModelFramework {
 public:
  virtual ~ModelFramework() = default;

  ModelMap models_;

  virtual bool load(const std::string& path_prefix, const std::string& config_file,
                    const std::string &model_name, const std::string &business_line);

  virtual bool predict(PredictResponse* predict_response, const PredictRequest& predict_request,
                       const std::string& model_full_name) const;

  virtual bool calculateVector(CalculateVectorResponse* calculate_vector_response,
                               const CalculateVectorRequest& calculate_vector_request,
                               const std::string& model_full_name) {
    return false;
  }

  static const std::shared_ptr<ModelFrameworkFactory> getModelFramework(const std::string& model_full_name,
                                                                        const std::string& req_id = "");
};

// registry to get the framework
struct ModelFrameworkFactory
    : public common::FunctionRegEntryBase<ModelFrameworkFactory, std::shared_ptr<ModelFramework> > {};

#define REGISTER_FRAMEWORK(ClassName, Name)                                                 \
  COMMON_REGISTRY_REGISTER(::predictor::ModelFrameworkFactory, ModelFrameworkFactory, Name) \
      .set_body(std::make_shared<ClassName>())

}  // namespace predictor
