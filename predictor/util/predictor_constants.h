#pragma once

#include "common/metrics/metrics.h"
#include <string>
#include <map>

namespace predictor {
//
// CONFIGS etc.
//
// return code
constexpr int RC_SUCCESS = 0;
constexpr int RC_ERROR = -1;

// info of config
constexpr char CONFIG_MODEL_FILE_SUFFIX[] = ".json";
constexpr char CONFIG_FRAMEWORK_NAME[] = "framework";
constexpr char CONFIG_MODEL_NAME[] = "model";
constexpr char CONFIG_VERSION_NAME[] = "model";
constexpr char CONFIG_FULL_NAME[] = "model_full_name";
constexpr char CONFIG_MODEL_DIR[] = "model_dir";
constexpr char CONFIG_FEATURE_EXTRACT[] = "feature_extract_config";
constexpr char MODEL_TIMESTAMP[] = "model_timestamp";
constexpr char COMMON_MODEL_FILE[] = "model_file";
constexpr char COMMON_FEALIB_PATH[] = "fealib_path";
constexpr char FEATURE_FRAMEWORK[] = "feature_framework";
constexpr char CONFIG_FEATURE_CLASS_NAME[] = "feature_class";

// feature extractor
constexpr char FEALIB_CONFIG[] = "fealib_config";
constexpr char FEALIB_FEATUREMASTER[] = "feature_master";
constexpr char FEALIB_XGBOOST_EXAMPLE[] = "xgboost_example";

// other
// currently only used for superfans requests
constexpr char ENABLE_MULTITHREAD[] = "enable_multithread";

// tf
constexpr char TF_PREDICT_TAG[] = "predict_tag";
constexpr char TF_MODEL_TAG[] = "model_tag";
constexpr char TF_OUTPUT_TAGS[] = "output_tags";
constexpr char TF_PREDICT_SIGNATURE[] = "predict_signature";

// strategy
constexpr char CTR_CORRECTION_FILE_NAME[] = "ctr_correction";

//
// METRICS
//
using MetricTagsMap = std::unordered_map<std::string, std::string>;
using PredRes = std::map<int64_t, std::map<std::string, double> >;

// common
const std::vector<int> LEVELS = {60};  // seconds
const std::vector<int> MINUTES = {1};  // meter minutes
const std::vector<double> PERCENTILES = {0.999, 0.99, 0.5};  // histogram percentiles

constexpr char ERROR[] = "error";
constexpr char SDK_MODULE_NAME[] = "predictor_sdk";
constexpr char TAG_CATEGORY[] = "category";
constexpr char TAG_MODEL[] = "model";
constexpr char TAG_BUSINESS_LINE[] = "businessline";
constexpr char TAG_CHANNEL[] = "channel";
constexpr char TAG_PHASE[] = "phase";
constexpr char TAG_SERVICE[] = "service";

// request type
constexpr char REQUEST_TYPE[] = "request_type";
constexpr char PREDICT_REQUEST[] = "predict";
constexpr char FEATURE_RECORD_REQUEST[] = "feature_record";

// server side
constexpr char SERVER_NAME[] = "predictor";
constexpr char PREDICTOR_ROUTER_SERVICE_NAME[] = "predictor_router_service";
constexpr char SERVER_MODE[] = "ServerMode";
constexpr char ROUTER_MODE[] = "RouterMode";

// router constants
constexpr char ROUTER_OP[] = "router_op";
constexpr char SIMPLE_FORWARD[] = "SimpleForward";
constexpr char SPLIT_REQUEST[] = "SplitRequest";

// request type
constexpr char TAG_REQ_TYPE[] = "request_type";
constexpr char REQ_TYPE_PREDICT[] = "predict";
constexpr char REQ_TYPE_MULTI_PREDICT[] = "multi_predict";
constexpr char REQ_TYPE_CALCULATE_VECTOR[] = "calculate_vector";
constexpr char REQ_TYPE_CALCULATE_BATCH_VECTOR[] = "calculate_batch_vector";

// timer
constexpr char TIME_CONSUMING[] = "consuming";
constexpr char ASYNC_TIME_CONSUMING[] = "async_consuming";
constexpr char SYNC_TIME_CONSUMING[] = "sync_consuming";
constexpr char MODEL_CONSUMING[] = "model_consuming";
constexpr char CHANNEL_TIME_CONSUMING[] = "channel_consuming";
constexpr char FORWARD_MULTI_PREDICT_TIME_CONSUMING[] = "forward_multi_predict_consuming";
constexpr char SPLIT_MULTI_PREDICT_TIME_CONSUMING[] = "split_multi_predict_consuming";
constexpr double TIMER_BUCKET_SCALE = 1;
constexpr double TIMER_MIN = 0;
constexpr double TIMER_MAX = 100;
constexpr char TIMER_TAG_req[] = "request";
constexpr char TIMER_TAG_ctr[] = "ctr_predict_request";
constexpr char TIMER_TAG_predictor_rpc[] = "predictor_rpc";
constexpr char TF_CONSUMING[] = "tf_consuming";
constexpr char FEATURE_CONSUMING[] = "feature_consuming";
constexpr char ITEM_CONSUMING[] = "item_consuming";
constexpr char REQUEST_TASK_CONSUMING[] = "request_task_consuming";
constexpr char HEAVY_TASK_CONSUMING[] = "heavy_task_consuming";
constexpr char TAG_TASK[] = "status";
constexpr char TASK_RUN[] = "run";
constexpr char TASK_WAIT[] = "wait";

constexpr char TIMER_TAG_calculate_vector[] = "calculate_vector_request";
constexpr char TIMER_TAG_calculate_vector_rpc[] = "calculate_vector_rpc";

// Histograms
constexpr char NUM_COUNT[] = "numcounting";
constexpr char MODEL_NUM_COUNT[] = "model_numcounting";
constexpr char CHANNEL_NUM_COUNT[] = "channel_numcounting";
constexpr char COMMON_FEATURE_NUM_COUNT[] = "common_feature_numcounting";
constexpr char ITEM_FEATURE_NUM_COUNT[] = "item_feature_numcounting";
constexpr char ROUTER_ITEM_RESP_RATIO[] = "router_item_resp_ratio";
constexpr char AVG_CTR[] = "avg_ctr";
constexpr double HISTOGRAMS_BUCKET_SCALE = 1;
constexpr double HISTOGRAMS_MIN = 0;
constexpr double HISTOGRAMS_MAX = 1000;
constexpr char HISTOGRAMS_TAG_CHANNEL[] = "channel";
constexpr char HISTOGRAMS_TAG_req[] = "request";
constexpr char DEFAULT_CHANNEL_NAME[] = "empty";

// dnn embedding missing Histograms
constexpr char EMBEDDING_METRIC_NAME[] = "embedding_missing_rate";
constexpr char EMBEDDING_MISSING_RATIO_CATEGORY[] = "dnn_embedding_missing_ratio";
constexpr char EMBEDDING_MISSING_RATIO_OVERALL[] = "embedding_overall";
constexpr double EMBEDDING_MISSING_RATIO_BUCKED = 0.00001;
constexpr double EMBEDDING_MISSING_RATIO_MIN = 0.00001;
constexpr double EMBEDDING_MISSING_RATIO_MAX = 1.00001;

// Meters
constexpr char METER_TAG_CHANNEL[] = "channel";  // common
constexpr char METER_TAG_SERVICE[] = "service";  // common
constexpr char DOWNGRADE[] = "downgrade";  // algo_service
constexpr char SYNC_REQ_METER[] = "sync_request_meter";  // sdk
constexpr char SYNC_PREDICT_ERROR[] = "sync_predict_error";  // sdk
constexpr char SYNC_PREDICT_EMPTY[] = "sync_predict_empty";  // sdk
constexpr char ASYNC_REQ_METER[] = "async_request_meter";  // sdk
constexpr char ASYNC_ERROR_METER[] = "async_error_meter";  // sdk
constexpr char ASYNC_GET_EXCEPTION_METER[] = "async_get_exception_meter";  // sdk
constexpr char ASYNC_GET_EMPTY_RESP_METER[] = "async_get_empty_resp_meter";  // sdk

// configs
constexpr char TF_THREAD_NUM[] = "tf_thread_num";
constexpr char HEAVY_TASKS_THREAD_NUM[] = "heavy_tasks_thread_num";
constexpr char FEATURE_EXTRACT_TASKS_NUM[] = "feature_extract_tasks_num";
constexpr char REQUEST_CPU_THREAD_NUM[] = "request_cpu_thread_num";
constexpr char TF_THREAD_RATIO[] = "tf_thread_ratio";
constexpr char HEAVY_TASKS_THREAD_RATIO[] = "heavy_tasks_thread_ratio";
constexpr char FEATURE_EXTRACT_TASKS_RATIO[] = "feature_extract_tasks_ratio";
constexpr char REQUEST_CPU_THREAD_RATIO[] = "request_cpu_thread_ratio";
}  // namespace predictor
