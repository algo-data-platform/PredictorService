#pragma once

#include <vector>
#include <unordered_map>
#include <rdkafkacpp.h>
#include <rdkafkacpp_int.h>
#include <functional>
#include <thread>  // NOLINT
#include <folly/Function.h>
#include <folly/io/IOBuf.h>
#include <memory>

namespace common {
namespace kafka {

class KRebalanceCb;
class KEventCb;
class Consumer {
 public:
  Consumer();
  virtual ~Consumer();

  void addTopicPartition(const std::string& topic, int partId = -1, int64_t offset = RdKafka::Topic::OFFSET_INVALID);
  int init(const std::unordered_map<std::string, std::string>& config);
  void start();
  void stop();

  virtual bool handler(std::string topicName, int partId, uint64_t offset,
                       const std::unique_ptr<folly::IOBuf>& message);
  virtual void statCallback(const std::string& stat);
  virtual void errorCallback(const std::string& errstr);
  void setConsumeTimeout(int consumerTimeout) { consumetimeout_ = consumerTimeout; }

 private:
  void threadFunc(void* data);

 private:
  std::unordered_map<std::string, std::string> configs_;
  std::vector<std::string> topics_;
  std::vector<std::shared_ptr<RdKafka::TopicPartitionImpl> > topic_partition_;
  int consumetimeout_ = 1000;
  bool is_runing_ = false;
  bool is_close_ = false;
  std::vector<std::shared_ptr<std::thread> > threads_;
  std::shared_ptr<RdKafka::KafkaConsumer> consumer_;
  std::shared_ptr<RdKafka::Conf> conf_;
  std::shared_ptr<RdKafka::Conf> tconf_;
  std::shared_ptr<KRebalanceCb> rebalancecb_;
  std::shared_ptr<KEventCb> eventcb_;
};
}  // namespace kafka
}  // namespace common

