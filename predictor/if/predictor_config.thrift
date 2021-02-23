namespace cpp2 predictor

struct ModelConfig {
  1: required string model_framework;  # tf
  2: required string model_class;      # estimator_multi
  3: required string feature_class;    # feature_extractor
  4: required string version;          # v0
  5: required string model_name;       # ctr
  6: string additional = "NA";         # arbitrary string, username etc.
  7: string model_file = "";           # actual model file path
}

struct TfConfig {
  1: required string export_dir;      # tf_model
  2: required list<string> tags;      # serve
  3: required string signature_def;   # serving_default
  4: required list<string> output_tensor_names;
  5: double negative_sampling_ratio = 1.0;
  6: string model_timestamp = "";
}
