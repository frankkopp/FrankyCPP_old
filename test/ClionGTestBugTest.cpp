#include <gtest/gtest.h>
using testing::Eq;

#include "types.h"
#include "TT.h"

struct Entry {
  // sorted by size to achieve smallest struct size
  // using bitfield for smallest size
  Key key = 0; // 64 bit
  Move move = MOVE_NONE; // 32 bit
  Value value = VALUE_NONE; // 16 bit signed
  Depth depth:7; // 0-127
  uint8_t age:3; // 0-7
  Value_Type type:2; // 4 values
  bool mateThreat:1; // 1-bit bool
};

TEST(SuiteName, TestName) {
  static constexpr uint64_t KB = 1024;
  static constexpr uint64_t MB = KB * KB;
  const unsigned long long int maxNumberOfEntries =
    1ULL << static_cast<uint64_t>(std::floor(std::log2(1'024 * MB / sizeof(Entry))));
  std::cout << maxNumberOfEntries << std::endl;
  Entry* _data = new Entry[maxNumberOfEntries];
}

TEST(SuiteName, TestName2) {
  TT tt = TT(1'024 * TT::MB);

}
