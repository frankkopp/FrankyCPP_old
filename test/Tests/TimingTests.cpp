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

#include <gtest/gtest.h>
#include <chrono>
#include <random>

#include "Logging.h"
#include "Bitboards.h"
#include "Position.h"

#include <boost/timer/timer.hpp>
using namespace boost::timer;

using namespace std;
using namespace Bitboards;

Position position;

class TimingTests : public ::testing::Test {
public:
  static void SetUpTestSuite() {
    NEWLINE;
    LOGGING::init();
    INIT::init();
    NEWLINE;
  }

protected:
  void SetUp() override {}

  void TearDown() override {}

  // Necessary because of function pointer use below.
  void testTiming(ostringstream &os, int rounds, int iterations, int repetitions,
                  vector<function<void(void)>> tests);
};

TEST_F(TimingTests, DISABLED_popcount) {
  ostringstream os;

  //// TESTS START
  std::function<void()> f1 = []() { popcount(DiagUpA1); };
  vector<std::function<void()>> tests;
  tests.push_back(f1);
  //// TESTS END

  testTiming(os, 5, 50, 100'000'000, tests);

  cout << os.str();
}

/**
 * Test the absolute speed of doMove, undoMove
 */
TEST_F(TimingTests, DISABLED_doMoveUndoMove) {
  ostringstream os;

  //// TESTS START
  position = Position("r3k2r/1ppqbppp/2n2n2/1B2p1B1/3p2b1/2NP1N2/1PPQPPPP/R3K2R w KQkq - 0 1");
  const Move move1 = createMove(SQ_E2, SQ_E4);
  const Move move2 = createMove(SQ_D4, SQ_E3);
  const Move move3 = createMove(SQ_D2, SQ_E3);
  const Move move4 = createMove(SQ_E8, SQ_C8);
  const Move move5 = createMove(SQ_E1, SQ_G1);

  std::function<void()> f1 = []() {
    //    string fen = position.printFen();
    //    cout << position.printBoard() << endl;
    position.doMove(move1);
    position.doMove(move2);
    position.doMove(move3);
    position.doMove(move4);
    position.doMove(move5);

    position.undoMove();
    position.undoMove();
    position.undoMove();
    position.undoMove();
    position.undoMove();
    // ASSERT_EQ(fen, position.printFen());
  };

  vector<std::function<void()>> tests;
  tests.push_back(f1);
  //// TESTS END

  testTiming(os, 5, 10, 2'000'000, tests);

  cout << os.str();
}

/**
 * Test difference for getMoves with pre rotated bb vs. on-the-fly rotated bb
 * Round  5 Test  1:  451.076.050 ns (  0,45107605 sec)
 * Round  5 Test  2:   17.723.886 ns ( 0,017723886 sec)
 */
TEST_F(TimingTests, DISABLED_rotation) {
  ostringstream os;

  //// TESTS START
  position = Position("r3k2r/1ppqbppp/2n2n2/1B2p1B1/3p2b1/2NP1N2/1PPQPPPP/R3K2R w KQkq - 0 1");

  std::function<void()> f1 = []() { Bitboards::getMovesDiagUp(SQ_D2, position.getOccupiedBB()); };
  std::function<void()> f2 = []() { Bitboards::getMovesDiagUpR(SQ_D2, position.getOccupiedBBR45()); };
  vector<std::function<void()> > tests;
  tests.push_back(f1);
  tests.push_back(f2);
  //// TESTS END

  testTiming(os, 5, 50, 10'000'000, tests);

  cout << os.str();
}

TEST_F(TimingTests, DISABLED_TThash) {
  ostringstream os;

  uint64_t* data1 = new uint64_t[2'500'000];
  uint64_t* data2 = new uint64_t[2'500'000];
  std::mt19937_64 eng1(12345);
  std::mt19937_64 eng2(12345);
  std::uniform_int_distribution<unsigned long long> distr1;
  std::uniform_int_distribution<unsigned long long> distr2;

  //// TESTS START
  std::function<void()> f1 = [&]() {
    data1[distr1(eng1) % 2'000'000] = std::chrono::high_resolution_clock::now().time_since_epoch().count();
  };
  std::function<void()> f2 = [&]() {
    data2[distr2(eng2) & 2'097'151] = std::chrono::high_resolution_clock::now().time_since_epoch().count();
  };
  vector<std::function<void()>> tests;
  tests.push_back(f1);
  tests.push_back(f2);
  //// TESTS END

  testTiming(os, 5, 50, 1'000'000, tests);

  cout << os.str();

  delete[] data1;
  delete[] data2;
}

unsigned popcount16(unsigned u) {
  u -= (u >> 1U) & 0x5555U;
  u = ((u >> 2U) & 0x3333U) + (u & 0x3333U);
  u = ((u >> 4U) + u) & 0x0F0FU;
  return (u * 0x0101U) >> 8U;
}

TEST_F(TimingTests, bitCount) {
  ostringstream os;

  std::mt19937_64 rg(12345);
  std::uniform_int_distribution<unsigned long long> randomU64;

  int i;

  uint8_t PopCnt16[1 << 16];
  // pre-computes 16-bit population counter to use in popcount(64-bit)
  for (unsigned i = 0; i < (1U << 16U); ++i)
    PopCnt16[i] = (uint8_t) popcount16(i);


  //// TESTS START
  std::function<void()> f1 = [&]() {
    union {
      Bitboard bb;
      uint16_t u[4];
    } v = {randomU64(rg)};
    i = PopCnt16[v.u[0]] + PopCnt16[v.u[1]] + PopCnt16[v.u[2]] + PopCnt16[v.u[3]];
  };
  std::function<void()> f2 = [&]() {
    i = __builtin_popcountll(randomU64(rg));
  };
  vector<std::function<void()>> tests;
  tests.push_back(f1);
  tests.push_back(f2);
  //// TESTS END

  testTiming(os, 5, 50, 10'000'000, tests);

  cout << os.str();
}

void
TimingTests::testTiming(ostringstream &os, int rounds, int iterations, int repetitions,
                        vector<function<void()>> tests) {

  nanosecond_type last = 0;

  cout.imbue(deLocale);
  os.imbue(deLocale);
  os << setprecision(9);

  os << endl;
  os << "Starting timing test: rounds=" << rounds << " iterations=" << iterations << " repetitions="
     << repetitions << endl;
  os << "======================================================================" << endl;

  // rounds
  for (int round = 1; round <= rounds; ++round) {
    cout << "Round " << round << " of " << rounds << " timing tests." << endl;
    // tests
    int testNr = 1;
    for (auto f : tests) {
      // iterations
      int i = 0;
      boost::timer::cpu_timer timer;
      timer.stop();
      while (i++ < iterations) {
        // repetitions
        timer.resume();
        for (int j = 0; j < repetitions; ++j) f();
        timer.stop();
      }

      cpu_times cpuTime = timer.elapsed();
      cpu_times avgTimes = cpu_times{cpuTime.wall / iterations, cpuTime.user / iterations,
                                     cpuTime.system / iterations};

      int percentFromLast = last ? (avgTimes.wall * 10'000) / last : 10'000;

      os << "Round " << std::setfill(' ') << setw(2) << round << " Test " << setw(2) << testNr++
         << ": " << std::setfill(' ') << setw(12) << avgTimes.wall << " ns"
         << " (" << std::setfill(' ') << setw(6) << (percentFromLast / 100) << "%)"
         << " (" << std::setfill(' ') << setw(12) << (avgTimes.wall / 1e9) << " sec)"
         << " (" << std::setfill(' ') << setw(12) << static_cast<double>(avgTimes.wall) / (repetitions * iterations) << " ns avg per test)"
         << " >> " << boost::timer::format(avgTimes, default_places);

      last = avgTimes.wall;
    }
    os << endl;
    last = 0;
  }
}



