#include "predictor/server/model_manager/framework/tf/tf_model_estimator.h"

#include "tensorflow/core/platform/env.h"
#include "tensorflow/core/public/session.h"
#include "tensorflow/core/platform/types.h"
#include "tensorflow/cc/saved_model/loader.h"
#include "tensorflow/core/protobuf/tensor_bundle.pb.h"
#include "tensorflow/core/example/example.pb.h"

#include "common/util.h"
#include "predictor/util/predictor_constants.h"

DECLARE_int64(tf_thread_num);

namespace predictor {

namespace {

constexpr char LOG_CATEGORY[] = "tf_model_estimator.cc: ";

}  // namespace

TFModelEstimator::TFModelEstimator() {}

bool TFModelEstimator::tfInit(const std::string& path_prefix, const rapidjson::Document& document) {
  // fetch config
  if (!document.HasMember(CONFIG_MODEL_DIR) || !document.HasMember(TF_PREDICT_TAG) ||
      !document.HasMember(TF_MODEL_TAG) || !document.HasMember(TF_PREDICT_SIGNATURE)) {
    LOG(ERROR) << "invalid tf config";
    return false;
  }
  std::string model_dir = common::pathJoin(path_prefix, document[CONFIG_MODEL_DIR].GetString());
  predict_tag_ = document[TF_PREDICT_TAG].GetString();
  model_tag_ = document[TF_MODEL_TAG].GetString();
  predict_signature_ = document[TF_PREDICT_SIGNATURE].GetString();
  if (document.HasMember(TF_OUTPUT_TAGS)) {
    const rapidjson::Value& output_tags = document[TF_OUTPUT_TAGS];
    if (output_tags.IsObject()) {
      for (auto iter = output_tags.MemberBegin(); iter != output_tags.MemberEnd(); iter++) {
        std::string output_name = iter->name.GetString();
        std::string output_tag = iter->value.GetString();
        LOG(INFO) << "Adding output_name=" << output_name << ", output_tag=" << output_tag;
        output_tags_map_[output_name] = output_tag;
      }
    }
  }
  if (document.HasMember(MODEL_TIMESTAMP)) {
    model_timestamp_ = document[MODEL_TIMESTAMP].GetString();
    LOG(INFO) << "model_timestamp_=" << model_timestamp_;
  }
  LOG(INFO) << "model_dir=" << model_dir << ", predict tag=" << predict_tag_ << ", model_tag=" << model_tag_
               << ", predict signature=" << predict_signature_;

  // load tf model
  std::unordered_set<std::string> tags = {model_tag_};
  tensorflow::SessionOptions options;
  if (FLAGS_tf_thread_num > 0) {
    options.config.session_inter_op_thread_pool();
    options.config.add_session_inter_op_thread_pool();
    std::string thread_pool_name{"tf_pool_size_"};
    thread_pool_name.append(folly::to<std::string>(FLAGS_tf_thread_num));
    options.config.mutable_session_inter_op_thread_pool(0)->set_global_name(thread_pool_name);
    options.config.mutable_session_inter_op_thread_pool(0)->set_num_threads(FLAGS_tf_thread_num);
  }
  bundle_.reset(new tensorflow::SavedModelBundle());
  tensorflow::Status load_status =
      tensorflow::LoadSavedModel(options, tensorflow::RunOptions(), model_dir, tags, bundle_.get());
  if (!load_status.ok()) {
    LOG(ERROR) << "Loading tf model failed. dir:" << model_dir << " status:" << load_status.ToString();
    return false;
  }

  // construct input map
  auto signature_map = bundle_->meta_graph_def.signature_def();
  auto signature_predict = signature_map[std::string(predict_signature_)];
  inputs_map_ = signature_predict.inputs();
  return true;
}


void TFModelEstimator::buildTfCache() {
  LOG(INFO) << "running an empty request to trigger the building of cache";
  std::vector<std::pair<std::string, tensorflow::Tensor>> inputs;
  std::vector<std::pair<int64_t, std::string>> examples;
  examples.emplace_back(std::make_pair(1, ""));
  int example_num = examples.size();
  for (const auto& imap : inputs_map_) {
    const auto& tensor_info = imap.second;
    tensorflow::Tensor x(tensor_info.dtype(), tensorflow::TensorShape({example_num, 1}));
    for (size_t i = 0; i < example_num; i++) {
      switch (tensor_info.dtype()) {
        case tensorflow::DataType::DT_STRING: {
          x.matrix<std::string>()(i, 0) = "";
          break;
        }
        case tensorflow::DataType::DT_FLOAT: {
          x.matrix<float>()(i, 0) = 0.0;
          break;
        }
        case tensorflow::DataType::DT_INT64: {
          x.matrix<tensorflow::int64>()(i, 0) = 0;
          break;
        }
        default: { break; }
      }
    }
    inputs.emplace_back(make_pair(tensor_info.name(), std::move(x)));
  }
  std::vector<tensorflow::Tensor> outputs;
  bundle_->session->Run(inputs, {predict_tag_}, {}, &outputs);
}

bool TFModelEstimator::checkTfInputStructure() {
  if (1 != inputs_map_.size()) {
    LOG(ERROR) << "wrong size of predict inputs. expect to be 1, but got "
                  << inputs_map_.size();
    return false;
  }

  LOG(INFO) << "succ. inputs_map_.size:" << inputs_map_.size();
  for (const auto& imap : inputs_map_) {
    LOG(INFO) << "input name = " << imap.first << ", input tensorinfo dtype=" << imap.second.dtype()
                 << ", input tensor name = " << imap.second.name()
                 << ", dim=" << imap.second.tensor_shape().dim().size();
    const auto& tensor_info = imap.second;
    if (imap.second.tensor_shape().dim().size() >= 1) {
      int feature_size_info = tensor_info.tensor_shape().dim(0).size();
      LOG(INFO) << "feature_size_info=" << feature_size_info;
    } else {
      LOG(INFO) << "imap.second.tensor_shape().dim().size()=0";
    }
    if (tensor_info.dtype() != tensorflow::DataType::DT_STRING) {
    LOG(ERROR) << "wrong tensor datatype! expects string, got " << tensor_info.dtype();
    return false;
    }
  }
  return true;
}

bool TFModelEstimator::init(const std::string& path_prefix, const rapidjson::Document& document) {
  if (!tfInit(path_prefix, document)) {
    return false;
  }

  if (!checkTfInputStructure()) {
    return false;
  }

  buildTfCache();
  return true;
}

}  // namespace predictor
