#include "predictor/server/http_service/http_service.h"

#include <iostream>
#include <algorithm>
#include <math.h>
#include "common/util.h"
#include "common/file/file_util.h"
#include "folly/String.h"
#include "folly/DynamicConverter.h"
#include "folly/executors/GlobalExecutor.h"
#include "service_router/thrift.h"
#include "service_router/router.h"
#include "predictor/util/predictor_constants.h"
#include "predictor/global_resource/resource_manager.h"
#include "predictor/util/predictor_util.h"
#include "predictor/config/config_util.h"

DEFINE_string(predictor_service_name, "predictor_service_dev", "predictor service name");
DEFINE_int32(normalize_server_weight, 1, "whether to normalize server weight");
DEFINE_int32(always_trust_content_service, 1, "always accept what content service has posted");
DEFINE_bool(enable_service_config, true, "use service config passed by content service");

DECLARE_string(service_name);
DECLARE_int32(port);
DECLARE_string(host);
DECLARE_string(qps);
DECLARE_string(qps_model);
DECLARE_string(trans_model);
DECLARE_int32(http_cpu_thread_num);
DECLARE_int64(tf_thread_num);
DECLARE_int64(feature_extract_tasks_num);
DECLARE_int64(heavy_tasks_thread_num);
DECLARE_int64(request_cpu_thread_num);
DECLARE_string(work_mode);

namespace predictor {

namespace {

constexpr char LOG_CATEGORY[] = "http_service.cc";
constexpr unsigned DEFAULT_SERVER_WEIGHT = 16;

}  // namespace

constexpr char FILE_PARAM[] = "file";

HttpService::HttpService(std::shared_ptr<ModelManager> model_manager)
    : model_manager_(model_manager) {
  auto configs_locked = configs_.wlock();
  *configs_locked = folly::dynamic::object;
}

bool HttpService::start(const std::string& host, uint16_t port) {
  if (!model_manager_) {
    util::logAndMetricError("model_manager_not_initialized");
    return false;
  }

  // init http cpu thread pool executors
  const auto core_num = ResourceMgr::core_num_;
  if (FLAGS_http_cpu_thread_num == 0) {
    FLAGS_http_cpu_thread_num = core_num;
  }
  http_cpu_thread_pool_ = std::make_shared<folly::CPUThreadPoolExecutor>(
    std::make_pair(FLAGS_http_cpu_thread_num, FLAGS_http_cpu_thread_num),  // (max, min)
    std::make_shared<folly::NamedThreadFactory>("HttpCPUThrdPool"));

  // get default server weight according to number of cpu cores
  default_server_weight_ = core_num;
  if (FLAGS_normalize_server_weight == 1) {
    default_server_weight_ = 5 * sqrt(core_num);
  }

  // register http url routes
  auto server_manager = service_framework::http::HttpServerManager::getInstance();
  
  server_manager->registerLocation(
      "/register_service_name",
      [this](service_framework::http::ServerResponse* response, std::unique_ptr<proxygen::HTTPMessage> request,
             std::unique_ptr<folly::IOBuf>) { this->registerServiceName(response, std::move(request)); });

  server_manager->registerLocation(
      "/unregister_service_name",
      [this](service_framework::http::ServerResponse* response, std::unique_ptr<proxygen::HTTPMessage> request,
             std::unique_ptr<folly::IOBuf>) { this->unregisterServiceName(response, std::move(request)); });

  server_manager->registerLocation(
      "/set_server_static_weight",
      [this](service_framework::http::ServerResponse* response, std::unique_ptr<proxygen::HTTPMessage> request,
             std::unique_ptr<folly::IOBuf>) { this->setServerStaticWeight(response, std::move(request)); });

  server_manager->registerLocation(
      "/load_and_register",
      [this](service_framework::http::ServerResponse* response, std::unique_ptr<proxygen::HTTPMessage> request,
             std::unique_ptr<folly::IOBuf> buf) {
        this->loadAndRegister(response, std::move(request), std::move(buf)); },
      proxygen::HTTPMethod::POST);

  server_manager->registerLocation(
      "/get_service_model_info",
      [this](service_framework::http::ServerResponse* response, std::unique_ptr<proxygen::HTTPMessage> request,
             std::unique_ptr<folly::IOBuf>) { this->getServiceModelInfo(response, std::move(request)); });

  server_manager->registerLocation(
      "/update_downgrade_percent",
      [this](service_framework::http::ServerResponse* response, std::unique_ptr<proxygen::HTTPMessage> request,
             std::unique_ptr<folly::IOBuf> buf) {
        this->updateDowngradePercent(response, std::move(request), std::move(buf)); },
      proxygen::HTTPMethod::POST);

  server_manager->registerLocation(
      "/reset_downgrade_percent",
      [this](service_framework::http::ServerResponse* response, std::unique_ptr<proxygen::HTTPMessage> request,
             std::unique_ptr<folly::IOBuf>) { this->resetDowngradePercent(response, std::move(request)); });

  server_manager->registerLocation(
      "/get_downgrade_percent",
      [this](service_framework::http::ServerResponse* response, std::unique_ptr<proxygen::HTTPMessage> request,
             std::unique_ptr<folly::IOBuf>) { this->getDowngradePercent(response, std::move(request)); });

  server_manager->registerLocation(
      "/set_stress_params", [this](service_framework::http::ServerResponse* response,
                                   std::unique_ptr<proxygen::HTTPMessage> request, std::unique_ptr<folly::IOBuf> buf) {
        this->setStressParams(response, std::move(request), std::move(buf));
      },
      proxygen::HTTPMethod::POST);

    server_manager->registerLocation(
      "/set_work_mode",
      [this](service_framework::http::ServerResponse* response, std::unique_ptr<proxygen::HTTPMessage> request,
             std::unique_ptr<folly::IOBuf>) { this->setWorkMode(response, std::move(request)); });

  server_manager->registerLocation(
      "/update_global_model_service_map",
      [this](service_framework::http::ServerResponse* response, std::unique_ptr<proxygen::HTTPMessage> request,
             std::unique_ptr<folly::IOBuf> buf) {
               this->updateGlobalModelServiceMap(response, std::move(request), std::move(buf)); },
      proxygen::HTTPMethod::POST);

  server_manager->registerLocation(
      "/get_global_model_service_map",
      [this](service_framework::http::ServerResponse* response, std::unique_ptr<proxygen::HTTPMessage> request,
             std::unique_ptr<folly::IOBuf>) { this->getGlobalModelServiceMap(response, std::move(request)); });

  // Start HTTPServer mainloop in a separate thread
  service_router::ServerAddress address;
  address.setHost(host);
  address.setPort(port);
  service_router::ServerOption server_option;
  server_option.setServiceName(FLAGS_predictor_service_name);
  server_option.setServerAddress(address);
  http_server_thread_ = std::make_shared<std::thread>([this, server_option]() {
    this->http_server_ = service_router::createAndRegisterHttpServer(server_option);
    this->http_server_->start();
  });
  save_config();
  return true;
}

void HttpService::stop() {
  http_server_->stop();
  http_server_thread_->join();
  http_server_.reset();
  unregisterAll();
  if (http_cpu_thread_pool_) {
    http_cpu_thread_pool_->stop();
  }
}

void HttpService::unregisterAll() {
  LOG(INFO) << "unregistering all services";
  google::FlushLogFiles(google::INFO);
  auto map_locked = service_map_.wlock();
  for (auto it = map_locked->begin(); it != map_locked->end();) {
    service_router::unregisterServer(*(it->second));
    it = map_locked->erase(it);
  }
  LOG(INFO) << "done unregistering all services";
  google::FlushLogFiles(google::INFO);
}

HttpService::~HttpService() { stop(); }

void HttpService::load_models(const std::set<ModelRecord> &model_records) {
  for (auto model_record : model_records) {
    {
      auto model_map_locked = model_map_.wlock();
      model_record.state = ModelRecord::State::loading;
      (*model_map_locked)[model_record.name] = model_record;
    }
    // execute model loading async-ly
    http_cpu_thread_pool_->add([this, model_record]() mutable {
      const auto model_package_dir = common::FileUtil::extract_dir(model_record.config_name);
      std::string &&model_config_filename = common::pathJoin(model_package_dir, MODEL_CONFIG);
      ModelConfig model_config;
      util::initFromFile(&model_config, model_config_filename);
      const bool load_success = this->model_manager_->loadModel(model_config,
                                                                model_package_dir,
                                                                model_record.business_line);
      model_record.state = load_success ? ModelRecord::State::loaded : ModelRecord::State::failed;
      if (load_success) {
        model_record.success_time = common::currentTimeInFormat("%Y%m%d_%H%M%S");
        model_record.state = ModelRecord::State::loaded;
        LOG(INFO) << "load model=" << model_record.full_name << " succeeded!";
      } else {
        LOG(ERROR) << "load model=" << model_record.full_name << " failed!";
        model_record.state = ModelRecord::State::failed;
        const MetricTagsMap err_tag{{TAG_CATEGORY, model_record.name + "-load_fail"}};
        metrics::Metrics::getInstance()->buildMeter(SERVER_NAME, ERROR, err_tag)->mark();
      }
      // set/update model_map_
      {
        auto model_map_locked = this->model_map_.wlock();
        (*model_map_locked)[model_record.name] = model_record;
      }
      google::FlushLogFiles(google::INFO);
      google::FlushLogFiles(google::ERROR);
    });
  }
}

void HttpService::unload_models(const std::set<std::string> &useless_model_names) {
  auto model_map_locked = model_map_.wlock();
  for (const auto &model_name : useless_model_names) {
    auto iter = model_map_locked->find(model_name);
    if (iter != model_map_locked->end()) {
      model_map_locked->erase(iter);
      LOG(INFO) << "logical unload model=" << model_name << " succ!";
    }
  }
}

void HttpService::register_services(const std::map<std::string, std::set<ModelRecord> > &service_models,
                                    const std::map<std::string, int> &service_weight) {
  // async-ly wait and register each service name
  for (const auto &kv : service_models) {
    const auto &service_name = kv.first;
    int server_weight = default_server_weight_;
    // normally there must be a weight config in service_weight map
    auto iter = service_weight.find(service_name);
    if (iter != service_weight.end()) {
      server_weight = iter->second;
    }
    http_cpu_thread_pool_->add([this, kv, server_weight]() {
      const auto &service_name = kv.first;
      const auto &model_records = kv.second;
      // wait for all models associated with this service name to be loaded, then register the service name
      while (true) {
        bool all_models_loaded = true;
        {
          auto model_map_locked = this->model_map_.rlock();
          for (const auto &model_record : model_records) {
            auto iter = model_map_locked->find(model_record.name);
            if (iter == model_map_locked->end()                      ||    // have not been inserted
                iter->second.state == ModelRecord::State::not_loaded ||    // inserted but have not been loaded
                iter->second.state == ModelRecord::State::loading     ) {  // loading
              // all of above are valid states, break and wait for next round of check
              all_models_loaded = false;
              break;
            } else if (iter->second.state == ModelRecord::State::loaded) {
              // this model has been loaded, continue to check next one
              continue;
            } else if (iter->second.state == ModelRecord::State::failed) {
              LOG(ERROR) << "model=" << model_record.full_name
                         << " failed to load, not registering service=" << service_name;
              return;
            } else {
              LOG(ERROR) << "model=" << model_record.full_name
                         << " has unknown state, not registering service=" << service_name;
              return;
            }
          }
        }
        if (all_models_loaded) {
          break;
        } else {
          std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }
      }

      // check if need register service name and server weight inside the function
      {
        register_service_name(service_name, server_weight);
      }
      // at this point it means all models associated with this service name are loaded
      // and it is safe to store/update service->models relationship
      // note: in service_models_map_ we only record the model name, the real model record
      // should be retrieved from models_map_ when queried (by http GET etc.)
      {
        auto map_locked = service_models_map_.wlock();
        (*map_locked)[service_name].clear();
        for (const auto &model_record : model_records) {
          (*map_locked)[service_name].emplace(model_record.name);
        }
      }
      // set model_ptr business_line
      for (const auto &model_record : model_records) {
        if (!model_manager_->setModelBusinessLine(model_record.name, service_name)) {
          LOG(ERROR) << "failed to set model=" << model_record.name << " to business_line=" << service_name;
        }
      }
    });
  }  // end for
}

void HttpService::register_service_name(const std::string &service_name, int server_weight) {
  if (service_name.empty()) {
    LOG(INFO) << "service name is empty! not going to register.";
    return;
  }
  bool already_registered = false;
  {
    // if service name not registered then register the service name
    auto map_locked = service_map_.wlock();
    if (map_locked->find(service_name) != map_locked->end()) {
      already_registered = true;
    } else {
      // mark in map so other threads know we are working on it
      (*map_locked)[service_name] = std::make_shared<service_router::Server>();

      // register server
      auto server = (*map_locked)[service_name];
      server->setServiceName(service_name);
      server->setHost(FLAGS_host);
      server->setPort(FLAGS_port);
      server->setProtocol(service_router::ServerProtocol::THRIFT);
      server->setStatus(service_router::ServerStatus::AVAILABLE);
      LOG(INFO) << "before registerServer service_name=" << service_name;
      google::FlushLogFiles(google::INFO);
      service_router::registerServer(*server, 0);
      LOG(INFO) << "after registerServer service_name=" << service_name;
      google::FlushLogFiles(google::INFO);
    }
  }
  if (!already_registered) {
    auto service_models_map_locked = service_models_map_.wlock();
    if (service_models_map_locked->find(service_name) == service_models_map_locked->end()) {
      (*service_models_map_locked)[service_name] = std::set<std::string> {};
    }
  }
  // read service weight dynamically from mysql config, accepting from content service request
  // if mysql config is zero(DEFAULT), we set the weight based on the cpu core number.
  if (server_weight <= 0) {
    server_weight = default_server_weight_;
  }
  auto service_weight_map_locked = service_weight_map_.wlock();
  auto iter = service_weight_map_locked->find(service_name);
  if (iter == service_weight_map_locked->end() || iter->second != server_weight) {
    // if server weight not set or server weight changed then set server weight
    if (!util::setServerStaticWeight(service_name, FLAGS_host,
                                                FLAGS_port, server_weight)) {
      LOG(ERROR) << "failed to set server weight for service_name=" << service_name;
    } else {
      (*service_weight_map_locked)[service_name] = server_weight;
      LOG(INFO) << "successfuly registered service_name=" << service_name << ", server_weight=" << server_weight;
    }
  }
  google::FlushLogFiles(google::INFO);
  google::FlushLogFiles(google::ERROR);
}

void HttpService::unregister_services_and_unload_models(const std::set<std::string> &useless_service_names,
  const std::set<std::string> &useless_model_names) {
  http_cpu_thread_pool_->add([this, useless_service_names, useless_model_names]()  {
    unregister_services(useless_service_names);
    unload_models(useless_model_names);
  });
}

void HttpService::unregister_services(const std::set<std::string> &useless_service_names) {
  for (const auto &service_name : useless_service_names) {
    unregister_service_name(service_name);
  }
}

void HttpService::unregister_service_name(const std::string &service_name) {
  if (service_name.empty()) {
    LOG(INFO) << "service name is empty! not going to unregister.";
    return;
  }
  {
    auto map_locked = service_map_.wlock();
    auto iter = map_locked->find(service_name);
    if (iter != map_locked->end()) {
      LOG(INFO) << "before unregisterServer service_name=" << service_name;
      google::FlushLogFiles(google::INFO);
      service_router::unregisterServer(*iter->second);
      LOG(INFO) << "unregistered service name : " << service_name;
      google::FlushLogFiles(google::INFO);
      map_locked->erase(iter);
    } else {
      LOG(ERROR) << "trying to unregister a service name that does not exist in service_map_, service name : "
                 << service_name;
    }
  }
  {
    auto service_weight_map_locked = service_weight_map_.wlock();
    auto iter = service_weight_map_locked->find(service_name);
    if (iter != service_weight_map_locked->end()) {
      service_weight_map_locked->erase(iter);
    }
  }
  {
    auto service_models_map_locked = service_models_map_.wlock();
    auto iter = service_models_map_locked->find(service_name);
    if (iter !=  service_models_map_locked->end()) {
      service_models_map_locked->erase(iter);
    }
  }
}

void HttpService::setServerStaticWeight(service_framework::http::ServerResponse* response,
                                     std::unique_ptr<proxygen::HTTPMessage> msg) {
  const std::string& weight_str = msg->getQueryParam("weight");
  std::string service_name = msg->getQueryParam("service_name");
  if (service_name.empty()) {
    service_name = FLAGS_service_name;
  }
  LOG(INFO) << "service_name is " << service_name;
  try {
    int weight = folly::to<int>(weight_str);
    if (util::setServerStaticWeight(service_name, FLAGS_host, FLAGS_port, weight)) {
      LOG(INFO) << "server weight has been changed to " << weight;
    } else {
      LOG(ERROR) << "failed to change server weight!";
    }
  }
  catch (...) {
    LOG(ERROR) << "invalid weight=" << weight_str;
  }
}

void HttpService::registerServiceName(service_framework::http::ServerResponse* response,
                                      std::unique_ptr<proxygen::HTTPMessage> msg) {
  folly::dynamic result = folly::dynamic::object;
  std::string response_result;
  const std::string& service_name_list = msg->getQueryParam("service_name_list");
  if (service_name_list.length() == 0) {
    LOG(ERROR) << "service name list is empty";
    result["code"] = -1;
    result["msg"] = "no param of service_name_list";
    common::toJson(&response_result, result);
    response->status(200, "OK").body(response_result).send();
    return;
  }
  LOG(INFO) << "registering service name list :" << service_name_list;
  std::vector<std::string> service_name_vec;
  folly::split(",", service_name_list.c_str(), service_name_vec);
  for (const auto &service_name : service_name_vec) {
    // zero means using default weight
    register_service_name(service_name, 0);
  }

  result["code"] = 0;
  result["msg"] = "register service_name_list success";
  common::toJson(&response_result, result);
  response->status(200, "OK").body(response_result).send();
  return;
}

void HttpService::unregisterServiceName(service_framework::http::ServerResponse* response,
                                        std::unique_ptr<proxygen::HTTPMessage> msg) {
  folly::dynamic result = folly::dynamic::object;
  std::string response_result;
  const std::string& service_name_list = msg->getQueryParam("service_name_list");
  if (service_name_list.length() == 0) {
    LOG(ERROR) << "service name list is empty";
    result["code"] = -1;
    result["msg"] = "no param of service_name_list";
    common::toJson(&response_result, result);
    response->status(200, "OK").body(response_result).send();
    return;
  }
  LOG(INFO) << "unregistering service name list :" << service_name_list;
  std::vector<std::string> service_name_vec;
  folly::split(",", service_name_list.c_str(), service_name_vec);
  for (auto service_name : service_name_vec) {
    if (service_name.empty()) {
      LOG(ERROR) << "service name is empty!";
      continue;
    }
    unregister_service_name(service_name);
  }

  result["code"] = 0;
  result["msg"] = "unregister service_name_list success";
  common::toJson(&response_result, result);
  response->status(200, "OK").body(response_result).send();
  return;
}

void HttpService::loadAndRegister(service_framework::http::ServerResponse* response,
                                  std::unique_ptr<proxygen::HTTPMessage> msg,
                                  std::unique_ptr<folly::IOBuf> buf) {
  folly::dynamic result = folly::dynamic::object;
  std::string response_result;

  std::map<std::string, std::set<ModelRecord> > service_models;
  std::map<std::string, int > service_weight;
  // used by register_service()
  std::set<std::string> req_model_names;
  // used by register_service()
  std::set<ModelRecord> comp_models;
  std::set<std::string> useless_model_names;
  std::set<std::string> useless_service_names;
  folly::dynamic config;
  // a unioned set of complement models to be loaded, used by load_models()
  try {
    folly::dynamic json = folly::parseJson(util::parseIOBuf(std::move(buf)));
    auto model_map_locked = model_map_.rlock();
    for (const auto &service : json["services"]) {
      auto service_name = service["service_name"].asString();
      auto weight = service["service_weight"].asInt();
      service_weight[service_name] = weight;
      if (service_models.find(service_name) == service_models.end()) {
        service_models[service_name] = {};  // init entry
      }
      for (const auto &model : service["model_records"]) {
        ModelRecord model_rec(model["name"].asString(), model["timestamp"].asString(),
                              model["fullName"].asString(), model["configName"].asString(),
                              model["md5"].asString(), service_name);
        // fill service_models map
        service_models[service_name].insert(model_rec);
        // fill comp_models
        if (FLAGS_always_trust_content_service) {
          // always trust what content service posted to algo_service
          auto iter = model_map_locked->find(model_rec.name);
          if (iter == model_map_locked->end()                  ||  // a brand new model
              iter->second.state == ModelRecord::State::failed ||  // an existing model but previously failed to load
              model_rec.timestamp != iter->second.timestamp) {     // an existing model with a different timestamp
            comp_models.emplace(std::move(model_rec));
          }
        } else {
          // old logic
          auto iter = model_map_locked->find(model_rec.name);
          if (iter == model_map_locked->end()                  ||  // a brand new model
              iter->second.state == ModelRecord::State::failed ||  // an existing model but previously failed to load
              model_rec.timestamp > iter->second.timestamp     ||  // an existing model with a newer timestamp
              (model["is_locked"].asInt() == 1 && model_rec.timestamp < iter->second.timestamp) ) {
            // an existing model with an older timestamp but locked
            comp_models.emplace(std::move(model_rec));
          }
        }
        // fill req_model_names
        req_model_names.insert(model["name"].asString());
      }
      if (config.empty() && service.find("service_config") != service.items().end()) {
        config = service["service_config"];
      }
    }
    for (const auto &kv : *model_map_locked) {
      const auto &model_name = kv.first;
      auto iter = req_model_names.find(model_name);
      if (iter == req_model_names.end()) {
        useless_model_names.insert(model_name);
      }
    }
    auto service_map_locked = service_map_.rlock();
    for (const auto &kv  : *service_map_locked) {
      const auto &service_name = kv.first;
      if (service_models.find(service_name) == service_models.end()) {
        useless_service_names.insert(service_name);
      }
    }
    if (!service_map_locked->empty() && useless_service_names.empty()) {
      config = nullptr;
    }
  } catch(const std::exception &e) {
    LOG(ERROR) << "failed to parse /load_and_register params, check with client who send this request: " << e.what();
    result["code"] = -1;
    result["msg"] = "loadAndRegister: invalid POST params (probably invalid json)";
    common::toJson(&response_result, result);
    response->status(200, "OK").body(response_result).send();
    const MetricTagsMap err_tag{{TAG_CATEGORY, "load_and_register-invalid_json"}};
    metrics::Metrics::getInstance()->buildMeter(SERVER_NAME, ERROR, err_tag)->mark();
    return;
  } catch(...) {
    LOG(ERROR) << "failed to parse /load_and_register params, check with client who send this request: UNKNOW ERROR";
    result["code"] = -1;
    result["msg"] = "loadAndRegister: invalid POST params (UNKNOW ERROR)";
    common::toJson(&response_result, result);
    response->status(200, "OK").body(response_result).send();
    const MetricTagsMap err_tag{{TAG_CATEGORY, "load_and_register-unknow_error"}};
    metrics::Metrics::getInstance()->buildMeter(SERVER_NAME, ERROR, err_tag)->mark();
    return;
  }

  if (FLAGS_enable_service_config) {
    reset_service_config(config);
  }

  // async fire off jobs, so we can send back http response immediately
  {
    unregister_services_and_unload_models(useless_service_names, useless_model_names);
    load_models(comp_models);
    register_services(service_models, service_weight);
  }

  // respond immediately
  result["code"] = 0;
  result["msg"] = "loadAndRegister success";
  common::toJson(&response_result, result);
  response->status(200, "OK").body(response_result).send();
}

void HttpService::getServiceModelInfo(service_framework::http::ServerResponse* response,
                                      std::unique_ptr<proxygen::HTTPMessage> msg) {
  folly::dynamic result = folly::dynamic::object;
  std::string response_result;
  folly::dynamic info_obj = folly::dynamic::object;
  try {
    info_obj["services"] = folly::dynamic::array;
    {
      auto service_models_map_locked = service_models_map_.rlock();
      auto service_weight_map_locked = service_weight_map_.rlock();
      auto model_map_locked = model_map_.rlock();
      for (const auto &kv : *service_models_map_locked) {
        const auto &service_name = kv.first;
        const auto &model_names = kv.second;
        folly::dynamic service_obj = folly::dynamic::object;
        service_obj["service_name"] = service_name;
        auto it = service_weight_map_locked->find(service_name);
        if (it != service_weight_map_locked->end()) {
          service_obj["service_weight"] = it->second;
        } else {
          service_obj["service_weight"] = -1;
        }
        service_obj["model_records"] = folly::dynamic::array;
        for (const auto &model_name : model_names) {
          auto iter = model_map_locked->find(model_name);
          if (iter != model_map_locked->end()) {
            service_obj["model_records"].push_back(iter->second.toDynamic());
          }
        }
        info_obj["services"].push_back(service_obj);
      }

      auto configs_locked = configs_.rlock();
      info_obj["config"] = *configs_locked;
    }
  } catch(...) {
    LOG(ERROR) << "failed to build /get_service_model_info response!";
    result["code"] = -1;
    result["msg"] = "getServiceModelInfo: internal error";
    common::toJson(&response_result, result);
    response->status(200, "OK").body(response_result).send();
    return;
  }

  result["code"] = 0;
  result["msg"] = info_obj;
  common::toJson(&response_result, result);
  response->status(200, "OK").body(response_result).send();
}

void HttpService::updateDowngradePercent(service_framework::http::ServerResponse* response,
                                     std::unique_ptr<proxygen::HTTPMessage> msg,
                                     std::unique_ptr<folly::IOBuf> buf) {
  folly::dynamic result = folly::dynamic::object;
  std::string response_result;
  /* json example
  {
    "model_1" : 20,
    "model_2" : 10
  }
  */
  try {
    folly::dynamic json = folly::parseJson(util::parseIOBuf(std::move(buf)));
    LOG(INFO) << "updateDowngradePercent request json=" << json;
    auto tmp_map_ptr = std::make_shared<std::map<std::string, int>>();
    for (const auto& item : json.items()) {
      int itemValue = item.second.asInt();
      if (itemValue < 0 || itemValue > 100) {
        throw std::runtime_error("downgrade percent is not allowed to be less than 0 or greater than 100");
      }
      (*tmp_map_ptr)[item.first.asString()] = itemValue;
    }
    model_manager_->set_model_downgrade_percent_map(tmp_map_ptr);
    LOG(INFO) << "updateDowngradePercent has been changed to " << json << ",tmp_map_ptr.size =" << tmp_map_ptr->size();
  }
  catch (const std::exception &e) {
    LOG(ERROR) << "invalid downgrade_percent, errmsg="<< e.what();
    result["code"] = -1;
    result["msg"] = "updateDowngradePercent: invalid POST params (probably invalid json)";
    common::toJson(&response_result, result);
    response->status(200, "OK").body(response_result).send();
    return;
  }
  LOG(INFO) << "update downgrade_percent success";
  result["code"] = 0;
  result["msg"] = "update downgrade_percent success";
  common::toJson(&response_result, result);
  response->status(200, "OK").body(response_result).send();
  return;
}

void HttpService::resetDowngradePercent(service_framework::http::ServerResponse* response,
                                     std::unique_ptr<proxygen::HTTPMessage> msg) {
  folly::dynamic result = folly::dynamic::object;
  std::string response_result;
  auto model_downgrade_percent_map_ptr = model_manager_->get_model_downgrade_percent_map();
  if (model_downgrade_percent_map_ptr) {
    model_manager_->set_model_downgrade_percent_map(nullptr);
  }

  LOG(INFO) << "reset downgrade_percent success";
  result["code"] = 0;
  result["msg"] = "reset downgrade_percent success";
  common::toJson(&response_result, result);
  response->status(200, "OK").body(response_result).send();
  return;
}

void HttpService::getDowngradePercent(service_framework::http::ServerResponse* response,
                                      std::unique_ptr<proxygen::HTTPMessage> msg) {
  folly::dynamic result = folly::dynamic::object;
  std::string response_result;
  folly::dynamic downgrade_obj = folly::dynamic::object;
  try {
    auto model_downgrade_percent_map_ptr = model_manager_->get_model_downgrade_percent_map();
    if (model_downgrade_percent_map_ptr) {
      for (const auto &kv : *model_downgrade_percent_map_ptr) {
        downgrade_obj[kv.first] = kv.second;
      }
    }
  } catch(const std::exception &e) {
    LOG(ERROR) << "failed to build /get_downgrade_percent response! errmsg=" << e.what();
    result["code"] = -1;
    result["msg"] = "getDowngradePercent: internal error";
    common::toJson(&response_result, result);
    response->status(200, "OK").body(response_result).send();
    return;
  }

  result["code"] = 0;
  result["msg"] = downgrade_obj;
  common::toJson(&response_result, result);
  response->status(200, "OK").body(response_result).send();
}

void HttpService::setStressParams(service_framework::http::ServerResponse* response,
                                  std::unique_ptr<proxygen::HTTPMessage> msg,
                                  std::unique_ptr<folly::IOBuf> buf) {
  folly::dynamic result = folly::dynamic::object;
  std::string response_result;
  std::string model_names{""};
  std::string qps{""};
  std::string service{""};
  try {
    folly::dynamic json = folly::parseJson(util::parseIOBuf(std::move(buf)));
    model_names = json["model_names"].asString();
    qps = json["qps"].asString();
    service = json["service"].asString();
  } catch (const std::exception& e) {
    LOG(ERROR) << "failed to parse /setStressParams params, check with client who send this request: " << e.what();
    result["code"] = -1;
    result["msg"] = "setStressParams: invalid POST params (probably invalid json)";
    common::toJson(&response_result, result);
    response->status(200, "OK").body(response_result).send();
    const MetricTagsMap tags_map{{TAG_CATEGORY, "setStressParams-invalid_json"}};
    auto err_meter = metrics::Metrics::getInstance()->buildMeter(SERVER_NAME, ERROR, tags_map);
    err_meter->mark();
    return;
  } catch (...) {
    LOG(ERROR) << "failed to parse /setStressParams params, check with client who send this request: UNKNOW ERROR";
    result["code"] = -1;
    result["msg"] = "setStressParams: invalid POST params (UNKNOW ERROR)";
    common::toJson(&response_result, result);
    response->status(200, "OK").body(response_result).send();
    const MetricTagsMap tags_map{{TAG_CATEGORY, "setStressParams-unknow_error"}};
    auto err_meter = metrics::Metrics::getInstance()->buildMeter(SERVER_NAME, ERROR, tags_map);
    err_meter->mark();
    return;
  }
  http_cpu_thread_pool_->add([this, model_names, qps, service]() { setStressInfos(model_names, qps, service); });

  result["code"] = 0;
  result["msg"] = "setStressParams success";
  common::toJson(&response_result, result);
  response->status(200, "OK").body(response_result).send();
}

void HttpService::setStressInfos(const std::string& models, const std::string& qps, const std::string& service) {
  while (true) {
    auto map_locked = service_map_.rlock();
    if (map_locked->find(service) != map_locked->end()) {
      break;
    } else {
      std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
  }

  auto map_locked = service_map_.wlock();
  auto server = (*map_locked)[service];
  auto router = service_router::Router::getInstance();
  // other settings
  router->setOtherSettings(*server, "qps", qps);
  router->setOtherSettings(*server, "qps_model", models);
  router->setOtherSettings(*server, "trans_model", FLAGS_trans_model);
}

void HttpService::reset_service_config(const folly::dynamic &config_object) {
  if (!config_object.isObject()) {
    return;
  }
  unsigned core_num = std::thread::hardware_concurrency();
  if (core_num <= 0) {
    LOG(ERROR) << "failed to get core_num";
    return;
  }

  auto configs_locked = configs_.wlock();
  auto set_cfg_value = [&] (std::string ratio_name, std::string cfg_name, int64_t &cfg_value) mutable {
    auto pos = config_object.find(ratio_name);
    if (pos == config_object.items().end() || !pos->second.isNumber()) {
      return;
    }

    int value = core_num * pos->second.asDouble();
    if (value >= 0 && value <= core_num * 2) {
      (*configs_locked)[ratio_name] = pos->second;
      (*configs_locked)[cfg_name] = value;
      cfg_value = value;
      if (cfg_name == HEAVY_TASKS_THREAD_NUM) {
        ResourceMgr::setHeavyTaskThreadPool(value);
      }
      if (cfg_name == REQUEST_CPU_THREAD_NUM) {
        model_manager_->setThreadPool(value);
      }
    }
  };

  set_cfg_value(FEATURE_EXTRACT_TASKS_RATIO, FEATURE_EXTRACT_TASKS_NUM, FLAGS_feature_extract_tasks_num);
  set_cfg_value(TF_THREAD_RATIO, TF_THREAD_NUM, FLAGS_tf_thread_num);
  set_cfg_value(HEAVY_TASKS_THREAD_RATIO, HEAVY_TASKS_THREAD_NUM, FLAGS_heavy_tasks_thread_num);
  set_cfg_value(REQUEST_CPU_THREAD_RATIO, REQUEST_CPU_THREAD_NUM, FLAGS_request_cpu_thread_num);

  LOG(INFO) << "reset FLAGS_feature_extract_tasks_num=" << FLAGS_feature_extract_tasks_num
            << " FLAGS_tf_thread_num=" << FLAGS_tf_thread_num
            << " FLAGS_heavy_tasks_thread_num=" << FLAGS_heavy_tasks_thread_num
            << " FLAGS_request_cpu_thread_num=" << FLAGS_request_cpu_thread_num;
}

void HttpService::save_config() {
  auto configs_locked = configs_.wlock();
  (*configs_locked)[TF_THREAD_NUM] = FLAGS_tf_thread_num;
  (*configs_locked)[FEATURE_EXTRACT_TASKS_NUM] = FLAGS_feature_extract_tasks_num;
  (*configs_locked)[HEAVY_TASKS_THREAD_NUM] = FLAGS_heavy_tasks_thread_num;
  (*configs_locked)[REQUEST_CPU_THREAD_NUM] = FLAGS_request_cpu_thread_num;
}

void HttpService::setWorkMode(service_framework::http::ServerResponse* response,
                              std::unique_ptr<proxygen::HTTPMessage> msg) {
  folly::dynamic result = folly::dynamic::object;
  std::string response_result;
  const std::string& work_mode = msg->getQueryParam("work_mode");
  if (work_mode.length() == 0) {
    LOG(ERROR) << "service name list is empty";
    result["code"] = -1;
    result["msg"] = "no param of work_mode";
  }
  if (work_mode == FLAGS_work_mode) {
    FB_LOG_EVERY_MS(INFO, 60000) << "work_mode maintained " + FLAGS_work_mode;
    result["code"] = 0;
    result["msg"] = "work_mode maintained " + FLAGS_work_mode;
  } else {
    unregisterAll();
    FLAGS_work_mode = work_mode;
    if (work_mode == ROUTER_MODE) {
      static folly::once_flag predictor_router_init_flag;
      folly::call_once(predictor_router_init_flag, PredictorRouter::init);
    }
    LOG(INFO) << "work_mode has been changed to:" << work_mode;
    result["code"] = 0;
    result["msg"] = "work_mode change success";
  }
  common::toJson(&response_result, result);
  response->status(200, "OK").body(response_result).send();
  return;
}

void HttpService::updateGlobalModelServiceMap(service_framework::http::ServerResponse* response,
                              std::unique_ptr<proxygen::HTTPMessage> msg,
                              std::unique_ptr<folly::IOBuf> buf) {
  folly::dynamic result = folly::dynamic::object;
  std::string response_result;
  /* json example
  {
    "model_1" : "app_service",
    "model_2" : "cpl_service"
  }
  */
  try {
    folly::dynamic json = folly::parseJson(util::parseIOBuf(std::move(buf)));
    LOG(INFO) << "updateGlobalModelServiceMap request json=" << json;
    auto global_model_service_map = std::make_shared<ModelServiceMap>();
    for (const auto& item : json.items()) {
      (*global_model_service_map)[item.first.asString()] = item.second.asString();
    }
    ResourceMgr::global_model_service_map_.store(global_model_service_map);
    LOG(INFO) << "updateGlobalModelServiceMap has been changed to " << json
              << ",global_model_service_map.size =" << global_model_service_map->size();
    result["code"] = 0;
    result["msg"] = "update global_model_service_map success";
  } catch (const std::exception &e) {
    LOG(ERROR) << "invalid global_model_service_map, errmsg="<< e.what();
    result["code"] = -1;
    result["msg"] = "updateGlobalModelServiceMap: invalid POST params (probably invalid json)";
  }
  common::toJson(&response_result, result);
  response->status(200, "OK").body(response_result).send();
  return;
}

void HttpService::getGlobalModelServiceMap(service_framework::http::ServerResponse* response,
                              std::unique_ptr<proxygen::HTTPMessage> msg) {
  folly::dynamic result = folly::dynamic::object;
  std::string response_result;
  folly::dynamic info_obj = folly::dynamic::object;
  try {
    auto global_model_service_map = ResourceMgr::global_model_service_map_.load();
    for (const auto &kv : *global_model_service_map) {
      info_obj[kv.first] = kv.second;
    }
    result["code"] = 0;
    result["msg"] = info_obj;
  } catch(...) {
    LOG(ERROR) << "failed to build /get_global_model_service_map response!";
    result["code"] = -1;
    result["msg"] = "getGlobalModelServiceMap: internal error";
  }
  common::toJson(&response_result, result);
  response->status(200, "OK").body(response_result).send();
}

}  // namespace predictor
