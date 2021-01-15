namespace cpp2 predictor

struct ModelConfig {
  1: required string model_framework;  # tf
  2: required string model_class;      # estimator_multi
  3: required string feature_class;    # feature_extractor
  4: required string version;          # v0
  5: required string model_name;       # ctr
  6: required string additional;       # arbitrary
  7: required string model_file;       # actual model file path
}
