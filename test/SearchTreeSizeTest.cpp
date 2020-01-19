/*
 * MIT License
 *
 * Copyright (c) 2018-2020 Frank Kopp
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

#include <utility>
#include "Logging.h"
#include "Search.h"
#include "SearchConfig.h"
#include "Engine.h"
#include "Test_Fens.h"

using testing::Eq;

class SearchTreeSizeTest : public ::testing::Test {
public:
  static constexpr int DEPTH = 10;
  static constexpr int NUMBER_OF_FENS = 20;

  const uint64_t* ptrToSpecial = nullptr;

  struct SingleTest {
    std::string name = "";
    uint64_t nodes = 0;
    uint64_t nps = 0;
    uint64_t time = 0;
    uint64_t special = 0;
    Move move = MOVE_NONE;
    Value value = VALUE_NONE;
    std::string pv = "";
  };

  struct Result {
    std::string fen = "";
    std::vector<SingleTest> tests{};

    explicit Result(std::string fen) : fen(std::move(fen)) {};
  };

  struct TestSums {
    uint64_t sumCounter{};
    uint64_t sumNodes{};
    uint64_t sumNps{};
    uint64_t sumTime{};
    uint64_t special{};
  };

  static void SetUpTestSuite() {
    NEWLINE;
    LOGGING::init();
    INIT::init();
    NEWLINE;

    // turn off info and below logging in the application
    spdlog::set_level(spdlog::level::info);
  }

  std::shared_ptr<spdlog::logger> LOG = spdlog::get("Test_Logger");

protected:

  void SetUp() override {}

  void TearDown() override {}

  Result featureMeasurements(int depth, const std::string &fen);
  SingleTest measureTreeSize(Search &search, const Position &position, SearchLimits searchLimits,
                             const std::string &featureName);

};

TEST_F(SearchTreeSizeTest, size_test) {

  LOG__INFO(LOG, "Start SIZE Test for depth {}", DEPTH);

  std::vector<std::string> fens = Test_Fens::getFENs();
  std::vector<Result> results{};

  // turn off info and below logging in the application
  spdlog::set_level(spdlog::level::debug);

  results.reserve(fens.size());
  auto iterEnd = NUMBER_OF_FENS > fens.size() ? fens.end() : fens.begin() + NUMBER_OF_FENS;
  for (auto fen = fens.begin(); fen != iterEnd; ++fen) {
    results.push_back(featureMeasurements(DEPTH, *fen));
  }

  spdlog::set_level(spdlog::level::trace);

  // Print result
  // @formatter:off
  NEWLINE;
  fmt::print("################## RESULTS for depth {} ##########################\n", DEPTH);
  NEWLINE;
  fmt::print("{:<15s} | {:>6s} | {:>8s} | {:>15s} | {:>12s} | {:>12s} | {:>12s} | {} | {}\n", "Test Name",
             "Move", "Value", "Nodes", "Nps", "Time", "Special", "PV", "Fen");
  println("-----------------------------------------------------------------------"
          "-----------------------------------------------------------------------");
  // @formatter:on

  setlocale(LC_NUMERIC, "de_DE.UTF-8");
  std::map<std::string, TestSums> sums{};

  for (const Result &result : results) {
    for (const SingleTest &test : result.tests) {
      sums[test.name].sumCounter++;
      sums[test.name].sumNodes += test.nodes;
      sums[test.name].sumNps += test.nps;
      sums[test.name].sumTime += test.time;
      sums[test.name].special += test.special;

      fmt::print("{:<15s} | {:>6s} | {:>8d} | {:>15n} | {:>12n} | {:>12n} | {:>12n} | {} | {}  \n",
                 test.name.c_str(), printMove(test.move).c_str(), test.value, test.nodes, test.nps,
                 test.time, test.special, test.pv.c_str(), result.fen.c_str());
    }
    fmt::print("\n");
  }

  NEWLINE;

  for (auto &sum : sums) {
    fmt::print("Test: {:<12s}  Nodes: {:>16n}  Nps: {:>16n}  Time: {:>16n} Special: {:>16n}\n", sum.first.c_str(),
               sum.second.sumNodes / sum.second.sumCounter, sum.second.sumNps / sum.second.sumCounter,
               sum.second.sumTime / sum.second.sumCounter, sum.second.special / sum.second.sumCounter);
  }
}

SearchTreeSizeTest::Result
SearchTreeSizeTest::featureMeasurements(int depth, const std::string &fen) {
  Search search{};
  SearchLimits searchLimits{};
  searchLimits.setDepth(depth);
  Result result(fen);
  Position position(fen);

  // turn off all options
  SearchConfig::USE_QUIESCENCE = false;
  SearchConfig::USE_ALPHABETA = false;
  SearchConfig::USE_KILLER_MOVES = false;
  SearchConfig::USE_TT = false;
  SearchConfig::TT_SIZE_MB = 64;
  SearchConfig::USE_TT_QSEARCH = false;
  SearchConfig::USE_MDP = false;
  SearchConfig::USE_MPP = false;
  SearchConfig::USE_PVS = false;
  SearchConfig::USE_PV_MOVE_SORTING = false;
  SearchConfig::USE_IID = false;
  SearchConfig::USE_RFP = false;
  SearchConfig::USE_RAZOR_PRUNING = false;
  SearchConfig::USE_NMP = false;
  SearchConfig::USE_EXTENSIONS = false;

  // ***********************************
  // TESTS

  LOG->set_level(spdlog::level::info);

  ptrToSpecial = &search.getSearchStats().no_moveForPVsorting;

  //  // pure MiniMax
  //  result.tests.push_back(measureTreeSize(search, position, searchLimits, "MINIMAX-QS"));
  //
  //  // pure MiniMax + quiescence
  //  SearchConfig::USE_QUIESCENCE = true;
  //  result.tests.push_back(measureTreeSize(search, position, searchLimits, "MM+QS"));
  //
  //  SearchConfig::USE_TT = true;
  //  result.tests.push_back(measureTreeSize(search, position, searchLimits, "MM+QS+TT"));
  //
  SearchConfig::USE_QUIESCENCE = true;
  SearchConfig::USE_ALPHABETA = true;
  SearchConfig::USE_PVS = true;
  SearchConfig::USE_KILLER_MOVES = true;
  SearchConfig::USE_PV_MOVE_SORTING = true;
  SearchConfig::USE_MPP = true;
  SearchConfig::USE_MDP = true;
  //  result.tests.push_back(measureTreeSize(search, position, searchLimits, "00 -TT"));

  SearchConfig::USE_TT = true;
  SearchConfig::USE_TT_QSEARCH = true;
  // result.tests.push_back(measureTreeSize(search, position, searchLimits, "10 +TT"));

  SearchConfig::USE_NMP = true;
  //result.tests.push_back(measureTreeSize(search, position, searchLimits, "20 NMP"));

  SearchConfig::USE_RFP = true;
  //result.tests.push_back(measureTreeSize(search, position, searchLimits, "30 RFP"));

  SearchConfig::USE_IID = true;
  result.tests.push_back(measureTreeSize(search, position, searchLimits, "40 IID"));

  // ***********************************

  return result;
}

SearchTreeSizeTest::SingleTest
SearchTreeSizeTest::measureTreeSize(Search &search, const Position &position,
                                    SearchLimits searchLimits, const std::string &featureName) {

  LOG__INFO(LOG, "");
  LOG__INFO(LOG, "Testing {}", featureName);
  search.clearHash();
  search.startSearch(position, searchLimits);
  search.waitWhileSearching();

  SingleTest test{};
  test.name = featureName;
  test.nodes = search.getSearchStats().nodesVisited;
  test.move = search.getLastSearchResult().bestMove;
  test.value = valueOf(search.getLastSearchResult().bestMove);
  test.nps =
    (1'000 * search.getSearchStats().nodesVisited) / (search.getSearchStats().lastSearchTime + 1);
  test.time = search.getSearchStats().lastSearchTime;
  test.special = ptrToSpecial ? *ptrToSpecial : 0;
  test.pv = printMoveListUCI(search.getPV());

  return test;
}





