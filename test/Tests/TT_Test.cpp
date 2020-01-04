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

#include <random>
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
    spdlog::set_level(spdlog::level::debug);
  }

  std::shared_ptr<spdlog::logger> LOG = spdlog::get("Test_Logger");
protected:
  void SetUp() override {
    LOG->set_level(spdlog::level::debug);
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
  ASSERT_EQ(524'288, tt.getMaxNumberOfEntries());
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
  ASSERT_EQ(2'147'483'648, tt.getMaxNumberOfEntries());
  ASSERT_EQ(0, tt.getNumberOfEntries());

  LOG->info("Trying to resize the TT with {:n} MB in size", 64);
  tt.resize(64 * TT::MB);
  LOG->info("Number of entries: {:n}", tt.getMaxNumberOfEntries());
  LOG->info("Number of bytes allocated: {:n}", tt.getSizeInByte());
  LOG->info("Number of entries: {:n}", tt.getNumberOfEntries());
  ASSERT_EQ(4'194'304, tt.getMaxNumberOfEntries());
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
  ASSERT_EQ(1'073'741'824, tt.getMaxNumberOfEntries());
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

TEST_F(TT_Test, put) {
  std::random_device rd;
  std::mt19937_64 rg(rd());
  std::uniform_int_distribution<uint64_t> randomKey;

  TT tt(10 * TT::MB);

  uint64_t collisionDistance = tt.maxNumberOfEntries;

  const Key key1 = randomKey(rg);
  const Key key2 = key1 + 13; // different bucket
  const Key key3 = key1 + collisionDistance; // same bucket - collision
  const Key key4 = key3 + collisionDistance; // same bucket - collision
  const Key key5 = key4 + collisionDistance; // same bucket - collision
  const Key key6 = key5 + collisionDistance; // same bucket - collision
  const Key key7 = key6 + collisionDistance; // same bucket - collision
  const Key key8 = key7 + collisionDistance; // same bucket - collision

  // new entry in empty bucket at pos 0
  tt.put(key1, Value(101), TT::TYPE_EXACT, Depth(6), createMove("e2e4"), false);
  ASSERT_EQ(1, tt.getNumberOfPuts());
  ASSERT_EQ(1, tt.getNumberOfEntries());
  ASSERT_EQ(0, tt.getNumberOfUpdates());
  ASSERT_EQ(0, tt.getNumberOfCollisions());
  ASSERT_EQ(0, tt.getNumberOfOverwrites());
  ASSERT_EQ(tt._keys[key1], key1);
  ASSERT_EQ(TT::getValue(tt._data[key1]), Value(101));

  // new entry in empty bucket at pos 0
  tt.put(key2, Value(102), TT::TYPE_EXACT, Depth(5), createMove("e2e4"), false);
  ASSERT_EQ(2, tt.getNumberOfPuts());
  ASSERT_EQ(2, tt.getNumberOfEntries());
  ASSERT_EQ(0, tt.getNumberOfUpdates());
  ASSERT_EQ(0, tt.getNumberOfCollisions());
  ASSERT_EQ(0, tt.getNumberOfOverwrites());
  ASSERT_EQ(tt._keys[key2], key2);
  ASSERT_EQ(TT::getValue(tt._data[key2]), Value(102));
  ASSERT_EQ(TT::getDepth(tt._data[key2]), Value(5));


  // new entry in bucket at pos 1 (collision)
  tt.put(key3, Value(103), TT::TYPE_EXACT, Depth(4), createMove("e2e4"), false);
  ASSERT_EQ(3, tt.getNumberOfPuts());
  ASSERT_EQ(3, tt.getNumberOfEntries());
  ASSERT_EQ(0, tt.getNumberOfUpdates());
  ASSERT_EQ(0, tt.getNumberOfCollisions());
  ASSERT_EQ(0, tt.getNumberOfOverwrites());
  ASSERT_EQ(tt._keys[key3], key3);
  ASSERT_EQ(TT::getValue(tt._data[key3]), Value(103));


  // new entry in bucket at pos 2 (collision)
  tt.put(key4, Value(104), TT::TYPE_EXACT, Depth(3), createMove("e2e4"), false);
  ASSERT_EQ(4, tt.getNumberOfPuts());
  ASSERT_EQ(4, tt.getNumberOfEntries());
  ASSERT_EQ(0, tt.getNumberOfUpdates());
  ASSERT_EQ(0, tt.getNumberOfCollisions());
  ASSERT_EQ(0, tt.getNumberOfOverwrites());
  ASSERT_EQ(tt._keys[key4], key4);
  ASSERT_EQ(TT::getValue(tt._data[key4]), Value(104));


  // new entry in bucket at pos 3 (collision)
  tt.put(key5, Value(105), TT::TYPE_EXACT, Depth(5), createMove("e2e4"), false);
  ASSERT_EQ(5, tt.getNumberOfPuts());
  ASSERT_EQ(5, tt.getNumberOfEntries());
  ASSERT_EQ(0, tt.getNumberOfUpdates());
  ASSERT_EQ(0, tt.getNumberOfCollisions());
  ASSERT_EQ(0, tt.getNumberOfOverwrites());
  ASSERT_EQ(tt._keys[key5], key5);
  ASSERT_EQ(TT::getValue(tt._data[key5]), Value(105));

  // REPLACE entry in bucket at pos 2 (collision) as pos has lowest depth (3)
  tt.put(key6, Value(206), TT::TYPE_EXACT, Depth(4), createMove("e2e4"), false);
  ASSERT_EQ(6, tt.getNumberOfPuts());
  ASSERT_EQ(5, tt.getNumberOfEntries());
  ASSERT_EQ(0, tt.getNumberOfUpdates());
  ASSERT_EQ(1, tt.getNumberOfCollisions());
  ASSERT_EQ(1, tt.getNumberOfOverwrites());
  ASSERT_EQ(tt._keys[key6], key6);
  ASSERT_EQ(TT::getValue(tt._data[key6]), Value(206));

  // REPLACE entry in bucket at pos 2 (collision) as pos has lowest depth (4) and is last of 2 entries with this depth
  tt.put(key7, Value(207), TT::TYPE_EXACT, Depth(4), createMove("e2e4"), false);
  ASSERT_EQ(7, tt.getNumberOfPuts());
  ASSERT_EQ(5, tt.getNumberOfEntries());
  ASSERT_EQ(0, tt.getNumberOfUpdates());
  ASSERT_EQ(2, tt.getNumberOfCollisions());
  ASSERT_EQ(2, tt.getNumberOfOverwrites());
  ASSERT_EQ(tt._keys[key7], key7);
  ASSERT_EQ(TT::getValue(tt._data[key7]), Value(207));

  // age first entry with depth 5
  tt._data[key1] = TT::increaseAge(tt._data[key1]);

  // REPLACE entry in bucket at pos 0 (collision) as it is oldest
  tt.put(key8, Value(208), TT::TYPE_EXACT, Depth(4), createMove("e2e4"), false);
  ASSERT_EQ(8, tt.getNumberOfPuts());
  ASSERT_EQ(5, tt.getNumberOfEntries());
  ASSERT_EQ(0, tt.getNumberOfUpdates());
  ASSERT_EQ(3, tt.getNumberOfCollisions());
  ASSERT_EQ(3, tt.getNumberOfOverwrites());
  ASSERT_EQ(tt._keys[key8], key8);
  ASSERT_EQ(TT::getValue(tt._data[key8]), Value(208));

}


TEST_F(TT_Test, get) {
  std::random_device rd;
  std::mt19937_64 rg(rd());
  std::uniform_int_distribution<uint64_t> randomKey;

  TT tt(10 * TT::MB);

  uint64_t collisionDistance = tt.maxNumberOfEntries;

  const Key key1 = randomKey(rg);
  const Key key2 = key1 + 13; // different bucket
  const Key key3 = key1 + collisionDistance; // same bucket - collision
  const Key key4 = key1 + 17;

  // new entry in empty slot
  tt.put(key1, Value(101), TT::TYPE_EXACT, Depth(6), createMove("e2e4"), false);
  TT::Entry e1 = tt.getEntry(key1);
  ASSERT_EQ(101, TT::getValue(e1));

  // new entry in empty slote
  tt.put(key2, Value(102), TT::TYPE_EXACT, Depth(5), createMove("e2e4"), false);
  TT::Entry e2 = tt.getEntry(key2);
  ASSERT_EQ(102, TT::getValue(e2));

  // new entry in occupoied slot
  tt.put(key3, Value(103), TT::TYPE_EXACT, Depth(7), createMove("e2e4"), false);
  TT::Entry e3 = tt.getEntry(key3);
  ASSERT_EQ(103, TT::getValue(e3));

  TT::Entry e4 = tt.getEntry(key4); // not in TT
  ASSERT_EQ(0, e4);

}

TEST_F(TT_Test, probe) {
  std::random_device rd;
  std::mt19937_64 rg(rd());
  std::uniform_int_distribution<uint64_t> randomKey;

  TT tt(10 * TT::MB);

  const Key key1 = randomKey(rg);
  const Key key2 = key1 + 13; // different bucket
  const Key key3 = key1 + 17; // same bucket - collision

  tt.put(key1, Value(101), TT::TYPE_EXACT, Depth(6), createMove("e2e4"), false);
  tt.put(key2, Value(102), TT::TYPE_ALPHA, Depth(5), createMove("e2e4"), false);
  tt.put(key3, Value(103), TT::TYPE_BETA, Depth(4), createMove("e2e4"), false);

  Value ttValue = VALUE_NONE;
  Move ttMove = MOVE_NONE;

  TT::Entry beforeProbe = tt.getEntry(key1);
  TT::Result r = tt.probe(key1, Depth(5), Value(-1000), Value(1000), ttValue, ttMove, false);
  const TT::Entry afterProbe = tt.getEntry(key1);
  ASSERT_EQ(TT::getAge(beforeProbe) - 1, TT::getAge(afterProbe)); // has entry aged?
  ASSERT_EQ(TT::TT_HIT, r);

  // TT entry has lower depth
  r = tt.probe(key1, Depth(7), Value(-1000), Value(1000), ttValue, ttMove, false);
  ASSERT_EQ(TT::TT_MISS, r);

  // TT entry was alpha within of bounds - MISS
  r = tt.probe(key2, Depth(4), Value(-1000), Value(1000), ttValue, ttMove, false);
  ASSERT_EQ(TT::TT_MISS, r);

  // TT entry was alpha and value < alpha
  r = tt.probe(key2, Depth(4), Value(103), Value(1000), ttValue, ttMove, false);
  ASSERT_EQ(TT::TT_HIT, r);

  // TT entry was alpha and value < alpha but PV
  r = tt.probe(key2, Depth(4), Value(103), Value(1000), ttValue, ttMove, true);
  ASSERT_EQ(TT::TT_MISS, r);

  // TT entry was beta within of bounds - MISS
  r = tt.probe(key3, Depth(4), Value(-1000), Value(1000), ttValue, ttMove, false);
  ASSERT_EQ(TT::TT_MISS, r);

  // TT entry was beta and value > beta - HIT
  r = tt.probe(key3, Depth(4), Value(-1000), Value(102), ttValue, ttMove, false);
  ASSERT_EQ(TT::TT_HIT, r);

  // TT entry was beta and value > beta but PV - MISS
  r = tt.probe(key3, Depth(4), Value(-1000), Value(102), ttValue, ttMove, true);
  ASSERT_EQ(TT::TT_MISS, r);

}

TEST_F(TT_Test, tt_perft) {
  std::random_device rd;
  std::default_random_engine rg1(rd());
  std::uniform_int_distribution<uint64_t> randomKey(1, 10'000'000);
  std::uniform_int_distribution<u_int8_t> randomDepth(0, DEPTH_MAX);
  std::uniform_int_distribution<int16_t> randomValue(VALUE_MIN, VALUE_MAX);
  std::uniform_int_distribution<int16_t> randomAlpha(VALUE_MIN, 0);
  std::uniform_int_distribution<int16_t> randomBeta(0, VALUE_MAX);
  std::uniform_int_distribution<u_int8_t> randomType(1, 3);

  TT tt(64 * TT::MB);
  tt.setThreads(4);

  Value ttValue = VALUE_NONE;
  Move ttMove = MOVE_NONE;

  fprintln("Start perft test for TT...");

  auto start = std::chrono::high_resolution_clock::now();

  const int rounds = 10;
  const int iterations = 1'000'000;
  for (int j = 0; j < rounds; ++j) {
    // puts
    for (int i = 0; i < iterations; ++i) {
      tt.put(true,
             randomKey(rg1),
             static_cast<Value>(randomValue(rg1)),
             static_cast<TT::EntryType>(randomType(rg1)),
             static_cast<Depth>(randomDepth(rg1)),
             createMove("e2e4"),
             false);
    }
    // probes
    for (int i = 0; i < iterations; ++i) {
      tt.probe(randomKey(rg1),
               static_cast<Depth>(randomDepth(rg1)),
               static_cast<Value>(randomAlpha(rg1)),
               static_cast<Value>(randomBeta(rg1)),
               ttValue,
               ttMove,
               false);
    }
    tt.ageEntries();
  }

  auto finish = std::chrono::high_resolution_clock::now();
  auto time = std::chrono::duration_cast<std::chrono::milliseconds>(finish - start).count();

  fprintln("Number of max entries: {:n}", tt.getMaxNumberOfEntries());
  fprintln("");
  fprintln("Number of puts:        {:n}", tt.getNumberOfPuts());
  fprintln("Number of entries:     {:n}", tt.getNumberOfEntries());
  fprintln("Number of updates      {:n}", tt.getNumberOfUpdates());
  fprintln("Number of collisions:  {:n}", tt.getNumberOfCollisions());
  fprintln("Number of overwrites:  {:n}", tt.getNumberOfOverwrites());
  fprintln("");
  fprintln("Number of probes:      {:n}", tt.getNumberOfProbes());
  fprintln("Number of hits:        {:n}", tt.getNumberOfHits());
  fprintln("Number of misses:      {:n}", tt.getNumberOfMisses());
  fprintln("");
  fprintln("Run time:              {:n} ms ({:n} put/probes per sec)",
           time, (rounds * 2 * iterations * 1000ULL) / time);
  fprintln("");

}



