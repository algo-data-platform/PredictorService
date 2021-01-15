#pragma once
#include <fstream>
#include <set>

#include "glog/logging.h"
#include "document.h"
#include "istreamwrapper.h"
#include "common/kafka/producer.h"

namespace predictor {

enum class Status:int{
  OK = 0,
  KAFKA_PRODUCER_CONFIG_FILE_ERROR = 101,
  KAFKA_PRODUCER_CONFIG_ERROR,
  KAFKA_PRODUCER_CONFIG_TOPIC_ERROR,
  KAFKA_PRODUCER_CONFIG_KAFKA_ERROR,
  KAFKA_PRODUCER_KAFKA_INIT_ERROR,
};
class KafkaProducer : public common::kafka::Producer {
 public:
  KafkaProducer():partition_num_(-1) {}
  ~KafkaProducer() {}
  Status initialize(const std::string &configfile);
  bool sendKafka(const std::string &message, int64_t hashid = -1);
  void errorCallback(const std::string &topicName, int partId, const std::string &message,
      const std::string &error) override {
    LOG(ERROR) << "flow producer error: "
               << "topic: " << topicName << ", partid: " << partId << ", message: " << message << ", error: " << error;
  }
  void statCallback(const std::string &stat) override { LOG(INFO) << "flow producer stat : " << stat; }

 private:
  std::string topic_;
  int32_t partition_num_;
};
}  // namespace predictor
