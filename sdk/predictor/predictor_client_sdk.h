#pragma once

#include <map>
#include <memory>
#include <string>
#include <thread>  //  NOLINT
#include <vector>
#include <sstream>

// forward declarations start
namespace folly {
class Init;
template <class T>
class Future;
template <class T>
class Optional;
}  // namespace folly
namespace proxygen {
class HTTPServer;
}  // namespace proxygen

namespace common {
void getLocalIpAddress(std::string* ip_addr);
}
namespace predictor {
//
// thrift structs
//
class PredictRequest;
class PredictRequests;
class PredictResponse;
class MultiPredictRequest;
class PredictResponses;
class MultiPredictResponse;
class CalculateVectorRequest;
class CalculateVectorRequests;
class CalculateVectorResponse;
class CalculateVectorResponses;
class CalculateBatchVectorRequest;
class CalculateBatchVectorRequests;
class CalculateBatchVectorResponse;
class CalculateBatchVectorResponses;
class PredictResponsesThriftFuture;
class MultiPredictResponseFuture;
class CalculateVectorResponsesThriftFuture;
class CalculateBatchVectorResponsesThriftFuture;
//
// non-thrift structs
//
class PredictClientRequest;
class PredictClientResponse;
class PredictResponsesFuture;
class CalculateVectorClientRequest;
class CalculateVectorClientResponse;
class CalculateBatchVectorClientRequest;
class CalculateBatchVectorClientResponse;
}  // namespace predictor
namespace service_router {
class ServerList;
class ClientOption;
}  // namespace service_router
namespace feature_master {
class Feature;
}  // namespace feature_master
// forward declarations end

namespace predictor {
class PredictorClientOption {
 public:
  PredictorClientOption() {
    connection_timeout = 40;
      // rpc连接超时时间（毫秒），仅第一次连接使用
    request_timeout = 60;
      // rpc请求超时时间（毫秒），每个请求使用
    ip_diff_range = 65535;
      // 同机房ip范围（ip前两段相同认为同机房）
    max_conn_per_server = 3;
      // 连接数（可设定为cpu线程数）
    predictor_service_name = "algo_service";
      // 服务发现中predictor服务端service name
    http_port = 8797;
      // http监控页面端口
    predictor_load_balance_method = "random";
      // rpc请求使用的负载均衡策略
    use_predictor_static_list = false;
      // 是否使用静态列表（即不使用动态服务注册发现），用于服务注册发现不可用的情形，如consul集群不可用
    server_ip = "";
    server_port = 0;
  }

  ~PredictorClientOption() = default;
  PredictorClientOption(const PredictorClientOption& rhs) = default;
  PredictorClientOption(PredictorClientOption&& rhs) = default;
  PredictorClientOption& operator=(const PredictorClientOption& rhs) = default;
  PredictorClientOption& operator=(PredictorClientOption&& rhs) = default;

  std::string print() const;

 public:
  std::string local_ip;
  int connection_timeout;
  int request_timeout;
  int ip_diff_range;
  int max_conn_per_server;
  std::string predictor_service_name;
  int http_port;
  std::string predictor_load_balance_method;
  bool use_predictor_static_list;
  std::string server_ip;
  int server_port;
};

class PredictorClientSDK {
 public:
  explicit PredictorClientSDK(const PredictorClientOption& option);
  virtual ~PredictorClientSDK() = default;

 public:
  //
  // thrift interfaces
  //
  static bool predict(std::vector<PredictResponse>* response_list,
                      const PredictRequests& requests);
  static bool future_predict(std::unique_ptr<PredictResponsesThriftFuture>* predictor_responses_future,
                             const PredictRequests& predict_requests);

  static bool multi_predict(MultiPredictResponse* multi_predict_resp,
                            const MultiPredictRequest& multi_predict_req);

  static bool future_multi_predict(std::unique_ptr<MultiPredictResponseFuture>* multi_predict_resp_future,
                             const MultiPredictRequest& multi_predict_req);

  static bool future_calculate_vector(
    std::unique_ptr<CalculateVectorResponsesThriftFuture>* calculate_vector_responses_future,
    const CalculateVectorRequests& calculate_vector_requests);

  static bool future_calculate_batch_vector(
  std::unique_ptr<CalculateBatchVectorResponsesThriftFuture>* batch_responses_future,
  const CalculateBatchVectorRequests& batch_requests);
  //
  // non-thrift interfaces
  //
  static bool predict(std::vector<PredictClientResponse>* client_response_list,
                      const std::vector<PredictClientRequest>& client_request_list,
                      const PredictorClientOption &predictor_client_option = PredictorClientOption());
  static bool future_predict(std::unique_ptr<PredictResponsesFuture>* predictor_responses_future,
                             const std::vector<PredictClientRequest>& client_request_list,
                             const PredictorClientOption &predictor_client_option = PredictorClientOption());
  static bool calculate_vector(std::vector<CalculateVectorClientResponse>* client_response_list,
                            const std::vector<CalculateVectorClientRequest>& calculate_vector_client_requests,
                            const PredictorClientOption &predictor_client_option = PredictorClientOption());
  static bool calculate_batch_vector(std::vector<CalculateBatchVectorClientResponse>* client_response_list,
                                const std::vector<CalculateBatchVectorClientRequest>& client_requests,
                                const PredictorClientOption &predictor_client_option = PredictorClientOption());
  //
  // data members
  //
 public:
  static std::string predictor_load_balance_method_;
  static std::string predictor_client_ip_;
  static std::string predictor_service_name_;
  static int32_t predictor_ip_diff_range_;
  static int32_t predictor_connection_timeout_;
  static int32_t predictor_request_timeout_;
  static int32_t predictor_max_conn_per_server_;
};

/////////////////////////////////
////  thrift client structs  ////
/////////////////////////////////
//
// future wrapper for thrift client
//
struct PredictResponsesThriftFuture {
  virtual ~PredictResponsesThriftFuture() {}
  virtual std::vector<PredictResponse> get() = 0;
};
struct PredictResponsesThriftFutureImpl : PredictResponsesThriftFuture {
  std::unique_ptr<folly::Future<PredictResponses>> fut_ptr;
  std::string req_ids;
  std::vector<std::pair<std::string, std::string>> channel_model_names;
  std::string service_name;
  std::vector<PredictResponse> get() override;
};
struct MultiPredictResponseFuture {
  virtual ~MultiPredictResponseFuture() {}
  virtual MultiPredictResponse get() = 0;
};
struct MultiPredictResponseFutureImpl : MultiPredictResponseFuture {
  std::unique_ptr<folly::Future<MultiPredictResponse>> fut_ptr;
  std::string req_id;
  std::vector<std::string> model_names;
  std::string channel;
  std::string service_name;
  std::string request_type;
  MultiPredictResponse get();
};
struct CalculateVectorResponsesThriftFuture {
  std::unique_ptr<folly::Future<CalculateVectorResponses>> fut_ptr;
  std::string req_ids;
  std::vector<std::pair<std::string, std::string>> channel_model_names;
  std::string service_name;
  std::vector<CalculateVectorResponse> get();
};

struct CalculateBatchVectorResponsesThriftFuture {
  std::unique_ptr<folly::Future<CalculateBatchVectorResponses>> fut_ptr;
  std::string req_ids;
  std::vector<std::pair<std::string, std::string>> channel_model_names;
  std::string service_name;
  std::vector<CalculateBatchVectorResponse> get();
};
//
// feature making utilities for thrift client
//
namespace client_util {
// make thrift feature with name and value
feature_master::Feature makeFeature(const std::string& name, int64_t value);
feature_master::Feature makeFeature(const std::string& name, double value);
feature_master::Feature makeFeature(const std::string& name, const std::string& value);
feature_master::Feature makeFeature(const std::string& name, const std::vector<int64_t>& value_vec);
feature_master::Feature makeFeature(const std::string& name, const std::vector<double>& value_vec);
feature_master::Feature makeFeature(const std::string& name, const std::vector<std::string>& value_vec);
}  // namespace client_util

/////////////////////////////////////
////  non-thrift client structs  ////
/////////////////////////////////////
//
// feature wrapper for non-thrift client
//
enum FeatureType {
  STRING_TYPE = 1,
  FLOAT_TYPE = 2,
  INT_TYPE = 3
};
struct PredictFeature {
  std::string feature_name;
  FeatureType feature_type;
  std::vector<std::string> string_values;
  std::vector<double> float_values;
  std::vector<int64_t> int64_values;

  PredictFeature() {}
  PredictFeature(const PredictFeature& rhs) = default;
  PredictFeature(PredictFeature&& rhs) = default;
  PredictFeature& operator=(const PredictFeature& rhs) = default;
  PredictFeature& operator=(PredictFeature&& rhs) = default;
};

//
// feature making utilities for non-thrift client
//
namespace client_util {
PredictFeature parse(const std::string& name, int64_t value);
PredictFeature parse(const std::string& name, double value);
PredictFeature parse(const std::string& name, const std::string& value);
PredictFeature parse(const std::string& name, const std::vector<int64_t>& value_vec);
PredictFeature parse(const std::string& name, const std::vector<double>& value_vec);
PredictFeature parse(const std::string& name, const std::vector<std::string>& value_vec);
}  // namespace client_util


//
// request/response wrapper for non-thrift client
//
struct PredictClientRequest {
  std::string req_id;
  std::string channel;
  std::string model_name;
  std::vector<PredictFeature> common_feature;
  std::map<int64_t, std::vector<PredictFeature>> item_features;

  PredictRequest toPredictRequest() const;
};
struct PredictClientResponse {
  std::string req_id;
  std::map<int64_t, double> item_results;
  std::map<int64_t, double> corrected_ctr_map;
};
struct CalculateVectorClientRequest {
  std::string req_id;
  std::string channel;
  std::string model_name;
  std::vector<PredictFeature> features;
  std::vector<std::string> output_names;

  CalculateVectorRequest toCalculateVectorRequest() const;
};
struct CalculateVectorClientResponse {
  std::string req_id;
  std::string model_timestamp;
  std::map<std::string, std::vector<double>> vector_map;
  int32_t return_code;
};

struct CalculateBatchVectorClientRequest {
  std::string req_id;
  std::string channel;
  std::string model_name;
  std::map<int64_t, std::vector<PredictFeature>> feature_map;
  std::vector<std::string> output_names;

  CalculateBatchVectorRequest toCalculateBatchVectorRequest() const;
};
struct CalculateBatchVectorClientResponse {
  std::string req_id;
  std::string model_timestamp;
  std::map<int64_t, std::map<std::string, std::vector<double>>> vector_map;
  int32_t return_code;
};

//
// future wrapper for non-thrift client
//
struct PredictResponsesFuture {
  virtual ~PredictResponsesFuture() {}
  virtual std::vector<PredictClientResponse> get() { return {}; }
};
struct PredictResponsesFutureImpl : PredictResponsesFuture {
  std::unique_ptr<folly::Future<PredictResponses>> fut_ptr;
  std::string req_ids;
  std::vector<std::pair<std::string, std::string>> channel_model_names;
  std::string service_name;
  std::vector<PredictClientResponse> get() override;
};
}  //  namespace predictor
