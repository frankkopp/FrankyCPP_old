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
#include "../../src/logging.h"
#include "../../src/Search.h"
#include "../../src/Engine.h"

using testing::Eq;

class SearchTest : public ::testing::Test {
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
    LOG->set_level(spdlog::level::info);
  }
  void TearDown() override {}
};

TEST_F(SearchTest, basic) {
  Search search;
  SearchLimits searchLimits;
  Position position;
  searchLimits.infinite = true;
  search.startSearch(position, searchLimits);
  sleep(2);
  search.stopSearch();
  search.waitWhileSearching();
}

TEST_F(SearchTest, selective_moves) {
  Search search;
  SearchLimits searchLimits;
  Position position;
  searchLimits.depth = 4;
  searchLimits.moves.push_back(createMove("e2e4"));
  searchLimits.setupLimits();
  search.startSearch(position, searchLimits);
  search.waitWhileSearching();
  ASSERT_EQ(13781, search.getSearchStats().leafPositionsEvaluated);
}

TEST_F(SearchTest, perft) {
  Search search;
  SearchLimits searchLimits;
  Position position;
  searchLimits.perft = true;
  searchLimits.depth = 5;
  searchLimits.setupLimits();
  search.startSearch(position, searchLimits);
  search.waitWhileSearching();
  LOG->info("Nodes per sec: {:n}", long(search.getSearchStats().leafPositionsEvaluated/(search.getSearchStats().lastSearchTime/1e9)));
  LOG->info("Leaf nodes:    {:n}", search.getSearchStats().leafPositionsEvaluated);
  ASSERT_EQ(4'865'609, search.getSearchStats().leafPositionsEvaluated);
  // 4 = 197'281
  // 5 = 4'865'609
  // 6 = 119'060'324
  // 7 = 3'195'901'860
}

TEST_F(SearchTest, depth) {
  Search search;
  SearchLimits searchLimits;
  Position position;
  searchLimits.depth = 6;
  searchLimits.setupLimits();
  search.startSearch(position, searchLimits);
  search.waitWhileSearching();
  ASSERT_EQ(6, search.getSearchStats().currentExtraSearchDepth);
}

TEST_F(SearchTest, nodes) {
  Search search;
  SearchLimits searchLimits;
  Position position;
  searchLimits.nodes = 1'000'000;
  searchLimits.setupLimits();
  search.startSearch(position, searchLimits);
  search.waitWhileSearching();
  ASSERT_EQ(1'000'000, search.getSearchStats().nodesVisited);
}

TEST_F(SearchTest, movetime) {
  Search search;
  SearchLimits searchLimits;
  Position position;
  searchLimits.moveTime = 2'000;
  searchLimits.setupLimits();
  search.startSearch(position, searchLimits);
  search.waitWhileSearching();
  ASSERT_TRUE(search.getSearchStats().lastSearchTime < (searchLimits.moveTime + 100));
}

TEST_F(SearchTest, timewhite) {
  Search search;
  SearchLimits searchLimits;
  Position position;
  searchLimits.whiteTime = 60'000;
  searchLimits.blackTime = 60'000;
  searchLimits.setupLimits();
  search.startSearch(position, searchLimits);
  search.waitWhileSearching();
  ASSERT_TRUE(search.getSearchStats().lastSearchTime < (searchLimits.whiteTime/40));
}

TEST_F(SearchTest, timeblack) {
  Search search;
  SearchLimits searchLimits;
  Position position;
  position.doMove(createMove("e2e4"));
  searchLimits.whiteTime = 60'000;
  searchLimits.blackTime = 60'000;
  searchLimits.setupLimits();
  search.startSearch(position, searchLimits);
  search.waitWhileSearching();
  ASSERT_TRUE(search.getSearchStats().lastSearchTime < (searchLimits.blackTime/40));
}

