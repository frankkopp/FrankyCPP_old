/*
 * MIT License
 *
 * Copyright (c) 2020 Frank Kopp
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

#include <iostream>
#include "Search.h"
#include "Engine.h"
#include "Logging.h"
#include "SearchConfig.h"
#include <gtest/gtest.h>

using testing::Eq;

class TestSuiteTests : public ::testing::Test {
public:
  static void SetUpTestSuite() {
    NEWLINE;
    LOGGING::init();
    INIT::init();
    NEWLINE;
  }

  std::shared_ptr<spdlog::logger> LOG = spdlog::get("Test_Logger");

  enum TestType {
    DM,
    BM
  };

  struct TestSet {
    std::string id;
    std::string fen;
    TestType type;
    std::string target;
  };

  static std::string getMoveString(const Position &position, const std::string &target) {
    std::istringstream inStream(target);
    MoveGenerator mg;
    const MoveList* legalMoves = mg.generateLegalMoves<MoveGenerator::GENALL>(position);


    return std::string();
  }

protected:
  void SetUp() override {

    // @formatter:off
    SearchConfig::USE_QUIESCENCE      = true;
    SearchConfig::USE_ALPHABETA       = true;
    SearchConfig::USE_PVS             = true;


    SearchConfig::USE_TT              = true;
    SearchConfig::USE_TT_QSEARCH      = true;

    SearchConfig::USE_KILLER_MOVES    = true;
    SearchConfig::USE_PV_MOVE_SORTING = true;
    SearchConfig::USE_IID             = true;

    SearchConfig::USE_MDP             = true;
    SearchConfig::USE_MPP             = true;
    SearchConfig::USE_QS_STANDPAT_CUT = true;
    SearchConfig::USE_RFP             = true;
    SearchConfig::USE_RAZOR_PRUNING   = true;
    SearchConfig::USE_NMP             = true;

    SearchConfig::USE_EXTENSIONS      = true;

    SearchConfig::TT_SIZE_MB          = 64;
    SearchConfig::MAX_EXTRA_QDEPTH    = static_cast<Depth>(20);
    SearchConfig::NO_KILLER_MOVES     = 2;

    // @formatter:on

  }

  void TearDown() override {}
};

TEST_F(TestSuiteTests, basic) {
  Search search;
  SearchLimits searchLimits;
  Position position;

  searchLimits.setMoveTime(5'000);
  search.startSearch(position, searchLimits);
  search.waitWhileSearching();
}

TEST_F(TestSuiteTests, easy_mates) {
  Search search;
  SearchLimits searchLimits;

  MilliSec moveTime = 5'000;

  std::vector<TestSet> ts = {
    {"Mate in 4",     "8/8/8/8/8/3K4/R7/5k2 w - -", DM, "4"},
    {"Best move Ke3", "8/8/8/8/8/3K4/R7/5k2 w - -", BM, "Ke3"},
  };

  for (TestSet t : ts) {
    LOG__INFO(LOG, "TestSet: ID \"{}\" {} {} {}", t.id, t.fen, t.type == DM ? "Mate in " : t.type == BM ? "Best Move " : "unknown opcode", t.target);
    const Position position(t.fen);
    switch (t.type) {
      case DM: {
        const int mateIn = std::stoi(t.target);
        searchLimits.setMate(mateIn);
        searchLimits.setMoveTime(moveTime);
        search.startSearch(position, searchLimits);
        search.waitWhileSearching();
        ASSERT_EQ("mate " + t.target, printValue(search.getLastSearchResult().bestMoveValue));
      }
        break;
      case BM:
        searchLimits.setMoveTime(moveTime);
        search.startSearch(position, searchLimits);
        search.waitWhileSearching();
        ASSERT_EQ(TestSuiteTests::getMoveString(position, t.target), printMove(search.getLastSearchResult().bestMove));
        break;
    }
  }
}
