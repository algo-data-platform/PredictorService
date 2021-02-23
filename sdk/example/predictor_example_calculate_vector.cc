#include <iostream>

#include "init.h"
#include "predictor_client_sdk.h"

namespace predictor {
}

void constructRequests(std::vector<predictor::CalculateVectorClientRequest>* predict_requests) {
      predictor::CalculateVectorClientRequest request;
      // 请求唯一标识
      request.req_id = "100";
      // 业务标识
      request.channel = "test";
      // 模型名
      request.model_name = "tf_estimator_native_basic_extractor_v0_vector_calc_demo";
      request.output_names = std::vector<std::string>{"user_vec", "bias"};

      // 构建request.Features
      auto& common_features = request.features;
      common_features.emplace_back(predictor::client_util::parse("gender", (int64_t)1));
      common_features.emplace_back(predictor::client_util::parse("location_code2", (int64_t)1));
      common_features.emplace_back(predictor::client_util::parse("platform_type", "1"));
      common_features.emplace_back(predictor::client_util::parse("os_version", (int64_t)1));
      common_features.emplace_back(predictor::client_util::parse("os_brand", (int64_t)1));
      common_features.emplace_back(predictor::client_util::parse("wm", "1"));
      common_features.emplace_back(predictor::client_util::parse("from", "1"));
      common_features.emplace_back(predictor::client_util::parse("age", (int64_t)1));
      common_features.emplace_back(predictor::client_util::parse("login_freq_num", (int64_t)1));
      common_features.emplace_back(predictor::client_util::parse("lift_state_all", "1"));
      common_features.emplace_back(predictor::client_util::parse("interest_category_all", "1"));
      common_features.emplace_back(predictor::client_util::parse("interest_word_all", "1"));

      // set predict_requests
      predict_requests->emplace_back(std::move(request));
}


int main(int argc, char* argv[]) {
  std::cout << "predictor tf test starts" << std::endl;
  std::string local_ip{""};
  common::getLocalIpAddress(&local_ip);
  if (local_ip.empty()) {
    std::cerr << "Failed to get local ip" << std::endl;
    return -1;
  }

  std::string params = local_ip + " -http-port=9999 -log_dir=/tmp/";
  predictor::Init init("", params);

  predictor::PredictorClientOption request_option;
  // 本机ip
  request_option.local_ip = local_ip;
  // load_balance 策略为iprange
  request_option.predictor_load_balance_method = "iprangefirst";
  // 连接超时时间（毫秒）
  request_option.connection_timeout = 30;
  // 请求超时时间（毫秒）
  request_option.request_timeout = 60;
  // 连接数（可设定为cpu线程数）
  request_option.max_conn_per_server = 8;
  // 同机房ip范围 （ip前两段相同认为同机房）
  request_option.ip_diff_range = 65535;

  request_option.server_ip = local_ip;
  request_option.server_port = 10047;

  // construct request
  std::vector<predictor::CalculateVectorClientRequest> requests;
  constructRequests(&requests);

  std::vector<predictor::CalculateVectorClientResponse> client_response_list;
  if (!predictor::PredictorClientSDK::calculate_vector(&client_response_list, requests, request_option)) {
    std::cerr << "calculate_vector predict failed!" << std::endl;
  } else {
    std::cout << "calculate_vector sync_predict successful!" << std::endl;
    for (const auto& resp : client_response_list) {
      std::cout << "resp.model_timestamp:" << resp.model_timestamp << std::endl;
      const std::map<std::string, std::vector<double>>& results_map = resp.vector_map;
      for (auto& result : results_map) {
        std::cout << "OutputName:" << result.first << " size is:" << result.second.size()<< std::endl;
        auto res = result.second;
        std::cout << "\tres = [";
        for (int i = 0; i < res.size(); i ++) {
          std::cout << res[i] << " ";
        }
        std::cout << "]" << std::endl;
      }
    }
    /*  
      // EXPECTED OUTPUTS ARE:
      resp.model_timestamp:20201221055500
      OutputName:bias size is:1
        res = [-1.14334 ]
      OutputName:user_vec size is:32
        res = [1061.27 -32316.6 -55635 28483.1 -24477.3 -91815.1 30770.1 28534.8 45262.6 73146.1 60562.9 37351 -9948.37 -282771 -162472 -82191 39045.4 -45073.3 49138.7 -269480 28617.6 35560.8 15200.9 88546.2 -73168 52228.7 -47212.8 13480.4 -42545.5 -1.93022e+06 41496.5 -2.38457e+06 ]
    */
  }
  std::this_thread::sleep_for(std::chrono::milliseconds(10000));
  // stop
  init.stop();

  return 0;
}
