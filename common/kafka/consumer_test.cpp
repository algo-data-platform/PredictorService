#include "consumer.h"
#include "gflags/gflags.h"
#include "glog/logging.h"
#include <functional>
#include <iostream>

DEFINE_string(server, "127.0.0.1:9092", "kafka server brokers address list");
DEFINE_string(topic, "test", "kafka topic");
DEFINE_int32(partition, -1, "partition NO.");
DEFINE_string(groupid, "test", "kafka groupid");
DEFINE_int32(num_threads, 1, "thread num");

class ReadKafka : public common::kafka::Consumer {
 public:
  ReadKafka() {}
  ~ReadKafka() {}
  int init2(const std::unordered_map<std::string, std::string> &config, std::string topicName) {
    m_topic = topicName;

    for (int i = 0; i < FLAGS_num_threads; i++) {
      if (FLAGS_partition > -1) {
        addTopicPartition(topicName, FLAGS_partition);
      } else {
        addTopicPartition(topicName);
      }
    }

    // addTopicPartition("test0");
    int ret = init(config);
    if (ret) {
      return ret;
    }
    start();
    return 0;
  }
  bool handler(std::string topicName, int partid, uint64_t offset, const std::unique_ptr<folly::IOBuf> &messageBuf) {
    std::string message(reinterpret_cast<const char *>(messageBuf->data()), messageBuf->length());
    LOG(INFO) << "topic: " << topicName << " partid: " << partid << " offset: " << offset;
    LOG(INFO) << "message: " << message << " length:" << message.length();
    LOG(INFO) << "message: " << message.c_str()[9] << " length:" << message.length();
    return true;
  }
  void errorCallback(const std::string &error) {}
  void statCallback(const std::string &stat) {}

 public:
  std::string m_topic;
  int32_t m_partition;
  // common::kafka::Consumer m_consumer;
};

int main(int argc, char *argv[]) {
  google::ParseCommandLineFlags(&argc, &argv, true);
  std::unordered_map<std::string, std::string> config;
  config["metadata.broker.list"] = FLAGS_server;
  config["group.id"] = FLAGS_groupid;
  ReadKafka rks;
  std::cout << FLAGS_topic << std::endl;
  rks.init2(config, FLAGS_topic);

  while (1) {
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
  return 0;
}
