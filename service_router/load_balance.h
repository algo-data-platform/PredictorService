#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "folly/Random.h"
#include "folly/Synchronized.h"
#include "folly/ThreadLocal.h"

#include "common/util.h"
#include "service_router/service_router_entity.h"

// LoadBalance 职责：
//   实现多种负载均衡策略的工厂
//
//   具体的负载均衡策略：
//     a. 随机负载均衡
//     b. 轮询负载均衡
//     c. 本地服务优先
//        本地服务优先策略首先去判断本地ip 和目标ip 是否相等，如果相等则尤其返回本地ip 地址，如果没有则判断本地 ip
//        和目标 ip 的差值范围来决定是否是本地组
//     d. 权重可配置的负载均衡器
//          根据配置的多个 weight 进行请求
//     e. hash 一致性
//          首先 根据请求计算 hash 值，最后挨个 (hash + i) % totalNodes 拿到可用的服务就返回
//     f. 低并发优化
//          根据服务 meta 信息进行策略控制，将并发低的有效使用, 实现该负载均衡器需要配合统一的服务 metrics
//     g. 同ip段服务优先
//        与"c. 本地服务优先"略有不同，此策略不判断本地ip 和目标ip 是否相等，直接判断本地 ip
//        和目标 ip 的差值范围来决定是否是同ip段组
//
namespace service_router {

class Router;
class LoadBalanceInterface {
 public:
  explicit LoadBalanceInterface(Router* router) : router_(router) {}
  virtual bool select(Server* server, const std::vector<Server>& list) = 0;
  // virtual bool select(Server* server, const folly::Synchronized<std::vector<Server>>& list) = 0;
  virtual ~LoadBalanceInterface() {}

 protected:
  Router* router_;
};

class LoadBalanceIdcFirst : public LoadBalanceInterface {
 public:
  explicit LoadBalanceIdcFirst(Router* router, const std::string& idc) : LoadBalanceInterface(router), idc_(idc) {}
  ~LoadBalanceIdcFirst() = default;
  bool select(Server* server, const std::vector<Server>& list) override;

 private:
  std::string idc_;
};

class LoadBalanceRandom : public LoadBalanceInterface {
 public:
  explicit LoadBalanceRandom(Router* router) : LoadBalanceInterface(router) {}
  ~LoadBalanceRandom() = default;
  bool select(Server* server, const std::vector<Server>& list) override;
};

class LoadBalanceRoundrobin : public LoadBalanceInterface {
 public:
  explicit LoadBalanceRoundrobin(Router* router) : LoadBalanceInterface(router) {}
  ~LoadBalanceRoundrobin() = default;
  bool select(Server* server, const std::vector<Server>& list) override;

 private:
  std::atomic<uint64_t> requests_{0};
};

class LoadBalanceLocalFirst : public LoadBalanceInterface {
 public:
  explicit LoadBalanceLocalFirst(Router* router, const BalanceLocalFirstConfig& config)
      : LoadBalanceInterface(router), config_(config) {
    local_ip_ = common::ipToInt(config_.getLocalIp());
  }
  virtual ~LoadBalanceLocalFirst() = default;
  bool select(Server* server, const std::vector<Server>& list) override;

 protected:
  uint32_t local_ip_{0};
  BalanceLocalFirstConfig config_;
};

class LoadBalanceIpRangeFirst : public LoadBalanceLocalFirst {
 public:
  explicit LoadBalanceIpRangeFirst(Router* router, const BalanceLocalFirstConfig& config)
      : LoadBalanceLocalFirst(router, config) {}
  ~LoadBalanceIpRangeFirst() = default;
  bool select(Server* server, const std::vector<Server>& list) override;
};

class LoadBalanceConfigurableWeight : public LoadBalanceInterface {
 public:
  explicit LoadBalanceConfigurableWeight(Router* router) : LoadBalanceInterface(router) {}
  ~LoadBalanceConfigurableWeight() = default;
  bool select(Server* server, const std::vector<Server>& list) override;

 private:
  struct CurrentStateWrapper {
    int32_t current_index = -1;
    int32_t current_weight = 0;
  };
  folly::ThreadLocal<CurrentStateWrapper> current_state_;

  inline uint32_t gcd(uint32_t a, uint32_t b) {
    if ((a % b) == 0) {
      return b;
    } else {
      return gcd(b, a % b);
    }
  }
};

class LoadBalanceStaticWeight : public LoadBalanceInterface {
 public:
  explicit LoadBalanceStaticWeight(Router* router) : LoadBalanceInterface(router) {}
  ~LoadBalanceStaticWeight() = default;
  bool select(Server* server, const std::vector<Server>& list) override;
};

}  // namespace service_router
