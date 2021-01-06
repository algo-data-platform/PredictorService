#include <string>
#include <iostream>
#include "common/hash.h"
#include "gtest/gtest.h"

TEST(BasicTest, CityHash) {
  using namespace std;
  {
    string str = "helloworld";
    auto key = CityHash64(str.c_str(), str.size());
    cout << key << endl;
  }
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
