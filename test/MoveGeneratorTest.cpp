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

#include "Logging.h"
#include "Bitboards.h"
#include "Position.h"
#include "MoveGenerator.h"

using namespace std;
using testing::Eq;

class MoveGenTest : public ::testing::Test {
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
};

/**
 * Test pawn move generation
 */
TEST_F(MoveGenTest, pawnMoves) {
  string fen;
  MoveGenerator mg;

  fen = "r3k2r/1ppn3p/2q1q1n1/4P3/2q1Pp2/B5R1/pbp2PPP/1R4K1 b kq e3";

  Position position(fen);
  MoveList moves;

  //cout << position.printBoard() << endl;

  mg.generatePawnMoves<MoveGenerator::GENCAP>(position, &moves);

  ASSERT_EQ(10, moves.size());
  string expected = "c2b1Qc2b1Rc2b1Bc2b1Na2b1Qa2b1Ra2b1Ba2b1Nf4g3f4e3";
  string actual = "";
  for (Move m : moves) actual.append(printMove(m));
  ASSERT_EQ(expected, actual);

  sort(moves.begin(), moves.end());
  expected = "a2b1Qc2b1Qa2b1Nc2b1Nf4g3f4e3a2b1Rc2b1Ra2b1Bc2b1B";
  actual = "";
  for (Move m : moves) actual.append(printMove(m));
  ASSERT_EQ(expected, actual);

  //cout << "Moves CAP: " << moves.size() << endl;
  //for (Move m : moves) {
  //  cout << printMoveVerbose(m) << " (" << int(m) << ")" << endl;
  //}

  NEWLINE;

  moves.clear();
  mg.generatePawnMoves<MoveGenerator::GENNONCAP>(position, &moves);

  ASSERT_EQ(13, moves.size());
  expected = "a2a1Qa2a1Ra2a1Ba2a1Nc2c1Qc2c1Rc2c1Bc2c1Nb7b5h7h5f4f3b7b6h7h6";
  actual = "";
  for (Move m : moves) actual.append(printMove(m));
  ASSERT_EQ(expected, actual);

  stable_sort(moves.begin(), moves.end());
  expected = "a2a1Qc2c1Qa2a1Nc2c1Nf4f3h7h6b7b5h7h5b7b6a2a1Bc2c1Ba2a1Rc2c1R";
  actual = "";
  for (Move m : moves) actual.append(printMove(m));
  ASSERT_EQ(expected, actual);

  cout << "Moves NONCAP: " << moves.size() << endl;
  for (Move m : moves) {
    cout << printMoveVerbose(m) << " (" << int(m) << ")" << endl;
  }

}

/**
 * TODO create real tests
 */
TEST_F(MoveGenTest, kingMoves) {
  string fen;
  MoveGenerator mg;
  MoveList moves;

  fen = "r3k2r/1ppn3p/2q1q1n1/4P3/2q1Pp2/B5R1/pbp2PPP/1R4K1 b kq e3";
  Position position(fen);
  cout << position.printBoard() << endl;

  moves.clear();
  mg.generateKingMoves<MoveGenerator::GENCAP>(position, &moves);
  cout << "Moves CAP: " << moves.size() << endl;
  sort(moves.begin(), moves.end());
  for (Move m : moves) {
    cout << printMoveVerbose(m) << " (" << int(m) << ")" << endl;
  }

  NEWLINE;

  moves.clear();
  mg.generateKingMoves<MoveGenerator::GENNONCAP>(position, &moves);
  cout << "Moves NONCAP: " << moves.size() << endl;
  sort(moves.begin(), moves.end());
  for (Move m : moves) {
    cout << printMoveVerbose(m) << " (" << int(m) << ")" << endl;
  }
}

/**
 * Test move generation
 // TODO create real tests
 */
TEST_F(MoveGenTest, normalMoves) {
  string fen;
  MoveGenerator mg;
  MoveList moves;

  fen = "r3k2r/1ppn3p/2q1q1n1/8/2q1Pp2/6R1/p1p2PPP/1R4K1 b kq e3";
  Position position(fen);
  cout << position.printBoard() << endl;

  moves.clear();
  mg.generateMoves<MoveGenerator::GENCAP>(position, &moves);
  cout << "Moves CAP: " << moves.size() << endl;
  sort(moves.begin(), moves.end());
  for (Move m : moves) {
    cout << printMoveVerbose(m) << " (" << int(m) << ")" << endl;
  }

  NEWLINE;

  moves.clear();
  mg.generateMoves<MoveGenerator::GENNONCAP>(position, &moves);
  cout << "Moves NONCAP: " << moves.size() << endl;
  sort(moves.begin(), moves.end());
  for (Move m : moves) {
    cout << printMoveVerbose(m) << " (" << int(m) << ")" << endl;
  }
}

TEST_F(MoveGenTest, castlingMoves) {
  string fen;
  MoveGenerator mg;
  MoveList moves;

  fen = "r3k2r/1ppn3p/2q1q1n1/8/2q1Pp2/6R1/p1p2PPP/1R4K1 b kq e3";
  Position position(fen);
  cout << position.printBoard() << endl;

  moves.clear();
  mg.generateCastling<MoveGenerator::GENCAP>(position, &moves);
  cout << "Moves CAP: " << moves.size() << endl;
  for (Move m : moves) {
    cout << printMoveVerbose(m) << endl;
  }

  NEWLINE;
  moves.clear();
  mg.generateCastling<MoveGenerator::GENNONCAP>(position, &moves);
  cout << "Moves NONCAP: " << moves.size() << endl;
  for (Move m : moves) {
    cout << printMoveVerbose(m) << endl;
  }
}


TEST_F(MoveGenTest, hasLegalMoves) {
  string fen;
  MoveGenerator mg;
  MoveList moves;

  fen = "rn2kbnr/pbpp1ppp/8/1p2p1q1/4K3/3P4/PPP1PPPP/RNBQ1BNR w kq -";
  Position position(fen);
  println(position.str());

  moves = *mg.generateLegalMoves<MoveGenerator::GENALL>(position);
  const bool expected = mg.hasLegalMove(position);

  ASSERT_EQ(0, moves.size());
  ASSERT_FALSE(expected);

}

TEST_F(MoveGenTest, pseudoLegalMoves) {
  string fen;
  MoveGenerator mg;
  MoveList moves;
  Position position;

  // Start pos
  fen = START_POSITION_FEN;
  position = Position(fen);
  cout << position.str() << endl;

  moves.clear();
  moves = *mg.generatePseudoLegalMoves<MoveGenerator::GENALL>(position);
  cout << "Moves ALL: " << moves.size() << endl;
  for (Move m : moves) {
    cout << printMoveVerbose(m) << " (" << int(m) << ")" << endl;
  }
  ASSERT_EQ(20, moves.size());
  NEWLINE;

  // 86 pseudo legal moves (incl. castling over attacked square)
  fen = "r3k2r/1ppn3p/2q1q1n1/4P3/2q1Pp2/B5R1/pbp2PPP/1R4K1 b kq e3";
  position = Position(fen);
  cout << position.str() << endl;

  moves.clear();
  moves = *mg.generatePseudoLegalMoves<MoveGenerator::GENALL>(position);
  cout << "Moves ALL: " << moves.size() << endl;
  for (Move m : moves) {
    cout << printMoveVerbose(m) << " (" << int(m) << ")" << endl;
  }
  ASSERT_EQ(86, moves.size());
  NEWLINE;

  // bug fixed positions
  fen = "rnbqkbnr/1ppppppp/8/p7/7P/8/PPPPPPP1/RNBQKBNR w KQkq a6";
  position = Position(fen);
  moves.clear();
  cout << position.str() << endl;

  moves = *mg.generatePseudoLegalMoves<MoveGenerator::GENALL>(position);
  cout << "Moves CAP: " << moves.size() << endl;
  for (Move m : moves) {
    cout << printMoveVerbose(m) << " (" << int(m) << ")" << endl;
  }
  ASSERT_EQ(21, moves.size());
  NEWLINE;

  fen = "rnbqkbnr/p2ppppp/8/1Pp5/8/8/1PPPPPPP/RNBQKBNR w KQkq c6";
  position = Position(fen);
  moves.clear();
  cout << position.str() << endl;

  moves = *mg.generatePseudoLegalMoves<MoveGenerator::GENALL>(position);
  cout << "Moves CAP: " << moves.size() << endl;
  for (Move m : moves) {
    cout << printMoveVerbose(m) << " (" << int(m) << ")" << endl;
  }
  ASSERT_EQ(26, moves.size());
  NEWLINE;
}

TEST_F(MoveGenTest, legalMoves) {
  string fen;
  MoveGenerator mg;
  MoveList moves;
  Position position;

  // 86 pseudo legal moves (incl. castling over attacked square)
  position = Position(START_POSITION_FEN);
  cout << position.printBoard() << endl;

  moves = *mg.generateLegalMoves<MoveGenerator::GENALL>(position);
  cout << "Legal Moves: " << moves.size() << endl;
  for (Move m : moves) {
    cout << printMoveVerbose(m) << endl;
  }
  ASSERT_EQ(20, moves.size());
  NEWLINE;

  // 86 pseudo legal moves (incl. castling over attacked square)
  fen = "r3k2r/1ppn3p/2q1q1n1/4P3/2q1Pp2/B5R1/pbp2PPP/1R4K1 b kq e3";
  position = Position(fen);
  cout << position.printBoard() << endl;

  moves = *mg.generatePseudoLegalMoves<MoveGenerator::GENALL>(position);
  cout << "Pseudo Legal Moves: " << moves.size() << endl;
  for (Move m : moves) {
    cout << printMoveVerbose(m) << endl;
  }
  ASSERT_EQ(86, moves.size());
  NEWLINE;

  moves.clear();
  moves = *mg.generateLegalMoves<MoveGenerator::GENALL>(position);
  cout << "Legal Moves: " << moves.size() << endl;
  for (Move m : moves) {
    cout << printMoveVerbose(m) << endl;
  }
  cout << moves << endl;
  ASSERT_FALSE(position.isLegalMove(createMove<CASTLING>(SQ_E8, SQ_G8)));
  ASSERT_EQ(83, moves.size());
  NEWLINE;
}


TEST_F(MoveGenTest, validateMove) {
  string fen;
  MoveGenerator mg;
  MoveList moves;

  // 86 pseudo legal moves (incl. castling over attacked square)
  fen = "r3k2r/1ppn3p/2q1q1n1/4P3/2q1Pp2/B5R1/pbp2PPP/1R4K1 b kq e3";
  Position position(fen);

  ASSERT_TRUE (mg.validateMove(position, createMove("b2e5")));
  ASSERT_TRUE (mg.validateMove(position, createMove("e6e5")));
  ASSERT_TRUE (mg.validateMove(position, createMove("c4e4")));
  ASSERT_TRUE (mg.validateMove(position, createMove("c6e4")));
  ASSERT_TRUE (mg.validateMove(position, createMove<PROMOTION>("a2a1q")));
  ASSERT_TRUE (mg.validateMove(position, createMove<PROMOTION>("c2c1q")));
  ASSERT_TRUE (mg.validateMove(position, createMove<PROMOTION>("a2a1n")));
  ASSERT_TRUE (mg.validateMove(position, createMove<PROMOTION>("c2c1n")));
  ASSERT_FALSE (mg.validateMove(position, createMove("e2e4")));
  ASSERT_FALSE (mg.validateMove(position, createMove("b8c8")));
  ASSERT_FALSE (mg.validateMove(position, createMove("a2b3")));
  ASSERT_FALSE (mg.validateMove(position, createMove("b1c3")));
  ASSERT_FALSE (mg.validateMove(position, MOVE_NONE));

}

TEST_F(MoveGenTest, onDemandGen) {
  string fen;
  MoveGenerator mg;
  MoveList moves;

  // 86 pseudo legal moves (incl. castling over attacked square)
  fen = "r3k2r/1ppn3p/2q1q1n1/4P3/2q1Pp2/B5R1/pbp2PPP/1R4K1 b kq e3";
  Position position(fen);
  cout << position.str() << endl;

  Move move;
  int counter = 0;
  while (true) {
    move = mg.getNextPseudoLegalMove<MoveGenerator::GENALL>(position);
    if (move == MOVE_NONE) break;
    cout << printMoveVerbose(move) << " (" << int(move) << ")" << endl;
    counter++;
  }
  println("Moves: " + to_string(counter));
  ASSERT_EQ(86, counter);
  NEWLINE;

  // 218 moves
  fen = "R6R/3Q4/1Q4Q1/4Q3/2Q4Q/Q4Q2/pp1Q4/kBNN1KB1 w - - 0 1";
  position = Position(fen);
  cout << position.str() << endl;

  counter = 0;
  while (true) {
    move = mg.getNextPseudoLegalMove<MoveGenerator::GENALL>(position);
    if (move == MOVE_NONE) break;
    cout << printMoveVerbose(move) << " (" << int(move) << ")" << endl;
    counter++;
  }
  println("Moves: " + to_string(counter));
  ASSERT_EQ(218, counter);
  NEWLINE;

}

TEST_F(MoveGenTest, storeKiller) {
  string fen;
  MoveGenerator mg;
  MoveList moves;

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
  NEWLINE;

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
  string fen;
  MoveGenerator mg;
  MoveList moves;

  // 86 pseudo legal moves (incl. castling over attacked square)
  fen = "r3k2r/1ppn3p/2q1q1n1/4P3/2q1Pp2/B5R1/pbp2PPP/1R4K1 b kq e3";
  Position position(fen);

  const MoveList* allMoves = mg.generatePseudoLegalMoves<MoveGenerator::GENALL>(position);
  int i = 0;
  for (auto m : *allMoves) {
    println("ORIG: " + std::to_string(i++) + " " + printMoveVerbose(m));
  }
  mg.storeKiller(allMoves->at(21), 2);
  mg.storeKiller(allMoves->at(81), 2);

  Move move;
  int counter = 0;
  while (true) {
    move = mg.getNextPseudoLegalMove<MoveGenerator::GENALL>(position);
    if (move == MOVE_NONE) break;
    cout << counter << " " << printMoveVerbose(move) << " (" << int(move) << ")" << endl;
    if (counter == 18) { ASSERT_EQ(allMoves->at(21), move); }
    else if (counter == 33) {ASSERT_EQ(allMoves->at(81), move); }
    counter++;
  }
  println("Moves: " + to_string(counter));
  ASSERT_EQ(86, counter);
}

TEST_F(MoveGenTest, pvMove) {
  string fen;
  MoveGenerator mg;
  MoveList moves;

  // 86 pseudo legal moves (incl. castling over attacked square)
  fen = "r3k2r/1ppn3p/2q1q1n1/4P3/2q1Pp2/B5R1/pbp2PPP/1R4K1 w kq e3";
  Position position(fen);
  // best move
  Move pvMove = createMove("b1e1");

  mg.setPV(pvMove);

  Move move;
  int counter = 0;
  while (true) {
    move = mg.getNextPseudoLegalMove<MoveGenerator::GENALL>(position);
    if (move == MOVE_NONE) break;
    fprintln("{} {} ({})", counter, printMoveVerbose(move), move);
    counter++;
  }
  println("Moves: " + to_string(counter));
  ASSERT_EQ(27, counter);
}

TEST_F(MoveGenTest, swap) {
  string fen;
  MoveGenerator mg;
  const MoveList* moves;
  Position position;

  // 86 pseudo legal moves (incl. castling over attacked square)
  fen = "r3k2r/1ppn3p/2q1q1n1/4P3/2q1Pp2/B5R1/pbp2PPP/1R4K1 b kq e3";
  position = Position(fen);
  moves = mg.generatePseudoLegalMoves<MoveGenerator::GENALL>(position);
  ASSERT_EQ(86, moves->size());

  MoveList test;
  test.swap(const_cast<deque<Move, allocator<Move>> &>(*moves));
  ASSERT_EQ(86, test.size());
  ASSERT_EQ(0, moves->size());

  // 86 pseudo legal moves (incl. castling over attacked square)
  fen = "r3k2r/1ppn3p/2q1q1n1/4P3/2q1Pp2/B5R1/pbp2PPP/1R4K1 b kq e3";
  position = Position(fen);
  moves = mg.generatePseudoLegalMoves<MoveGenerator::GENALL>(position);
  ASSERT_EQ(86, moves->size());

}

TEST_F(MoveGenTest, DISABLED_PERFT_mps) {
  string fen;
  MoveGenerator mg;
  Position position;
  uint64_t generatedMoves = 0, sum = 0;

  // 86 pseudo legal moves (incl. castling over attacked square)
  fen = "r3k2r/1ppn3p/2q1q1n1/4P3/2q1Pp2/B5R1/pbp2PPP/1R4K1 b kq e3";
  position = Position(fen);

  const MoveList* moves = mg.generatePseudoLegalMoves<MoveGenerator::GENALL>(position);
  Move killer1 = moves->at(35);
  Move killer2 = moves->at(85);

  auto start = std::chrono::high_resolution_clock::now();
  auto finish = std::chrono::high_resolution_clock::now();

  for (int i = 0; i < 1'000'000; i++) {
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

  const double sec = double(sum) / 1'000'000'000;
  fmt::print("Move generated: {:n} in {:f} seconds\n", generatedMoves, sec);
  uint64_t mps = generatedMoves / sec;
  fmt::print("Move generated per second: {:n}", mps);
  NEWLINE;
  SUCCEED();
}
