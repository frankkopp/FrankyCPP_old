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
#include "../../src/Logging.h"
#include "../../src/Search.h"
#include "../../src/SearchConfig.h"
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
    LOG->set_level(spdlog::level::trace);
  }
  
  void TearDown() override {}
};

TEST_F(SearchTest, basic) {
  Search search;
  SearchLimits searchLimits;
  Position position;
  searchLimits.setInfinite(true);
  search.startSearch(position, searchLimits);
  sleep(2);
  search.stopSearch();
  search.waitWhileSearching();
}

TEST_F(SearchTest, selective_moves) {
  Search search;
  SearchLimits searchLimits;
  Position position;
  searchLimits.setDepth(4);
  searchLimits.setMoves(MoveList({createMove("a2a4")}));
  search.startSearch(position, searchLimits);
  search.waitWhileSearching();
  ASSERT_EQ(moveOf(createMove("a2a4")), moveOf(search.getLastSearchResult().bestMove));
}

TEST_F(SearchTest, depth) {
  Search search;
  SearchLimits searchLimits;
  Position position;
  searchLimits.setDepth(6);
  search.startSearch(position, searchLimits);
  search.waitWhileSearching();
  ASSERT_EQ(6, search.getSearchStats().currentSearchDepth);
}

TEST_F(SearchTest, nodes) {
  Search search;
  SearchLimits searchLimits;
  Position position;
  searchLimits.setNodes(1'000'000);
  search.startSearch(position, searchLimits);
  search.waitWhileSearching();
  ASSERT_EQ(1'000'000, search.getSearchStats().nodesVisited);
}

TEST_F(SearchTest, movetime) {
  Search search;
  SearchLimits searchLimits;
  Position position;
  searchLimits.setMoveTime(2'000);
  search.startSearch(position, searchLimits);
  search.waitWhileSearching();
  ASSERT_TRUE(search.getSearchStats().lastSearchTime < (searchLimits.getMoveTime() + 100));
}

TEST_F(SearchTest, timewhite) {
  Search search;
  SearchLimits searchLimits;
  Position position;
  searchLimits.setWhiteTime(60'000);
  searchLimits.setBlackTime(60'000);
  search.startSearch(position, searchLimits);
  search.waitWhileSearching();
  ASSERT_TRUE(search.getSearchStats().lastSearchTime < (searchLimits.getWhiteTime() / 40));
}

TEST_F(SearchTest, timeblack) {
  Search search;
  SearchLimits searchLimits;
  Position position;
  position.doMove(createMove("e2e4"));
  searchLimits.setWhiteTime(60'000);
  searchLimits.setBlackTime(60'000);
  search.startSearch(position, searchLimits);
  search.waitWhileSearching();
  ASSERT_TRUE(search.getSearchStats().lastSearchTime < (searchLimits.getBlackTime() / 40));
}

TEST_F(SearchTest, mate0Search) {
  Search search;
  SearchLimits searchLimits;
  Position position("8/8/8/8/8/6K1/8/R5k1 b - - 0 8");
  searchLimits.setMate(0);
  searchLimits.setDepth(1);
  search.startSearch(position, searchLimits);
  search.waitWhileSearching();
  ASSERT_EQ(-VALUE_CHECKMATE, valueOf(search.getLastSearchResult().bestMove));
}

TEST_F(SearchTest, mate1Search) {
  Search search;
  SearchLimits searchLimits;
  Position position("8/8/8/8/8/6K1/R7/6k1 w - - 0 8");
  searchLimits.setMate(1);
  searchLimits.setDepth(4);
  search.startSearch(position, searchLimits);
  search.waitWhileSearching();
  ASSERT_EQ(VALUE_CHECKMATE - 1, valueOf(search.getLastSearchResult().bestMove));
}

TEST_F(SearchTest, mate2Search) {
  Search search;
  SearchLimits searchLimits;
  Position position("8/8/8/8/8/5K2/R7/7k w - - 0 7");
  searchLimits.setMate(2);
  searchLimits.setDepth(4);
  search.startSearch(position, searchLimits);
  search.waitWhileSearching();
  ASSERT_EQ(VALUE_CHECKMATE - 3, valueOf(search.getLastSearchResult().bestMove));
}

TEST_F(SearchTest, repetitionForce) {
  Search search;
  SearchLimits searchLimits;
  Position position("8/p3Q1bk/1p4p1/5q2/P1N2p2/1P5p/2b4P/6K1 w - - 0 38");
  // 1. Qh4+ Kg8 2. Qd8+ Kh7 3. Qh4+ Kg8 4. Qd8+ Kh7 5. Qh4+ 1/2-1/2
  position.doMove(createMove("e7h4"));
  position.doMove(createMove("h7g8"));
  position.doMove(createMove("h4d8"));
  position.doMove(createMove("g8h7"));
  position.doMove(createMove("d8h4"));
  position.doMove(createMove("h7g8"));
  position.doMove(createMove("h4d8"));
  position.doMove(createMove("g8h7"));
  // next white move would be 3-fold draw
  
  searchLimits.setDepth(4);
  search.startSearch(position, searchLimits);
  search.waitWhileSearching();
  
  LOG->info("Repetition move: {}", printMoveVerbose(search.getLastSearchResult().bestMove));
  
  ASSERT_EQ("d8h4", printMove(search.getLastSearchResult().bestMove));
  ASSERT_EQ(VALUE_DRAW, valueOf(search.getLastSearchResult().bestMove));
}

TEST_F(SearchTest, repetitionAvoid) {
  Search search;
  SearchLimits searchLimits;
  Position position("8/p3Q1bk/1p4p1/5q2/P1N2p2/1P5p/2b4P/6K1 w - - 0 38");
  // 1. Qh4+ Kg8 2. Qd8+ Kh7 3. Qh4+ Kg8 4. Qd8+ Kh7 5. Qh4+ 1/2-1/2
  position.doMove(createMove("e7h4"));
  position.doMove(createMove("h7g8"));
  position.doMove(createMove("h4d8"));
  position.doMove(createMove("g8h7"));
  position.doMove(createMove("d8h4"));
  position.doMove(createMove("h7g8"));
  position.doMove(createMove("h4d8"));
  // black should not move Kg8h7 as this would enable white to  3-fold repetition
  // although black is winning
  
  searchLimits.setDepth(4);
  search.startSearch(position, searchLimits);
  search.waitWhileSearching();
  
  LOG->info("Repetition avoidance move: {}",
            printMoveVerbose(search.getLastSearchResult().bestMove));
  
  ASSERT_NE("g8f7", printMove(search.getLastSearchResult().bestMove));
  ASSERT_NE(VALUE_DRAW, valueOf(search.getLastSearchResult().bestMove));
}


TEST_F(SearchTest, goodCapture) {
  Search search;
  Position position;
  
  // no capture
  position = Position();
  ASSERT_FALSE(search.goodCapture(position, createMove("e2e4")));
  
  // TODO goodCapture Tests
  // 2q1r1k1/rppb4/3p1Pp1/p4n1p/2P1n1PN/7P/PP3Q1K/2BRRB2 b - - 0 2
  //    +---+---+---+---+---+---+---+---+
  // 8 |   |   | q |   | r |   | k |   |
  //   +---+---+---+---+---+---+---+---+
  // 7 | r | * | * | b |   |   |   |   |
  //   +---+---+---+---+---+---+---+---+
  // 6 |   |   |   | * |   | O | * |   |
  //   +---+---+---+---+---+---+---+---+
  // 5 | * |   |   |   |   | n |   | * |
  //   +---+---+---+---+---+---+---+---+
  // 4 |   |   | O |   | n |   | O | N |
  //   +---+---+---+---+---+---+---+---+
  // 3 |   |   |   |   |   |   |   | O |
  //   +---+---+---+---+---+---+---+---+
  // 2 | O | O |   |   |   | Q |   | K |
  //   +---+---+---+---+---+---+---+---+
  // 1 |   |   | B | R | R | B |   |   |
  //   +---+---+---+---+---+---+---+---+
  //     A   B   C   D   E   F   G   H
  
  position = Position("2q1r1k1/rppb4/3p1Pp1/p4n1p/2P1n1PN/7P/PP3Q1K/2BRRB2 w - -");
  ASSERT_TRUE(search.goodCapture(position, createMove("g4f5"))); // pawn capture
  ASSERT_TRUE(search.goodCapture(position, createMove("g4h5"))); // pawn capture
  ASSERT_TRUE(search.goodCapture(position, createMove("f2a7"))); // not defended
  ASSERT_TRUE(search.goodCapture(position, createMove("h4g6"))); // not defended
  ASSERT_FALSE(search.goodCapture(position, createMove("h4f5"))); // Nxn
  ASSERT_FALSE(search.goodCapture(position, createMove("e1e4"))); // Rxn
  ASSERT_FALSE(search.goodCapture(position, createMove("f2f5"))); // Qxn
  ASSERT_FALSE(search.goodCapture(position, createMove("d1d6"))); // Rxp
  
  position = Position("2q1r1k1/rpp5/3p1Pp1/p4n1p/b1P1n1PN/5Q1P/PP5K/2BRRB2 w - -");
  position.doMove(createMove("e1e4"));
  ASSERT_TRUE(search.goodCapture(position, createMove("e8e4"))); // recapture
  ASSERT_TRUE(search.goodCapture(position, createMove("a4d1"))); // bxR
  ASSERT_TRUE(search.goodCapture(position, createMove("f5h4"))); // nor defended
  ASSERT_TRUE(search.goodCapture(position, createMove("h5g4"))); // pawn
  
  ASSERT_FALSE(search.goodCapture(position, createMove("h4f5"))); // Nxn
  
}

TEST_F(SearchTest, quiescenceTest) {
  
  Search search;
  SearchLimits searchLimits;
  Position position;
  searchLimits.setDepth(2);
  
  SearchConfig::USE_ALPHABETA = false;
  
  SearchConfig::USE_QUIESCENCE = false;
  search.startSearch(position, searchLimits);
  search.waitWhileSearching();
  auto nodes1 = search.getSearchStats().nodesVisited;
  auto extra1 = search.getSearchStats().currentExtraSearchDepth;
  
  SearchConfig::USE_QUIESCENCE = true;
  search.startSearch(position, searchLimits);
  search.waitWhileSearching();
  auto nodes2 = search.getSearchStats().nodesVisited;
  auto extra2 = search.getSearchStats().currentExtraSearchDepth;
  
  LOG->info("Nodes without Quiescence: {:n} Nodes with Quiescence: {:n}", nodes1, nodes2);
  LOG->info("Extra without Quiescence: {:n} Extra with Quiescence: {:n}", extra1, extra2);
  
  ASSERT_GT(nodes2, nodes1);
  ASSERT_GT(extra2, extra1);
}

TEST_F(SearchTest, alphaBetaTest) {
  
  Search search;
  SearchLimits searchLimits;
  Position position;
  searchLimits.setDepth(4);
  
  SearchConfig::USE_QUIESCENCE = true;
  
  SearchConfig::USE_ALPHABETA = false;
  search.startSearch(position, searchLimits);
  search.waitWhileSearching();
  auto leafPositionsEvaluated1 = search.getSearchStats().leafPositionsEvaluated;
  auto nodesVisited1 = search.getSearchStats().nodesVisited;
  
  SearchConfig::USE_ALPHABETA = true;
  search.startSearch(position, searchLimits);
  search.waitWhileSearching();
  auto leafPositionsEvaluated2 = search.getSearchStats().leafPositionsEvaluated;
  auto nodesVisited2 = search.getSearchStats().nodesVisited;
  
  LOG->info("Nodes without AlphaBeta: Visited: {:n} Evaluated {:n}", nodesVisited1,
            leafPositionsEvaluated1);
  LOG->info("Nodes with AlphaBeta: Visited: {:n} Evaluated {:n}", nodesVisited2,
            leafPositionsEvaluated2);
  
  ASSERT_GT(nodesVisited1, nodesVisited2);
}

TEST_F(SearchTest, perft) {

  int DEPTH = 5;

  long perftResults[] = { 0, 20, 400, 8'902, 197'281, 4'865'609, 119'060'324, 3'195'901'860 };

  Search search;
  SearchLimits searchLimits;
  Position position;
  searchLimits.setPerft(true);
  searchLimits.setDepth(DEPTH);
  search.startSearch(position, searchLimits);
  search.waitWhileSearching();
  LOG->info("Leaf nodes per sec: {:n}", (search.getSearchStats().leafPositionsEvaluated * 1'000) /
                                   search.getSearchStats().lastSearchTime);
  LOG->info("Leaf nodes:         {:n}", search.getSearchStats().leafPositionsEvaluated);
  ASSERT_EQ(perftResults[DEPTH], search.getSearchStats().leafPositionsEvaluated);
}

TEST_F(SearchTest, PERFT_nps) {
  
  SearchConfig::USE_QUIESCENCE = true;
  SearchConfig::USE_ALPHABETA = true;
  SearchConfig::USE_KILLER_MOVES = true;
  SearchConfig::USE_TT = true;
  SearchConfig::USE_TT_QSEARCH = true;

  Search search;
  SearchLimits searchLimits;
  Position position;
  searchLimits.setMoveTime(30'000);
  search.startSearch(position, searchLimits);
  search.waitWhileSearching();
  
  LOG->info("Nodes: {:n} Time: {:n} ms NPS: {:n}",
            search.getSearchStats().nodesVisited,
            search.getSearchStats().lastSearchTime,
            (search.getSearchStats().nodesVisited * 1'000)
            / search.getSearchStats().lastSearchTime);
}

// for debugging
TEST_F(SearchTest, debugging) {

  SearchConfig::USE_QUIESCENCE = true;
  SearchConfig::USE_ALPHABETA = true;
  SearchConfig::USE_KILLER_MOVES = true;
  SearchConfig::USE_TT = true;
  SearchConfig::USE_TT_QSEARCH = true;

  Search search;
  SearchLimits searchLimits;
  Position position("r3k2r/1ppn3p/2q1q1n1/4P3/2q1Pp2/6R1/pbp2PPP/1R4K1 b kq e3");
  position = Position("2r3k1/pppR1pp1/4p3/4P1P1/5P2/1P4K1/P1P5/8 w - -");
  searchLimits.setDepth(8);
  //searchLimits.setNodes(25'000'000);
  search.startSearch(position, searchLimits);
  search.waitWhileSearching();
}


