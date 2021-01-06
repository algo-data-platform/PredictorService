#include <sys/time.h>
#include <glog/logging.h>
#include <functional>
#include "consumer.h"

namespace common {
namespace kafka {

class Consumer;

class KEventCb : public RdKafka::EventCb {
 public:
  explicit KEventCb(Consumer* consumer) : consumer_(consumer) {}
  void event_cb(RdKafka::Event& event) {
    switch (event.type()) {
      case RdKafka::Event::EVENT_ERROR:
        LOG(ERROR) << "ERROR (" << RdKafka::err2str(event.err()) << "): " << event.str();
        break;

      case RdKafka::Event::EVENT_STATS:
        consumer_->statCallback(event.str());
        break;

      case RdKafka::Event::EVENT_LOG:
        if (event.severity() == RdKafka::Event::EVENT_SEVERITY_DEBUG) {
          VLOG(2) << "LOG-" << event.severity() << "-" << event.fac() << ":" << event.str();
        } else if (event.severity() == RdKafka::Event::EVENT_SEVERITY_INFO) {
          VLOG(2) << "LOG-" << event.severity() << "-" << event.fac() << ":" << event.str();
        } else {
          VLOG(2) << "LOG-" << event.severity() << "-" << event.fac() << ":" << event.str();
        }
        break;

      case RdKafka::Event::EVENT_THROTTLE:
        VLOG(2) << "THROTTLED: " << event.throttle_time() << "ms by " << event.broker_name() << " id "
                << static_cast<int>(event.broker_id());
        break;

      default:
        VLOG(2) << "EVENT " << event.type() << " (" << RdKafka::err2str(event.err()) << "): " << event.str();
        break;
    }
  }
  ~KEventCb() {}

 private:
  Consumer* consumer_;
};

class KRebalanceCb : public RdKafka::RebalanceCb {
 private:
  static void partPrint(const std::vector<RdKafka::TopicPartition*>& partitions) {
    for (unsigned int i = 0; i < partitions.size(); i++) {
      VLOG(1) << "\t" << partitions[i]->topic() << "[" << partitions[i]->partition() << "]";
    }
  }
  Consumer* consumer_;

 public:
  explicit KRebalanceCb(Consumer* consumer) : consumer_(consumer) {}
  void rebalance_cb(RdKafka::KafkaConsumer* consumer, RdKafka::ErrorCode err,
                    std::vector<RdKafka::TopicPartition*>& partitions) {
    VLOG(1) << "RebalanceCb: " << RdKafka::err2str(err) << ": ";
    partPrint(partitions);

    if (err == RdKafka::ERR__ASSIGN_PARTITIONS) {
      consumer->assign(partitions);
    } else {
      consumer->unassign();
    }
  }
  ~KRebalanceCb() {}
};

Consumer::Consumer() {}

Consumer::~Consumer() {}

void Consumer::stop() {
  is_runing_ = false;
  int try_num = 10;
  while (!is_close_ && try_num-- > 0) {
    VLOG(2) << "Stopping consumer.";
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
}

void Consumer::start() {
  is_runing_ = true;
  for (unsigned int i = 0; i < topics_.size(); ++i) {
    std::shared_ptr<std::thread> pthread(
        std::make_shared<std::thread>(std::bind(&Consumer::threadFunc, this, nullptr)));
    threads_.push_back(pthread);
  }
  for (unsigned int i = 0; i < topic_partition_.size(); ++i) {
    std::shared_ptr<std::thread> pthread(
        std::make_shared<std::thread>(std::bind(&Consumer::threadFunc, this, nullptr)));
    threads_.push_back(pthread);
  }
}

void Consumer::threadFunc(void*) {
  while (is_runing_) {
    std::unique_ptr<RdKafka::Message> message(consumer_->consume(consumetimeout_));
    std::string topicName = message->topic_name();
    int32_t partId = message->partition();
    switch (message->err()) {
      case RdKafka::ERR__TIMED_OUT:
        break;

      case RdKafka::ERR_NO_ERROR: {
        /* Real message */
        VLOG(1) << "Read msg at offset " << message->offset() << " topic:" << topicName << " part:" << partId;
        std::unique_ptr<folly::IOBuf> ret_msg = folly::IOBuf::wrapBuffer(message->payload(), message->len());
        handler(topicName, partId, message->offset(), ret_msg);
      } break;

      case RdKafka::ERR__PARTITION_EOF:
        VLOG(2) << "Consume failed: " << message->errstr();
        break;
      case RdKafka::ERR__UNKNOWN_TOPIC:
      case RdKafka::ERR__UNKNOWN_PARTITION:
        errorCallback(message->errstr());
        LOG(ERROR) << "Consume failed: " << message->errstr();
        break;

      default:
        /* Errors */
        errorCallback(message->errstr());
        LOG(ERROR) << "Consume failed: " << message->errstr();
    }
  }
  consumer_->close();
  RdKafka::wait_destroyed(5000);
  is_close_ = true;
}

void Consumer::addTopicPartition(const std::string& topic, int partId, int64_t offset) {
  if (partId == -1) {
    topics_.push_back(topic);
  } else {
    topic_partition_.push_back(std::make_shared<RdKafka::TopicPartitionImpl>(topic, partId, offset));
  }
}
int Consumer::init(const std::unordered_map<std::string, std::string>& configs) {
  configs_ = configs;

  if ((topic_partition_.size() > 0 && topics_.size() > 0) || (topic_partition_.size() <= 0 && topics_.size() <= 0)) {
    LOG(ERROR) << "no topic is added";
    return 1;
  }
  std::string errstr;
  conf_.reset(RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL));
  tconf_.reset(RdKafka::Conf::create(RdKafka::Conf::CONF_TOPIC));
  // int config

  for (auto config : configs_) {
    if (config.first.substr(0, 1) == "T") {
      tconf_->set(config.first.substr(1, config.first.length() - 1), config.second, errstr);
    } else {
      conf_->set(config.first, config.second, errstr);
    }
  }
  rebalancecb_.reset(new KRebalanceCb(this));
  if (topics_.size() > 0 && RdKafka::Conf::CONF_OK != conf_->set("rebalance_cb", rebalancecb_.get(), errstr)) {
    LOG(ERROR) << "rebalance_cb error :" << errstr;
  }
  eventcb_.reset(new KEventCb(this));
  if (RdKafka::Conf::CONF_OK != conf_->set("event_cb", eventcb_.get(), errstr)) {
    LOG(ERROR) << "event_cb error :" << errstr;
  }
  std::list<std::string>* dump;
  dump = conf_->dump();
  for (auto it = dump->begin(); it != dump->end();) {
    std::string key = *it++;
    std::string value = *it++;
    VLOG(2) << "Global config:" << key << "=" << value;
  }
  dump = tconf_->dump();
  for (auto it = dump->begin(); it != dump->end();) {
    std::string key = *it++;
    std::string value = *it++;
    VLOG(2) << "Topic config:" << key << "=" << value;
  }
  conf_->set("default_topic_conf", tconf_.get(), errstr);
  consumer_.reset(RdKafka::KafkaConsumer::create(conf_.get(), errstr));
  if (topics_.size() > 0) {
    RdKafka::ErrorCode err = consumer_->subscribe(topics_);
    if (err) {
      LOG(ERROR) << "Failed to subscribe to  topic: " << RdKafka::err2str(err);
      return 1;
    }
  }
  if (topic_partition_.size() > 0) {
    std::vector<RdKafka::TopicPartition*> topic_partition;
    for (unsigned int i = 0; i < topic_partition_.size(); ++i) {
      topic_partition.push_back(topic_partition_[i].get());
    }
    RdKafka::ErrorCode err = consumer_->assign(topic_partition);
    if (err) {
      LOG(ERROR) << "Failed to subscribe to  topic: " << RdKafka::err2str(err);
      return 1;
    }
  }
  VLOG(1) << "% Created consumer " << consumer_->name() << " rdkafka version:" << RdKafka::version_str();
  return 0;
}

void Consumer::statCallback(const std::string& stat) { VLOG(1) << "stat: " << stat; }

bool Consumer::handler(std::string topicName, int partId, uint64_t offset,
                       const std::unique_ptr<folly::IOBuf>& messageBuf) {
  std::string message(reinterpret_cast<const char*>(messageBuf->data()), messageBuf->length());
  VLOG(1) << "topic: " << topicName << " partid: " << partId << " offset: " << offset;
  VLOG(1) << "message: " << message;
  return true;
}
void Consumer::errorCallback(const std::string& errstr) { VLOG(1) << "error: " << errstr; }
}  // namespace kafka
}  // namespace common
