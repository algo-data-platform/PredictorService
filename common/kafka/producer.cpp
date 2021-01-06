#include <sys/time.h>
#include "glog/logging.h"
#include "producer.h"
#include <functional>

namespace common {
namespace kafka {
class Producer;

class KDeliveredCbProducer : public RdKafka::DeliveryReportCb {
 public:
  explicit KDeliveredCbProducer(Producer* producer) : producer_(producer) {}
  void dr_cb(RdKafka::Message& message) {
    std::string topicName = message.topic_name();
    if (topicName.empty()) {
      LOG(ERROR) << " topic name is empty";
      return;
    }
    if (message.err() == RdKafka::ERR_NO_ERROR) {
      VLOG(1) << "Message delivery for (" << message.len() << " bytes): " << message.errstr();
    } else {
      std::string buffer;
      buffer.append(static_cast<const char*>(message.payload()), message.len());
      producer_->errorCallback(topicName, message.partition(), buffer, message.errstr());
    }
  }
  ~KDeliveredCbProducer() {}

 private:
  Producer* producer_;
};

class KEventCbProducer : public RdKafka::EventCb {
 public:
  explicit KEventCbProducer(Producer* producer) : producer_(producer) {}
  void event_cb(RdKafka::Event& event) {
    switch (event.type()) {
      case RdKafka::Event::EVENT_ERROR:
        LOG(ERROR) << "ERROR (" << RdKafka::err2str(event.err()) << "): " << event.str();
        break;

      case RdKafka::Event::EVENT_STATS:
        producer_->statCallback(event.str());
        break;

      case RdKafka::Event::EVENT_LOG:
        if (event.severity() == RdKafka::Event::EVENT_SEVERITY_INFO) {
          VLOG(0) << "LOG-" << event.severity() << "-" << event.fac() << ":" << event.str();
        } else if (event.severity() == RdKafka::Event::EVENT_SEVERITY_INFO) {
          VLOG(0) << "LOG-" << event.severity() << "-" << event.fac() << ":" << event.str();
        } else {
          VLOG(1) << "LOG-" << event.severity() << "-" << event.fac() << ":" << event.str();
        }
        break;

      case RdKafka::Event::EVENT_THROTTLE:
        VLOG(0) << "THROTTLED: " << event.throttle_time() << "ms by " << event.broker_name() << " id "
                << static_cast<int>(event.broker_id());
        break;

      default:
        VLOG(1) << "EVENT " << event.type() << " (" << RdKafka::err2str(event.err()) << "): " << event.str();
        break;
    }
  }
  ~KEventCbProducer() {}

 private:
  Producer* producer_;
};

Producer::Producer() {}

Producer::~Producer() {
  if (producer_) {
    RdKafka::ErrorCode errorCode = producer_->flush(3000);
    if (errorCode == RdKafka::ERR__TIMED_OUT) {
      VLOG(1) << "Flush message timeout, message size:" << producer_->outq_len();
    }
  }
}
int Producer::send(const std::unique_ptr<folly::IOBuf>& message, int partId) {
  if (topicname_.empty()) {
    std::string error = "Topic name is empty";
    return 1;
  }

  if (message->length() == 0) {
    return 0;
  }
  if (!topic_) {
    VLOG(0) << "Start create topic object `" << topicname_ << "`";
    std::string errstr;
    topic_.reset(RdKafka::Topic::create(producer_.get(), topicname_, tconf_.get(), errstr));
    if (topic_ == nullptr) {
      VLOG(1) << "Create kafka topic instance error, " << errstr;
      return 2;
    }
  }

  if (partId == -1) {
    partId = RdKafka::Topic::PARTITION_UA;
  }

  RdKafka::ErrorCode resp = producer_->produce(topic_.get(), partId, RdKafka::Producer::RK_MSG_COPY /* Copy payload */,
                                               message->writableData(), message->length(), nullptr, nullptr);
  if (resp != RdKafka::ERR_NO_ERROR) {
    VLOG(1) << "Send message fail, topic:" << topicname_ << " partition: " << partId
            << " err: " << RdKafka::err2str(resp);
    return 3;
  }
  producer_->poll(0);

  if (queuelen_ > 0 && producer_->outq_len() > queuelen_) {
    producer_->poll(100);
  }
  return 0;
}

int Producer::init(const std::string& topicName, const std::unordered_map<std::string, std::string>& configs,
                   int queueLen) {
  topicname_ = topicName;
  configs_ = configs;
  queuelen_ = queueLen;

  if (topicName.empty()) {
    LOG(ERROR) << "Topic name is empty";
    return 1;
  }
  topicname_ = topicName;

  std::string errstr;
  conf_.reset(RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL));
  tconf_.reset(RdKafka::Conf::create(RdKafka::Conf::CONF_TOPIC));

  eventcb_.reset(new KEventCbProducer(this));
  if (RdKafka::Conf::CONF_OK != conf_->set("event_cb", eventcb_.get(), errstr)) {
    LOG(ERROR) << errstr;
    return 2;
  }
  drcb_.reset(new KDeliveredCbProducer(this));
  if (RdKafka::Conf::CONF_OK != conf_->set("dr_cb", drcb_.get(), errstr)) {
    LOG(ERROR) << errstr;
    return 3;
  }

  for (auto config : configs_) {
    if (config.first.substr(0, 1) == "T") {
      tconf_->set(config.first.substr(1, config.first.length() - 1), config.second, errstr);
    } else {
      conf_->set(config.first, config.second, errstr);
    }
  }

  std::list<std::string>* dump;
  dump = conf_->dump();
  for (auto it = dump->begin(); it != dump->end();) {
    std::string key = *it++;
    std::string value = *it++;
    VLOG(1) << "Global config:" << key << "=" << value;
  }
  dump = tconf_->dump();
  for (auto it = dump->begin(); it != dump->end();) {
    std::string key = *it++;
    std::string value = *it++;
    VLOG(1) << "Topic config:" << key << "=" << value;
  }
  producer_.reset(RdKafka::Producer::create(conf_.get(), errstr));
  if (producer_ == nullptr) {
    LOG(ERROR) << "Failed to create producer:" << errstr;
    return 4;
  }
  VLOG(1) << "Created producer " << producer_->name();
  return 0;
}

void Producer::errorCallback(const std::string& topicName, int partid, const std::string& message,
                             const std::string& errstr) {
  VLOG(1) << "ERRORCALLBACK: "
          << "topic: " << topicName << ", partid: " << partid << ", message: " << message << ", error: " << errstr;
}
void Producer::statCallback(const std::string& stat) { VLOG(1) << "stat: " << stat; }
}  // namespace kafka
}  // namespace common
