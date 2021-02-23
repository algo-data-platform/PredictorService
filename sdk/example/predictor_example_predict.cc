#include "init.h"
#include "predictor_client_sdk.h"


void constructRequest(predictor::PredictClientRequest* request_ptr) {
  request_ptr->channel = "test";
  request_ptr->req_id = "100";
  request_ptr->model_name = "tf_estimator_native_basic_extractor_v0_logistic_demo";
  // user level/common features
  request_ptr->common_feature.emplace_back(predictor::client_util::parse("gender", "1"));
  request_ptr->common_feature.emplace_back(predictor::client_util::parse("plat", "5"));

  // item level/item features
  std::vector<predictor::PredictFeature> item_feature_list;
  item_feature_list.push_back(predictor::client_util::parse("cuid", "1191965271"));
  item_feature_list.push_back(predictor::client_util::parse("ctr1", 147.0));
  item_feature_list.push_back(predictor::client_util::parse("ctr2", 146.0));
  item_feature_list.push_back(predictor::client_util::parse("intimate", 0.0));
  item_feature_list.push_back(predictor::client_util::parse("pv", 0.0));
  item_feature_list.push_back(predictor::client_util::parse("clk", 0.0));
  request_ptr->item_features[10001] = item_feature_list;
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

  // construct predictor client
  predictor::PredictorClientOption option;
  option.local_ip = local_ip;
  option.connection_timeout = 30;
  option.request_timeout = 60;

  option.server_ip = local_ip;
  option.server_port = 10047;
  predictor::PredictorClientSDK predictor_client(option);

  // construct request
  predictor::PredictClientRequest request;
  constructRequest(&request);

  // sync mode
  std::vector<predictor::PredictClientResponse> client_response_list;
  std::vector<predictor::PredictClientRequest> client_request_list;
  client_request_list.push_back(request);
  if (!predictor::PredictorClientSDK::predict(&client_response_list, client_request_list, option)) {
    std::cerr << "sync_predict failed!" << std::endl;
  } else {
    std::cout << "sync_predict successful!" << std::endl;
    for (const auto& resp : client_response_list[0].item_results) {
      std::cout << resp.first << ":" << resp.second << std::endl;  // response ctr should be 0.155444
    }
  }

  // aysnc mode
  std::unique_ptr<predictor::PredictResponsesFuture> response_unique_ptr;
  if (!predictor::PredictorClientSDK::future_predict(&response_unique_ptr, client_request_list, option)) {
    std::cout << "async_predict failed!" << std::endl;
  } else {
    const std::vector<predictor::PredictClientResponse>& client_response_list = response_unique_ptr->get();
    if (1 != client_response_list.size()) {
      std::cout << "size of responses is not 1!" << std::endl;
    } else {
      std::cout << "async_predict successful!" << std::endl;
      for (const auto& resp : client_response_list[0].item_results) {
        std::cout << resp.first << ":" << resp.second << std::endl;  // response ctr should be 0.155444
      }
    }
  }

  // stop
  init.stop();

  return 0;
}
