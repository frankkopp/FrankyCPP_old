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

#include "../../src/Position.h"
#include "../../src/MoveGenerator.h"

using namespace std;

using testing::Eq;

/**
 * Test pawn move generation
 // TODO create real tests
 */
TEST(MoveGenTest, pawnMoves) {
  INIT::init();
  NEWLINE;

  string fen;
  MoveGenerator mg;

  fen = "r3k2r/1ppn3p/2q1q1n1/4P3/2q1Pp2/B5R1/pbp2PPP/1R4K1 b kq e3";

  Position position(fen);
  MoveList moves;

  cout << position.printBoard() << endl;

  GenMode genMode = GENCAP;
  mg.generatePawnMoves(genMode, &position, &moves);
  cout << "Moves CAP: " << moves.size() << endl;
  sort(moves.begin(), moves.end());
  for (Move m : moves) {
    cout << printMove(m) << " (" << int(m) << ")" << endl;
  }

  NEWLINE;

  moves.clear();
  genMode = GENNONCAP;
  mg.generatePawnMoves(genMode, &position, &moves);
  cout << "Moves NONCAP: " << moves.size() << endl;
  sort(moves.begin(), moves.end());
  for (Move m : moves) {
    cout << printMove(m) << " (" << int(m) << ")" << endl;
  }

}

TEST(MoveGenTest, kingMoves) {
  INIT::init();
  NEWLINE;

  string fen;
  MoveGenerator mg;
  GenMode genMode;
  MoveList moves;

  fen = "r3k2r/1ppn3p/2q1q1n1/4P3/2q1Pp2/B5R1/pbp2PPP/1R4K1 b kq e3";
  Position position(fen);
  cout << position.printBoard() << endl;

  moves.clear();
  genMode = GENCAP;
  mg.generateKingMoves(genMode, &position, &moves);
  cout << "Moves CAP: " << moves.size() << endl;
  sort(moves.begin(), moves.end());
  for (Move m : moves) {
    cout << printMove(m) << " (" << int(m) << ")" << endl;
  }

  NEWLINE;

  moves.clear();
  genMode = GENNONCAP;
  mg.generateKingMoves(genMode, &position, &moves);
  cout << "Moves NONCAP: " << moves.size() << endl;
  sort(moves.begin(), moves.end());
  for (Move m : moves) {
    cout << printMove(m) << " (" << int(m) << ")" << endl;
  }
}

/**
 * Test move generation
 // TODO create real tests
 */
TEST(MoveGenTest, normalMoves) {
  INIT::init();
  NEWLINE;

  string fen;
  MoveGenerator mg;
  GenMode genMode;
  MoveList moves;

  fen = "r3k2r/1ppn3p/2q1q1n1/8/2q1Pp2/6R1/p1p2PPP/1R4K1 b kq e3";
  Position position(fen);
  cout << position.printBoard() << endl;

  moves.clear();
  genMode = GENCAP;
  mg.generateMoves(genMode, &position, &moves);
  cout << "Moves CAP: " << moves.size() << endl;
  sort(moves.begin(), moves.end());
  for (Move m : moves) {
    cout << printMove(m) << " (" << int(m) << ")" << endl;
  }

  NEWLINE;

  moves.clear();
  genMode = GENNONCAP;
  mg.generateMoves(genMode, &position, &moves);
  cout << "Moves NONCAP: " << moves.size() << endl;
  sort(moves.begin(), moves.end());
  for (Move m : moves) {
    cout << printMove(m) << " (" << int(m) << ")" << endl;
  }
}

TEST(MoveGenTest, castlingMoves) {
  INIT::init();
  NEWLINE;

  string fen;
  MoveGenerator mg;
  GenMode genMode;
  MoveList moves;

  fen = "r3k2r/1ppn3p/2q1q1n1/8/2q1Pp2/6R1/p1p2PPP/1R4K1 b kq e3";
  Position position(fen);
  cout << position.printBoard() << endl;

  moves.clear();
  genMode = GENCAP;
  mg.generateCastling(genMode, &position, &moves);
  cout << "Moves CAP: " << moves.size() << endl;
  for (Move m : moves) {
    cout << printMove(m) << endl;
  }

  NEWLINE;
  moves.clear();
  genMode = GENNONCAP;
  mg.generateCastling(genMode, &position, &moves);
  cout << "Moves NONCAP: " << moves.size() << endl;
  for (Move m : moves) {
    cout << printMove(m) << endl;
  }
}

TEST(MoveGenTest, pseudoLegalMoves) {
  INIT::init();
  NEWLINE;

  string fen;
  MoveGenerator mg;
  GenMode genMode;
  MoveList moves;
  Position position;

  // Start pos
  fen = START_POSITION_FEN;
  position = Position(fen);
  cout << position.str() << endl;

  moves.clear();
  genMode = GENALL;
  moves = mg.generatePseudoLegalMoves(genMode, &position);
  cout << "Moves ALL: " << moves.size() << endl;
  for (Move m : moves) {
    cout << printMove(m) << " (" << int(m) << ")" << endl;
  }
  ASSERT_EQ(20, moves.size());
  NEWLINE;

  // 86 pseudo legal moves (incl. castling over attacked square)
  fen = "r3k2r/1ppn3p/2q1q1n1/4P3/2q1Pp2/B5R1/pbp2PPP/1R4K1 b kq e3";
  position = Position(fen);
  cout << position.str() << endl;

  moves.clear();
  genMode = GENALL;
  moves = mg.generatePseudoLegalMoves(genMode, &position);
  cout << "Moves ALL: " << moves.size() << endl;
  for (Move m : moves) {
    cout << printMove(m) << " (" << int(m) << ")" << endl;
  }
  ASSERT_EQ(86, moves.size());
  NEWLINE;

  // bug fixed positions
  fen = "rnbqkbnr/1ppppppp/8/p7/7P/8/PPPPPPP1/RNBQKBNR w KQkq a6";
  position = Position(fen);
  moves.clear();
  cout << position.str() << endl;

  genMode = GENALL;
  moves = mg.generatePseudoLegalMoves(genMode, &position);
  cout << "Moves CAP: " << moves.size() << endl;
  for (Move m : moves) {
    cout << printMove(m) << " (" << int(m) << ")" << endl;
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
    cout << printMove(m) << " (" << int(m) << ")" << endl;
  }
  ASSERT_EQ(26, moves.size());
  NEWLINE;
}

TEST(MoveGenTest, legalMoves) {
  INIT::init();
  NEWLINE;

  string fen;
  MoveGenerator mg;
  GenMode genMode;
  MoveList moves;
  Position position;

  // 86 pseudo legal moves (incl. castling over attacked square)
  position = Position(START_POSITION_FEN);
  cout << position.printBoard() << endl;

  genMode = GENALL;
  moves = mg.generateLegalMoves(genMode, &position);
  cout << "Legal Moves: " << moves.size() << endl;
  for (Move m : moves) {
    cout << printMove(m) << endl;
  }
  ASSERT_EQ(20, moves.size());
  NEWLINE;

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
  cout << moves << endl;
  ASSERT_FALSE(position.isLegalMove(createMove<CASTLING>(SQ_E8, SQ_G8)));
  ASSERT_EQ(83, moves.size());
  NEWLINE;
}

TEST(MoveGenTest, onDemandGen) {
  INIT::init();
  NEWLINE;

  string fen;
  MoveGenerator mg;
  GenMode genMode;
  MoveList moves;

  // 86 pseudo legal moves (incl. castling over attacked square)
  fen = "r3k2r/1ppn3p/2q1q1n1/4P3/2q1Pp2/B5R1/pbp2PPP/1R4K1 b kq e3";
  Position position(fen);
  cout << position.str() << endl;

  genMode = GENALL;
  Move move;
  int counter = 0;
  while (true) {
    move = mg.getNextPseudoLegalMove(genMode, &position);
    if (move == NOMOVE) break;
    cout << printMove(move) << " (" << int(move) << ")" << endl;
    counter++;
  }
  println("Moves: " + to_string(counter));
  ASSERT_EQ(86, counter);
  NEWLINE;

  // 218 moves
  fen = "R6R/3Q4/1Q4Q1/4Q3/2Q4Q/Q4Q2/pp1Q4/kBNN1KB1 w - - 0 1";
  position = Position(fen);
  cout << position.str() << endl;

  genMode = GENALL;
  counter = 0;
  while (true) {
    move = mg.getNextPseudoLegalMove(genMode, &position);
    if (move == NOMOVE) break;
    cout << printMove(move) << " (" << int(move) << ")" << endl;
    counter++;
  }
  println("Moves: " + to_string(counter));
  ASSERT_EQ(218, counter);
  NEWLINE;

}

TEST(MoveGenTest, hasLegalMoves) {
  Position::init();
  Bitboards::init();
  NEWLINE;

  string fen;
  MoveGenerator mg;
  MoveList moves;

  fen = "rn2kbnr/pbpp1ppp/8/1p2p1q1/4K3/3P4/PPP1PPPP/RNBQ1BNR w kq -";
  Position position(fen);
  println(position.str());

  moves = mg.generateLegalMoves(GENALL, &position);
  const bool expected = mg.hasLegalMove(&position);

  ASSERT_EQ(0, moves.size());
  ASSERT_FALSE(expected);

}