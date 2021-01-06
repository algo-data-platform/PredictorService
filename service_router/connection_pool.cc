#include "connection_pool.h"

namespace service_router {

void stop_connection_pool() {
  ConnectionPool<service_router::HeaderClientChannelPtr>::getInstance()->stop();
  ConnectionPool<service_router::Http2ClientConnectionIfPtr>::getInstance()->stop();
  ConnectionPool<service_router::RSocketClientChannelPtr>::getInstance()->stop();
}

}  // namespace service_router
