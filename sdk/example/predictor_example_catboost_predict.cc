#include "init.h"
#include "predictor_client_sdk.h"

void constructRequest(predictor::PredictClientRequest* request_ptr) {
  request_ptr->channel = "test";
  request_ptr->req_id = "100";
  request_ptr->model_name = "catboost_catboost_calcer_basic_extractor_v0_native_demo";
  // catboost model does not have common features

  // item features
  std::vector<double> float_features = {12.0, 3.0, 34.0, 23.0, 1.0, 3.0};
  std::vector<predictor::PredictFeature> item_feature_list;
  item_feature_list.push_back(predictor::client_util::parse("float_features", float_features));
  item_feature_list.push_back(predictor::client_util::parse("cat_features", {"a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k"}));  // NOLINT
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
      std::cout << resp.first << ":" << resp.second << std::endl;  // response ctr should be 1.04698
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
        std::cout << resp.first << ":" << resp.second << std::endl;  // response ctr should be 1.04698
      }
    }
  }

  // stop
  init.stop();

  return 0;
}
