#pragma once

#include <string>
#include <vector>
#include <memory>
#include "folly/executors/IOThreadPoolExecutor.h"
#include "predictor/server/predictor_service/predictor_service.h"
#include "service_router/http.h"
#include "proxygen/lib/http/HTTPMessage.h"
#include "proxygen/httpserver/ResponseBuilder.h"
#include "proxygen/httpserver/RequestHandlerFactory.h"
#include "predictor/util/predictor_util.h"
#include "predictor/util/predictor_data.h"

namespace predictor {

class HttpService {
 public:
  HttpService(const HttpService&) = delete;
  HttpService(HttpService&&) = delete;
  HttpService& operator=(const HttpService&) = delete;
  HttpService& operator=(HttpService&&) = delete;
  explicit HttpService(std::shared_ptr<ModelManager> model_manager);
  ~HttpService();

  bool start(const std::string& host, uint16_t port);
  void stop();
  void unregisterAll();

 private:
  // http API functions
  void registerServiceName(service_framework::http::ServerResponse* response,
                           std::unique_ptr<proxygen::HTTPMessage> msg);

  void unregisterServiceName(service_framework::http::ServerResponse* response,
                           std::unique_ptr<proxygen::HTTPMessage> msg);

  void setServerStaticWeight(service_framework::http::ServerResponse* response,
                             std::unique_ptr<proxygen::HTTPMessage> msg);

  void loadAndRegister(service_framework::http::ServerResponse* response,
                       std::unique_ptr<proxygen::HTTPMessage> msg,
                       std::unique_ptr<folly::IOBuf> buf);

  void getServiceModelInfo(service_framework::http::ServerResponse* response,
                           std::unique_ptr<proxygen::HTTPMessage> msg);

  void updateDowngradePercent(service_framework::http::ServerResponse* response,
                           std::unique_ptr<proxygen::HTTPMessage> msg,
                           std::unique_ptr<folly::IOBuf> buf);

  void resetDowngradePercent(service_framework::http::ServerResponse* response,
                           std::unique_ptr<proxygen::HTTPMessage> msg);

  void getDowngradePercent(service_framework::http::ServerResponse* response,
                           std::unique_ptr<proxygen::HTTPMessage> msg);

  void setStressParams(service_framework::http::ServerResponse* response,
                       std::unique_ptr<proxygen::HTTPMessage> msg,
                       std::unique_ptr<folly::IOBuf> buf);
  void setStressInfos(const std::string& models, const std::string& qps, const std::string& service);

  void setWorkMode(service_framework::http::ServerResponse* response,
                   std::unique_ptr<proxygen::HTTPMessage> msg);

  void updateGlobalModelServiceMap(service_framework::http::ServerResponse* response,
                   std::unique_ptr<proxygen::HTTPMessage> msg,
                   std::unique_ptr<folly::IOBuf> buf);

  void getGlobalModelServiceMap(service_framework::http::ServerResponse* response,
                   std::unique_ptr<proxygen::HTTPMessage> msg);

  // internal functions
  void load_models(const std::set<ModelRecord> &model_records);
  void unload_models(const std::set<std::string> &useless_model_names);
  void register_services(const std::map<std::string, std::set<ModelRecord> > &service_models,
                         const std::map<std::string, int> &service_weight);
  void register_service_name(const std::string &service_name, int server_weight);
  void unregister_services(const std::set<std::string> &useless_service_names);
  void unregister_service_name(const std::string &service_name);
  void unregister_services_and_unload_models(const std::set<std::string> &useless_service_names,
    const std::set<std::string> &useless_model_names);
  void reset_service_config(const folly::dynamic &config_object);
  void save_config();

 private:
  std::shared_ptr<folly::CPUThreadPoolExecutor> http_cpu_thread_pool_;
  std::shared_ptr<ModelManager> model_manager_;
  std::shared_ptr<std::thread> http_server_thread_;
  std::shared_ptr<proxygen::HTTPServer> http_server_;

  folly::Synchronized<std::map<std::string, std::shared_ptr<service_router::Server> > > service_map_;
  folly::Synchronized<std::map<std::string, ModelRecord> > model_map_;
  folly::Synchronized<std::map<std::string, std::set<std::string> > > service_models_map_;
  folly::Synchronized<std::map<std::string, int> > service_weight_map_;
  folly::Synchronized<folly::dynamic> configs_;
  int default_server_weight_;
};

}  // namespace predictor
