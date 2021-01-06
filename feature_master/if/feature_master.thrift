// ref: https://github.com/tensorflow/tensorflow/tree/master/tensorflow/core/example

namespace cpp2 feature_master

// label 定义
const string LABEL_SHOW_SERVER = "show_server";
const string LABEL_SHOW_CLIENT = "show_client";
const string LABEL_CLICK = "click";  
const string LABEL_LIKE = "like";  
const string LABEL_FORWARD = "forward";  
const string LABEL_COMMENT = "comment";  

enum FeatureType {  // 原始特征类别
  STRING_LIST = 1,
  FLOAT_LIST = 2,
  INT64_LIST = 3,
}

struct Feature {
  1: required string feature_name,
  2: required FeatureType feature_type,  // Each feature can be exactly one kind !!!
  3: required list<string> string_values,
  4: required list<double> float_values,
  5: required list<i64> int64_values,
}

struct Features {
  1: required list<Feature> features,
}

struct ExampleTrain {  // 一个训练样本
  1: required Features features,
  2: required Features labels,
  3: required i64 timestamp,
}

struct ExamplePredict {  // 一个预测样本
  1: required Features features,
}

struct PredictResults {  // 预测结果
  1: required map<string, double> preds  // 记录对于不同label的pXtr
}

struct ExamplePredictSnapshot {  // 快照预估时候的特征场景, 用于跟用户行为做样本拼接
  1: required string join_key,  // 多半是可能是 ad mark_id
  2: required Features features,
  3: required PredictResults results  // 记录对于不同label的pXtr
}
