#pragma once

#include <string>

#include "proxygen/httpserver/HTTPServerOptions.h"
#include "proxygen/httpserver/HTTPServer.h"

#include "service_router/router.h"
#include "service_framework/http/http_server_manager.h"

namespace service_router {

using HttpServerModifier = folly::Optional<folly::Function<void(proxygen::HTTPServerOptions&)>>;

std::shared_ptr<proxygen::HTTPServer> createAndRegisterHttpServer(const ServerOption& option,
                                                       HttpServerModifier server_modifier = folly::none,
                                                       int wait_milliseconds = 0,
                                                       const ServerStatus& status = ServerStatus::AVAILABLE);

void httpServiceServer(const ServerOption& option, HttpServerModifier server_modifier = folly::none,
                       int wait_milliseconds = 0,
                       const ServerStatus& status = ServerStatus::AVAILABLE);
void httpServiceServer(const std::string& service_name, const ServerAddress& address,
                       HttpServerModifier server_modifier = folly::none, int wait_milliseconds = 0,
                       const ServerStatus& status = ServerStatus::AVAILABLE);
void httpServiceServer(const std::string& service_name, const std::string& host, uint16_t port,
                       HttpServerModifier server_modifier = folly::none, int wait_milliseconds = 0,
                       const ServerStatus& status = ServerStatus::AVAILABLE);
// anonymously service, no registration in consul
void httpServiceServer(const std::string& host, uint16_t port);

}  // namespace service_router
