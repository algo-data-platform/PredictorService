#include "predictor_client_sdk.h"
#include "predictor_sdk_util.h"
#include "common/serialize_util.h"
#include "folly/GLog.h"
#include "folly/Random.h"
#include "folly/init/Init.h"
#include "gflags/gflags.h"
#include "gflags/gflags_declare.h"
#include <dirent.h>
#include <sstream>
#include <sys/types.h>

namespace service_router {
DECLARE_string(router_consul_addresses);
DECLARE_int32(thrift_timeout_retry);
DECLARE_int32(thrift_connection_retry);
}  // namespace service_router

namespace predictor {
std::string PredictorClientSDK::predictor_load_balance_method_ = "localfirst";  //  NOLINT
std::string PredictorClientSDK::predictor_client_ip_ = "127.0.0.1";             //  NOLINT
std::string PredictorClientSDK::predictor_service_name_ = "predictor_service";  //  NOLINT
int32_t PredictorClientSDK::predictor_ip_diff_range_ = 65535;
int32_t PredictorClientSDK::predictor_connection_timeout_ = 10;
int32_t PredictorClientSDK::predictor_request_timeout_ = 30;
int32_t PredictorClientSDK::predictor_max_conn_per_server_ = 40;

PredictorClientSDK::PredictorClientSDK(const PredictorClientOption& option) {
  // initialize some gflags
  predictor_client_ip_ = option.local_ip;
  predictor_connection_timeout_ = option.connection_timeout;
  predictor_request_timeout_ = option.request_timeout;
  predictor_ip_diff_range_ = option.ip_diff_range;
  predictor_max_conn_per_server_ = option.max_conn_per_server;
  predictor_service_name_ = option.predictor_service_name;
  predictor_load_balance_method_ = option.predictor_load_balance_method;

  LOG(INFO) << "predictor client config: consul_addresses=" << service_router::FLAGS_router_consul_addresses
            << " local_ip=" << predictor_client_ip_ << " connection_timeout=" << predictor_connection_timeout_
            << " request_timeout=" << predictor_request_timeout_ << " ip_diff_range=" << predictor_ip_diff_range_
            << " max_conn_per_server=" << predictor_max_conn_per_server_ << " service_name=" << predictor_service_name_
            << " connection_retry=" << service_router::FLAGS_thrift_connection_retry
            << " timeout_retry=" << service_router::FLAGS_thrift_timeout_retry
            << " predictor_load_balance_method_=" << predictor_load_balance_method_;
  LOG(INFO) << "PredictorClientSDK constructor done";
  google::FlushLogFiles(google::INFO);
  google::FlushLogFiles(google::ERROR);
}

std::string PredictorClientOption::print() const {
  std::stringstream ss;
  ss << "local_ip=" << local_ip
     << ", connection_timeout=" << connection_timeout << ", request_timeout=" << request_timeout
     << ", ip_diff_range=" << ip_diff_range << ", max_conn_per_server=" << max_conn_per_server
     << ", predictor_service_name=" << predictor_service_name << ", http_port=" << http_port
     << ", predictor_load_balance_method=" << predictor_load_balance_method
     << ", use_predictor_static_list=" << use_predictor_static_list;
  return ss.str();
}
}  //  namespace predictor
