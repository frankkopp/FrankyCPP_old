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
#include <ostream>
#include <string>

#include "../../src/logging.h"
#include "../../src/Bitboards.h"
#include "../../src/Position.h"
#include "../../src/MoveGenerator.h"

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

  cout << position.printBoard() << endl;

  mg.generatePawnMoves<MoveGenerator::GENCAP>(&position, &moves);

  ASSERT_EQ(10, moves.size());
  string expected = "c2b1qc2b1rc2b1bc2b1na2b1qa2b1ra2b1ba2b1nf4g3f4e3";
  string actual = "";
  for (Move m : moves) actual.append(printMove(m));
  ASSERT_EQ(expected, actual);

  sort(moves.begin(), moves.end());
  expected = "a2b1qc2b1qa2b1nc2b1nf4g3f4e3a2b1rc2b1ra2b1bc2b1b";
  actual = "";
  for (Move m : moves) actual.append(printMove(m));
  ASSERT_EQ(expected, actual);

  cout << "Moves CAP: " << moves.size() << endl;
  for (Move m : moves) {
    cout << printMoveVerbose(m) << " (" << int(m) << ")" << endl;
  }

  NEWLINE;

  moves.clear();
  mg.generatePawnMoves<MoveGenerator::GENNONCAP>(&position, &moves);

  ASSERT_EQ(13, moves.size());
  expected = "a2a1qa2a1ra2a1ba2a1nc2c1qc2c1rc2c1bc2c1nb7b5h7h5f4f3b7b6h7h6";
  actual = "";
  for (Move m : moves) actual.append(printMove(m));
  ASSERT_EQ(expected, actual);

  sort(moves.begin(), moves.end());
  expected = "a2a1qc2c1qa2a1nc2c1nf4f3h7h6b7b5h7h5b7b6a2a1bc2c1ba2a1rc2c1r";
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
  mg.generateKingMoves<MoveGenerator::GENCAP>(&position, &moves);
  cout << "Moves CAP: " << moves.size() << endl;
  sort(moves.begin(), moves.end());
  for (Move m : moves) {
    cout << printMoveVerbose(m) << " (" << int(m) << ")" << endl;
  }

  NEWLINE;

  moves.clear();
  mg.generateKingMoves<MoveGenerator::GENNONCAP>(&position, &moves);
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
  mg.generateMoves<MoveGenerator::GENCAP>(&position, &moves);
  cout << "Moves CAP: " << moves.size() << endl;
  sort(moves.begin(), moves.end());
  for (Move m : moves) {
    cout << printMoveVerbose(m) << " (" << int(m) << ")" << endl;
  }

  NEWLINE;

  moves.clear();
  mg.generateMoves<MoveGenerator::GENNONCAP>(&position, &moves);
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
  mg.generateCastling<MoveGenerator::GENCAP>(&position, &moves);
  cout << "Moves CAP: " << moves.size() << endl;
  for (Move m : moves) {
    cout << printMoveVerbose(m) << endl;
  }

  NEWLINE;
  moves.clear();
  mg.generateCastling<MoveGenerator::GENNONCAP>(&position, &moves);
  cout << "Moves NONCAP: " << moves.size() << endl;
  for (Move m : moves) {
    cout << printMoveVerbose(m) << endl;
  }
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
  moves = *mg.generatePseudoLegalMoves<MoveGenerator::GENALL>(&position);
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
  moves = *mg.generatePseudoLegalMoves<MoveGenerator::GENALL>(&position);
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

  moves = *mg.generatePseudoLegalMoves<MoveGenerator::GENALL>(&position);
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

  moves = *mg.generatePseudoLegalMoves<MoveGenerator::GENALL>(&position);
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

  moves = *mg.generateLegalMoves<MoveGenerator::GENALL>(&position);
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

  moves = *mg.generatePseudoLegalMoves<MoveGenerator::GENALL>(&position);
  cout << "Pseudo Legal Moves: " << moves.size() << endl;
  for (Move m : moves) {
    cout << printMoveVerbose(m) << endl;
  }
  ASSERT_EQ(86, moves.size());
  NEWLINE;

  moves.clear();
  moves = *mg.generateLegalMoves<MoveGenerator::GENALL>(&position);
  cout << "Legal Moves: " << moves.size() << endl;
  for (Move m : moves) {
    cout << printMoveVerbose(m) << endl;
  }
  cout << moves << endl;
  ASSERT_FALSE(position.isLegalMove(createMove<CASTLING>(SQ_E8, SQ_G8)));
  ASSERT_EQ(83, moves.size());
  NEWLINE;
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
    move = mg.getNextPseudoLegalMove<MoveGenerator::GENALL>(&position);
    if (move == NOMOVE) break;
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
    move = mg.getNextPseudoLegalMove<MoveGenerator::GENALL>(&position);
    if (move == NOMOVE) break;
    cout << printMoveVerbose(move) << " (" << int(move) << ")" << endl;
    counter++;
  }
  println("Moves: " + to_string(counter));
  ASSERT_EQ(218, counter);
  NEWLINE;

}

TEST_F(MoveGenTest, hasLegalMoves) {
  string fen;
  MoveGenerator mg;
  MoveList moves;

  fen = "rn2kbnr/pbpp1ppp/8/1p2p1q1/4K3/3P4/PPP1PPPP/RNBQ1BNR w kq -";
  Position position(fen);
  println(position.str());

  moves = *mg.generateLegalMoves<MoveGenerator::GENALL>(&position);
  const bool expected = mg.hasLegalMove(&position);

  ASSERT_EQ(0, moves.size());
  ASSERT_FALSE(expected);

}

TEST_F(MoveGenTest, debug) {
  string fen;
  MoveGenerator mg;
  MoveList moves;

  fen = "r1b1kbnr/ppp2ppp/8/4n3/8/4Q3/PP2PPPP/RNB1KBNR b KQkq - 1 7"; // e5d3
  fen = "rn1qkbnr/ppp1pQpp/8/6N1/8/8/Pp1PPPP1/RNB1KB1R b KQkq - 0 6"; // b2a1q
  Position position(fen);
  println(position.str());

  moves = *mg.generateLegalMoves<MoveGenerator::GENALL>(&position);
  println(printMoveList(moves));

}