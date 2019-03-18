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
#include <gmock/gmock.h>
#include <ostream>
#include <string>

#include "../../src/MoveGenerator.h"

using namespace std;

using testing::Eq;

/**
 * Test pawn move generation
 // TODO create real tests
 */
TEST(MoveGenTest, pawnMoves) {
  Position::init();
  Bitboards::init();
  NEWLINE;

  string fen;
  MoveGenerator mg;

  fen = "r3k2r/1ppn3p/2q1q1n1/8/2q1Pp2/6R1/p1p2PPP/1R4K1 b kq e3";

  Position position(fen);
  vector<Move> moves;

  cout << position.printBoard() << endl;

  GenMode genMode = GENCAP;
  mg.generatePawnMoves(genMode, &position, &moves);
  cout << "Moves CAP: " << moves.size() << endl;
  for (Move m : moves) {
    cout << printMove(m) << endl;
  }

  NEWLINE;
  moves.clear();
  genMode = GENNONCAP;
  mg.generatePawnMoves(genMode, &position, &moves);
  cout << "Moves NONCAP: " << moves.size() << endl;
  for (Move m : moves) {
    cout << printMove(m) << endl;
  }

}

/**
 * Test move generation
 // TODO create real tests
 */
TEST(MoveGenTest, kingMoves) {
  Position::init();
  Bitboards::init();
  NEWLINE;

  string fen;
  MoveGenerator mg;
  GenMode genMode;
  vector<Move> moves;

  fen = "r3k2r/1ppn3p/2q1q1n1/8/2q1Pp2/6R1/p1p2PPP/1R4K1 b kq e3";
  Position position(fen);
  cout << position.printBoard() << endl;

  genMode = GENCAP;
  mg.generateKingMoves(genMode, &position, &moves);
  cout << "Moves CAP: " << moves.size() << endl;
  for (Move m : moves) {
    cout << printMove(m) << endl;
  }

  NEWLINE;

  genMode = GENNONCAP;
  mg.generateKingMoves(genMode, &position, &moves);
  cout << "Moves NONCAP: " << moves.size() << endl;
  for (Move m : moves) {
    cout << printMove(m) << endl;
  }
}

/**
 * Test move generation
 // TODO create real tests
 */
TEST(MoveGenTest, normalMoves) {
  Position::init();
  Bitboards::init();
  NEWLINE;

  string fen;
  MoveGenerator mg;
  GenMode genMode;
  vector<Move> moves;

  fen = "r3k2r/1ppn3p/2q1q1n1/8/2q1Pp2/6R1/p1p2PPP/1R4K1 b kq e3";
  Position position(fen);
  cout << position.printBoard() << endl;

  genMode = GENCAP;
  mg.generateMoves(genMode, &position, &moves);
  cout << "Moves CAP: " << moves.size() << endl;
  for (Move m : moves) {
    cout << printMove(m) << endl;
  }

  NEWLINE;

  genMode = GENNONCAP;
  mg.generateMoves(genMode, &position, &moves);
  cout << "Moves NONCAP: " << moves.size() << endl;
  for (Move m : moves) {
    cout << printMove(m) << endl;
  }
}

/**
 * Test move generation
 // TODO create real tests
 */
TEST(MoveGenTest, castlingMoves) {
  Position::init();
  Bitboards::init();
  NEWLINE;

  string fen;
  MoveGenerator mg;
  GenMode genMode;
  vector<Move> moves;

  fen = "r3k2r/1ppn3p/2q1q1n1/8/2q1Pp2/6R1/p1p2PPP/1R4K1 b kq e3";
  Position position(fen);
  cout << position.printBoard() << endl;

  genMode = GENCAP;
  mg.generateCastling(genMode, &position, &moves);
  cout << "Moves CAP: " << moves.size() << endl;
  for (Move m : moves) {
    cout << printMove(m) << endl;
  }

  NEWLINE;

  genMode = GENNONCAP;
  mg.generateCastling(genMode, &position, &moves);
  cout << "Moves NONCAP: " << moves.size() << endl;
  for (Move m : moves) {
    cout << printMove(m) << endl;
  }
}

/**
* Test move generation
// TODO create real tests
*/
TEST(MoveGenTest, pseudoLegalMoves) {
  Position::init();
  Bitboards::init();
  NEWLINE;

  string fen;
  MoveGenerator mg;
  GenMode genMode;
  vector<Move> moves;
  Position position;

  // 86 pseudo legal moves (incl. castling over attacked square)
  fen = "r3k2r/1ppn3p/2q1q1n1/4P3/2q1Pp2/B5R1/pbp2PPP/1R4K1 b kq e3";
  position = Position(fen);
  cout << position.printBoard() << endl;

  genMode = GENCAP;
  moves = mg.generatePseudoLegalMoves(genMode, &position);
  cout << "Moves CAP: " << moves.size() << endl;
  for (Move m : moves) {
    cout << printMove(m) << endl;
  }
  ASSERT_EQ(18, moves.size());
  NEWLINE;

  genMode = GENNONCAP;
  moves = mg.generatePseudoLegalMoves(genMode, &position);
  cout << "Moves NONCAP: " << moves.size() << endl;
  for (Move m : moves) {
    cout << printMove(m) << endl;
  }
  ASSERT_EQ(68, moves.size());
  NEWLINE;

  genMode = GENALL;
  moves = mg.generatePseudoLegalMoves(genMode, &position);
  cout << "Moves ALL: " << moves.size() << endl;
  for (Move m : moves) {
    cout << printMove(m) << endl;
  }
  ASSERT_EQ(86, moves.size());

  // bug fixed positions
  fen = "rnbqkbnr/1ppppppp/8/p7/7P/8/PPPPPPP1/RNBQKBNR w KQkq a6";
  position = Position(fen);
  moves.clear();
  cout << position.str() << endl;

  genMode = GENALL;
  moves = mg.generatePseudoLegalMoves(genMode, &position);
  cout << "Moves CAP: " << moves.size() << endl;
  for (Move m : moves) {
    cout << printMove(m) << endl;
  }
  ASSERT_EQ(21, moves.size());
  NEWLINE;

  fen = "rnbqkbnr/p2ppppp/8/1Pp5/8/8/1PPPPPPP/RNBQKBNR w KQkq c6";
  position = Position(fen);
  moves.clear();
  cout << position.str() << endl;

  genMode = GENALL;
  moves = mg.generatePseudoLegalMoves(genMode, &position);
  cout << "Moves CAP: " << moves.size() << endl;
  for (Move m : moves) {
    cout << printMove(m) << endl;
  }
  ASSERT_EQ(26, moves.size());
  NEWLINE;
}

TEST(MoveGenTest, legalMoves) {
  Position::init();
  Bitboards::init();
  NEWLINE;

  string fen;
  MoveGenerator mg;
  GenMode genMode;
  vector<Move> moves;
  Position position;

  // 86 pseudo legal moves (incl. castling over attacked square)
  fen = "r3k2r/1ppn3p/2q1q1n1/4P3/2q1Pp2/B5R1/pbp2PPP/1R4K1 b kq e3";
  position = Position(fen);
  cout << position.printBoard() << endl;

  genMode = GENALL;
  moves = mg.generatePseudoLegalMoves(genMode, &position);
  cout << "Pseudo Legal Moves: " << moves.size() << endl;
  for (Move m : moves) {
    cout << printMove(m) << endl;
  }
  ASSERT_EQ(86, moves.size());
  NEWLINE;

  moves.clear();
  genMode = GENALL;
  moves = mg.generateLegalMoves(genMode, &position);
  cout << "Legal Moves: " << moves.size() << endl;
  for (Move m : moves) {
    cout << printMove(m) << endl;
  }
  ASSERT_FALSE(position.isLegalMove(createMove<CASTLING>(SQ_E8, SQ_G8)));
  ASSERT_EQ(83, moves.size());
  NEWLINE;
}

