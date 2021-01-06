#pragma once

#include <string>
#include "common/factory_registry.h"
#include "predictor/if/gen-cpp2/PredictorService.h"
#include "thirdparty/rapidjson/document.h"
#include "predictor/server/model_manager/framework/fea_extract/fea_extract_interface.h"

namespace predictor {

/*
 *  BASE CLASS OF ALL MODELS
 */
class Model {
 public:
  //
  // constructors
  //
  virtual ~Model() = default;

  //
  // api functions
  //
  virtual bool init(const std::string& path_prefix, const rapidjson::Document& document) = 0;
  virtual bool predict(PredictResponse* predict_response, const PredictRequest& predict_request) { return false;}
  virtual bool calculateVector(CalculateVectorResponse* calculate_vector_response,
                               const CalculateVectorRequest& calculate_vector_request) { return false; }

  //
  // member variables
  //
  std::string model_full_name_;
  std::string business_line_;
  std::shared_ptr<FeaExtractInterface> fea_extractor_;
};

// registry to get the models
struct ModelFactory
    : public common::FunctionRegEntryBase<ModelFactory, std::function<std::shared_ptr<Model>()> > {};

#define REGISTER_MODEL(ClassName, Name)                                    \
  COMMON_REGISTRY_REGISTER(::predictor::ModelFactory, ModelFactory, Name) \
      .set_body([]() { return std::shared_ptr<Model>(new ClassName()); })

}  // namespace predictor
