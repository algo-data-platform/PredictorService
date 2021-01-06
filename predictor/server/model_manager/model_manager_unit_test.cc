#include "glog/logging.h"
#include "gtest/gtest.h"

#include "predictor/server/model_manager/model_manager.h"

DEFINE_int32(example_snapshot_frequency, 0, "predictor example_snapshot copy 1/n");
DEFINE_int64(concurrency_cpu_thread_num, 16, "num of concurrency threads for large num of items");
DEFINE_int64(request_cpu_thread_num, 8, "num of concurrency threads for processing requests");
DEFINE_int64(min_items_per_thread, 20, "minimum num of items assigned to cpu thread");
DEFINE_int32(fea_extract_snapshot_frequency, 0, "predictor example_snapshot copy 1/n");

namespace predictor {

class ModelManagerTest : public ::testing::Test {
 protected:
  void SetUp() override { google::InitGoogleLogging("ModelManagerTest"); }
  void TearDown() override { google::ShutdownGoogleLogging(); }
};

TEST_F(ModelManagerTest, Sanity) { EXPECT_EQ(1, 1); }

TEST_F(ModelManagerTest, shouldDowngradeModel) {
  ModelManager model_manager;
  auto model_downgrade_percent_map_ptr = std::make_shared<std::map<std::string, int>>();
  (*model_downgrade_percent_map_ptr)["model_a"] = 0;
  (*model_downgrade_percent_map_ptr)["model_b"] = 100;
  (*model_downgrade_percent_map_ptr)["model_d"] = 20;
  (*model_downgrade_percent_map_ptr)["model_e"] = 70;
  (*model_downgrade_percent_map_ptr)["model_f"] = 90;
  model_manager.set_model_downgrade_percent_map(model_downgrade_percent_map_ptr);

  std::string model_full_name;
  bool res;
  {
    // case 1 : 降级0，100%不命中
    model_full_name = "model_a";
    int predict_num = 100;
    int expect_num = 100;
    int not_hit_num = 0;
    for (int i = 0; i < predict_num; i ++) {
      res = model_manager.shouldDowngradeModel(model_full_name);
      if (false == res) {
        not_hit_num++;
      }
    }
    EXPECT_EQ(not_hit_num, expect_num);
  }

  {
    // case 2 : 降级100，100%命中
    model_full_name = "model_b";
    int predict_num = 100;
    int expect_num = 100;
    int hit_num = 0;
    for (int i = 0; i < predict_num; i ++) {
      res = model_manager.shouldDowngradeModel(model_full_name);
      if (true == res) {
        hit_num++;
      }
    }
    EXPECT_EQ(hit_num, expect_num);
  }

  {
    // case 3 : 模型未配置降级，100%不命中
    model_full_name = "model_c";
    int predict_num = 100;
    int expect_num = 100;
    int not_hit_num = 0;
    for (int i = 0; i < predict_num; i ++) {
      res = model_manager.shouldDowngradeModel(model_full_name);
      if (false == res) {
        not_hit_num++;
      }
    }
    EXPECT_EQ(not_hit_num, expect_num);
  }

  {
    // case 4 : 降级20，大概20%命中
    model_full_name = "model_d";
    int predict_num = 1000;
    int expect_num = 200;
    int hit_num = 0;
    for (int i = 0; i < predict_num; i ++) {
      res = model_manager.shouldDowngradeModel(model_full_name);
      if (true == res) {
        hit_num++;
      }
    }
    double diff_percentage = std::abs(static_cast<double>(hit_num) - static_cast<double>(expect_num)) /
      static_cast<double>(predict_num);
    EXPECT_LT(diff_percentage, 0.1);
  }

  {
    // case 5 : 降级70，大概70%命中
    model_full_name = "model_e";
    int predict_num = 1000;
    int expect_num = 700;
    int hit_num = 0;
    for (int i = 0; i < predict_num; i ++) {
      res = model_manager.shouldDowngradeModel(model_full_name);
      if (true == res) {
        hit_num++;
      }
    }
    double diff_percentage = std::abs(static_cast<double>(hit_num) - static_cast<double>(expect_num)) /
      static_cast<double>(predict_num);
    EXPECT_LT(diff_percentage, 0.1);
  }

  {
    // case 6 : 降级90，大概90%命中
    model_full_name = "model_f";
    int predict_num = 1000;
    int expect_num = 900;
    int hit_num = 0;
    for (int i = 0; i < predict_num; i ++) {
      res = model_manager.shouldDowngradeModel(model_full_name);
      if (true == res) {
        hit_num++;
      }
    }
    double diff_percentage = std::abs(static_cast<double>(hit_num) - static_cast<double>(expect_num)) /
      static_cast<double>(predict_num);
    EXPECT_LT(diff_percentage, 0.1);
  }
}

}  // namespace predictor
