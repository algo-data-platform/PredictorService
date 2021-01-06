#include "service_framework/consul/kv_client.h"

namespace service_framework {
namespace consul {

static constexpr char CONSUL_KV_KEY[] = "v1/kv/";
static constexpr char CONSUL_KV_PARAM_TOKEN[] = "token";
static constexpr char CONSUL_KV_PARAM_RECURSE[] = "recurse";
static constexpr char CONSUL_KV_RETURN_BOOL[] = "true";

folly::Optional<ConsulResponse<bool>> KVClient::setValueSync(const std::string& key, const std::string& value,
                                                               const std::string& token,
                                                               const QueryParams& query_params,
                                                               const PutParams& put_params) {
  auto future = setValue(key, value, token, query_params, put_params);
  return processBool(future.get());
}

folly::Optional<ConsulResponse<Value>> KVClient::getValueSync(const std::string& key, const std::string& token,
                                                                   const QueryParams& query_params) {
  auto future = getValue(key, token, query_params);
  return processValue(future.get());
}

folly::Optional<ConsulResponse<std::vector<Value>>> KVClient::getValuesSync(const std::string& key_prefix,
                                                                                 const std::string& token,
                                                                                 const QueryParams& query_params) {
  auto future = getValues(key_prefix, token, query_params);
  return processValues(future.get());
}

folly::Optional<ConsulResponse<bool>> KVClient::deleteValueSync(const std::string& key, const std::string& token,
                                                                  const QueryParams& query_params) {
  auto future = deleteValue(key, token, query_params);
  return processBool(future.get());
}

folly::Optional<ConsulResponse<bool>> KVClient::deleteValuesSync(const std::string& key_prefix,
                                                                   const std::string& token,
                                                                   const QueryParams& query_params) {
  auto future = deleteValues(key_prefix, token, query_params);
  return processBool(future.get());
}

// @ref https://www.consul.io/api/kv.html#read-key
folly::Future<RawClient::HttpResponse> KVClient::getValue(const std::string& key, const std::string& token,
                                                            const QueryParams& query_params) {
  return service_framework::http::makeGetRequest(createUrl(CONSUL_KV_KEY + key), getHttpOption(),
                                                 [token, query_params](auto& req) {
    if (!token.empty()) {
      req.setQueryParam(CONSUL_KV_PARAM_TOKEN, token);
    }
    query_params.makeParams(&req);
  });
}

// @ref https://www.consul.io/api/kv.html#read-key
folly::Future<RawClient::HttpResponse> KVClient::getValues(const std::string& key_prefix, const std::string& token,
                                                             const QueryParams& query_params) {
  return service_framework::http::makeGetRequest(createUrl(CONSUL_KV_KEY + key_prefix), getHttpOption(),
                                                 [token, query_params](auto& req) {
    if (!token.empty()) {
      req.setQueryParam(CONSUL_KV_PARAM_TOKEN, token);
    }
    // 会递归查找所有的 key
    req.setQueryParam(CONSUL_KV_PARAM_RECURSE, "1");
    query_params.makeParams(&req);
  });
}

folly::Future<RawClient::HttpResponse> KVClient::setValue(const std::string& key, const std::string& value,
                                                            const std::string& token, const QueryParams& query_params,
                                                            const PutParams& put_params) {
  return service_framework::http::makePutRequest(createUrl(CONSUL_KV_KEY + key), value, getHttpOption(),
                                                 [token, query_params, put_params](auto& req) {
    if (!token.empty()) {
      req.setQueryParam(CONSUL_KV_PARAM_TOKEN, token);
    }
    query_params.makeParams(&req);
    put_params.makeParams(&req);
  });
}

folly::Future<RawClient::HttpResponse> KVClient::deleteValue(const std::string& key, const std::string& token,
                                                               const QueryParams& query_params) {
  return service_framework::http::makeDeleteRequest(createUrl(CONSUL_KV_KEY + key), getHttpOption(),
                                                    [token, query_params](auto& req) {
    if (!token.empty()) {
      req.setQueryParam(CONSUL_KV_PARAM_TOKEN, token);
    }
    query_params.makeParams(&req);
  });
}

folly::Future<RawClient::HttpResponse> KVClient::deleteValues(const std::string& key_prefix, const std::string& token,
                                                                const QueryParams& query_params) {
  return service_framework::http::makeDeleteRequest(createUrl(CONSUL_KV_KEY + key_prefix), getHttpOption(),
                                                    [token, query_params](auto& req) {
    if (!token.empty()) {
      req.setQueryParam(CONSUL_KV_PARAM_TOKEN, token);
    }
    req.setQueryParam(CONSUL_KV_PARAM_RECURSE, "1");
    query_params.makeParams(&req);
  });
}

folly::Optional<ConsulResponse<bool>> KVClient::processBool(const RawClient::HttpResponse& response) {
  auto message = response->moveMessage();
  if (!message) {
    return folly::none;
  }

  auto http_message = std::move(*message);
  VLOG(4) << "http code:" << http_message->getStatusCode();
  auto body = response->moveToFbString();
  if (!body) {
    return folly::none;
  }

  bool value = (*body == CONSUL_KV_RETURN_BOOL);
  ConsulResponse<bool> consul_response(std::move(http_message), value);
  return folly::Optional<ConsulResponse<bool>>(consul_response);
}

folly::Optional<ConsulResponse<std::vector<Value>>> KVClient::processValues(
    const RawClient::HttpResponse& response) {
  auto message = response->moveMessage();
  if (!message) {
    return folly::none;
  }

  auto http_message = std::move(*message);
  auto body = response->moveToFbString();
  if (!body) {
    return folly::none;
  }
  VLOG(10) << "http code:" << http_message->getStatusCode() << " Body:" << *body;

  folly::dynamic values;
  if (!common::fromJson(&values, *body)) {
    LOG(INFO) << *body;
    return folly::none;
  }

  if (!values.isArray()) {
    return folly::none;
  }

  std::vector<Value> get_values;
  for (size_t i = 0; i < values.size(); i++) {
    Value get_value;
    if (get_value.deserialize(values[i])) {
      get_values.push_back(get_value);
    }
  }
  ConsulResponse<std::vector<Value>> consul_response(std::move(http_message), get_values);
  return folly::Optional<ConsulResponse<std::vector<Value>>>(consul_response);
}

folly::Optional<ConsulResponse<Value>> KVClient::processValue(const RawClient::HttpResponse& response) {
  auto message = response->moveMessage();
  if (!message) {
    return folly::none;
  }

  auto http_message = std::move(*message);
  VLOG(4) << *(http_message.get());
  auto body = response->moveToFbString();
  if (!body) {
    return folly::none;
  }

  folly::dynamic values;
  if (!common::fromJson(&values, *body)) {
    LOG(INFO) << *body;
    return folly::none;
  }

  if (!values.isArray() || values.size() != 1) {
    return folly::none;
  }
  folly::dynamic value = values[0];
  Value get_value;
  if (!get_value.deserialize(value)) {
    return folly::none;
  }

  ConsulResponse<Value> consul_response(std::move(http_message), get_value);
  return folly::Optional<ConsulResponse<Value>>(consul_response);
}

}  // namespace consul
}  // namespace service_framework
