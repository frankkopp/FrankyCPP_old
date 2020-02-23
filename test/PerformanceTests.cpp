/*
 * MIT License
 *
 * Copyright (c) 2018 Frank Kopp
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
#include <chrono>
#include "types.h"
#include "Logging.h"
#include "Position.h"
#include "TT.h"
#include "Evaluator.h"
#include "Search.h"

#include <gtest/gtest.h>
#include <boost/timer/timer.hpp>

using namespace boost::timer;
using testing::Eq;

class PerformanceTests : public ::testing::Test {
public:
  static void SetUpTestSuite() {
    NEWLINE;
    INIT::init();
    NEWLINE;
    Logger::get().TEST_LOG->set_level(spdlog::level::debug);
  }

protected:
  void SetUp() override {}
  void TearDown() override {}
};

/*
23:41 24.1.2020 CYGWIN
WALL Time: 11.494.326.800 ns (11.494327 sec)
CPU  Time: 11.468.750.000 ns (11.468750 sec)
Move/Undo per sec: 43.596.730 pps
Move/undo time:    22 ns

15.2. UBUNTU WSL
WALL Time: 9.111.689.100 ns (9.111689 sec)
CPU  Time: 9.110.000.000 ns (9.110000 sec)
Move/Undo per sec: 54.884.742 pps
Move/undo time:    18 ns

23.2. MSVC Compiler
WALL Time: 10.216.573.000 ns (10.216573 sec)
CPU  Time: 10.187.500.000 ns (10.187500 sec)
Move/Undo per sec: 49.079.754 pps
Move/undo time:    20 ns
 */
TEST_F(PerformanceTests, Position_PPS) {

  std::string fen;
  const uint64_t iterations = 50'000'000;
  const uint64_t rounds = 5;

  for (uint64_t round = 0; round < rounds; ++round) {
    fprintln("ROUND: {}", round + 1);
    Position position;
    auto timer = cpu_timer();
    for (uint64_t i = 0; i < iterations; ++i) {
      position.doMove(createMove<NORMAL>(SQ_E2, SQ_E4));
      position.doMove(createMove<NORMAL>(SQ_D7, SQ_D5));
      position.doMove(createMove<NORMAL>(SQ_E4, SQ_D5));
      position.doMove(createMove<NORMAL>(SQ_D8, SQ_D5));
      position.doMove(createMove<NORMAL>(SQ_B1, SQ_C3));
      position.undoMove();
      position.undoMove();
      position.undoMove();
      position.undoMove();
      position.undoMove();
    }
    timer.stop();
    const nanosecond_type cpuTime = timer.elapsed().user + timer.elapsed().system;
    fprintln("WALL Time: {:n} ns ({:3f} sec)", timer.elapsed().wall, static_cast<double>(timer.elapsed().wall) / nanoPerSec);
    fprintln("CPU  Time: {:n} ns ({:3f} sec)", cpuTime, static_cast<double>(cpuTime) / nanoPerSec);
    fprintln("Move/Undo per sec: {:n} pps", (10 * iterations * nanoPerSec) / cpuTime);
    fprintln("Move/undo time:    {:n} ns", cpuTime / (iterations * 10));
    NEWLINE;
  }
}

/**
23:50 24.1.2020 CYGWIN
Move generated: 86.000.000 in 3.051670 seconds
Move generated per second: 28.181.293
GCC9.2
Move generated: 86.000.000 in 2.374672 seconds
Move generated per second: 36.215.520

23.2. UBUNTU WSL
Move generated: 86.000.000 in 2.088923 seconds
Move generated per second: 41.169.532

23.2. MSVC
Move generated: 86.000.000 in 4.054877 seconds
Move generated per second: 21.209.029
 */
TEST_F(PerformanceTests, MoveGeneration_MPS) {
  std::string fen;
  MoveGenerator mg;
  Position position;
  uint64_t generatedMoves = 0, sum = 0;

  // 86 pseudo legal moves (incl. castling over attacked square)
  fen = "r3k2r/1ppn3p/2q1q1n1/4P3/2q1Pp2/B5R1/pbp2PPP/1R4K1 b kq e3";
  position = Position(fen);

  const MoveList* moves = mg.generatePseudoLegalMoves<MoveGenerator::GENALL>(position);
  Move killer1 = moves->at(35);
  Move killer2 = moves->at(85);

  fprintln("Move Gen Performance Test started.");

  auto start = std::chrono::high_resolution_clock::now();
  auto finish = std::chrono::high_resolution_clock::now();

  const uint64_t rounds = 15;
  const int iterations = 1'000'000;
  for (uint64_t round = 0; round < rounds; ++round) {
    fprintln("ROUND: {}", round + 1);
    sum = generatedMoves = 0;
    for (int i = 0; i < iterations; i++) {
      int j = 0;
      mg.reset();
      mg.storeKiller(killer1, 2);
      mg.storeKiller(killer2, 2);
      start = std::chrono::high_resolution_clock::now();
      while (mg.getNextPseudoLegalMove<MoveGenerator::GENALL>(position) != MOVE_NONE) j++;
      finish = std::chrono::high_resolution_clock::now();
      generatedMoves += j;
      sum += std::chrono::duration_cast<std::chrono::nanoseconds>(finish - start).count();
      ASSERT_EQ(86, j);
    }
    const double sec = double(sum) / nanoPerSec;
    uint64_t mps = generatedMoves / sec;
    fprintln("Move generated: {:n} in {:f} seconds", generatedMoves, sec);
    fprintln("Move generated per second: {:n}", mps);
    NEWLINE;
  }
  SUCCEED();
}

/*
 * 23:54 24.1.2020 CYGWIN
 * Leaf nodes per sec: 3.989.689
 * 
 * 15.2. UBUNTU WSL
 * Leaf nodes per sec: 8.811.450
 */
TEST_F(PerformanceTests, Perft_NPS) {
  Logger::get().SEARCH_LOG->set_level(spdlog::level::warn);
  
  int DEPTH = 6;

  uint64_t perftResults[] = {0,
                             20,             // 1
                             400,            // 2
                             8'902,          // 3
                             197'281,        // 4
                             4'865'609,      // 5
                             119'060'324,    // 6
                             3'195'901'860}; // 7

  Search search;
  SearchLimits searchLimits;
  Position position;
  searchLimits.setPerft(true);
  searchLimits.setDepth(DEPTH);
  search.startSearch(position, searchLimits);
  search.waitWhileSearching();
  LOG__INFO(Logger::get().TEST_LOG, "Leaf nodes per sec: {:n}",
            (search.getSearchStats().leafPositionsEvaluated * 1'000) /
            search.getSearchStats().lastSearchTime);
  LOG__INFO(Logger::get().TEST_LOG, "Leaf nodes:         {:n}",
            search.getSearchStats().leafPositionsEvaluated);
  ASSERT_EQ(perftResults[DEPTH],
            search.getSearchStats().leafPositionsEvaluated);
}

/**
 * 23:50 24.1.2020 CYGWIN
 * Run time      : 1.765.625.000 ns (56.637.168 put/probes per sec)
 *
 * 15.2. UBUNTU WSL
 * Run time      : 1.660.000.000 ns (60.240.963 put/probes per sec)
 */
TEST_F(PerformanceTests, TT_PPS) {
  std::random_device rd;
  std::default_random_engine rg1(rd());
  std::uniform_int_distribution<unsigned long long> randomKey(1, 10'000'000);
  std::uniform_int_distribution<unsigned short> randomDepth(0, DEPTH_MAX);
  std::uniform_int_distribution<int> randomValue(VALUE_MIN, VALUE_MAX);
  std::uniform_int_distribution<int> randomAlpha(VALUE_MIN, 0);
  std::uniform_int_distribution<unsigned int> randomBeta(0, VALUE_MAX);
  std::uniform_int_distribution<unsigned short> randomType(1, 3);

  TT tt(1'024);

  fprintln("Start perft test for TT...");
  fprintln("TT Stats: {:s}", tt.str());

  const Move move = createMove("e2e4");

  const int rounds = 5;
  const int iterations = 10'000'000;
  for (int j = 0; j < rounds; ++j) {
    // puts
    cpu_timer timer;
    for (int i = 0; i < iterations; ++i) {
      const unsigned long long int key = randomKey(rg1);
      auto depth = static_cast<Depth>(randomDepth(rg1));
      auto value = static_cast<Value>(randomValue(rg1));
      auto type = static_cast<Value_Type>(randomType(rg1));
      tt.put(key, depth, move, value, type, false, true);
    }
    // probes
    for (int i = 0; i < iterations; ++i) {
      const unsigned long long int key = randomKey(rg1);
      tt.probe(key);
    }
    timer.stop();
    auto time = timer.elapsed().user + timer.elapsed().system;
    fprintln("TT Statistics : {:s}", tt.str());
    fprintln("Run time      : {:n} ns ({:n} put/probes per sec)", time, (rounds * 2 * iterations * nanoPerSec) / time);
    fprintln("Run time      : {} ", timer.format());
    fprintln("");
  }
}

/*
 * 25.1.2020 00:22 CYGWIN
 * EPS:       4.812.030 eps
 * TPE:       207 ns
 *
 * 15.2. UBUNTU WSL
 * EPS:       5.464.480 eps
 * TPE:       183 ns
 */
TEST_F(PerformanceTests, Evaluator_EPS) {
  std::string fen;
  Position position;
  const int nano_sec = 1'000'000'000;

  fen = "r3k2r/1ppn3p/2q1q1nb/4P2N/2q1Pp2/B5RP/pbp2PP1/1R4K1 w kq - 0 1";
  position = Position(fen);
  const uint64_t iterations = 50'000'000;
  const uint64_t rounds = 5;

  Evaluator evaluator;
  evaluator.config.USE_MATERIAL = true;
  evaluator.config.USE_POSITION = true;
  evaluator.config.USE_PAWNEVAL = true;
  evaluator.config.USE_PAWN_TABLE = true;
  evaluator.config.PAWN_TABLE_SIZE = 2'097'152;
  evaluator.config.USE_CHECK_BONUS = true;
  evaluator.config.USE_MOBILITY = true;
  evaluator.config.USE_PIECE_BONI = true;
  evaluator.config.USE_KING_CASTLE_SAFETY = true;
  evaluator.resizePawnTable(evaluator.config.PAWN_TABLE_SIZE);

  for (uint64_t round = 0; round < rounds; ++round) {
    fprintln("ROUND: {}", round + 1);
    auto timer = cpu_timer();
    for (uint64_t i = 0; i < iterations; ++i) {
      evaluator.evaluate(position);
    }
    timer.stop();
    const nanosecond_type cpuTime = timer.elapsed().user + timer.elapsed().system;
    fprintln("WALL Time: {:n} ns ({:3f} sec)", timer.elapsed().wall, static_cast<double>(timer.elapsed().wall) / nano_sec);
    fprintln("CPU  Time: {:n} ns ({:3f} sec)", cpuTime, static_cast<double>(cpuTime) / nano_sec);
    fprintln("EPS:       {:n} eps", (iterations * nano_sec) / cpuTime);
    fprintln("TPE:       {:n} ns", cpuTime / iterations);
    NEWLINE;
  }
}

/*
 * 25.1.2020 cygwin
 * Nodes: 55.967.744 Time: 30.047 ms NPS: 1.862.673
 *
 * 15.2. UBUNTU WSL
 * Nodes: 57.690.518 Time: 30.005 ms NPS: 1.922.696
 */
TEST_F(PerformanceTests, Search_NPS) {
  Logger::get().TT_LOG->set_level(spdlog::level::debug);
  Logger::get().SEARCH_LOG->set_level(spdlog::level::warn);
  Search search;
  SearchLimits searchLimits;
  Position position;

  search.setHashSize(1'024);
  searchLimits.setMoveTime(30'000);

  search.startSearch(position, searchLimits);
  search.waitWhileSearching();

  const uint64_t nps = (search.getSearchStats().nodesVisited * 1'000) /
                       search.getSearchStats().lastSearchTime;

  LOG__INFO(Logger::get().TEST_LOG, "Nodes: {:n} Time: {:n} ms NPS: {:n}",
            search.getSearchStats().nodesVisited,
            search.getSearchStats().lastSearchTime,
            nps);

  EXPECT_LT(1'800'000, nps);
}