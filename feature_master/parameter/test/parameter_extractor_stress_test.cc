#include <iostream>
#include <queue>
#include <fstream>
#include <time.h>

#include "folly/executors/CPUThreadPoolExecutor.h"
#include "folly/executors/task_queue/UnboundedBlockingQueue.h"
#include "folly/init/Init.h"
#include "gflags/gflags.h"

#include "feature_master/parameter/parameter_extractor.h"

double genRandDouble() {
  static unsigned seed = 1;
  return 0.01 * (rand_r(&seed) % 100);
}

std::string genRandString(const int len) {
  static unsigned seed = 1;
  static const char alphanum[] =
      "0123456789"
      "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
      "abcdefghijklmnopqrstuvwxyz";

  std::string output;
  for (int i = 0; i < len; ++i) {
    output += alphanum[rand_r(&seed) % (sizeof(alphanum) - 1)];
  }
  return output;
}

// TODO(changyu1): dynamically generate the features based on some config
std::shared_ptr<feature_master::Features> genFeatures() {
  feature_master::Feature feature;
  feature.set_feature_name("id1");
  feature.set_feature_type(feature_master::FeatureType::STRING_LIST);
  feature.set_string_values({genRandString(3), genRandString(4), genRandString(4), genRandString(9),
                             genRandString(4), genRandString(4), genRandString(4), genRandString(10)});
  feature_master::Feature feature2;
  feature2.set_feature_name("id2");
  feature2.set_feature_type(feature_master::FeatureType::STRING_LIST);
  feature2.set_string_values({genRandString(7), genRandString(9)});
  feature_master::Feature feature9;
  feature9.set_feature_name("id3");
  feature9.set_feature_type(feature_master::FeatureType::STRING_LIST);
  feature9.set_string_values({genRandString(7), genRandString(9)});
  feature_master::Feature feature10;
  feature10.set_feature_name("id4");
  feature10.set_feature_type(feature_master::FeatureType::STRING_LIST);
  feature10.set_string_values({genRandString(7), genRandString(9)});
  feature_master::Feature feature11;
  feature11.set_feature_name("id5");
  feature11.set_feature_type(feature_master::FeatureType::STRING_LIST);
  feature11.set_string_values({genRandString(7), genRandString(9)});
  feature_master::Feature feature3;
  feature3.set_feature_name("ctr1");
  feature3.set_feature_type(feature_master::FeatureType::FLOAT_LIST);
  feature3.set_float_values({genRandDouble(), genRandDouble(), genRandDouble(), genRandDouble(),
                             genRandDouble(), genRandDouble(), genRandDouble()});
  feature_master::Feature feature4;
  feature4.set_feature_name("ctr2");
  feature4.set_feature_type(feature_master::FeatureType::FLOAT_LIST);
  feature4.set_float_values({genRandDouble(), genRandDouble(), genRandDouble(), genRandDouble(),
                             genRandDouble(), genRandDouble(), genRandDouble()});
  feature_master::Feature feature12;
  feature12.set_feature_name("ctr3");
  feature12.set_feature_type(feature_master::FeatureType::FLOAT_LIST);
  feature12.set_float_values({genRandDouble(), genRandDouble(), genRandDouble(), genRandDouble(),
                              genRandDouble(), genRandDouble(), genRandDouble()});
  feature_master::Feature feature13;
  feature13.set_feature_name("ctr4");
  feature13.set_feature_type(feature_master::FeatureType::FLOAT_LIST);
  feature13.set_float_values({genRandDouble(), genRandDouble(), genRandDouble(), genRandDouble(),
                              genRandDouble(), genRandDouble(), genRandDouble()});
  feature_master::Feature feature14;
  feature14.set_feature_name("ctr5");
  feature14.set_feature_type(feature_master::FeatureType::FLOAT_LIST);
  feature14.set_float_values({genRandDouble(), genRandDouble(), genRandDouble(), genRandDouble(),
                              genRandDouble(), genRandDouble(), genRandDouble()});
  feature_master::Feature feature5;
  feature5.set_feature_name("city");
  feature5.set_feature_type(feature_master::FeatureType::STRING_LIST);
  feature5.set_string_values({genRandString(9), genRandString(8), genRandString(7), genRandString(7)});
  feature_master::Feature feature6;
  feature6.set_feature_name("gender");
  feature6.set_feature_type(feature_master::FeatureType::STRING_LIST);
  feature6.set_string_values({genRandString(10), genRandString(8), genRandString(8), genRandString(8)});
  feature_master::Feature feature7;
  feature7.set_feature_name("tags");
  feature7.set_feature_type(feature_master::FeatureType::STRING_LIST);
  feature7.set_string_values({genRandString(6), genRandString(7)});
  feature_master::Feature feature8;
  feature8.set_feature_name("follow_list");
  feature8.set_feature_type(feature_master::FeatureType::STRING_LIST);
  feature8.set_string_values({genRandString(7)});
  std::shared_ptr<feature_master::Features> features(new feature_master::Features());
  features->set_features(std::vector<feature_master::Feature>{feature,   feature2,  feature3,  feature4, feature5,
                                                              feature6,  feature7,  feature8,  feature9, feature10,
                                                              feature11, feature12, feature13, feature14});
  return features;
}

DEFINE_int32(thread_num, 21, "number of threads");
DEFINE_int64(record_num, 2000000, "number of records to be processed");
DEFINE_string(config_file, "", "json config file for parameter extractor");

int main(int argc, char** argv) {
  // Parse arguments
  FLAGS_logtostderr = 1;
  folly::init(&argc, &argv);
  int thread_num = FLAGS_thread_num;
  int64_t record_num = FLAGS_record_num;
  std::string config_file = FLAGS_config_file;
  std::cout << "number of threads=" << thread_num << "\nconfig file=" << config_file
            << "\nnumber of records=" << record_num << std::endl;

  if (config_file == "") {
    std::cerr << "No config file is provided!" << std::endl;
    return -1;
  }

  // Generate records
  std::cout << "generating records..." << std::endl;
  std::queue<std::shared_ptr<feature_master::Features>> features_queue;
  for (int i = 0; i < record_num; ++i) {
    features_queue.push(genFeatures());
  }

  auto features = features_queue.front();
  int feature_num = features->get_features().size();
  int raw_data_num = 0;
  for (auto feature : features->get_features()) {
    switch (feature.get_feature_type()) {
      case feature_master::FeatureType::STRING_LIST: {
        raw_data_num += feature.get_string_values().size();
        break;
      }
      case feature_master::FeatureType::INT64_LIST: {
        raw_data_num += feature.get_int64_values().size();
        break;
      }
      case feature_master::FeatureType::FLOAT_LIST: {
        raw_data_num += feature.get_float_values().size();
        break;
      }
      default: { break; }
    }
  }

  std::cout << "Number of features in each record=" << feature_num
            << "\nNumber of raw data points in each record=" << raw_data_num << std::endl;

  // Extract parameters
  std::ifstream ifile(config_file.c_str());
  if (!ifile) {
    std::cerr << "Failed to open test data file!" << std::endl;
  }
  std::ostringstream buf;
  char ch;
  while (buf && ifile.get(ch)) buf.put(ch);
  std::string json_config = buf.str();

  feature_master::ParameterExtractorManager manager(json_config);

  auto queue = std::make_unique<folly::UnboundedBlockingQueue<folly::CPUThreadPoolExecutor::CPUTask>>();
  folly::CPUThreadPoolExecutor cpuExe(thread_num, std::move(queue));
  time_t timer1;
  time(&timer1);
  std::cout << "extracting parameters..." << std::endl;
  std::atomic<int64_t> parameter_num(0);
  while (!features_queue.empty()) {
    std::shared_ptr<feature_master::Features> features = features_queue.front();
    features_queue.pop();
    auto f2 = [features, &manager, &parameter_num] {
      feature_master::Parameters parameters;
      bool ret = manager.ExtractParameters(&parameters, *features);
      if (!ret) {
        std::cout << "ExtractParameters failed" << std::endl;
      }
      parameter_num += parameters.size();
    };
    cpuExe.add(f2);
  }
  cpuExe.join();
  time_t timer2;
  time(&timer2);
  std::cout << "time spent for parameter extract=" << timer2 - timer1 << " seconds"
            << "\nnumber of parameters produced=" << parameter_num << "\n" << parameter_num / (timer2 - timer1)
            << " parameters / second" << std::endl;
  return 0;
}
