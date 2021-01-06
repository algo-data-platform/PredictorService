#include "glog/logging.h"
#include "gtest/gtest.h"

#include "util.h"

namespace common {
class CommonUtilTest : public ::testing::Test {
 protected:
  void SetUp() override { google::InitGoogleLogging("CommonUtilTest"); }
  void TearDown() override { google::ShutdownGoogleLogging(); }
};

TEST_F(CommonUtilTest, Sanity) { EXPECT_EQ(1, 1); }

TEST_F(CommonUtilTest, SplitSlice) {
  unsigned total_num = 0;
  unsigned min_item_num_per_thread = 0;
  unsigned max_thread_num = 0;

  // test 1 : wrong min_item_num_per_thread or max_thread_num
  std::vector<common::Range> ranges;
  bool ret = split_slice(&ranges, total_num, min_item_num_per_thread, max_thread_num);
  EXPECT_FALSE(ret);

  // test 2 : 20, 5, 8, so [0,5),[5,10),[10,15),[15,20), total_num / max_thread_num + 1 < min_item_num_per_thread
  total_num = 20;
  min_item_num_per_thread = 5;
  max_thread_num = 8;
  std::vector<common::Range> ranges1;
  ret = split_slice(&ranges1, total_num, min_item_num_per_thread, max_thread_num);
  EXPECT_EQ(ranges1.size(), 4);
  EXPECT_EQ(ranges1[0].begin, 0);
  EXPECT_EQ(ranges1[0].end, 5);
  EXPECT_EQ(ranges1[1].begin, 5);
  EXPECT_EQ(ranges1[1].end, 10);
  EXPECT_EQ(ranges1[2].begin, 10);
  EXPECT_EQ(ranges1[2].end, 15);
  EXPECT_EQ(ranges1[3].begin, 15);
  EXPECT_EQ(ranges1[3].end, 20);

  // test 3 : 20, 5, 3, so [0,7),[7,14),[14,20), total_num / max_thread_num + 1 >= min_item_num_per_thread
  total_num = 20;
  min_item_num_per_thread = 5;
  max_thread_num = 3;
  std::vector<common::Range> ranges2;
  ret = split_slice(&ranges2, total_num, min_item_num_per_thread, max_thread_num);
  EXPECT_EQ(ranges2.size(), 3);
  EXPECT_EQ(ranges2[0].begin, 0);
  EXPECT_EQ(ranges2[0].end, 7);
  EXPECT_EQ(ranges2[1].begin, 7);
  EXPECT_EQ(ranges2[1].end, 14);
  EXPECT_EQ(ranges2[2].begin, 14);
  EXPECT_EQ(ranges2[2].end, 20);

  // test 4 : 4, 5, 8, so [0,4), total_num < min_item_num_per_thread
  total_num = 4;
  min_item_num_per_thread = 5;
  max_thread_num = 8;
  std::vector<common::Range> ranges3;
  ret = split_slice(&ranges3, total_num, min_item_num_per_thread, max_thread_num);
  EXPECT_EQ(ranges3.size(), 1);
  EXPECT_EQ(ranges3[0].begin, 0);
  EXPECT_EQ(ranges3[0].end, 4);
}
}  // namespace common
