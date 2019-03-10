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

#include "../../src/globals.h"
#include "../../src/Position.h"
#include "../../src/Bitboards.h"

using namespace std;

using testing::Eq;

TEST(PositionTest, ZobristTest) {
  Position::init();
  NEWLINE;
  Key z = 0ULL;

  z ^= Zobrist::pieces[WHITE_KING][SQ_E1];
  z ^= Zobrist::pieces[BLACK_KING][SQ_E8];
  z ^= Zobrist::castlingRights[ANY_CASTLING];
  z ^= Zobrist::enPassantFile[FILE_NONE];
  Key expected = z;
  std::cout << "Zobrist= " << z << std::endl;
  ASSERT_EQ(3127863183353006913, z);

  z ^= Zobrist::pieces[WHITE_KING][SQ_E1];
  z ^= Zobrist::pieces[WHITE_KING][SQ_E2];
  std::cout << "Zobrist= " << z << std::endl;

  z ^= Zobrist::pieces[WHITE_KING][SQ_E2];
  z ^= Zobrist::pieces[WHITE_KING][SQ_E1];
  std::cout << "Zobrist= " << z << std::endl;
  ASSERT_EQ(expected, z);

  z ^= Zobrist::castlingRights[WHITE_CASTLING];
  std::cout << "Zobrist= " << z << std::endl;

  z ^= Zobrist::castlingRights[WHITE_CASTLING];
  std::cout << "Zobrist= " << z << std::endl;
  ASSERT_EQ(expected, z);

  z ^= Zobrist::castlingRights[WHITE_OO];
  std::cout << "Zobrist= " << z << std::endl;

  z ^= Zobrist::castlingRights[WHITE_OO];
  std::cout << "Zobrist= " << z << std::endl;
  ASSERT_EQ(expected, z);

  z ^= Zobrist::enPassantFile[fileOf(SQ_D3)];
  std::cout << "Zobrist= " << z << std::endl;

  z ^= Zobrist::enPassantFile[fileOf(SQ_D3)];
  std::cout << "Zobrist= " << z << std::endl;
  ASSERT_EQ(expected, z);

  z ^= Zobrist::nextPlayer;
  std::cout << "Zobrist= " << z << std::endl;

  z ^= Zobrist::nextPlayer;
  std::cout << "Zobrist= " << z << std::endl;
  ASSERT_EQ(expected, z);
}

TEST(PositionTest, Setup) {
  Bitboards::init();
  Position::init();
  Values::init();
  NEWLINE;
  Position position;

  ASSERT_EQ(WHITE, position.getNextPlayer());
  ASSERT_EQ(BLACK, ~position.getNextPlayer());
  ASSERT_EQ(position.getMaterial(WHITE), position.getMaterial(BLACK));
  ASSERT_EQ(24, position.getGamePhase());
  ASSERT_EQ(position.getMgPosValue(WHITE),position.getMgPosValue(BLACK));
  ASSERT_EQ(-225,position.getMgPosValue(WHITE));
  ASSERT_EQ(-225,position.getMgPosValue(BLACK));
  ASSERT_EQ(WHITE_KING, position.getPiece(SQ_E1));
  ASSERT_EQ(BLACK_KING, position.getPiece(SQ_E8));
  ASSERT_EQ(WHITE_KNIGHT, position.getPiece(SQ_B1));
  ASSERT_EQ(BLACK_KNIGHT, position.getPiece(SQ_B8));

  string fen("r3k2r/1ppn3p/2q1q1n1/8/2q1Pp2/6R1/p1p2PPP/1R4K1 b kq e3 10 113");
  position = Position(fen.c_str());
  ASSERT_EQ(fen, position.printFen());
  ASSERT_EQ(SQ_E3, position.getEnPassantSquare());
  ASSERT_EQ(BLACK, position.getNextPlayer());
  ASSERT_EQ(3400, position.getMaterial(WHITE));
  ASSERT_EQ(6940, position.getMaterial(BLACK));
  ASSERT_EQ(22, position.getGamePhase());
  ASSERT_EQ(90,position.getMgPosValue(WHITE));
  ASSERT_EQ(7,position.getMgPosValue(BLACK));
  ASSERT_EQ(WHITE_KING, position.getPiece(SQ_G1));
  ASSERT_EQ(BLACK_KING, position.getPiece(SQ_E8));
  ASSERT_EQ(WHITE_ROOK, position.getPiece(SQ_G3));
  ASSERT_EQ(BLACK_QUEEN, position.getPiece(SQ_C6));
}

TEST(PositionTest, Output) {
  Bitboards::init();
  Position::init();
  NEWLINE;

  ostringstream expected;
  ostringstream actual;

  // start pos
  Position position;
  ASSERT_EQ(START_POSITION_FEN, position.printFen());
  expected << "  +---+---+---+---+---+---+---+---+\n"
              "8 | r | n | b | q | k | b | n | r |\n"
              "  +---+---+---+---+---+---+---+---+\n"
              "7 | p | p | p | p | p | p | p | p |\n"
              "  +---+---+---+---+---+---+---+---+\n"
              "6 |   |   |   |   |   |   |   |   |\n"
              "  +---+---+---+---+---+---+---+---+\n"
              "5 |   |   |   |   |   |   |   |   |\n"
              "  +---+---+---+---+---+---+---+---+\n"
              "4 |   |   |   |   |   |   |   |   |\n"
              "  +---+---+---+---+---+---+---+---+\n"
              "3 |   |   |   |   |   |   |   |   |\n"
              "  +---+---+---+---+---+---+---+---+\n"
              "2 | P | P | P | P | P | P | P | P |\n"
              "  +---+---+---+---+---+---+---+---+\n"
              "1 | R | N | B | Q | K | B | N | R |\n"
              "  +---+---+---+---+---+---+---+---+\n"
              "    A   B   C   D   E   F   G   H  \n\n";
  actual << position.printBoard();
  ASSERT_EQ(expected.str(), actual.str());

  string fen("r3k2r/1ppn3p/2q1q1n1/8/2q1Pp2/6R1/p1p2PPP/1R4K1 b kq e3 10 113");
  position = Position(fen.c_str());
  ASSERT_EQ(fen, position.printFen());

  fen = string("r1b1k2r/pp2ppbp/2n3p1/q7/3pP3/2P1BN2/P2Q1PPP/2R1KB1R w Kkq - 0 11");
  position = Position(fen.c_str());
  ASSERT_EQ(fen, position.printFen());

  fen = string("rnbqkbnr/1ppppppp/8/p7/Q1P5/8/PP1PPPPP/RNB1KBNR b KQkq - 1 2");
  position = Position(fen.c_str());
  ASSERT_EQ(fen, position.printFen());
}

TEST(PositionTest, Copy) {
  Bitboards::init();
  Position::init();
  NEWLINE;

  string fen("r3k2r/1ppn3p/2q1q1n1/8/2q1Pp2/6R1/p1p2PPP/1R4K1 b kq e3 10 113");
  Position position(fen.c_str());
  Position copy(position);
  ASSERT_EQ(position.getZobristKey(), copy.getZobristKey());
  ASSERT_EQ(position.printFen(), copy.printFen());
  ASSERT_EQ(position.printBoard(), copy.printBoard());
  ASSERT_EQ(position.getOccupiedBB(WHITE), copy.getOccupiedBB(WHITE));
  ASSERT_EQ(position.getOccupiedBB(BLACK), copy.getOccupiedBB(BLACK));
  ASSERT_EQ(SQ_E3, copy.getEnPassantSquare());
  ASSERT_EQ(BLACK, copy.getNextPlayer());
}

TEST(PositionTest, PosValue) {
  Bitboards::init();
  Position::init();
  Values::init();
  NEWLINE;

  Position position("8/8/8/8/8/8/8/8 w - - 0 1");
  //  cout << position.str() << endl;

  position.putPiece(WHITE_KING, SQ_E1);
  position.putPiece(BLACK_KING, SQ_E8);
  position.putPiece(WHITE_KNIGHT, SQ_E4);
  position.putPiece(BLACK_KNIGHT, SQ_D5);
  // cout << position.str() << endl;
  ASSERT_EQ(2, position.getGamePhase());
  ASSERT_EQ(2320, position.getMaterial(WHITE));
  ASSERT_EQ(2320, position.getMaterial(BLACK));
  ASSERT_EQ(0, position.getMgPosValue(WHITE));
  ASSERT_EQ(0, position.getMgPosValue(BLACK));
  ASSERT_EQ(-10, position.getEgPosValue(WHITE));
  ASSERT_EQ(-10, position.getEgPosValue(BLACK));


}
