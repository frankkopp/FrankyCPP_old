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
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
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

#include "Search.h"
#include "Engine.h"
#include "Logging.h"
#include "Position.h"
#include "SearchConfig.h"
#include <gtest/gtest.h>
#include <sstream>

using testing::Eq;

class SearchTest : public ::testing::Test {
public:
  static void SetUpTestSuite() {
    NEWLINE;
    INIT::init();
    NEWLINE;
  }

protected:
  void SetUp() override {
    Logger::get().TEST_LOG->set_level(spdlog::level::debug);
    Logger::get().SEARCH_LOG->set_level(spdlog::level::debug);
    Logger::get().BOOK_LOG->set_level(spdlog::level::debug);
    SearchConfig::USE_BOOK = false;
  }

  void TearDown() override {}
};

TEST_F(SearchTest, basic) {
  Search       search;
  SearchLimits searchLimits;
  Position     position;
  searchLimits.setInfinite(true);
  search.startSearch(position, searchLimits);
  sleepForSec(2);
  search.stopSearch();
  search.waitWhileSearching();
  SUCCEED();
}

TEST_F(SearchTest, selective_moves) {
  Search       search;
  SearchLimits searchLimits;
  Position     position;
  searchLimits.setDepth(4);
  searchLimits.setMoves(MoveList({ createMove("a2a4") }));
  search.startSearch(position, searchLimits);
  search.waitWhileSearching();
  ASSERT_EQ(moveOf(createMove("a2a4")), moveOf(search.getLastSearchResult().bestMove));
}

TEST_F(SearchTest, depth) {
  Search       search;
  SearchLimits searchLimits;
  Position     position;
  searchLimits.setDepth(6);
  search.startSearch(position, searchLimits);
  search.waitWhileSearching();
  ASSERT_EQ(6, search.getSearchStats().currentSearchDepth);
}

TEST_F(SearchTest, nodes) {
  Search       search;
  SearchLimits searchLimits;
  Position     position;
  searchLimits.setNodes(1'000'000);
  search.startSearch(position, searchLimits);
  search.waitWhileSearching();
  ASSERT_EQ(1'000'000, search.getSearchStats().nodesVisited);
}

TEST_F(SearchTest, timerTest) {
  Search       search;
  SearchLimits searchLimits;
  Position     position;

  SearchConfig::USE_BOOK              = false;
  SearchConfig::USE_ASPIRATION_WINDOW = false;

  searchLimits.setWhiteTime(60'000); //  1.475 ms
  searchLimits.setBlackTime(60'000);
  search.startSearch(position, searchLimits);
  search.addExtraTime(2.0); // 2.950 ms
  search.waitWhileSearching();
  EXPECT_GE(search.getSearchStats().lastSearchTime, 2'950);
  EXPECT_LT(search.getSearchStats().lastSearchTime, 3'500);

  searchLimits.setWhiteTime(60'000); //  1.475 ms
  searchLimits.setBlackTime(60'000);
  search.startSearch(position, searchLimits);
  search.addExtraTime(0.5); // 0.737
  search.waitWhileSearching();
  EXPECT_GE(search.getSearchStats().lastSearchTime, 737);
  EXPECT_LT(search.getSearchStats().lastSearchTime, 1'200);
}

TEST_F(SearchTest, movetime) {
  Search       search;
  SearchLimits searchLimits;
  Position     position;
  SearchConfig::USE_BOOK = false;
  searchLimits.setMoveTime(2'000);
  search.startSearch(position, searchLimits);
  search.waitWhileSearching();
  ASSERT_TRUE(search.getSearchStats().lastSearchTime < (searchLimits.getMoveTime() + 100));
}

TEST_F(SearchTest, timewhite) {
  Search       search;
  SearchLimits searchLimits;
  Position     position;
  SearchConfig::USE_ASPIRATION_WINDOW = false;
  searchLimits.setWhiteTime(60'000);
  searchLimits.setBlackTime(60'000);
  search.startSearch(position, searchLimits);
  search.waitWhileSearching();
  ASSERT_TRUE(search.getSearchStats().lastSearchTime < (searchLimits.getWhiteTime() / 40) + 200);
}

TEST_F(SearchTest, timeblack) {
  Search       search;
  SearchLimits searchLimits;
  Position     position;
  SearchConfig::USE_ASPIRATION_WINDOW = false;
  position.doMove(createMove("e2e4"));
  searchLimits.setWhiteTime(60'000);
  searchLimits.setBlackTime(60'000);
  search.startSearch(position, searchLimits);
  search.waitWhileSearching();
  ASSERT_TRUE(search.getSearchStats().lastSearchTime < (searchLimits.getBlackTime() / 40) + 200);
}

TEST_F(SearchTest, mate0Search) {
  Search       search;
  SearchLimits searchLimits;
  Position     position("8/8/8/8/8/6K1/8/R5k1 b - - 0 8");
  searchLimits.setMate(0);
  searchLimits.setDepth(1);
  search.startSearch(position, searchLimits);
  search.waitWhileSearching();
  ASSERT_EQ(-VALUE_CHECKMATE, search.getLastSearchResult().bestMoveValue);
}

TEST_F(SearchTest, mate1Search) {
  Search       search;
  SearchLimits searchLimits;
  Position     position("8/8/8/8/8/6K1/R7/6k1 w - - 0 8");
  searchLimits.setMate(1);
  searchLimits.setDepth(4);
  search.startSearch(position, searchLimits);
  search.waitWhileSearching();
  ASSERT_EQ(VALUE_CHECKMATE - 1, valueOf(search.getLastSearchResult().bestMove));
}

TEST_F(SearchTest, mate2Search) {
  Search       search;
  SearchLimits searchLimits;
  Position     position("8/8/8/8/8/5K2/R7/7k w - - 0 7");
  searchLimits.setMate(2);
  searchLimits.setDepth(4);
  search.startSearch(position, searchLimits);
  search.waitWhileSearching();
  ASSERT_EQ(VALUE_CHECKMATE - 3, valueOf(search.getLastSearchResult().bestMove));
}

TEST_F(SearchTest, repetitionForce) {
  Search       search;
  SearchLimits searchLimits;
  Position     position("8/p3Q1bk/1p4p1/5q2/P1N2p2/1P5p/2b4P/6K1 w - - 0 38");
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

  LOG__INFO(Logger::get().TEST_LOG, "Repetition move: {}",
            printMoveVerbose(search.getLastSearchResult().bestMove));

  ASSERT_EQ("d8h4", printMove(search.getLastSearchResult().bestMove));
  ASSERT_EQ(VALUE_DRAW, valueOf(search.getLastSearchResult().bestMove));
}

TEST_F(SearchTest, repetitionAvoid) {
  Search       search;
  SearchLimits searchLimits;
  Position     position("8/p3Q1bk/1p4p1/5q2/P1N2p2/1P5p/2b4P/6K1 w - - 0 38");
  // 1. Qh4+ Kg8 2. Qd8+ Kh7 3. Qh4+ Kg8 4. Qd8+ Kh7 5. Qh4+ 1/2-1/2
  position.doMove(createMove("e7h4"));
  position.doMove(createMove("h7g8"));
  position.doMove(createMove("h4d8"));
  position.doMove(createMove("g8h7"));
  position.doMove(createMove("d8h4"));
  position.doMove(createMove("h7g8"));
  position.doMove(createMove("h4d8"));
  // black should not move Kg8h7 as this would enable white to  3-fold
  // repetition although black is winning

  searchLimits.setDepth(4);
  search.startSearch(position, searchLimits);
  search.waitWhileSearching();

  LOG__INFO(Logger::get().TEST_LOG, "Repetition avoidance move: {}",
            printMoveVerbose(search.getLastSearchResult().bestMove));

  ASSERT_NE("g8f7", printMove(search.getLastSearchResult().bestMove));
  ASSERT_NE(VALUE_DRAW, valueOf(search.getLastSearchResult().bestMove));
}

TEST_F(SearchTest, goodCapture) {
  Search   search;
  Position position;

  // no capture
  // goodCapture does not check this any longer - unnecessary if only called for
  // capture moves.
  //  position = Position();
  //  ASSERT_FALSE(search.goodCapture(position, createMove("e2e4")));

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
  ASSERT_TRUE(search.goodCapture(position, createMove("g4f5")));  // pawn capture
  ASSERT_FALSE(search.goodCapture(position, createMove("g4h5"))); // pawn capture
  ASSERT_TRUE(search.goodCapture(position, createMove("f2a7")));  // not defended
  ASSERT_TRUE(search.goodCapture(position, createMove("h4g6")));  // not defended
  ASSERT_FALSE(search.goodCapture(position, createMove("h4f5"))); // Nxn
  ASSERT_FALSE(search.goodCapture(position, createMove("e1e4"))); // Rxn
  ASSERT_FALSE(search.goodCapture(position, createMove("f2f5"))); // Qxn
  ASSERT_FALSE(search.goodCapture(position, createMove("d1d6"))); // Rxp

  position = Position("2q1r1k1/rpp5/3p1Pp1/p4n1p/b1P1n1PN/5Q1P/PP5K/2BRRB2 w - -");
  position.doMove(createMove("e1e4"));
  ASSERT_TRUE(search.goodCapture(position, createMove("e8e4")));  // recapture
  ASSERT_TRUE(search.goodCapture(position, createMove("a4d1")));  // bxR
  ASSERT_TRUE(search.goodCapture(position, createMove("f5h4")));  // nor defended
  ASSERT_FALSE(search.goodCapture(position, createMove("h5g4"))); // pawn

  // k6q/3n1n2/3b4/4p3/3P1P2/3N1N2/8/K7 w - -
  // only works with SEE
  position = Position("k6q/3n1n2/3b4/4p3/3P1P2/3N1N2/8/K7 w - -");
  ASSERT_TRUE(search.goodCapture(position, createMove("d3e5")));
  ASSERT_TRUE(search.goodCapture(position, createMove("f3e5")));
}

TEST_F(SearchTest, quiescenceTest) {

  Search       search;
  SearchLimits searchLimits;
  Position     position;
  searchLimits.setDepth(2);

  SearchConfig::USE_ALPHABETA = false;
  SearchConfig::USE_TT        = false;

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

  LOG__INFO(Logger::get().TEST_LOG, "Nodes without Quiescence: {:n} Nodes with Quiescence: {:n}",
            nodes1, nodes2);
  LOG__INFO(Logger::get().TEST_LOG, "Extra without Quiescence: {:n} Extra with Quiescence: {:n}",
            extra1, extra2);

  ASSERT_GT(nodes2, nodes1);
  ASSERT_GT(extra2, extra1);
}

TEST_F(SearchTest, alphaBetaTest) {

  Search       search;
  SearchLimits searchLimits;
  Position     position;
  searchLimits.setDepth(4);

  SearchConfig::USE_QUIESCENCE = true;

  SearchConfig::USE_ALPHABETA = false;
  search.startSearch(position, searchLimits);
  search.waitWhileSearching();
  auto leafPositionsEvaluated1 = search.getSearchStats().leafPositionsEvaluated;
  auto nodesVisited1           = search.getSearchStats().nodesVisited;

  SearchConfig::USE_ALPHABETA = true;
  search.startSearch(position, searchLimits);
  search.waitWhileSearching();
  auto leafPositionsEvaluated2 = search.getSearchStats().leafPositionsEvaluated;
  auto nodesVisited2           = search.getSearchStats().nodesVisited;

  LOG__INFO(Logger::get().TEST_LOG, "Nodes without AlphaBeta: Visited: {:n} Evaluated {:n}",
            nodesVisited1, leafPositionsEvaluated1);
  LOG__INFO(Logger::get().TEST_LOG, "Nodes with AlphaBeta: Visited: {:n} Evaluated {:n}",
            nodesVisited2, leafPositionsEvaluated2);

  ASSERT_GT(nodesVisited1, nodesVisited2);
}

TEST_F(SearchTest, Book) {

  SearchConfig::USE_BOOK  = true;
  SearchConfig::BOOK_PATH = "./books/book_smalltest.txt";
  SearchConfig::BOOK_TYPE = OpeningBook::BookFormat::SIMPLE;

  Search       search;
  SearchLimits searchLimits;
  Position     position;
  searchLimits.setMoveTime(2'000);
  search.startSearch(position, searchLimits);
  search.waitWhileSearching();

  LOG__INFO(Logger::get().TEST_LOG, "Book move has no value: move={} value={}",
            printMoveVerbose(search.getLastSearchResult().bestMove),
            search.getLastSearchResult().bestMoveValue);
  EXPECT_EQ(VALUE_NONE, search.getLastSearchResult().bestMoveValue);
}

TEST_F(SearchTest, MDPMPP) {

  SearchConfig::USE_QUIESCENCE   = true;
  SearchConfig::USE_ALPHABETA    = true;
  SearchConfig::USE_KILLER_MOVES = true;
  SearchConfig::USE_TT           = true;
  SearchConfig::USE_TT_QSEARCH   = true;
  SearchConfig::USE_MDP          = true;
  SearchConfig::USE_MPP          = true;

  Search       search;
  SearchLimits searchLimits;
  Position     position("r3k2r/1ppn3p/2q1q1n1/4P3/2q1Pp2/6R1/pbp2PPP/1R4K1 w kq -");
  searchLimits.setNodes(5'000'000);
  search.startSearch(position, searchLimits);
  search.waitWhileSearching();

  LOG__INFO(Logger::get().TEST_LOG, "MDP: {:n} MPP: {:n}", search.getSearchStats().mateDistancePrunings,
            search.getSearchStats().minorPromotionPrunings);
  ASSERT_GT(search.getSearchStats().mateDistancePrunings, 1'000);
  ASSERT_GT(search.getSearchStats().minorPromotionPrunings, 1'000);
}

TEST_F(SearchTest, PV_MOVE) {

  SearchConfig::USE_PVS          = true;
  SearchConfig::USE_PV_MOVE_SORT = true;

  Search       search;
  SearchLimits searchLimits;
  Position     position("r3k2r/1ppn3p/2q1q1n1/4P3/2q1Pp2/6R1/pbp2PPP/1R4K1 w kq -");
  searchLimits.setNodes(30'000'000);
  search.startSearch(position, searchLimits);
  search.waitWhileSearching();

  LOG__INFO(Logger::get().TEST_LOG,
            "PVS ROOT CUTS {:n} PVS ROOT RE-SEARCH {:n} PVS CUTS: {:n} PVS "
            "RE-SEARCH: {:n}",
            search.getSearchStats().pvs_root_cutoffs, search.getSearchStats().pvs_root_researches,
            search.getSearchStats().pvs_cutoffs, search.getSearchStats().pvs_root_researches);
  ASSERT_GT(search.getSearchStats().pvs_cutoffs, 10);
  ASSERT_GT(search.getSearchStats().pvs_researches, 10);
}

TEST_F(SearchTest, TT) {
  Search       search;
  SearchLimits searchLimits;
  Position     position;

  search.setHashSize(256);

  searchLimits.setDepth(6);
  search.startSearch(position, searchLimits);
  search.waitWhileSearching();

  LOG__INFO(Logger::get().TEST_LOG, "Nodes: {:n} Time: {:n} ms NPS: {:n}",
            search.getSearchStats().nodesVisited, search.getSearchStats().lastSearchTime,
            (search.getSearchStats().nodesVisited * 1'000) / search.getSearchStats().lastSearchTime);
  LOG__INFO(Logger::get().TEST_LOG, "TT Hits: {:n} TT Misses: {:n} TT Hit rate: {}%",
            search.getSearchStats().tt_Cuts, search.getSearchStats().tt_NoCuts,
            (static_cast<double>(search.getSearchStats().tt_Cuts * 100)
             / (search.getSearchStats().tt_Cuts + search.getSearchStats().tt_NoCuts)));

  searchLimits.setDepth(6);
  search.startSearch(position, searchLimits);
  search.waitWhileSearching();

  LOG__INFO(Logger::get().TEST_LOG, "Nodes: {:n} Time: {:n} ms NPS: {:n}",
            search.getSearchStats().nodesVisited, search.getSearchStats().lastSearchTime,
            (search.getSearchStats().nodesVisited * 1'000) / search.getSearchStats().lastSearchTime);
  LOG__INFO(Logger::get().TEST_LOG, "TT Hits: {:n} TT Misses: {:n} TT Hit rate: {}%",
            search.getSearchStats().tt_Cuts, search.getSearchStats().tt_NoCuts,
            (static_cast<double>(search.getSearchStats().tt_Cuts * 100)
             / (search.getSearchStats().tt_Cuts + search.getSearchStats().tt_NoCuts)));
}

TEST_F(SearchTest, null_move) {
  Search       search;
  SearchLimits searchLimits;
  Position     position;

  search.setHashSize(256);

  searchLimits.setMoveTime(5'000);
  search.startSearch(position, searchLimits);
  search.waitWhileSearching();

  LOG__INFO(Logger::get().TEST_LOG, "Nodes: {:n} Time: {:n} ms NPS: {:n}",
            search.getSearchStats().nodesVisited, search.getSearchStats().lastSearchTime,
            (search.getSearchStats().nodesVisited * 1'000) / search.getSearchStats().lastSearchTime);

  LOG__INFO(Logger::get().TEST_LOG, "Number of Null Moves Prunings: {:n} Verifications {:n}",
            search.getSearchStats().nullMovePrunings, search.getSearchStats().nullMoveVerifications);
}

TEST_F(SearchTest, extensions) {
  Search       search;
  SearchLimits searchLimits;
  Position     position("r3k2r/1ppn3p/2q1q1n1/4P3/2q1Pp2/6R1/pbp2PPP/1R4K1 w kq -");
  searchLimits.setMoveTime(5'000);
  search.startSearch(position, searchLimits);
  search.waitWhileSearching();
}

TEST_F(SearchTest, aspirationWindow) {
  Search       search;
  SearchLimits searchLimits;
  Position     position; //("r3k2r/1ppn3p/2q1q1n1/4P3/2q1Pp2/6R1/pbp2PPP/1R4K1 w kq -");
  searchLimits.setDepth(10);
  search.startSearch(position, searchLimits);
  search.waitWhileSearching();
}

TEST_F(SearchTest, perft) {
  int DEPTH = 6;

  uint64_t perftResults[] = { 0,
                              20,              // 1
                              400,             // 2
                              8'902,           // 3
                              197'281,         // 4
                              4'865'609,       // 5
                              119'060'324,     // 6
                              3'195'901'860 }; // 7

  Search       search;
  SearchLimits searchLimits;
  Position     position;
  searchLimits.setPerft(true);
  searchLimits.setDepth(DEPTH);
  search.startSearch(position, searchLimits);
  search.waitWhileSearching();
  LOG__INFO(Logger::get().TEST_LOG, "Leaf nodes per sec: {:n}",
            (search.getSearchStats().leafPositionsEvaluated * 1'000) / search.getSearchStats().lastSearchTime);
  LOG__INFO(Logger::get().TEST_LOG, "Leaf nodes:         {:n}", search.getSearchStats().leafPositionsEvaluated);
  ASSERT_EQ(perftResults[DEPTH], search.getSearchStats().leafPositionsEvaluated);
}

// for debugging
TEST_F(SearchTest, DISABLED_nmpStats) {
  Search       search;
  SearchLimits searchLimits;

  // Position position;
  // Position position("rn2kbnr/ppp1pppp/8/8/6b1/2NP1N2/PPP2P1P/R1BQKB1q w Qkq -
  // 1 6"); Position position("r3k2r/1ppn3p/2q1q1n1/4P3/2q1Pp2/6R1/pbp2PPP/1R4K1
  // w kq -"); Position
  // position("r1b2rk1/2q1b1pp/p2ppn2/1p6/3QP3/1BN1B3/PPP3PP/R4RK1 w - - ");
  // Position position("2rr2k1/1p2qp1p/1pn1pp2/1N6/3P4/P6P/1P2QPP1/2R2RK1 w -
  // -"); Position position("8/7p/R7/5p1k/5P2/7P/P1P1nP1K/5q2 w - - 3 33");
  // Position position("r3k2r/1ppn3p/2q1q1n1/4P3/4q3/5Pp1/pb4PP/2q2RK1 w kq - 0
  // 1"); //
  Position position("6k1/6p1/4p2p/1p6/1qn5/6N1/5PPP/2Q3K1 w - -");

  const int depth = 8;
  searchLimits.setDepth(depth);

  std::stringstream str;
  SearchConfig::USE_NMP       = false;
  SearchConfig::NMP_DEPTH     = static_cast<Depth>(0);
  SearchConfig::NMP_REDUCTION = static_cast<Depth>(0);
  search.startSearch(position, searchLimits);
  search.waitWhileSearching();
  search.clearHash();
  str << fmt::format("NMP {:5s} DEPTH {:2d} RED {:2d} : value = {:5d} nodes = "
                     "{:11n} time = {:7}\n",
                     boolStr(0), 0, 0, search.getLastSearchResult().bestMoveValue,
                     search.getSearchStats().nodesVisited, search.getSearchStats().lastSearchTime);

  for (int dep = 2; dep <= depth; dep++) {
    for (int red = 1; red <= dep; red++) {
      SearchConfig::USE_NMP       = true;
      SearchConfig::NMP_DEPTH     = static_cast<Depth>(dep);
      SearchConfig::NMP_REDUCTION = static_cast<Depth>(red);
      search.startSearch(position, searchLimits);
      search.waitWhileSearching();
      search.clearHash();
      str << fmt::format("NMP {:5s} DEPTH {:2d} RED {:2d} : value = {:5d} "
                         "nodes = {:11n} time = {:7}\n",
                         boolStr(1), dep, red, search.getLastSearchResult().bestMoveValue,
                         search.getSearchStats().nodesVisited, search.getSearchStats().lastSearchTime);
    }
  }
  std::cout << str.str();
}

TEST_F(SearchTest, DISABLED_debuggingIID) {

  Search       search;
  SearchLimits searchLimits;
  Position     position;

  // @formatter:off
  SearchConfig::USE_QUIESCENCE      = true;
  SearchConfig::USE_ALPHABETA       = true;
  SearchConfig::USE_KILLER_MOVES    = true;
  SearchConfig::USE_TT              = true;
  SearchConfig::TT_SIZE_MB          = 64;
  SearchConfig::USE_TT_QSEARCH      = true;
  SearchConfig::USE_MDP             = true;
  SearchConfig::USE_MPP             = true;
  SearchConfig::USE_PVS             = true;
  SearchConfig::USE_PV_MOVE_SORT    = true;
//  SearchConfig::USE_IID             = false;
//  SearchConfig::IID_DEPTH           = Depth{7}; // remaining depth to do IID
//  SearchConfig::IID_DEPTH_REDUCTION = Depth{5}; // reduction of depth for IID
  SearchConfig::USE_RFP             = true;
//  SearchConfig::USE_RAZOR_PRUNING   = true;
  SearchConfig::USE_NMP             = true;
  // @formatter:on

  const int depth = 9;
  position        = Position("3r1rk1/1pp2p1p/p3bq2/4bp2/1QP5/P2B2N1/1P3PPP/4RRK1 w - - 3 20");
  searchLimits.setDepth(depth);
  search.startSearch(position, searchLimits);
  search.waitWhileSearching();

  EXPECT_NE("e1e5", printMove(search.getLastSearchResult().bestMove));
  EXPECT_NE("d3f5", printMove(search.getLastSearchResult().bestMove));
}

TEST_F(SearchTest, DISABLED_debuggingTTMove) {
  Search       search;
  SearchLimits searchLimits;
  Position     position;

  SearchConfig::USE_TT         = true;
  SearchConfig::USE_TT_QSEARCH = true;
  SearchConfig::USE_RFP        = true;
  SearchConfig::USE_NMP        = true;
  //  SearchConfig::USE_IID = true;

  const int depth = 3;
  position        = Position("rnb1kbnr/ppp2ppp/8/3PN1q1/3Pp3/8/PPP2PPP/RNBQKB1R b KQkq d3 0 5");
  searchLimits.setDepth(depth);

  search.startSearch(position, searchLimits);
  search.waitWhileSearching();
}

TEST_F(SearchTest, debugging) {

  Logger::get().TEST_LOG->set_level(spdlog::level::debug);
  Logger::get().SEARCH_LOG->set_level(spdlog::level::info);

  Search       search;
  SearchLimits searchLimits;
  Position     position;

  // @formatter:off
  SearchConfig::USE_QUIESCENCE        = true;
  SearchConfig::USE_QS_SEE            = true;
  SearchConfig::USE_TT                = true;
  SearchConfig::USE_TT_QSEARCH        = true;
  SearchConfig::TT_SIZE_MB            = 128;
  SearchConfig::USE_ALPHABETA         = true;
  SearchConfig::USE_PVS               = true;
  SearchConfig::USE_PV_MOVE_SORT      = true;
  SearchConfig::USE_KILLER_MOVES      = true;
  SearchConfig::NO_KILLER_MOVES       = 2;
  SearchConfig::USE_PV_MOVE_SORT      = true;
  SearchConfig::USE_ASPIRATION_WINDOW = true;
  SearchConfig::USE_MDP               = true;
  SearchConfig::USE_MPP               = true;
  SearchConfig::USE_RFP               = true;
  SearchConfig::USE_NMP               = false;
  SearchConfig::NMP_VERIFICATION      = false;

  SearchConfig::USE_EXTENSIONS        = true;
  SearchConfig::USE_FP                = true;
  SearchConfig::USE_EFP               = true;
  SearchConfig::USE_LMR               = true;
  // @formatter:on

  position = Position(START_POSITION_FEN);

  //searchLimits.setDepth(11);
  searchLimits.setMoveTime(5'000);
  search.startSearch(position, searchLimits);
  search.waitWhileSearching();
}
