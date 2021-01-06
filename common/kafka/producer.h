
#pragma once

#include <vector>
#include <unordered_map>
#include <rdkafkacpp.h>
#include <memory>
#include <folly/io/IOBuf.h>

namespace common {
namespace kafka {

class KDeliveredCbProducer;
class KEventCbProducer;
class Producer {
 public:
  Producer();
  virtual ~Producer();

  int init(const std::string& topicName, const std::unordered_map<std::string, std::string>& config, int queueLen = -1);
  int send(const std::unique_ptr<folly::IOBuf>& message, int partId = -1);

  virtual void statCallback(const std::string& stat);
  virtual void errorCallback(const std::string& topicName, int partId, const std::string& message,
                             const std::string& error);

 private:
  std::string topicname_;
  std::shared_ptr<RdKafka::Producer> producer_;
  std::unordered_map<std::string, std::string> configs_;
  std::shared_ptr<RdKafka::Conf> conf_;
  std::shared_ptr<RdKafka::Conf> tconf_;
  std::shared_ptr<RdKafka::Topic> topic_;
  std::shared_ptr<KEventCbProducer> eventcb_;
  std::shared_ptr<KDeliveredCbProducer> drcb_;

  int queuelen_;
};
}  // namespace kafka
}  // namespace common

