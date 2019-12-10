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
  std::shared_ptr<spdlog::logger> LOG = spdlog::stdout_color_mt("Test_Logger");
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
}

TEST_F(SearchTest, selective_moves) {
  Search search;
  SearchLimits searchLimits;
  Position position;
  searchLimits.depth = 3;
  searchLimits.moves.push_back(createMove("e2e4"));
  search.startSearch(position, searchLimits);
}

TEST_F(SearchTest, depth) {
  Search search;
  SearchLimits searchLimits;
  Position position;
  searchLimits.depth = 3;
  searchLimits.setupLimits();
  search.startSearch(position, searchLimits);
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
  setlocale(LC_ALL, "de-DE");
  LOG->info("nps: {0:n}", 1234567890);
  LOG->info("Leaf nodes: {}", search.getSearchStats().leafPositionsEvaluated);
  LOG->info("nps: {0:n}", long(search.getSearchStats().leafPositionsEvaluated/(search.getSearchStats().lastSearchTime/1e9)));
  ASSERT_EQ(4'865'609, search.getSearchStats().leafPositionsEvaluated);
  // 4 = 197'281
  // 5 = 4'865'609
  // 6 = 119'060'324
}