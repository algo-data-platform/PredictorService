#include <iostream>
#include <boost/algorithm/string.hpp>

#include "glog/logging.h"
#include "folly/String.h"

#include "kafka_producer.h"

namespace predictor {

Status KafkaProducer::initialize(const std::string& configfile) {
  std::ifstream fin(configfile);
  rapidjson::IStreamWrapper isw(fin);
  rapidjson::Document config;
  config.ParseStream(isw);
  if (!config.IsObject()) {
    LOG(ERROR) << "parse kafka config file error:" << configfile;
    return Status::KAFKA_PRODUCER_CONFIG_FILE_ERROR;
  }
  if (!config.HasMember("kafka_producer_config")) {
    LOG(ERROR) << "kafka producer config member:kafka_producer_config is not found:" << configfile;
    return Status::KAFKA_PRODUCER_CONFIG_ERROR;
  }
  const rapidjson::Value& kafka_producer_config = config["kafka_producer_config"];
  if (!kafka_producer_config.IsObject()) {
    LOG(ERROR) << "parse kafka producer config member:kafka_config error:" << configfile;
    return Status::KAFKA_PRODUCER_CONFIG_ERROR;
  }
  if (!kafka_producer_config.HasMember("topic")) {
    LOG(ERROR) << "kafka producer config member:topic is not found:" << configfile;
    return Status::KAFKA_PRODUCER_CONFIG_TOPIC_ERROR;
  }
  topic_ = kafka_producer_config["topic"].GetString();
  if (kafka_producer_config.HasMember("partition_num")) {
    partition_num_ = kafka_producer_config["partition_num"].GetInt();
  }
  LOG(INFO) << "topic:" << topic_ << " partition:" << partition_num_;

  if (!kafka_producer_config.HasMember("kafka_config")) {
    LOG(ERROR) << "kafka producer config member:kafka_config is not found:" << configfile;
    return Status::KAFKA_PRODUCER_CONFIG_KAFKA_ERROR;
  }
  const rapidjson::Value& kafka_config = kafka_producer_config["kafka_config"];
  if (!kafka_config.IsArray()) {
    LOG(ERROR) << "get kafka producer kafka_config error:"  << configfile;
    return Status::KAFKA_PRODUCER_CONFIG_KAFKA_ERROR;
  }
  std::unordered_map<std::string, std::string> config_map;
  for (rapidjson::SizeType i = 0; i < kafka_config.Size() - 1; i = i + 2) {
    config_map[kafka_config[i].GetString()] = kafka_config[i + 1].GetString();
  }
  int ret = init(topic_, config_map);
  if (ret) {
    return Status::KAFKA_PRODUCER_KAFKA_INIT_ERROR;
  }
  return Status::OK;
}

bool KafkaProducer::sendKafka(const std::string& message, int64_t hashid) {
  std::unique_ptr<folly::IOBuf> iobuf = folly::IOBuf::wrapBuffer(message.c_str(), message.length());
  int partition = -1;
  if (hashid != -1 && partition_num_ != -1) {
    partition = hashid % partition_num_;
  }
  int ret = common::kafka::Producer::send(iobuf, partition);
  if (ret) {
    LOG(ERROR) << "send kafka error, message:" << message << "; size:" << message.length();
    return false;
  }
  return true;
}
}  // namespace predictor
