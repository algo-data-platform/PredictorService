#include <chrono>  // NOLINT
#include <iostream>
#include <thread>  // NOLINT

#include "folly/Singleton.h"
#include "folly/init/Init.h"
#include "gflags/gflags.h"
#include "glog/logging.h"
#include "common/file/file_util.h"

#include "gtest/gtest.h"

namespace common {

class FileUtilTest : public ::testing::Test {
 protected:
  void SetUp() override {}
  void TearDown() override {}
};

TEST_F(FileUtilTest, Sanity) { EXPECT_EQ(1, 1); }

}  // namespace common

int main(int argc, char** argv) {
  folly::init(&argc, &argv);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
