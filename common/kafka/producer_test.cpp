#include "producer.h"
#include "gflags/gflags.h"
#include <functional>
#include <thread>  // NOLINT

DEFINE_string(server, "127.0.0.1:9092", "kafka server brokers address list");
DEFINE_string(topic, "test", "kafka topic");
DEFINE_int32(num_threads, 1, "thread num");
DEFINE_int32(num_send, 1, "send num");
DEFINE_int32(partition, -1, "partition NO.");

class SendKafka : public common::kafka::Producer {
 public:
  SendKafka() { m_partition = RdKafka::Topic::PARTITION_UA; }
  ~SendKafka() {}
  int init2(const std::unordered_map<std::string, std::string> &config, std::string topicName) {
    m_topic = topicName;
    return init(topicName, config);
  }
  int handler() {
    for (int i = 0; i < FLAGS_num_send; i++) {
      char buf[10] = {0x00};
      buf[9] = 'd';
      std::unique_ptr<folly::IOBuf> iobuf =  folly::IOBuf::wrapBuffer(buf, sizeof(buf));
      send(iobuf, FLAGS_partition);
    }
    return 0;
  }
  void errorCallback(const std::string &topicName, int partId, const std::string &message, const std::string &error) {}
  void statCallback(const std::string &stat) {}

 public:
  std::string m_topic;
  int32_t m_partition;
};

int main(int argc, char *argv[]) {
  google::ParseCommandLineFlags(&argc, &argv, true);
  std::unordered_map<std::string, std::string> config;
  config["metadata.broker.list"] = FLAGS_server;
  std::vector<std::thread> threads;
  std::vector<SendKafka> sks;
  sks.resize(FLAGS_num_threads);
  for (int i = 0; i < FLAGS_num_threads; i++) {
    sks[i].init2(config, FLAGS_topic);
    threads.push_back(std::thread(std::bind(&SendKafka::handler, &sks[i])));
  }

  // sk.handler();
  // sleep(1);
  for (auto &thr : threads) {
    thr.join();
  }
  return 0;
}
