include "feature_master/if/feature_master.thrift"

namespace cpp2 predictor

struct RequestOption {
  1: required string local_ip = "127.0.0.1"  # 本地ip
  2: required i32 connection_timeout = 40  # 连接超时时间（毫秒）
  3: required i32 request_timeout = 60  # 请求超时时间（毫秒）
  4: required i32 ip_diff_range = 65535  # 同机房ip范围 （ip前两段相同认为同机房）
  5: required i32 max_conn_per_server = 3  # 连接数（可设定为cpu线程数）
  6: required string predictor_service_name = ""  # consul中predictor服务名 eg:app_service
  7: required i32 http_port = 8797  # http监控端口号
  8: required string predictor_load_balance_method = "random"  # 负载均衡策略
  9: required bool use_predictor_static_list = false  # 是否使用predictor静态列表
}

struct PredictRequest {
  1: required string req_id
  2: required string channel  # hot-feed , banner , etc 
  3: required string model_name 
  4: required map<i64, feature_master.Features> item_features // per item features
  5: required feature_master.Features common_features  // user or context features to be shared
  6: optional map<string, string> context    // additional context
}

struct PredictResponse {
  1: required string req_id
  2: map<i64, feature_master.PredictResults> results_map // 记录对于不同label的pXtr
}

struct PredictRequests {
  1: required list<PredictRequest> reqs
  2: required RequestOption request_option   // option use by service_router and thrift
  3: optional map<string, string> context    // additional context
}

struct PredictResponses {
  1: required list<PredictResponse> resps 
}

struct MultiPredictRequest {
  1: required string req_id
  2: required list<string> model_names       // list of model names that share features
  3: required PredictRequest single_request  // wraps shared common and item features
  4: required RequestOption request_option   // option use by service_router and thrift
  5: optional map<string, string> context    // additional context
}

struct MultiPredictResponse {
  1: required string req_id
  2: required map<string, PredictResponse> model_responses  // <model_name, response>
  3: required i32 return_code
}

exception PredictException {
  1: string message
}

struct CalculateVectorRequest {
  1: required string req_id
  2: required string channel  # hot-feed , banner , etc 
  3: required string model_name 
  4: required feature_master.Features features
  5: required list<string> output_names
}

struct CalculateVectorResponse {
  1: required string req_id
  2: required string model_timestamp
  3: required map<string, list<double>> vector_map # output name -> vector
  4: optional i32 return_code
}

struct CalculateVectorRequests {
  1: required list<CalculateVectorRequest> reqs
  2: required RequestOption request_option   // option use by service_router and thrift
  3: optional map<string, string> context    // additional context
}

struct CalculateVectorResponses {
  1: required list<CalculateVectorResponse> resps 
}

exception CalculateVectorException {
  1: string message
}

service PredictorService {
  PredictResponses predict(1: PredictRequests reqs) throws (1: PredictException e)
  MultiPredictResponse multiPredict(1: MultiPredictRequest multi_req) throws (1: PredictException e)
  CalculateVectorResponses calculateVector(1: CalculateVectorRequests reqs) throws (1: CalculateVectorException e)
}
