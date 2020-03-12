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
#include <ostream>
#include <string>

#include "Bitboards.h"
#include "Logging.h"
#include "MoveGenerator.h"
#include "Position.h"

using namespace std;
using testing::Eq;

class MoveGenTest : public ::testing::Test {
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

/**
 * Test pawn move generation
 */
TEST_F(MoveGenTest, pawnMoves) {
  string        fen;
  MoveGenerator mg;

  fen = "r3k2r/1ppn3p/2q1q1n1/4P3/2q1Pp2/B5R1/pbp2PPP/1R4K1 b kq e3";

  Position position(fen);
  MoveList moves;

  mg.generatePawnMoves<MoveGenerator::GENCAP>(position, &moves);

  ASSERT_EQ(10, moves.size());
  string expected = "c2b1Qc2b1Rc2b1Bc2b1Na2b1Qa2b1Ra2b1Ba2b1Nf4g3f4e3";
  string actual   = "";
  for (Move m : moves) actual.append(printMove(m));
  ASSERT_EQ(expected, actual);

  sort(moves.begin(), moves.end());
  expected = "a2b1Qc2b1Qa2b1Nc2b1Nf4g3f4e3a2b1Rc2b1Ra2b1Bc2b1B";
  actual   = "";
  for (Move m : moves) actual.append(printMove(m));
  ASSERT_EQ(expected, actual);

  NEWLINE;

  moves.clear();
  mg.generatePawnMoves<MoveGenerator::GENNONCAP>(position, &moves);

  ASSERT_EQ(13, moves.size());
  expected = "a2a1Qa2a1Na2a1Ba2a1Rc2c1Qc2c1Nc2c1Bc2c1Rb7b5h7h5f4f3b7b6h7h6";
  actual   = "";
  for (Move m : moves) actual.append(printMove(m));
  ASSERT_EQ(expected, actual);

  stable_sort(moves.begin(), moves.end());
  expected = "a2a1Qc2c1Qa2a1Nc2c1Nf4f3h7h6b7b5h7h5b7b6a2a1Bc2c1Ba2a1Rc2c1R";
  actual   = "";
  for (Move m : moves) actual.append(printMove(m));
  ASSERT_EQ(expected, actual);
}

TEST_F(MoveGenTest, kingMoves) {
  string        fen;
  MoveGenerator mg;
  MoveList      moves;

  fen = "r3k2r/1ppn3p/2q1q1n1/4P3/2q1Pp2/B5R1/pbp2PPP/1R4K1 b kq e3";
  Position position(fen);
  LOG__DEBUG(Logger::get().TEST_LOG, "\n{}", position.printBoard());

  moves.clear();
  mg.generateKingMoves<MoveGenerator::GENCAP>(position, &moves);
  LOG__DEBUG(Logger::get().TEST_LOG, "Capture moves = {}", moves.size());
  EXPECT_EQ(0, moves.size());

  moves.clear();
  mg.generateKingMoves<MoveGenerator::GENNONCAP>(position, &moves);
  LOG__DEBUG(Logger::get().TEST_LOG, "Non capture moves = {}", moves.size());
  EXPECT_EQ(4, moves.size());
}

/**
 * Test move generation
 */
TEST_F(MoveGenTest, normalMoves) {
  string        fen;
  MoveGenerator mg;
  MoveList      moves;

  fen = "r3k2r/1ppn3p/2q1q1n1/8/2q1Pp2/6R1/p1p2PPP/1R4K1 b kq e3";
  Position position(fen);
  LOG__DEBUG(Logger::get().TEST_LOG, "\n{}", position.printBoard());

  moves.clear();
  mg.generateMoves<MoveGenerator::GENCAP>(position, &moves);
  LOG__DEBUG(Logger::get().TEST_LOG, "Capture moves = {}", moves.size());
  EXPECT_EQ(3, moves.size());

  moves.clear();
  mg.generateMoves<MoveGenerator::GENNONCAP>(position, &moves);
  LOG__DEBUG(Logger::get().TEST_LOG, "Non capture moves = {}", moves.size());
  EXPECT_EQ(49, moves.size());
}

TEST_F(MoveGenTest, castlingMoves) {
  string        fen;
  MoveGenerator mg;
  MoveList      moves;

  fen = "r3k2r/1ppn3p/2q1q1n1/8/2q1Pp2/6R1/p1p2PPP/1R4K1 b kq e3";
  Position position(fen);
  LOG__DEBUG(Logger::get().TEST_LOG, "\n{}", position.printBoard());

  moves.clear();
  mg.generateCastling<MoveGenerator::GENCAP>(position, &moves);
  LOG__DEBUG(Logger::get().TEST_LOG, "Capture moves = {}", moves.size());
  EXPECT_EQ(0, moves.size());

  moves.clear();
  mg.generateCastling<MoveGenerator::GENNONCAP>(position, &moves);
  LOG__DEBUG(Logger::get().TEST_LOG, "Non capture moves = {}", moves.size());
  EXPECT_EQ(2, moves.size());
}

TEST_F(MoveGenTest, pseudoLegalMoves) {
  string        fen;
  MoveGenerator mg;
  MoveList      moves;
  Position      position;

  // Start pos
  fen      = START_POSITION_FEN;
  position = Position(fen);
  LOG__DEBUG(Logger::get().TEST_LOG, "\n{}", position.str());
  moves.clear();
  moves = *mg.generatePseudoLegalMoves<MoveGenerator::GENALL>(position);
  LOG__DEBUG(Logger::get().TEST_LOG, "Moves = {}", moves.size());
  for (Move m : moves) {
    LOG__DEBUG(Logger::get().TEST_LOG, "{}", printMoveVerbose(m));
  }
  ASSERT_EQ(20, moves.size());

  // 86 pseudo legal moves (incl. castling over attacked square)
  fen      = "r3k2r/1ppn3p/2q1q1n1/4P3/2q1Pp2/B5R1/pbp2PPP/1R4K1 b kq e3";
  position = Position(fen);
  LOG__DEBUG(Logger::get().TEST_LOG, "\n{}", position.str());
  moves.clear();
  moves = *mg.generatePseudoLegalMoves<MoveGenerator::GENALL>(position);
  LOG__DEBUG(Logger::get().TEST_LOG, "Moves = {}", moves.size());
  for (Move m : moves) {
    LOG__DEBUG(Logger::get().TEST_LOG, "{}", printMoveVerbose(m));
  }
  ASSERT_EQ(86, moves.size());

  // bug fixed positions
  fen      = "rnbqkbnr/1ppppppp/8/p7/7P/8/PPPPPPP1/RNBQKBNR w KQkq a6";
  position = Position(fen);
  LOG__DEBUG(Logger::get().TEST_LOG, "\n{}", position.str());
  moves.clear();
  moves = *mg.generatePseudoLegalMoves<MoveGenerator::GENALL>(position);
  LOG__DEBUG(Logger::get().TEST_LOG, "Moves = {}", moves.size());
  for (Move m : moves) {
    LOG__DEBUG(Logger::get().TEST_LOG, "{}", printMoveVerbose(m));
  }
  ASSERT_EQ(21, moves.size());

  fen      = "rnbqkbnr/p2ppppp/8/1Pp5/8/8/1PPPPPPP/RNBQKBNR w KQkq c6";
  position = Position(fen);
  LOG__DEBUG(Logger::get().TEST_LOG, "\n{}", position.str());
  moves.clear();
  moves = *mg.generatePseudoLegalMoves<MoveGenerator::GENALL>(position);
  LOG__DEBUG(Logger::get().TEST_LOG, "Moves = {}", moves.size());
  for (Move m : moves) {
    LOG__DEBUG(Logger::get().TEST_LOG, "{}", printMoveVerbose(m));
  }
  ASSERT_EQ(26, moves.size());
}

TEST_F(MoveGenTest, legalMoves) {
  string        fen;
  MoveGenerator mg;
  MoveList      moves;
  Position      position;

  // Startpos
  position = Position(START_POSITION_FEN);
  LOG__DEBUG(Logger::get().TEST_LOG, "\n{}", position.str());
  moves.clear();
  moves = *mg.generateLegalMoves<MoveGenerator::GENALL>(position);
  LOG__DEBUG(Logger::get().TEST_LOG, "Moves = {}", moves.size());
  for (Move m : moves) {
    LOG__DEBUG(Logger::get().TEST_LOG, "{}", printMoveVerbose(m));
  }
  ASSERT_EQ(20, moves.size());

  // 86 pseudo legal moves - 83 legal (incl. castling over attacked square)
  fen      = "r3k2r/1ppn3p/2q1q1n1/4P3/2q1Pp2/B5R1/pbp2PPP/1R4K1 b kq e3";
  position = Position(fen);
  LOG__DEBUG(Logger::get().TEST_LOG, "\n{}", position.str());
  moves.clear();
  moves = *mg.generateLegalMoves<MoveGenerator::GENALL>(position);
  LOG__DEBUG(Logger::get().TEST_LOG, "Moves = {}", moves.size());
  for (Move m : moves) {
    LOG__DEBUG(Logger::get().TEST_LOG, "{}", printMoveVerbose(m));
  }
  ASSERT_EQ(83, moves.size());
  ASSERT_FALSE(position.isLegalMove(createMove<CASTLING>(SQ_E8, SQ_G8)));
}


TEST_F(MoveGenTest, hasLegalMoves) {
  Position position;
  MoveGenerator mg;
  MoveList      moves;

  // check mate position
  position = Position("rn2kbnr/pbpp1ppp/8/1p2p1q1/4K3/3P4/PPP1PPPP/RNBQ1BNR w kq -");
  LOG__DEBUG(Logger::get().TEST_LOG, "\n{}\n", position.str());
  moves               = *mg.generateLegalMoves<MoveGenerator::GENALL>(position);
  ASSERT_EQ(0, moves.size());
  ASSERT_FALSE(mg.hasLegalMove(position));
  ASSERT_TRUE(position.hasCheck());

  // stale mate position
  position = Position("7k/5K2/6Q1/8/8/8/8/8 b - -");
  LOG__DEBUG(Logger::get().TEST_LOG, "\n{}\n", position.str());
  moves               = *mg.generateLegalMoves<MoveGenerator::GENALL>(position);
  ASSERT_EQ(0, moves.size());
  ASSERT_FALSE(mg.hasLegalMove(position));
  ASSERT_FALSE(position.hasCheck());

  // only en passant
  position = Position("8/8/8/8/5Pp1/6P1/7k/K3BQ2 b - f3");
  LOG__DEBUG(Logger::get().TEST_LOG, "\n{}\n", position.str());
  moves               = *mg.generateLegalMoves<MoveGenerator::GENALL>(position);
  ASSERT_EQ(1, moves.size());
  ASSERT_TRUE(mg.hasLegalMove(position));
  ASSERT_FALSE(position.hasCheck());
}

TEST_F(MoveGenTest, validateMove) {
  string        fen;
  MoveGenerator mg;
  MoveList      moves;

  // 86 pseudo legal moves (incl. castling over attacked square)
  fen = "r3k2r/1ppn3p/2q1q1n1/4P3/2q1Pp2/B5R1/pbp2PPP/1R4K1 b kq e3";
  Position position(fen);

  ASSERT_TRUE(mg.validateMove(position, createMove("b2e5")));
  ASSERT_TRUE(mg.validateMove(position, createMove("e6e5")));
  ASSERT_TRUE(mg.validateMove(position, createMove("c4e4")));
  ASSERT_TRUE(mg.validateMove(position, createMove("c6e4")));
  ASSERT_TRUE(mg.validateMove(position, createMove<PROMOTION>("a2a1q")));
  ASSERT_TRUE(mg.validateMove(position, createMove<PROMOTION>("c2c1q")));
  ASSERT_TRUE(mg.validateMove(position, createMove<PROMOTION>("a2a1n")));
  ASSERT_TRUE(mg.validateMove(position, createMove<PROMOTION>("c2c1n")));
  ASSERT_FALSE(mg.validateMove(position, createMove("e2e4")));
  ASSERT_FALSE(mg.validateMove(position, createMove("b8c8")));
  ASSERT_FALSE(mg.validateMove(position, createMove("a2b3")));
  ASSERT_FALSE(mg.validateMove(position, createMove("b1c3")));
  ASSERT_FALSE(mg.validateMove(position, MOVE_NONE));
}

TEST_F(MoveGenTest, onDemandGen) {
  string        fen;
  MoveGenerator mg;
  MoveList      moves;

  // 86 pseudo legal moves (incl. castling over attacked square)
  fen = "r3k2r/1ppn3p/2q1q1n1/4P3/2q1Pp2/B5R1/pbp2PPP/1R4K1 b kq e3";
  Position position(fen);
  LOG__DEBUG(Logger::get().TEST_LOG, "\n{}\n", position.str());

  Move move;
  int  counter = 0;
  while ((move = mg.getNextPseudoLegalMove<MoveGenerator::GENALL>(position)) != MOVE_NONE) {
    LOG__DEBUG(Logger::get().TEST_LOG, "{}", printMoveVerbose(move));
    counter++;
  }
  LOG__DEBUG(Logger::get().TEST_LOG, "Moves = {}", counter);
  ASSERT_EQ(86, counter);

  // 218 moves
  fen      = "R6R/3Q4/1Q4Q1/4Q3/2Q4Q/Q4Q2/pp1Q4/kBNN1KB1 w - - 0 1";
  position = Position(fen);
  LOG__DEBUG(Logger::get().TEST_LOG, "\n{}\n", position.str());
  counter = 0;
  while ((move = mg.getNextPseudoLegalMove<MoveGenerator::GENALL>(position)) != MOVE_NONE) {
    LOG__DEBUG(Logger::get().TEST_LOG, "{}", printMoveVerbose(move));
    counter++;
  }
  LOG__DEBUG(Logger::get().TEST_LOG, "Moves = {}", counter);
  ASSERT_EQ(218, counter);
}

TEST_F(MoveGenTest, storeKiller) {
  string        fen;
  MoveGenerator mg;
  MoveList      moves;

  // 86 pseudo legal moves (incl. castling over attacked square)
  fen = "r3k2r/1ppn3p/2q1q1n1/4P3/2q1Pp2/B5R1/pbp2PPP/1R4K1 b kq e3";
  Position position(fen);

  const MoveList* allMoves = mg.generatePseudoLegalMoves<MoveGenerator::GENNONCAP>(position);

  // add first two killers
  mg.storeKiller(allMoves->at(11), 2);
  mg.storeKiller(allMoves->at(21), 2);
  ASSERT_EQ(mg.maxNumberOfKiller, mg.killerMoves.size());
  ASSERT_EQ(allMoves->at(11), mg.killerMoves.at(1));
  ASSERT_EQ(allMoves->at(21), mg.killerMoves.at(0));

  // add a killer already in the list - should not change
  mg.storeKiller(allMoves->at(21), 2);
  ASSERT_EQ(mg.maxNumberOfKiller, mg.killerMoves.size());
  ASSERT_EQ(allMoves->at(21), mg.killerMoves.at(0));
  ASSERT_EQ(allMoves->at(11), mg.killerMoves.at(1));

  // add a killer NOT already in the list - should change
  mg.storeKiller(allMoves->at(31), 2);
  ASSERT_EQ(mg.maxNumberOfKiller, mg.killerMoves.size());
  ASSERT_EQ(allMoves->at(31), mg.killerMoves.at(0));
  ASSERT_EQ(allMoves->at(21), mg.killerMoves.at(1));

  mg.reset();
  ASSERT_EQ(0, mg.killerMoves.size());

  // need to regenerate moves as reset has reset list
  allMoves = mg.generatePseudoLegalMoves<MoveGenerator::GENNONCAP>(position);

  // add a killer NOT already in the list - should change
  mg.storeKiller(allMoves->at(31), 2);
  ASSERT_EQ(1, mg.killerMoves.size());
  ASSERT_EQ(allMoves->at(31), mg.killerMoves.at(0));
}

TEST_F(MoveGenTest, pushKiller) {
  string        fen;
  MoveGenerator mg;
  MoveList      moves;

  // 86 pseudo legal moves (incl. castling over attacked square)
  fen = "r3k2r/1ppn3p/2q1q1n1/4P3/2q1Pp2/B5R1/pbp2PPP/1R4K1 b kq e3";
  Position position(fen);

  const MoveList* allMoves = mg.generatePseudoLegalMoves<MoveGenerator::GENALL>(position);
  int             i        = 0;
  LOG__DEBUG(Logger::get().TEST_LOG, "Moves w/o pushed killer:");
  for (auto m : *allMoves) {
    LOG__DEBUG(Logger::get().TEST_LOG, "{} {}", std::to_string(++i), printMoveVerbose(m));
  }
  ASSERT_EQ(86, i);
  mg.storeKiller(allMoves->at(21), 2);
  mg.storeKiller(allMoves->at(81), 2);
  LOG__DEBUG(Logger::get().TEST_LOG, "Killer: {} {}",
             printMove(allMoves->at(21)), printMove(allMoves->at(81)));

  LOG__DEBUG(Logger::get().TEST_LOG, "Moves with pushed killer:");
  Move move;
  int  counter = 0;
  while ((move = mg.getNextPseudoLegalMove<MoveGenerator::GENALL>(position)) != MOVE_NONE) {
    if (counter == 18) {
      EXPECT_EQ(moveOf(allMoves->at(21)), moveOf(move));
      LOG__DEBUG(Logger::get().TEST_LOG, "Killer");
    }
    else if (counter == 33) {
      EXPECT_EQ(moveOf(allMoves->at(81)), moveOf(move));
      LOG__DEBUG(Logger::get().TEST_LOG, "Killer");
    }
    counter++;
    LOG__DEBUG(Logger::get().TEST_LOG, "{} {}", std::to_string(counter), printMoveVerbose(move));
  }
  LOG__DEBUG(Logger::get().TEST_LOG, "Moves: {}", counter);
  ASSERT_EQ(86, counter);
}

TEST_F(MoveGenTest, pvMove) {
  string        fen;
  MoveGenerator mg;
  MoveList      moves;

  // 86 pseudo legal moves (incl. castling over attacked square)
  fen = "r3k2r/1ppn3p/2q1q1n1/4P3/2q1Pp2/B5R1/pbp2PPP/1R4K1 w kq e3";
  Position position(fen);

  // Test #1: best move is capturing and generating all moves
  Move pvMove = createMove("b1b2");
  mg.setPV(pvMove);
  Move move;
  int  counter = 0;
  // generate all moves
  while ((move = mg.getNextPseudoLegalMove<MoveGenerator::GENALL>(position)) != MOVE_NONE) {
    if (counter == 0) { // first move must be pv move
      EXPECT_EQ(pvMove, move);
    }
    else { // no more pv move after first move
      EXPECT_NE(pvMove, move);
    }
    counter++;
  }
  ASSERT_EQ(27, counter);
  mg.resetOnDemand();

  // Test #2: best move is capturing and generating capturing moves
  pvMove = createMove("b1b2");
  mg.setPV(pvMove);
  counter = 0;
  // generate all moves
  while ((move = mg.getNextPseudoLegalMove<MoveGenerator::GENCAP>(position)) != MOVE_NONE) {
    if (counter == 0) { // first move must be pv move
      EXPECT_EQ(pvMove, move);
    }
    else { // no more pv move after first move
      EXPECT_NE(pvMove, move);
    }
    counter++;
  }
  ASSERT_EQ(4, counter);
  mg.resetOnDemand();

  // Test #3: best move is non-capturing and generating all moves
  pvMove = createMove("h2h3");
  mg.setPV(pvMove);
  counter = 0;
  // generate all moves
  while ((move = mg.getNextPseudoLegalMove<MoveGenerator::GENALL>(position)) != MOVE_NONE) {
    if (counter == 0) { // first move must be pv move
      EXPECT_EQ(pvMove, move);
    }
    else { // no more pv move after first move
      EXPECT_NE(pvMove, move);
    }
    counter++;
  }
  ASSERT_EQ(27, counter);
  mg.resetOnDemand();

  // Test #4: best move is non-capturing and generating capturing moves
  pvMove = createMove("h2h3");
  mg.setPV(pvMove);
  counter = 0;
  // generate all moves
  while ((move = mg.getNextPseudoLegalMove<MoveGenerator::GENCAP>(position)) != MOVE_NONE) {
    if (counter == 0) { // first move can't be non capturing pv move
      EXPECT_NE(pvMove, move);
    }
    else { // no more pv move after first move
      EXPECT_NE(pvMove, move);
    }
    counter++;
  }
  ASSERT_EQ(4, counter);
  mg.resetOnDemand();

  // Test #4: best move is non-capturing and generating non-capturing moves
  // not very relevant for searching
  pvMove = createMove("h2h3");
  mg.setPV(pvMove);
  counter = 0;
  // generate all moves
  while ((move = mg.getNextPseudoLegalMove<MoveGenerator::GENNONCAP>(position)) != MOVE_NONE) {
    if (counter == 0) { // first move must be pv move
      EXPECT_EQ(pvMove, move);
    }
    else { // no more pv move after first move
      EXPECT_NE(pvMove, move);
    }
    counter++;
  }
  ASSERT_EQ(23, counter);
  mg.resetOnDemand();
}
