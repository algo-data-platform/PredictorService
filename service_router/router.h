#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "folly/Random.h"
#include "folly/SharedMutex.h"
#include "folly/executors/CPUThreadPoolExecutor.h"
#include "folly/io/async/ScopedEventBaseThread.h"

#include "common/metrics/metrics.h"
#include "common/util.h"
#include "service_router/load_balance.h"
#include "service_router/registry.h"
#include "service_router/service_router_entity.h"

//
// 接口说明
//
// 1. 调用通用服务注册函数 registerServer() 或者特化的 thriftServerProductor() 来创建服务
// 2. 在 server handler 处理过程中用到其他服务时，使用 serviceClient() 或者特化的 thriftServiceClient() 来获取
// 服务连接或者 address 进行连接
// 3. 调用 getConfigs() 或者 getConfig() 主动可以获取服务的配置
// 4. 使用 subscribeConfig() 可以订阅配置变更，当某项配置发送变更，服务可以做对应处理
// 5. 服务 stop 时调用 unregisterAll() 主动注销服务
//
// 使用举例
// 比如一个 geo 服务有一个 ipSearch 接口对外提供 ip 查询，另外一个 index 服务 target 接口会依赖 geo 服务的 ipSearch
// 我们假设 geo 服务已经完成，目前开发 index 服务：
//
// // 入口
//
//  int main(int argc, char** argv) {
//    // 初始化 service_router
//    service_router::init();
//
//    // 创建 server 对象
//    service_router::ServerOption option;
//
//    service_router::ServerAddress address;
//    address.setHost(FLAGS_index_host);
//    address.setPort(FLAGS_index_port);
//
//    option.setServerAddress(address);
//    option.setServiceName(FLAGS_index_name);
//    auto server = service_router::thriftServerProductor<IndexServiceHandler>(option);
//
//    // 注册配置监听回调
//    // subscribe 功能是为了解决一些实时跟新配置的需求,在配置发生变更时做对应的操作
//    service_router::subscribe(FLAGS_index_name, "config1", [](const std::string& value) {
//      // process
//    })
//
//    server->serve();
//  }
//
// 对于 target 接口调用 geo 实现如下：
//
//   void IndexServiceHandler::target(TargetResponse& response, std::unique_ptr<TargetRequest> req) {
//     // 根据服务名称获取 客户端对象
//     auto client = thriftServiceClient<GeoAsyncClient>(FLAGS_geo_name);
//     IpSearchRequest ipRequest;
//     auto result = client->ipSearch(ipRequest);
//     // process
//   }
//
namespace service_router {

DECLARE_int32(discover_wait_milliseconds);
DECLARE_int32(idle_timeout);
DECLARE_int32(thrift_timeout_retry);
DECLARE_int32(thrift_connection_retry);

constexpr char DEFAULT_DC[] = "default";

class Router;
template <typename Type>
class ServiceInfoPuller;
class ServiceConfigPull;
class ServiceDiscoverPull;
class ServerWithHeartbeat;

class ServerList {
 public:
  explicit ServerList(const std::vector<Server>& server_list);
  ServerList() = default;
  ~ServerList() = default;
  folly::Optional<Server> selectServers(const std::shared_ptr<LoadBalanceInterface>& load) const;
  const std::vector<Server> getServers();
  inline uint64_t getHashCode() { return hash_code_; }

 private:
  std::vector<Server> servers_;
  uint64_t hash_code_{0};
};

class ServiceRegistry {
 public:
  ServiceRegistry() = default;
  ~ServiceRegistry() = default;
  explicit ServiceRegistry(const std::unordered_map<int64_t, std::shared_ptr<ServerList>>& services);
  folly::Optional<std::shared_ptr<ServerList>> selectServers(int64_t key);
  const std::unordered_map<int64_t, std::shared_ptr<ServerList>> getServices();

 private:
  std::unordered_map<int64_t, std::shared_ptr<ServerList>> services_;
};

// RouterDb 职责：
//
//   1. 服务 -> Server 对象的映射关系
//      提供线程安全的查询、更新策略、服务删除
//
//   2. 服务+分区 -> Server
//      提供线程安全的查询、更新策略、服务删除
//
// thread safe
class RouterDb {
 public:
  RouterDb();
  ~RouterDb() = default;
  folly::Optional<std::shared_ptr<ServerList>> selectServers(const std::string& service_name,
                                                             const ServerProtocol& protocol, int64_t route_id,
                                                             const ShardType& type, const std::string& dc = DEFAULT_DC);
  std::unordered_map<int64_t, std::shared_ptr<ServerList>> selectServers(const std::string& service_name,
                                                                         const ServerProtocol& protocol,
                                                                         const std::vector<int64_t>& route_ids,
                                                                         const ShardType& type,
                                                                         const std::string& dc = DEFAULT_DC);
  void updateServers(const std::string& service_name, const std::vector<Server>& list);

  const std::string getConfig(const std::string& name, const std::string& key);
  const ServiceRouterConfig getRouterConfig(const std::string& service_name);
  const ServiceConfig getConfig(const std::string& service_name);
  void updateConfig(const std::string& service_name, const ServiceConfig& config);

 private:
  std::shared_ptr<ServiceRegistry> serviceRegistry_;
  folly::SharedMutexReadPriority update_server_mutex_;
  folly::Synchronized<std::unordered_map<std::string, ServiceConfig>> configs_;
  const std::vector<Server> pickServers(const std::string& service_name, const std::vector<Server>& list);
  int64_t getServiceKey(const std::string& service_name, const ServerProtocol& protocol, int64_t route_id,
                        const ShardType& type, const std::string& dc = DEFAULT_DC);
  std::shared_ptr<metrics::Timers> timers_;
  folly::Synchronized<std::unordered_map<std::string, std::unordered_set<int64_t>>> last_partition_keys_;
};

class RouterThread : public folly::ScopedEventBaseThread {
 public:
  RouterThread() : folly::ScopedEventBaseThread("serviceRouterTimer") {}
  ~RouterThread() = default;
};

// Router 职责：
//  1. 配置管理
//    a. 首次获取配置
//    b. 创建定时 pull 数据定时器
//    c. 设置各种 subscribe 回调
//  2. 服务注册，注销
//    a. 服务自身心跳维护定时器 2/3 * TTL
//    b. 服务注销
//  3. 服务发现
//    a. 首次获取服务列表并进行 cache 映射处理
//    b. 创建定时 pull 数据定时器
//    c. 设置各种 subscribe 回调
//    d. 服务发现自我保护策略实现（consol 、zk 等数据层服务挂掉时）
//    e. 提供非stateful、shared 分区等类型的数据候选节点查询
class Router {
 public:
  static std::shared_ptr<Router> getInstance();
  Router();
  virtual ~Router() { LOG(ERROR) << "Router deleted"; }

  // 用于创建服务注册的 Registry，注册自身服务
  virtual void createServiceRegistry(const std::string& service_name, RegistryType type, const std::string& address);

  // 手动创建Registry
  virtual void setServiceRegistry(const std::string& service_name, std::shared_ptr<RegistryInterface> registry);

  // 用于创建服务发现的 Registry，发现目标服务
  virtual void createTargetServiceRegistry(const std::string& service_name, RegistryType type,
                                           const std::string& address);

  virtual void waitForConfig(const std::string& service_name, bool is_once = true);

  virtual void waitForDiscover(const std::string& service_name, bool is_once = true);

  virtual void waitUntilConfig(const std::string& service_name, int milliseconds, bool is_once = true);

  virtual void waitUntilDiscover(const std::string& service_name, int milliseconds, bool is_once = true);

  virtual folly::Optional<Server> discover(const ClientOption& option);
  virtual folly::Optional<Server> edgeDiscover(const ClientOption& option);
  virtual std::unordered_map<int64_t, folly::Optional<Server>> batchDiscover(const ClientOption& option,
                                                                             const std::vector<int64_t>& route_ids);

  virtual folly::Optional<std::shared_ptr<ServerList>> getServerList(const std::string& service_name,
                                                                     const ServerProtocol& protocol, int64_t route_id,
                                                                     const ShardType& type,
                                                                     const std::string& dc = DEFAULT_DC);
  virtual std::unordered_map<int64_t, std::shared_ptr<ServerList>> getServerList(const std::string& service_name,
                                                                                 const ServerProtocol& protocol,
                                                                                 const std::vector<int64_t>& route_ids,
                                                                                 const ShardType& type,
                                                                                 const std::string& dc = DEFAULT_DC);

  virtual bool registerServerSync(const Server server);

  virtual void registerServer(const Server server);

  virtual void setStatus(const Server& server, const ServerStatus& status);

  virtual folly::Optional<ServerStatus> getStatus(const Server& server);

  virtual void setWeight(const Server& server, int weight);

  virtual void setIdc(const Server& server, const std::string& idc);

  virtual void setDc(const Server& server, const std::string& dc);

  virtual void setShardList(const Server& server, const std::vector<uint32_t>& shard_list);

  virtual void setAvailableShardList(const Server& server, const std::vector<uint32_t>& shard_list);

  virtual void setFollowerShardList(const Server& server, const std::vector<uint32_t>& shard_list);

  virtual void setFollowerAvailableShardList(const Server& server, const std::vector<uint32_t>& shard_list);

  virtual void setOtherSettings(const Server& server,
                                const std::unordered_map<std::string, std::string>& other_settings);

  virtual void setOtherSettings(const Server& server, const std::string& key, const std::string& value);

  virtual void setStatus(const std::string& service_name, const ServerStatus& status);

  virtual folly::Optional<ServerStatus> getStatus(const std::string& service_name);

  virtual void setIsEdgeNode(const Server& server, bool is_edge_node);

  virtual void setPartitionList(const Server& server, const std::vector<int64_t> partition_list);

  virtual bool unregisterServerSync(const Server server);

  virtual void unregisterServer(const Server server);

  virtual void unregisterAll();

  virtual void subscribeConfig(const std::string& service_name, NotifyConfig notify);
  virtual void subscribeConfigItem(const std::string& service_name, const std::string& key, NotifyConfigItem notify);

  virtual void unsubscribeConfig(const std::string& service_name);
  virtual void unsubscribeConfigItem(const std::string& service_name, const std::string& key);

  virtual const ServiceRouterConfig getRouterConfig(const std::string& service_name);

  virtual const std::unordered_map<std::string, std::string> getConfigs(const std::string& service_name);

  virtual void stop();

  // registry name 实际上就是依赖的service name
  void getAllRegistryName(std::vector<std::string>* registry_names);
  std::shared_ptr<RegistryInterface> getRegistry(const std::string& service_name);
  std::shared_ptr<RegistryInterface> getOrCreateRegistry(const std::string& service_name);

 private:
  folly::EventBase* evb_;
  folly::Synchronized<std::unordered_map<std::string, std::shared_ptr<RegistryInterface>>> registries_;
  std::unique_ptr<RouterDb> db_;
  std::unique_ptr<RouterThread> router_thread_;
  std::shared_ptr<folly::CPUThreadPoolExecutor> callback_thread_pool_;
  // 注册的所有的 server
  folly::Synchronized<std::unordered_map<std::string, std::shared_ptr<ServerWithHeartbeat>>> servers_;
  using ServiceConfigInfoPullerPtr = std::shared_ptr<ServiceInfoPuller<ServiceConfigPull>>;
  using ServiceDiscoverPullerPtr = std::shared_ptr<ServiceInfoPuller<ServiceDiscoverPull>>;
  folly::Synchronized<std::unordered_map<std::string, ServiceConfigInfoPullerPtr>> config_pullers_;
  folly::Synchronized<std::unordered_map<std::string, ServiceDiscoverPullerPtr>> discover_pullers_;
  folly::Synchronized<std::unordered_map<uint64_t, std::shared_ptr<LoadBalanceInterface>>> balancers_;

  const std::string getServerKey(const Server& server);
  std::shared_ptr<ServiceInfoPuller<ServiceConfigPull>> getOrCreateConfigPuller(const std::string& service_name);
  std::shared_ptr<ServiceInfoPuller<ServiceDiscoverPull>> getOrCreateDiscoverPuller(const std::string& service_name);
  std::shared_ptr<LoadBalanceInterface> getOrCreateBalance(const ClientOption& option);
  std::shared_ptr<RegistryInterface> createRegistry(const std::string& service_name, RegistryType type,
                                                    const std::string& address);
};

// ServiceRouter 支持实例中的每个 service_name 对应一个 registry 实例，每个 registry 实例可以连接
// 不同的 consul 或者 agent 集群（目前只支持 consul，后续会加入对 agent 的支持），使用时在 main 函数
// 入口处为每个服务创建一个 registry 即可。
//
// *** Examples：
// 例1. TestServer 需要创建两个服务，分别是 test_service_dev 和 test_service_dev_replication，
// 可以在 main 函数入口增加如下代码：
//
// service_router::createServiceRegistry("test_service_dev", service_router::RegistryType::CONSUL, "ip:port");
// service_router::createServiceRegistry("test_service_dev_replication", service_router::RegistryType::CONSUL,
//                                       "ip:port");
//
//
// 例2. TestKeyTest 作为 TestServer 的客户端，需要创建两个 registry，一个连接到 TestServer 进行服务发现，一个作为
// 自身服务注册使用，例子中两个 registry 连接到不同的 consul 地址：
//
// 创建 registry 用于 TestServer 服务发现
// service_router::createTargetServiceRegistry("test_service_dev", service_router::RegistryType::CONSUL, "ip:port");
// 创建 registry 自身服务注册
// service_router::createServiceRegistry("test_service_client", service_router::RegistryType::CONSUL, "ip.200:port");
//
// *** 兼容性：
// 为了兼容原有的使用 service_router 的服务，在服务没有调用下面两个接口显式创建 registry 时，内部会为每个 service 调用
// getOrCreateRegistry(const std::string& service_name) 方法，以原有的 router_consul_addresses gflags 作为默认地址，
// 创建连接 consul 的 registry。
//
// 用于创建服务注册的 Registry，注册自身服务
void createServiceRegistry(const std::string& service_name, RegistryType type, const std::string& address);

// 用于创建服务发现的 Registry，发现目标服务
void createTargetServiceRegistry(const std::string& service_name, RegistryType type, const std::string& address);

// 通用 服务注册
// 默认会持续阻塞，知道配置获取到才会注册服务
void registerServer(const Server& server, int wait_milliseconds = 0);
bool registerServerSync(const Server& server, int wait_milliseconds = 0);

void unregisterServer(const Server& server);
bool unregisterServerSync(const Server& server);
void unregisterAll();
void stop();

// 订阅配置变更
void subscribeConfig(const std::string& service_name, NotifyConfig notify);
void subscribeConfigItem(const std::string& service_name, const std::string& key, NotifyConfigItem notify);
void unsubscribeConfig(const std::string& service_name);
void unsubscribeConfigItem(const std::string& service_name, const std::string& key);

const std::unordered_map<std::string, std::string> getConfigs(const std::string& service_name,
                                                              int wait_milliseconds = 0);
// 获取服务
// 默认会阻塞直到获取到服务列表
bool serviceClient(ServerAddress* address, const ClientOption& option, int wait_milliseconds = 0);

}  // namespace service_router
