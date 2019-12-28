/*
 * MIT License
 *
 * Copyright (c) 2019 Frank Kopp
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */


#include <gtest/gtest.h>
#include "../../src/Logging.h"
#include "../../src/TT.h"
#include "../../src/Position.h"

using testing::Eq;

class TT_Test : public ::testing::Test {
public:
  static void SetUpTestSuite() {
    NEWLINE;
    LOGGING::init();
    INIT::init();
    NEWLINE;
    // turn off info and below logging in the application
    spdlog::set_level(spdlog::level::trace);
  }
  
  std::shared_ptr<spdlog::logger> LOG = spdlog::get("Test_Logger");
protected:
  void SetUp() override {
    LOG->set_level(spdlog::level::trace);
  }
  
  void TearDown() override {}
};

TEST_F(TT_Test, basic) {
  LOG->info("Trying to create a TT with {:n} MB in size (default)", TT::DEFAULT_TT_SIZE);
  TT tt;
  LOG->info("Number of entries: {:n}", tt.getMaxNumberOfEntries());
  LOG->info("Number of bytes allocated: {:n}", tt.getSizeInByte());
  LOG->info("Number of entries: {:n}", tt.getNumberOfEntries());
  
  LOG->info("Trying to resize the TT with {:n} MB in size", 10);
  tt.resize(10 * TT::MB);
  LOG->info("Number of entries: {:n}", tt.getMaxNumberOfEntries());
  LOG->info("Number of bytes allocated: {:n}", tt.getSizeInByte());
  LOG->info("Number of entries: {:n}", tt.getNumberOfEntries());
  ASSERT_EQ(10 * TT::MB / TT::ENTRY_SIZE, tt.getMaxNumberOfEntries());
  ASSERT_EQ(10 * TT::MB, tt.getSizeInByte());
  ASSERT_EQ(0, tt.getNumberOfEntries());
  
  LOG->info("Trying to resize the TT with {:n} MB in size", 1'000);
  tt.resize(1'000 * TT::MB);
  LOG->info("Number of entries: {:n}", tt.getMaxNumberOfEntries());
  LOG->info("Number of bytes allocated: {:n}", tt.getSizeInByte());
  LOG->info("Number of entries: {:n}", tt.getNumberOfEntries());
  
  LOG->info("Trying to resize the TT with {:n} MB in size", 10'000);
  tt.resize(10'000 * TT::MB);
  LOG->info("Number of entries: {:n}", tt.getMaxNumberOfEntries());
  LOG->info("Number of bytes allocated: {:n}", tt.getSizeInByte());
  LOG->info("Number of entries: {:n}", tt.getNumberOfEntries());
  
  LOG->info("Trying to resize the TT with {:n} MB in size", 50'000);
  tt.resize(50'000 * TT::MB);
  LOG->info("Number of entries: {:n}", tt.getMaxNumberOfEntries());
  LOG->info("Number of bytes allocated: {:n}", tt.getSizeInByte());
  LOG->info("Number of entries: {:n}", tt.getNumberOfEntries());
  ASSERT_EQ(50'000 * TT::MB / TT::ENTRY_SIZE, tt.getMaxNumberOfEntries());
  ASSERT_EQ(50'000 * TT::MB, tt.getSizeInByte());
  ASSERT_EQ(0, tt.getNumberOfEntries());
  
  LOG->info("Trying to resize the TT with {:n} MB in size", 64);
  tt.resize(64 * TT::MB);
  LOG->info("Number of entries: {:n}", tt.getMaxNumberOfEntries());
  LOG->info("Number of bytes allocated: {:n}", tt.getSizeInByte());
  LOG->info("Number of entries: {:n}", tt.getNumberOfEntries());
  ASSERT_EQ(64 * TT::MB / TT::ENTRY_SIZE, tt.getMaxNumberOfEntries());
  ASSERT_EQ(64 * TT::MB, tt.getSizeInByte());
  ASSERT_EQ(0, tt.getNumberOfEntries());
}

TEST_F(TT_Test, zero) {
  LOG->info("Trying to create a TT with {:n} MB in size", 0);
  TT tt;
  tt.resize(0);
  LOG->info("Number of entries: {:n}", tt.getMaxNumberOfEntries());
  LOG->info("Number of bytes allocated: {:n}", tt.getSizeInByte());
  LOG->info("Number of entries: {:n}", tt.getNumberOfEntries());
}

TEST_F(TT_Test, parallelClear) {
  const int sizeInMB = 32'000;
  LOG->info("Trying to create a TT with {:n} MB in size", sizeInMB);
  TT tt = TT(sizeInMB * TT::MB);
  LOG->info("Number of entries: {:n}", tt.getMaxNumberOfEntries());
  LOG->info("Number of bytes allocated: {:n}", tt.getSizeInByte());
  LOG->info("Number of entries: {:n}", tt.getNumberOfEntries());
  ASSERT_EQ(sizeInMB * TT::MB / TT::ENTRY_SIZE, tt.getMaxNumberOfEntries());
  ASSERT_EQ(sizeInMB * TT::MB, tt.getSizeInByte());
  ASSERT_EQ(0, tt.getNumberOfEntries());
  
  tt.setThreads(4);
  tt.clear();
}

TEST_F(TT_Test, setGetBestMove) {
  TT::Entry entry(0);
  Move move = createMove("e2e4");
  LOG->info("Storing move {} into entry {}", printMoveVerbose(move), entry);
  entry = TT::setBestMove(entry, move);
  LOG->info("New entry: {} ({})", entry, TT::printBitString(entry));
  Move moveRead = TT::getBestMove(entry);
  LOG->info("Move read from entry: {}", printMoveVerbose(moveRead));
  ASSERT_EQ(move, moveRead);
  
  move = createMove("e7e5");
  LOG->info("Storing move {} into entry {}", printMoveVerbose(move), entry);
  entry = TT::setBestMove(entry, move);
  LOG->info("New entry: {} ({})", entry, TT::printBitString(entry));
  moveRead = TT::getBestMove(entry);
  LOG->info("Move read from entry: {}", printMoveVerbose(moveRead));
  ASSERT_EQ(move, moveRead);
}

TEST_F(TT_Test, setGetValue) {
  TT::Entry entry(0);
  auto val = Value(299);
  LOG->info("Storing value {} into entry {}", val, entry);
  entry = TT::setValue(entry, val);
  LOG->info("New entry: {} ({})", entry, TT::printBitString(entry));
  Value valRead = TT::getValue(entry);
  LOG->info("Value read from entry: {}", valRead);
  ASSERT_EQ(val, valRead);
  
  val = Value(-313);
  LOG->info("Storing value {} into entry {}", val, entry);
  entry = TT::setValue(entry, val);
  LOG->info("New entry: {} ({})", entry, TT::printBitString(entry));
  valRead = TT::getValue(entry);
  LOG->info("Value read from entry: {}", valRead);
  ASSERT_EQ(val, valRead);
}

TEST_F(TT_Test, setGetDepth) {
  TT::Entry entry(0);
  auto depth = Depth(5);
  LOG->info("Storing depth {} into entry {}", depth, entry);
  entry = TT::setDepth(entry, depth);
  LOG->info("New entry: {} ({})", entry, TT::printBitString(entry));
  int depthRead = TT::getDepth(entry);
  LOG->info("Depth read from entry: {}", depthRead);
  ASSERT_EQ(depth, depthRead);
  
  depth = Depth(13);
  LOG->info("Storing depth {} into entry {}", depth, entry);
  entry = TT::setDepth(entry, depth);
  LOG->info("New entry: {} ({})", entry, TT::printBitString(entry));
  depthRead = TT::getDepth(entry);
  LOG->info("Depth read from entry: {}", depthRead);
  ASSERT_EQ(depth, depthRead);
  
}

TEST_F(TT_Test, setGetAge) {
  TT::Entry entry(0);
  int age = 5;
  LOG->info("Storing age {} into entry {}", age, entry);
  entry = TT::setAge(entry, age);
  LOG->info("New entry: {} ({})", entry, TT::printBitString(entry));
  int ageRead = TT::getAge(entry);
  LOG->info("Age read from entry: {}", ageRead);
  ASSERT_EQ(age, ageRead);
  
  // increase
  LOG->info("Increasing age in entry {}", entry);
  entry = TT::increaseAge(entry);
  LOG->info("New entry: {} ({})", entry, TT::printBitString(entry));
  ageRead = TT::getAge(entry);
  LOG->info("Age read from entry: {}", ageRead);
  ASSERT_EQ(age + 1, ageRead);
  
  // decrease
  LOG->info("Decreasing age in entry {}", entry);
  entry = TT::decreaseAge(entry);
  LOG->info("New entry: {} ({})", entry, TT::printBitString(entry));
  ageRead = TT::getAge(entry);
  LOG->info("Age read from entry: {}", ageRead);
  ASSERT_EQ(age, ageRead);
  
  age = 12;
  LOG->info("Storing age {} into entry {}", age, entry);
  entry = TT::setAge(entry, age);
  LOG->info("New entry: {} ({})", entry, TT::printBitString(entry));
  ageRead = TT::getAge(entry);
  LOG->info("Age read from entry: {}", ageRead);
  ASSERT_EQ(7, ageRead);
  
  LOG->info("Resetting age in entry {}", entry);
  entry = TT::resetAge(entry);
  LOG->info("New entry: {} ({})", entry, TT::printBitString(entry));
  ageRead = TT::getAge(entry);
  LOG->info("Age read from entry: {}", ageRead);
  ASSERT_EQ(1, ageRead);
  
  // decrease form 1
  LOG->info("Decreasing age in entry {}", entry);
  entry = TT::decreaseAge(entry);
  LOG->info("New entry: {} ({})", entry, TT::printBitString(entry));
  ageRead = TT::getAge(entry);
  LOG->info("Age read from entry: {}", ageRead);
  ASSERT_EQ(0, ageRead);
  
  // decrease from 0
  LOG->info("Decreasing age in entry {}", entry);
  entry = TT::decreaseAge(entry);
  LOG->info("New entry: {} ({})", entry, TT::printBitString(entry));
  ageRead = TT::getAge(entry);
  LOG->info("Age read from entry: {}", ageRead);
  ASSERT_EQ(0, ageRead);
}

TEST_F(TT_Test, setGetType) {
  TT::Entry entry(0);
  TT::EntryType type = TT::EntryType::TYPE_EXACT;
  LOG->info("Storing type {} into entry {}", TT::str(type), entry);
  entry = TT::setType(entry, type);
  LOG->info("New entry: {} ({})", entry, TT::printBitString(entry));
  TT::EntryType typeRead = TT::getType(entry);
  LOG->info("Type read from entry: {}", TT::str(typeRead));
  ASSERT_EQ(type, typeRead);
  
  type = TT::EntryType::TYPE_ALPHA;
  LOG->info("Storing type {} into entry {}", TT::str(type), entry);
  entry = TT::setType(entry, type);
  LOG->info("New entry: {} ({})", entry, TT::printBitString(entry));
  typeRead = TT::getType(entry);
  LOG->info("Type read from entry: {}", TT::str(typeRead));
  ASSERT_EQ(type, typeRead);
}

TEST_F(TT_Test, setGetMateThreat) {
  TT::Entry entry(0);
  bool mateThreat = true;
  LOG->info("Storing mateThreat {} into entry {}", boolStr(mateThreat), entry);
  entry = TT::setMateThreat(entry, mateThreat);
  LOG->info("New entry: {} ({})", entry, TT::printBitString(entry));
  bool mateThreatRead = TT::hasMateThreat(entry);
  LOG->info("Mate threat read from entry: {}", boolStr(mateThreatRead));
  ASSERT_EQ(mateThreat, mateThreatRead);
  
  mateThreat = false;
  LOG->info("Storing mateThreat {} into entry {}", boolStr(mateThreat), entry);
  entry = TT::setMateThreat(entry, mateThreat);
  LOG->info("New entry: {} ({})", entry, TT::printBitString(entry));
  mateThreatRead = TT::hasMateThreat(entry);
  LOG->info("Mate threat read from entry: {}", boolStr(mateThreatRead));
  ASSERT_EQ(mateThreat, mateThreatRead);
}

TEST_F(TT_Test, setGetAll) {
  TT::Entry entry(0);
  Move move = createMove("e2e4");
  auto value = Value(199);
  Depth depth = Depth(5);
  int age = 3;
  TT::EntryType type = TT::EntryType::TYPE_BETA;
  bool mateThreat = true;
  
  LOG->info("Storing move {} into entry {}", printMoveVerbose(move), entry);
  entry = TT::setBestMove(entry, move);
  LOG->info("New entry: {} ({})", entry, TT::printBitString(entry));
  
  LOG->info("Storing value {} into entry {}", value, entry);
  entry = TT::setValue(entry, value);
  LOG->info("New entry: {} ({})", entry, TT::printBitString(entry));
  
  LOG->info("Storing depth {} into entry {}", depth, entry);
  entry = TT::setDepth(entry, depth);
  LOG->info("New entry: {} ({})", entry, TT::printBitString(entry));
  
  LOG->info("Storing age {} into entry {}", age, entry);
  entry = TT::setAge(entry, age);
  LOG->info("New entry: {} ({})", entry, TT::printBitString(entry));
  
  LOG->info("Storing type {} into entry {}", TT::str(type), entry);
  entry = TT::setType(entry, type);
  LOG->info("New entry: {} ({})", entry, TT::printBitString(entry));
  
  LOG->info("Storing mateThreat {} into entry {}", boolStr(mateThreat), entry);
  entry = TT::setMateThreat(entry, mateThreat);
  LOG->info("New entry: {} ({})", entry, TT::printBitString(entry));
  
  Move moveRead = TT::getBestMove(entry);
  LOG->info("Move read from entry: {}", printMoveVerbose(moveRead));
  ASSERT_EQ(move, moveRead);
  
  Value valRead = TT::getValue(entry);
  LOG->info("Value read from entry: {}", valRead);
  ASSERT_EQ(value, valRead);
  
  int depthRead = TT::getDepth(entry);
  LOG->info("Depth read from entry: {}", depthRead);
  ASSERT_EQ(depth, depthRead);
  
  int ageRead = TT::getAge(entry);
  LOG->info("Age read from entry: {}", ageRead);
  ASSERT_EQ(age, ageRead);
  
  TT::EntryType typeRead = TT::getType(entry);
  LOG->info("Type read from entry: {}", TT::str(typeRead));
  ASSERT_EQ(type, typeRead);
  
  bool mateThreatRead = TT::hasMateThreat(entry);
  LOG->info("Mate threat read from entry: {}", boolStr(mateThreatRead));
  ASSERT_EQ(mateThreat, mateThreatRead);
}

TEST_F(TT_Test, putGet) {
  TT tt(10 * TT::MB);
  Position position;
  
  tt.put(false, position.getZobristKey(), VALUE_DRAW, TT::TYPE_EXACT, Depth(4), createMove("e2e4"), false);
  ASSERT_EQ(1, tt.getNumberOfPuts());
  ASSERT_EQ(1, tt.getNumberOfEntries());

  // new
  position.doMove(createMove("e2e4"));
  tt.put(false, position.getZobristKey(), VALUE_DRAW, TT::TYPE_BETA, Depth(4), createMove("e7e5"), false);
  ASSERT_EQ(2, tt.getNumberOfPuts());
  ASSERT_EQ(2, tt.getNumberOfEntries());
  TT::Entry e = tt.get(position.getZobristKey());
  ASSERT_EQ(VALUE_DRAW, TT::getValue(e));
  ASSERT_EQ(1, tt.getNumberOfHits());

  // update with exact type
  tt.put(false, position.getZobristKey(), Value(1), TT::TYPE_EXACT, Depth(4), createMove("e7e5"), false);
  ASSERT_EQ(3, tt.getNumberOfPuts());
  ASSERT_EQ(2, tt.getNumberOfEntries());
  ASSERT_EQ(1, tt.getNumberOfUpdates());
  e = tt.get(position.getZobristKey());
  ASSERT_EQ(Value(1), TT::getValue(e));
  ASSERT_EQ(2, tt.getNumberOfHits());

  // update with greater depth
  tt.put(false, position.getZobristKey(), Value(2), TT::TYPE_EXACT, Depth(4), createMove("e7e5"), false);
  ASSERT_EQ(4, tt.getNumberOfPuts());
  ASSERT_EQ(2, tt.getNumberOfEntries());
  ASSERT_EQ(2, tt.getNumberOfUpdates());
  e = tt.get(position.getZobristKey());
  ASSERT_EQ(1, TT::getValue(e));
  ASSERT_EQ(3, tt.getNumberOfHits());

  // update (should not update value as worse quality and same depth)
  tt.put(false, position.getZobristKey(), Value(3), TT::TYPE_BETA, Depth(5), createMove("e7e5"), false);
  ASSERT_EQ(5, tt.getNumberOfPuts());
  ASSERT_EQ(2, tt.getNumberOfEntries());
  ASSERT_EQ(3, tt.getNumberOfUpdates());
  e = tt.get(position.getZobristKey());
  ASSERT_EQ(3, TT::getValue(e));
  ASSERT_EQ(4, tt.getNumberOfHits());

  // not in TT
  position.doMove(createMove("e7e5"));
  e = tt.get(position.getZobristKey());
  ASSERT_EQ(4, tt.getNumberOfHits());
  ASSERT_EQ(1, tt.getNumberOfMisses());
  
  ASSERT_EQ(5, tt.getNumberOfProbes());
}

TEST_F(TT_Test, putTest) {
  Position position;

  TT tt;
  tt.setThreads(4);
  const int newsize = 1 * TT::MB;
  tt.resize(newsize);

  const int noOfEntries = 1'000;
  LOG->info("Filling the TT with {:n} entries", noOfEntries);
  for (int i = 0; i < noOfEntries; ++i) {
    tt.put(true, i, VALUE_DRAW, TT::TYPE_EXACT, Depth(4), createMove("e2e4"), false);
  }
  LOG->info("{}", tt.str());
  LOG->info("{}", TT::str(944, tt.get(944)));
  tt.ageEntries();
  tt.ageEntries();
  tt.ageEntries();
  ASSERT_EQ(3, TT::getAge(tt.get(944)));
  LOG->info("{}", TT::str(944, tt.get(944)));
}



