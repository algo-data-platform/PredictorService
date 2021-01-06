#pragma once

#include <string>
#include <vector>

#include "folly/dynamic.h"

#include "common/util.h"
#include "service_framework/consul/raw_client.h"
#include "service_framework/consul/params.h"
#include "service_framework/consul/entity.h"

namespace service_framework {
namespace consul {

class AgentClient : protected RawClient {
 public:
  AgentClient(const std::string& host, int port) : RawClient(host, port) {}
  AgentClient(const std::string& host, int port, int connect_timeout, int socket_timeout)
      : RawClient(host, port, connect_timeout, socket_timeout) {}
  ~AgentClient() = default;

  // sync request
  // async request
  folly::Future<RawClient::HttpResponse> getMembers();

  // process responses methods
  folly::Optional<ConsulResponse<std::vector<Member>>> processMembers(const RawClient::HttpResponse& response);
};

}  // namespace consul
}  // namespace service_framework
