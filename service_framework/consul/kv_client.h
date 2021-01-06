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

class KVClient : protected RawClient {
 public:
  KVClient(const std::string& host, int port) : RawClient(host, port) {}
  KVClient(const std::string& host, int port, int connect_timeout, int socket_timeout)
      : RawClient(host, port, connect_timeout, socket_timeout) {}
  ~KVClient() = default;

  static const QueryParams defaultQueryParams() {
    QueryParams query_params;
    return query_params;
  }

  static const PutParams defaultPutParams() {
    PutParams put_params;
    return put_params;
  }

  // sync request
  folly::Optional<ConsulResponse<bool>> setValueSync(const std::string& key, const std::string& value,
                                                     const std::string& token = "",
                                                     const QueryParams& query_params = defaultQueryParams(),
                                                     const PutParams& put_params = defaultPutParams());

  folly::Optional<ConsulResponse<Value>> getValueSync(const std::string& key, const std::string& token = "",
                                                      const QueryParams& query_params = defaultQueryParams());

  folly::Optional<ConsulResponse<std::vector<Value>>> getValuesSync(const std::string& key_prefix,
                                                                    const std::string& token = "",
                                                                    const QueryParams& query_params =
                                                                        defaultQueryParams());

  folly::Optional<ConsulResponse<bool>> deleteValueSync(const std::string& key, const std::string& token = "",
                                                        const QueryParams& query_params = defaultQueryParams());

  folly::Optional<ConsulResponse<bool>> deleteValuesSync(const std::string& key_prefix, const std::string& token = "",
                                                         const QueryParams& query_params = defaultQueryParams());

  // async request
  folly::Future<RawClient::HttpResponse> getValue(const std::string& key, const std::string& token = "",
                                                  const QueryParams& query_params = defaultQueryParams());

  folly::Future<RawClient::HttpResponse> getValues(const std::string& key_prefix, const std::string& token = "",
                                                   const QueryParams& query_params = defaultQueryParams());

  folly::Future<RawClient::HttpResponse> setValue(const std::string& key, const std::string& value,
                                                  const std::string& token = "",
                                                  const QueryParams& query_params = defaultQueryParams(),
                                                  const PutParams& put_params = defaultPutParams());

  folly::Future<RawClient::HttpResponse> deleteValue(const std::string& key, const std::string& token = "",
                                                     const QueryParams& query_params = defaultQueryParams());

  folly::Future<RawClient::HttpResponse> deleteValues(const std::string& key_prefix, const std::string& token = "",
                                                      const QueryParams& query_params = defaultQueryParams());

  // process responses methods
  folly::Optional<ConsulResponse<bool>> processBool(const RawClient::HttpResponse& response);
  folly::Optional<ConsulResponse<Value>> processValue(const RawClient::HttpResponse& response);
  folly::Optional<ConsulResponse<std::vector<Value>>> processValues(const RawClient::HttpResponse& response);
};

}  // namespace consul
}  // namespace service_framework
