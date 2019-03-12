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

#include "../../src/globals.h"
#include "../../src/Bitboards.h"

using namespace std;
using testing::Eq;

TEST(GlobalsTest, labels) {
  // all squares and label of squares
  string actual;
  for (int i = 0; i < SQ_NONE; ++i) {
    ASSERT_TRUE(isSquare(Square(i)));
    actual += squareLabel(Square(i));
  }
  string expected = "a1b1c1d1e1f1g1h1a2b2c2d2e2f2g2h2a3b3c3d3e3f3g3h3a4b4c4"
                         "d4e4f4g4h4a5b5c5d5e5f5g5h5a6b6c6d6e6f6g6h6a7b7c7d7e7f7"
                         "g7h7a8b8c8d8e8f8g8h8";
  ASSERT_EQ(expected, actual);
}

TEST(GlobalsTest, filesAndRanks) {
  // all squares and label of squares
  string actual;
  for (int i = 0; i < SQ_NONE; ++i) {
    ASSERT_EQ(Square(i), getSquare(File(fileOf(Square(i))), Rank(rankOf(Square(i)))));
  }
}

TEST(GlobalsTest, pieces) {

  // make piece
  ASSERT_EQ(WHITE_KING, makePiece(WHITE, KING));
  ASSERT_EQ(BLACK_KING, makePiece(BLACK, KING));
  ASSERT_EQ(WHITE_QUEEN, makePiece(WHITE, QUEEN));
  ASSERT_EQ(BLACK_QUEEN, makePiece(BLACK, QUEEN));

  // colorOf
  ASSERT_EQ(WHITE, colorOf(WHITE_KING));
  ASSERT_EQ(WHITE, colorOf(WHITE_QUEEN));
  ASSERT_EQ(WHITE, colorOf(WHITE_PAWN));
  ASSERT_EQ(WHITE, colorOf(WHITE_ROOK));
  ASSERT_EQ(BLACK, colorOf(BLACK_KING));
  ASSERT_EQ(BLACK, colorOf(BLACK_QUEEN));
  ASSERT_EQ(BLACK, colorOf(BLACK_PAWN));
  ASSERT_EQ(BLACK, colorOf(BLACK_ROOK));

  // typeOf
  ASSERT_EQ(KING, typeOf(WHITE_KING));
  ASSERT_EQ(QUEEN, typeOf(WHITE_QUEEN));
  ASSERT_EQ(PAWN, typeOf(WHITE_PAWN));
  ASSERT_EQ(ROOK, typeOf(WHITE_ROOK));
  ASSERT_EQ(KING, typeOf(BLACK_KING));
  ASSERT_EQ(QUEEN, typeOf(BLACK_QUEEN));
  ASSERT_EQ(PAWN, typeOf(BLACK_PAWN));
  ASSERT_EQ(ROOK, typeOf(BLACK_ROOK));
}

TEST(GlobalsTest, operators) {
  ASSERT_EQ(WHITE, ~BLACK);
  ASSERT_EQ(BLACK, ~WHITE);

  Color color = WHITE;
  ASSERT_EQ(BLACK, ++color);

  ASSERT_EQ(SQ_A2, SQ_A1 + NORTH);
  ASSERT_TRUE(SQ_H8 + NORTH > 63);
  ASSERT_TRUE(SQ_H1 + SOUTH < 0);
  ASSERT_EQ(SQ_H8, SQ_A1 + (7 * NORTH_EAST));
  ASSERT_EQ(SQ_A8, SQ_H1 + (7 * NORTH_WEST));
}

TEST(MoveTest, moves) {
  Move move = createMove<NORMAL>(SQ_A1, SQ_H1);
  ASSERT_TRUE(isMove(move));
  ASSERT_EQ(SQ_A1, fromSquare(move));
  ASSERT_EQ(SQ_H1, toSquare(move));
  ASSERT_EQ(NORMAL, typeOf(move));
  ASSERT_EQ(KNIGHT, promotionType(move)); // not useful is not type PROMOTION

  move = createMove<PROMOTION>(SQ_A7, SQ_A8, QUEEN);
  ASSERT_TRUE(isMove(move));
  ASSERT_EQ(SQ_A7, fromSquare(move));
  ASSERT_EQ(SQ_A8, toSquare(move));
  ASSERT_EQ(PROMOTION, typeOf(move));
  ASSERT_EQ(QUEEN, promotionType(move)); // not useful is not type PROMOTION

  stringstream buffer1, buffer2;
  buffer1 << "a7a8";
  buffer2 << move;
  ASSERT_EQ(buffer1.str(), buffer2.str());
  ASSERT_EQ("a7a8 (PROMOTION)", printMove(move));
}

TEST(CastlingTest, castling) {
  ASSERT_EQ(0b1000, BLACK | QUEEN_SIDE);
  ASSERT_EQ(BLACK_OOO, BLACK | QUEEN_SIDE);

  CastlingRights cr = ANY_CASTLING;
  ASSERT_EQ(0b1110, cr - WHITE_OO);
  ASSERT_EQ(0b1101, cr - WHITE_OOO);
  ASSERT_EQ(0b1011, cr - BLACK_OO);
  ASSERT_EQ(0b0111, cr - BLACK_OOO);

  cr = NO_CASTLING;
  ASSERT_TRUE(cr == NO_CASTLING);

  cr += WHITE_OO;
  ASSERT_EQ(0b0001, cr);
  ASSERT_TRUE(cr == WHITE_OO);
  ASSERT_TRUE(cr != WHITE_OOO);
  ASSERT_TRUE(cr != NO_CASTLING);
  ASSERT_TRUE(cr != BLACK_OO);
  ASSERT_TRUE(cr != BLACK_OOO);
  ASSERT_TRUE(cr != BLACK_CASTLING);

  cr += WHITE_OOO;
  ASSERT_EQ(0b0011, cr);
  ASSERT_TRUE(cr == WHITE_OO);
  ASSERT_TRUE(cr == WHITE_OOO);
  ASSERT_TRUE(cr == WHITE_CASTLING);
  ASSERT_TRUE(cr != NO_CASTLING);
  ASSERT_TRUE(cr != BLACK_OO);
  ASSERT_TRUE(cr != BLACK_OOO);
  ASSERT_TRUE(cr != BLACK_CASTLING);

  cr += BLACK_OO;
  ASSERT_EQ(0b0111, cr);
  ASSERT_EQ(0b1111, cr + BLACK_OOO);

  cr = ANY_CASTLING;
  cr -= WHITE | QUEEN_SIDE;
  ASSERT_EQ(0b1101, cr);
  cr += WHITE | QUEEN_SIDE;
  ASSERT_EQ(0b1111, cr);

  cr = ANY_CASTLING;
  ASSERT_TRUE(cr == WHITE_OOO);
  ASSERT_FALSE(cr != WHITE_OOO);
  cr -= BLACK | KING_SIDE;
  ASSERT_TRUE(cr != BLACK_OO);
  ASSERT_FALSE(cr == BLACK_OO);
}

TEST(CastlingTest, Iteration) {
  cout << endl;
  for (CastlingRights cr = NO_CASTLING; cr <= ANY_CASTLING; ++cr) {
    cout << "Castling: " << cr << " " << Bitboards::printFlat(cr) << endl;
  }
}