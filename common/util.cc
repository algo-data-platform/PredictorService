#include "common/util.h"

#include "folly/ExceptionWrapper.h"
#include "folly/ThreadLocal.h"
#include <ifaddrs.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>  ///< close

namespace common {

bool fromJson(folly::dynamic* result, const folly::StringPiece& json) {
  bool ret = false;
  try {
    VLOG(10) << "parse json: " << json;
    *result = folly::parseJson(json);
  }
  catch (std::runtime_error& e) {
    FB_LOG_EVERY_MS(ERROR, 3000) << "from json fail, " << e.what();
  }
  catch (std::logic_error& e) {
    FB_LOG_EVERY_MS(ERROR, 3000) << "from json fail, " << e.what();
  }
  catch (...) {
    FB_LOG_EVERY_MS(ERROR, 3000) << "from json fail. unknown error";
  }
  ret = true;
  return ret;
}

bool toJson(std::string* result, const folly::dynamic& data) {
  try {
    *result = folly::toJson(data);
  }
  catch (std::runtime_error& e) {
    FB_LOG_EVERY_MS(ERROR, 3000) << "to json fail, " << e.what();
  }
  catch (std::logic_error& e) {
    FB_LOG_EVERY_MS(ERROR, 3000) << "to json fail, " << e.what();
  }
  catch (...) {
    FB_LOG_EVERY_MS(ERROR, 3000) << "to json fail. unknown error";
  }

  return true;
}

std::string pathJoin(const std::string& a, const std::string& b) { return a.back() == '/' ? (a + b) : (a + "/" + b); }

std::time_t currentTimeInMs() {
  std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds> tp =
      std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());
  auto epoch = std::chrono::duration_cast<std::chrono::milliseconds>(tp.time_since_epoch());
  return epoch.count();
}

std::time_t currentTimeInNs() {
  std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds> tp =
      std::chrono::time_point_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now());
  auto epoch = std::chrono::duration_cast<std::chrono::nanoseconds>(tp.time_since_epoch());
  return epoch.count();
}

std::string currentTimeToStr(std::time_t cur_time) {
  std::stringstream time_stream;
  time_stream << std::put_time(std::localtime(&cur_time), "%Y-%m-%d %H:%M:%S");
  return time_stream.str();
}

std::string currentTimeInFormat(const std::string& format) {
  auto now = std::chrono::system_clock::now();
  auto in_time_t = std::chrono::system_clock::to_time_t(now);
  std::stringstream ret;
  ret << std::put_time(std::localtime(&in_time_t), format.c_str());
  return ret.str();
}

// 失败返回 0, 对于 0.0.0.0 在该场景下无意义，所以不考虑
uint32_t ipToInt(const std::string& ip) {
  std::vector<int> result;
  folly::split('.', ip, result, true);
  if (result.size() != 4) {
    return 0;
  }

  int i = 3;
  int total = 0;
  for (auto& t : result) {
    total += t * std::pow(256, i);
    i--;
  }
  return total;
}

bool split_slice(std::vector<Range>* slice_vec, unsigned total_num, unsigned min_item_num_per_thread,
                 unsigned max_thread_num) {
  if (max_thread_num == 0 || min_item_num_per_thread == 0) {
    return false;
  }
  unsigned avg = total_num / max_thread_num + 1;
  if (avg < min_item_num_per_thread) {
    avg = min_item_num_per_thread;
  }

  unsigned slice_num = total_num / avg;
  unsigned tmp_index = 0;
  for (unsigned i = 0; i < slice_num; ++i) {
    Range range;
    range.begin = tmp_index;
    tmp_index += avg;
    range.end = tmp_index;
    slice_vec->emplace_back(std::move(range));
  }

  if (tmp_index < total_num) {
    Range range;
    range.begin = tmp_index;
    range.end = total_num;
    slice_vec->emplace_back(std::move(range));
  }

  return true;
}

void getLocalIpAddress(std::string* ip_addr) {
  // 防止在某些场景下频繁调用创建连接获取 ip 开销太大，采用本地线程缓存
  static folly::ThreadLocal<std::string> local_ip_address;
  std::string* ip = local_ip_address.get();
  if (!ip->empty()) {
    *ip_addr = *ip;
    return;
  }
  constexpr char REMOTE_ADDRESS[] = "10.255.255.255";
  struct sockaddr_in remote_server;
  int sock = socket(AF_INET, SOCK_DGRAM, 0);
  if (sock < 0) {
    LOG(ERROR) << "Get local ip address, create sock failed.";
    return;
  }
  memset(&remote_server, 0, sizeof(remote_server));
  remote_server.sin_family = AF_INET;
  remote_server.sin_addr.s_addr = inet_addr(REMOTE_ADDRESS);
  remote_server.sin_port = htons(80);

  int err = connect(sock, reinterpret_cast<const struct sockaddr*>(&remote_server), sizeof(remote_server));
  if (err < 0) {
    LOG(ERROR) << "Get local ip address, error no:" << errno << " error:" << strerror(errno);
    return;
  }

  struct sockaddr_in local_addr;
  socklen_t local_addr_len = sizeof(local_addr);
  err = getsockname(sock, reinterpret_cast<struct sockaddr*>(&local_addr), &local_addr_len);
  char buffer[INET_ADDRSTRLEN];
  const char* p = inet_ntop(AF_INET, &local_addr.sin_addr, buffer, INET_ADDRSTRLEN);
  if (p != nullptr) {
    *ip_addr = buffer;
    *(local_ip_address.get()) = buffer;
  } else {
    LOG(ERROR) << "Get local ip address, error no:" << errno << " error:" << strerror(errno);
    return;
  }
  close(sock);
}

}  // namespace common
