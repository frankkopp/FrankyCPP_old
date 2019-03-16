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

#include "../../src/Bitboards.h"
#include "../../src/Position.h"

using namespace std;
using namespace Bitboards;
using testing::Eq;

//TEST(BitboardsTest, print) {
//
//  // TODO do some asserts to really test
//
//  Bitboards::init();
//
//  std::cout << "\n";
//
//  std::cout << Bitboards::print(EMPTY_BB) << std::endl;
//  std::cout << Bitboards::printFlat(EMPTY_BB) << std::endl;
//  std::cout << Bitboards::print(ALL_BB) << std::endl;
//  std::cout << Bitboards::printFlat(ALL_BB) << std::endl;
//
//  for (Square i = SQ_A1; i <= SQ_H8; ++i) {
//    std::cout << squareLabel(i) << std::endl;
//    std::cout << Bitboards::print(squareBB[i]) << std::endl;
//  }
//
//  std::cout << Bitboards::print(squareBB[SQ_A1]) << std::endl;
//  std::cout << Bitboards::printFlat(squareBB[SQ_A1]) << std::endl;
//  std::cout << Bitboards::print(squareBB[SQ_H1]) << std::endl;
//  std::cout << Bitboards::printFlat(squareBB[SQ_H1]) << std::endl;
//  std::cout << Bitboards::print(squareBB[SQ_A8]) << std::endl;
//  std::cout << Bitboards::printFlat(squareBB[SQ_A8]) << std::endl;
//  std::cout << Bitboards::print(squareBB[SQ_H8]) << std::endl;
//  std::cout << Bitboards::printFlat(squareBB[SQ_H8]) << std::endl;
//
//  std::cout << Bitboards::print(squareBB[SQ_H8]) << std::endl;
//  std::cout << Bitboards::printFlat(squareBB[SQ_H8]) << std::endl;
//
//  std::cout << Bitboards::print(ALL_BB) << std::endl;
//}

TEST(BitboardsTest, BitboardSquareTest) {
  Bitboards::init();

  ASSERT_EQ(squareBB[SQ_E4], ALL_BB & SQ_E4);
  ASSERT_EQ(squareBB[SQ_A1], ALL_BB & SQ_A1);
  ASSERT_EQ(squareBB[SQ_H8], ALL_BB & SQ_H8);
  ASSERT_EQ(squareBB[SQ_A8], ALL_BB & SQ_A8);
  ASSERT_NE(squareBB[SQ_A8], ALL_BB & SQ_A1);
}

TEST(BitboardsTest, SquareDistanceTest) {
  Bitboards::init();

  ASSERT_EQ(6, distance(FILE_A, FILE_G));
  ASSERT_EQ(7, distance(RANK_1, RANK_8));

  ASSERT_EQ(7, distance(SQ_A1, SQ_H1));
  ASSERT_EQ(7, distance(SQ_A1, SQ_H8));
  ASSERT_EQ(2, distance(SQ_A1, SQ_A3));
  ASSERT_EQ(4, distance(SQ_A1, SQ_E1));
  ASSERT_EQ(7, distance(SQ_A1, SQ_G8));
}


TEST(BitboardsTest, shiftTest) {
  Bitboards::init();

  Bitboard shifted = Bitboards::shift(EAST, FileABB);
  ASSERT_EQ(FileBBB, shifted);

  shifted = Bitboards::shift(WEST, FileABB);
  ASSERT_EQ(EMPTY_BB, shifted);

  shifted = Bitboards::shift(NORTH, Rank1BB);
  ASSERT_EQ(Rank2BB, shifted);

  shifted = Bitboards::shift(SOUTH, Rank8BB);
  ASSERT_EQ(Rank7BB, shifted);

  shifted = Bitboards::shift(NORTH, Rank8BB);
  ASSERT_EQ(EMPTY_BB, shifted);

  shifted = Bitboards::shift(NORTH_EAST, squareBB[SQ_E4]);
  ASSERT_EQ(squareBB[SQ_F5], shifted);

  shifted = Bitboards::shift(SOUTH_EAST, squareBB[SQ_E4]);
  ASSERT_EQ(squareBB[SQ_F3], shifted);

  shifted = Bitboards::shift(SOUTH_WEST, squareBB[SQ_E4]);
  ASSERT_EQ(squareBB[SQ_D3], shifted);

  shifted = Bitboards::shift(NORTH_WEST, squareBB[SQ_E4]);
  ASSERT_EQ(squareBB[SQ_D5], shifted);
}


TEST(BitboardsTest, Diagonals) {
  Bitboards::init();

  ASSERT_EQ(DiagUpA1, squareDiagUpBB[SQ_A1]);
  ASSERT_EQ(DiagUpA1, squareDiagUpBB[SQ_C3]);
  ASSERT_EQ(DiagUpA1, squareDiagUpBB[SQ_G7]);
  ASSERT_EQ(DiagUpA1, squareDiagUpBB[SQ_H8]);

  ASSERT_EQ(DiagDownH1, squareDiagDownBB[SQ_A8]);
  ASSERT_EQ(DiagDownH1, squareDiagDownBB[SQ_C6]);
  ASSERT_EQ(DiagDownH1, squareDiagDownBB[SQ_G2]);
  ASSERT_EQ(DiagDownH1, squareDiagDownBB[SQ_H1]);
}

TEST(BitboardsTest, bitScans) {
  Bitboards::init();

  ASSERT_EQ(1, popcount(squareBB[SQ_D3]));
  ASSERT_EQ(2, popcount(squareBB[SQ_D3] | squareBB[SQ_H2]));
  ASSERT_EQ(8, popcount(DiagUpA1));

  ASSERT_EQ(19, lsb(squareBB[SQ_D3]));
  ASSERT_EQ(19, msb(squareBB[SQ_D3]));

  Bitboard tmp = DiagUpA1;
  int i = 0;
  while (tmp) {
    i++;
    popLSB(&tmp);
  }
  ASSERT_EQ(8, i);
}

TEST(BitboardsTest, R90) {
  // NEWLINE;
  Bitboards::init();

  const Bitboard bb = FileABB | Rank4BB;
  const string &actual = Bitboards::print(Bitboards::rotateR90(bb));
  //  cout << Bitboards::print(bb) << endl;
  //  cout << "R90" << endl;
  //  cout << actual << endl;
  string expected = "+---+---+---+---+---+---+---+---+\n"
                    "| X | X | X | X | X | X | X | X |\n"
                    "+---+---+---+---+---+---+---+---+\n"
                    "|   |   |   | X |   |   |   |   |\n"
                    "+---+---+---+---+---+---+---+---+\n"
                    "|   |   |   | X |   |   |   |   |\n"
                    "+---+---+---+---+---+---+---+---+\n"
                    "|   |   |   | X |   |   |   |   |\n"
                    "+---+---+---+---+---+---+---+---+\n"
                    "|   |   |   | X |   |   |   |   |\n"
                    "+---+---+---+---+---+---+---+---+\n"
                    "|   |   |   | X |   |   |   |   |\n"
                    "+---+---+---+---+---+---+---+---+\n"
                    "|   |   |   | X |   |   |   |   |\n"
                    "+---+---+---+---+---+---+---+---+\n"
                    "|   |   |   | X |   |   |   |   |\n"
                    "+---+---+---+---+---+---+---+---+\n";
  ASSERT_EQ(expected, actual);
}

TEST(BitboardsTest, L90) {
  // NEWLINE;
  Bitboards::init();

  const Bitboard bb = FileABB | Rank4BB;
  const string &actual = Bitboards::print(Bitboards::rotateL90(bb));
  //  cout << Bitboards::print(bb) << endl;
  //  cout << "L90" << endl;
  //  cout << actual << endl;
  string expected = "+---+---+---+---+---+---+---+---+\n"
                    "|   |   |   |   | X |   |   |   |\n"
                    "+---+---+---+---+---+---+---+---+\n"
                    "|   |   |   |   | X |   |   |   |\n"
                    "+---+---+---+---+---+---+---+---+\n"
                    "|   |   |   |   | X |   |   |   |\n"
                    "+---+---+---+---+---+---+---+---+\n"
                    "|   |   |   |   | X |   |   |   |\n"
                    "+---+---+---+---+---+---+---+---+\n"
                    "|   |   |   |   | X |   |   |   |\n"
                    "+---+---+---+---+---+---+---+---+\n"
                    "|   |   |   |   | X |   |   |   |\n"
                    "+---+---+---+---+---+---+---+---+\n"
                    "|   |   |   |   | X |   |   |   |\n"
                    "+---+---+---+---+---+---+---+---+\n"
                    "| X | X | X | X | X | X | X | X |\n"
                    "+---+---+---+---+---+---+---+---+\n";
  ASSERT_EQ(expected, actual);
}

TEST(BitboardsTest, R45) {
  // NEWLINE;
  Bitboards::init();

  const Bitboard bb = DiagUpA1;
  const string &actual = Bitboards::print(Bitboards::rotateR45(bb));
  //  cout << Bitboards::print(bb) << endl;
  //  cout << "R45" << endl;
  //  cout << actual << endl;
  string expected = "+---+---+---+---+---+---+---+---+\n"
                    "|   |   |   |   |   |   |   |   |\n"
                    "+---+---+---+---+---+---+---+---+\n"
                    "|   |   |   |   |   |   |   |   |\n"
                    "+---+---+---+---+---+---+---+---+\n"
                    "|   |   |   |   |   |   |   |   |\n"
                    "+---+---+---+---+---+---+---+---+\n"
                    "| X | X | X | X |   |   |   |   |\n"
                    "+---+---+---+---+---+---+---+---+\n"
                    "|   |   |   |   | X | X | X | X |\n"
                    "+---+---+---+---+---+---+---+---+\n"
                    "|   |   |   |   |   |   |   |   |\n"
                    "+---+---+---+---+---+---+---+---+\n"
                    "|   |   |   |   |   |   |   |   |\n"
                    "+---+---+---+---+---+---+---+---+\n"
                    "|   |   |   |   |   |   |   |   |\n"
                    "+---+---+---+---+---+---+---+---+\n";
  ASSERT_EQ(expected, actual);
}

TEST(BitboardsTest, L45) {
  // NEWLINE;
  Bitboards::init();

  const Bitboard bb = DiagDownH1;
  const string &actual = Bitboards::print(Bitboards::rotateL45(bb));
  //  cout << Bitboards::print(bb) << endl;
  //  cout << "R45" << endl;
  //  cout << actual << endl;
  string expected = "+---+---+---+---+---+---+---+---+\n"
                    "|   |   |   |   |   |   |   |   |\n"
                    "+---+---+---+---+---+---+---+---+\n"
                    "|   |   |   |   |   |   |   |   |\n"
                    "+---+---+---+---+---+---+---+---+\n"
                    "|   |   |   |   |   |   |   |   |\n"
                    "+---+---+---+---+---+---+---+---+\n"
                    "| X | X | X | X |   |   |   |   |\n"
                    "+---+---+---+---+---+---+---+---+\n"
                    "|   |   |   |   | X | X | X | X |\n"
                    "+---+---+---+---+---+---+---+---+\n"
                    "|   |   |   |   |   |   |   |   |\n"
                    "+---+---+---+---+---+---+---+---+\n"
                    "|   |   |   |   |   |   |   |   |\n"
                    "+---+---+---+---+---+---+---+---+\n"
                    "|   |   |   |   |   |   |   |   |\n"
                    "+---+---+---+---+---+---+---+---+\n";
  ASSERT_EQ(expected, actual);
}

//TEST(BitboardsTest, movesRank) {
//  // NEWLINE;
//  Bitboards::init();
//  for (Bitboard j = 0b0000'0000; j < 0b1111'1111; j++) {
//    for (Square sq = SQ_A1; sq <= SQ_H8; ++sq) {
//      File file = fileOf(sq);
//      Rank rank = rankOf(sq);
//
//      const Bitboard rankMask = rankBB(rank);
//      const Bitboard blocker = j << 8 * rank;
//      const Bitboard shifted = blocker >> 8 * rank;
//      const Bitboard contentIdx = shifted & 255;
//      const Bitboard moves = movesRank[sq][j];
//
//      cout << squareLabel(sq) << std::left << setw(15) << " Rank Mask:"
//           << Bitboards::printFlat(rankMask) << endl;
//      cout << squareLabel(sq) << std::left << setw(15) << " Blocker:"
//           << Bitboards::printFlat(blocker) << endl;
//      cout << squareLabel(sq) << std::left << setw(15) << " ContentIdx:"
//           << Bitboards::printFlat(contentIdx) << endl;
//      cout << squareLabel(sq) << std::left << setw(15) << " Square Mask:"
//           << Bitboards::printFlat(squareBB[sq]) << endl;
//      cout << squareLabel(sq) << std::left << setw(15) << " Moves:"
//           << Bitboards::printFlat(moves) << endl;
//
//      // NEWLINE;
//    }
//  }
//}

//TEST(BitboardsTest, movesFile) {
//  // NEWLINE;
//  Bitboards::init();
//  for (Bitboard j = 0b0000'0000; j < 0b1111'1111; j++) {
//    for (Square sq = SQ_A1; sq <= SQ_H8; ++sq) {
//      File file = fileOf(sq);
//      Rank rank = rankOf(sq);
//
//      cout << squareLabel(sq) << endl;
//      cout << Bitboards::print(movesFile[sq][j]) << endl << endl;
//
//      // NEWLINE;
//    }
//  }
//}

TEST(BitboardsTest, movesRankTest) {
  // NEWLINE;
  Bitboards::init();
  string expected, actual;

  Position position("r1b1k2r/pp2ppbp/2n3p1/q7/3pP3/2P1BN2/P2Q1PPP/2R1KB1R w Kkq -");

  //  cout << position.printBoard();
  //  cout << Bitboards::print(position.getOccupiedBB());
  //  cout << Bitboards::print(Bitboards::getMovesRank(SQ_H8, position.getOccupiedBB()));

  expected = "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "| X | X |   | X | X |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n";
  actual = Bitboards::print(Bitboards::getMovesRank(SQ_C1, position.getOccupiedBB()));
  ASSERT_EQ(expected, actual);

  expected = "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   | X | X | X |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n";
  actual = Bitboards::print(Bitboards::getMovesRank(SQ_H8, position.getOccupiedBB()));
  ASSERT_EQ(expected, actual);
}

TEST(BitboardsTest, movesFileTest) {
  // NEWLINE;
  Bitboards::init();
  string expected, actual;

  Position position("r1b1k2r/pp2ppbp/2n3p1/q7/3pP3/2P1BN2/P2Q1PPP/2R1KB1R w Kkq -");

  //  cout << position.printBoard();
  //  cout << Bitboards::print(position.getOccupiedBB());
  //  cout << Bitboards::print(Bitboards::getMovesFile(SQ_A5, position.getOccupiedBB()));

  expected = "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "| X |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "| X |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "| X |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "| X |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "| X |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n";
  actual = Bitboards::print(Bitboards::getMovesFile(SQ_A5, position.getOccupiedBB()));
  ASSERT_EQ(expected, actual);

  //  cout << Bitboards::print(position.getOccupiedBB());
  //  cout << Bitboards::print(Bitboards::getMovesFile(SQ_D2, position.getOccupiedBB()));
  expected = "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   | X |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   | X |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   | X |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n";
  actual = Bitboards::print(Bitboards::getMovesFile(SQ_D2, position.getOccupiedBB()));
  ASSERT_EQ(expected, actual);

  //  cout << Bitboards::print(position.getOccupiedBB());
  //  cout << Bitboards::print(Bitboards::getMovesFile(SQ_C1, position.getOccupiedBB()));
  expected = "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   | X |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   | X |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n";
  actual = Bitboards::print(Bitboards::getMovesFile(SQ_C1, position.getOccupiedBB()));
  ASSERT_EQ(expected, actual);
}

TEST(BitboardsTest, movesDiagUpTest) {
  // NEWLINE;
  Bitboards::init();
  string expected, actual;

  /// is pre-computed movesDiagUp correct?
  //  cout << "Moves: " << squareLabel(SQ_A5) << endl
  //       << Bitboards::print(movesDiagUp[SQ_A5][0b0000'0001]) << endl;
  expected = "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   | X |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   | X |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   | X |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n";
  actual = Bitboards::print(movesDiagUp[SQ_A5][0b0000'0001]);
  ASSERT_EQ(expected, actual);

  ASSERT_EQ(8, lengthDiagUp[SQ_A1]);
  ASSERT_EQ(1, lengthDiagUp[SQ_A8]);
  ASSERT_EQ(4, lengthDiagUp[SQ_A5]);
  ASSERT_EQ(4, lengthDiagUp[SQ_E1]);
  //  cout << squareLabel(SQ_A1) << " Length = " << lengthDiagUp[SQ_A1] << endl;
  //  cout << squareLabel(SQ_A8) << " Length = " << lengthDiagUp[SQ_A8] << endl;
  //  cout << squareLabel(SQ_A5) << " Length = " << lengthDiagUp[SQ_A5] << endl;
  //  cout << squareLabel(SQ_E1) << " Length = " << lengthDiagUp[SQ_E1] << endl;

  Position position("r1b1k2r/pp2ppbp/2n3p1/q7/3pP3/2P1BN2/P2Q1PPP/2R1KB1R w Kkq -");

  //  cout << position.printBoard();
  //  cout << Bitboards::print(position.getOccupiedBB());
  //  cout << Bitboards::print(Bitboards::getMovesDiagUp(SQ_A5, position.getOccupiedBB()));

  expected = "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   | X |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   | X |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   | X |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n";
  actual = Bitboards::print(Bitboards::getMovesDiagUp(SQ_A5, position.getOccupiedBB()));
  ASSERT_EQ(expected, actual);

  //  cout << Bitboards::print(position.getOccupiedBB());
  //  cout << Bitboards::print(Bitboards::getMovesDiagUp(SQ_E3, position.getOccupiedBB()));
  expected = "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   | X |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   | X |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   | X |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   | X |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n";
  actual = Bitboards::print(Bitboards::getMovesDiagUp(SQ_E3, position.getOccupiedBB()));
  ASSERT_EQ(expected, actual);

  //  cout << Bitboards::print(position.getOccupiedBB());
  //  cout << Bitboards::print(Bitboards::getMovesDiagUp(SQ_G7, position.getOccupiedBB()));
  expected = "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   | X |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   | X |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   | X |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   | X |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n";
  actual = Bitboards::print(Bitboards::getMovesDiagUp(SQ_G7, position.getOccupiedBB()));
  ASSERT_EQ(expected, actual);

  //  cout << Bitboards::print(position.getOccupiedBB());
  //  cout << Bitboards::print(Bitboards::getMovesDiagUp(SQ_D6, position.getOccupiedBB()));
  expected = "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   | X |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   | X |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   | X |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "| X |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n";
  actual = Bitboards::print(Bitboards::getMovesDiagUp(SQ_D6, position.getOccupiedBB()));
  ASSERT_EQ(expected, actual);
}

TEST(BitboardsTest, movesDiagDownTest) {
  // NEWLINE;
  Bitboards::init();
  string expected, actual;

  /// is pre-computed movesDiagDown correct?
  //  cout << "Moves: " << squareLabel(SQ_A5) << endl
  //       << Bitboards::print(movesDiagDown[SQ_A5][0b0000'0001]) << endl;
  expected = "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   | X |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   | X |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   | X |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   | X |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n";
  actual = Bitboards::print(movesDiagDown[SQ_A5][0b0000'0001]);
  ASSERT_EQ(expected, actual);

  /// is length for diags correct?
  ASSERT_EQ(1, lengthDiagDown[SQ_A1]);
  ASSERT_EQ(8, lengthDiagDown[SQ_A8]);
  ASSERT_EQ(5, lengthDiagDown[SQ_A5]);
  ASSERT_EQ(5, lengthDiagDown[SQ_E1]);
  //  cout << squareLabel(SQ_A1) << " Length = " << lengthDiagDown[SQ_A1] << endl;
  //  cout << squareLabel(SQ_A8) << " Length = " << lengthDiagDown[SQ_A8] << endl;
  //  cout << squareLabel(SQ_A5) << " Length = " << lengthDiagDown[SQ_A5] << endl;
  //  cout << squareLabel(SQ_E1) << " Length = " << lengthDiagDown[SQ_E1] << endl;

  Position position("r1b1k2r/pp2ppbp/2n3p1/q7/3pP3/2P1BN2/P2Q1PPP/2R1KB1R w Kkq -");

  //  cout << position.printBoard();
  //  cout << Bitboards::print(position.getOccupiedBB());
  //  cout << Bitboards::print(Bitboards::getMovesDiagDown(SQ_A5, position.getOccupiedBB()));
  expected = "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   | X |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   | X |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n";
  actual = Bitboards::print(Bitboards::getMovesDiagDown(SQ_A5, position.getOccupiedBB()));
  ASSERT_EQ(expected, actual);

  //  cout << Bitboards::print(position.getOccupiedBB());
  //  cout << Bitboards::print(Bitboards::getMovesDiagDown(SQ_E3, position.getOccupiedBB()));
  expected = "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   | X |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   | X |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n";
  actual = Bitboards::print(Bitboards::getMovesDiagDown(SQ_E3, position.getOccupiedBB()));
  ASSERT_EQ(expected, actual);

  //  cout << Bitboards::print(position.getOccupiedBB());
  //  cout << Bitboards::print(Bitboards::getMovesDiagDown(SQ_G7, position.getOccupiedBB()));
  expected = "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   | X |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   | X |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n";
  actual = Bitboards::print(Bitboards::getMovesDiagDown(SQ_G7, position.getOccupiedBB()));
  ASSERT_EQ(expected, actual);

  //  cout << Bitboards::print(position.getOccupiedBB());
  //  cout << Bitboards::print(Bitboards::getMovesDiagDown(SQ_D6, position.getOccupiedBB()));
  expected = "+---+---+---+---+---+---+---+---+\n"
             "|   | X |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   | X |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   | X |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   | X |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   | X |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   | X |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n";
  actual = Bitboards::print(Bitboards::getMovesDiagDown(SQ_D6, position.getOccupiedBB()));
  ASSERT_EQ(expected, actual);
}


TEST(BitboardsTest, indexRotation) {
  // NEWLINE;
  Bitboards::init();
  string expected, actual;

  ASSERT_EQ(SQ_A8, rotateSquareR90(SQ_A1));
  ASSERT_EQ(SQ_B7, rotateSquareR90(SQ_B2));
  ASSERT_EQ(SQ_E4, rotateSquareR90(SQ_E5));
  ASSERT_EQ(SQ_H1, rotateSquareR90(SQ_H8));

  ASSERT_EQ(SQ_H1, rotateSquareL90(SQ_A1));
  ASSERT_EQ(SQ_G2, rotateSquareL90(SQ_B2));
  ASSERT_EQ(SQ_D5, rotateSquareL90(SQ_E5));
  ASSERT_EQ(SQ_A8, rotateSquareL90(SQ_H8));

  ASSERT_EQ(28, rotateSquareR45(SQ_A1));
  ASSERT_EQ(29, rotateSquareR45(SQ_B2));
  ASSERT_EQ(32, rotateSquareR45(SQ_E5));
  ASSERT_EQ(35, rotateSquareR45(SQ_H8));

  ASSERT_EQ(0, rotateSquareL45(SQ_A1));
  ASSERT_EQ(4, rotateSquareL45(SQ_B2));
  ASSERT_EQ(39, rotateSquareL45(SQ_E5));
  ASSERT_EQ(63, rotateSquareL45(SQ_H8));
}

TEST(BitboardsTest, pawnAttacksMoves) {
  NEWLINE;
  init();
  string expected, actual;

  expected = "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   | X |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n";
  actual = print(pawnAttacks[WHITE][SQ_A2]);
//  cout << actual;
  ASSERT_EQ(expected, actual);

  expected = "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   | X |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n";
  actual = print(pawnAttacks[BLACK][SQ_H7]);
//  cout << actual;
  ASSERT_EQ(expected, actual);

  expected = "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   | X |   | X |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n";
  actual = print(pawnAttacks[BLACK][SQ_D5]);
//  cout << actual;
  ASSERT_EQ(expected, actual);

  expected = "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   | X |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   | X |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n";
  actual = print(pawnMoves[WHITE][SQ_E2]);
//  cout << actual;
  ASSERT_EQ(expected, actual);

  expected = "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   | X |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   | X |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n";
  actual = print(pawnMoves[BLACK][SQ_E7]);
//  cout << actual;
  ASSERT_EQ(expected, actual);

  expected = "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   | X |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n";
  actual = print(pawnMoves[WHITE][SQ_E4]);
//  cout << actual;
  ASSERT_EQ(expected, actual);
}

TEST(BitboardsTest, knightAttacks) {
  NEWLINE;
  init();
  string expected, actual;

  expected = "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   | X |   | X |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   | X |   |   |   | X |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   | X |   |   |   | X |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   | X |   | X |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n";
  actual = print(pseudoAttacks[KNIGHT][SQ_E4]);
  cout << actual;
  ASSERT_EQ(expected, actual);

  expected = "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   | X |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   | X |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   | X |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n";
  actual = print(pseudoAttacks[KNIGHT][SQ_H2]);
  cout << actual;
  ASSERT_EQ(expected, actual);
}

TEST(BitboardsTest, kingAttacks) {
  NEWLINE;
  init();
  string expected, actual;

  expected = "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   | X |   | X |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   | X |   |   |   | X |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   | X |   |   |   | X |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   | X |   | X |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n";
  actual = print(pseudoAttacks[KNIGHT][SQ_E4]);
  cout << actual;
  ASSERT_EQ(expected, actual);

  expected = "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   | X | X |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   | X |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   | X | X |\n"
             "+---+---+---+---+---+---+---+---+\n";
  actual = print(pseudoAttacks[KING][SQ_H2]);
  cout << actual;
  ASSERT_EQ(expected, actual);
}


TEST(BitboardsTest, slidingAttacks) {
  NEWLINE;
  init();
  string expected, actual;

  expected = "+---+---+---+---+---+---+---+---+\n"
             "| X |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   | X |   |   |   |   |   | X |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   | X |   |   |   | X |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   | X |   | X |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   | X |   | X |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   | X |   |   |   | X |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   | X |   |   |   |   |   | X |\n"
             "+---+---+---+---+---+---+---+---+\n";
  actual = print(pseudoAttacks[BISHOP][SQ_E4]);
  cout << actual;
  ASSERT_EQ(expected, actual);

  expected = "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   | X |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   | X |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   | X |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   | X |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "| X | X | X | X |   | X | X | X |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   | X |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   | X |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   | X |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n";
  actual = print(pseudoAttacks[ROOK][SQ_E4]);
  cout << actual;
  ASSERT_EQ(expected, actual);

  expected = "+---+---+---+---+---+---+---+---+\n"
             "| X |   |   |   | X |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   | X |   |   | X |   |   | X |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   | X |   | X |   | X |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   | X | X | X |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "| X | X | X | X |   | X | X | X |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   | X | X | X |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   | X |   | X |   | X |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   | X |   |   | X |   |   | X |\n"
             "+---+---+---+---+---+---+---+---+\n";
  actual = print(pseudoAttacks[QUEEN][SQ_E4]);
  cout << actual;
  ASSERT_EQ(expected, actual);
  ASSERT_EQ(pseudoAttacks[QUEEN][SQ_E4], (pseudoAttacks[BISHOP][SQ_E4] | pseudoAttacks[ROOK][SQ_E4]));
}

TEST(BitboardsTest, masks) {
  NEWLINE;
  init();
  string expected, actual;

  expected = "+---+---+---+---+---+---+---+---+\n"
             "| X | X | X | X |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "| X | X | X | X |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "| X | X | X | X |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "| X | X | X | X |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "| X | X | X | X |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "| X | X | X | X |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "| X | X | X | X |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "| X | X | X | X |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n";
  actual = print(filesWestMask[SQ_E4]);
  cout << actual;
  ASSERT_EQ(expected, actual);

  expected = "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   | X | X | X |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   | X | X | X |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   | X | X | X |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   | X | X | X |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   | X | X | X |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   | X | X | X |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   | X | X | X |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   | X | X | X |\n"
             "+---+---+---+---+---+---+---+---+\n";
  actual = print(filesEastMask[SQ_E4]);
  cout << actual;
  ASSERT_EQ(expected, actual);

  expected = "+---+---+---+---+---+---+---+---+\n"
             "| X | X | X | X | X | X | X | X |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "| X | X | X | X | X | X | X | X |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "| X | X | X | X | X | X | X | X |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "| X | X | X | X | X | X | X | X |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n";
  actual = print(ranksNorthMask[SQ_E4]);
  cout << actual;
  ASSERT_EQ(expected, actual);

  expected = "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "| X | X | X | X | X | X | X | X |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "| X | X | X | X | X | X | X | X |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "| X | X | X | X | X | X | X | X |\n"
             "+---+---+---+---+---+---+---+---+\n";
  actual = print(ranksSouthMask[SQ_E4]);
  cout << actual;
  ASSERT_EQ(expected, actual);
}

TEST(BitboardsTest, rays) {
  NEWLINE;
  init();
  string expected, actual;

  expected = "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   | X |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   | X |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   | X |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   | X |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n";
  actual = print(rays[N][SQ_E4]);
  cout << actual;
  ASSERT_EQ(expected, actual);

  expected = "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   | X |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   | X |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   | X |\n"
             "+---+---+---+---+---+---+---+---+\n";
  actual = print(rays[SE][SQ_E4]);
  cout << actual;
  ASSERT_EQ(expected, actual);
}

TEST(BitboardsTest, intermediates) {
  NEWLINE;
  init();
  string expected, actual;

  expected = "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   | X |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   | X |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   | X |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n";
  actual = print(intermediateBB[SQ_C3][SQ_G7]);
  cout << actual;
  ASSERT_EQ(expected, actual);

  expected = "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   | X |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   | X |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   | X |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   | X |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n";
  actual = print(intermediateBB[SQ_A7][SQ_F2]);
  cout << actual;
  ASSERT_EQ(expected, actual);

  expected = "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "| X |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "| X |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "| X |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "| X |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n";
  actual = print(intermediateBB[SQ_A7][SQ_A2]);
  cout << actual;
  ASSERT_EQ(expected, actual);

  expected = "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n";
  actual = print(intermediateBB[SQ_A7][SQ_H1]);
  cout << actual;
  ASSERT_EQ(expected, actual);

  expected = "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   | X | X | X |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   |   |   |   |   |   |   |   |\n"
             "+---+---+---+---+---+---+---+---+\n";
  actual = print(intermediateBB[SQ_H7][SQ_D7]);
  cout << actual;
  ASSERT_EQ(expected, actual);


}

TEST(BitboardsTest, checkers) {
  NEWLINE;
  init();
  string expected, actual;

  expected = "+---+---+---+---+---+---+---+---+\n"
             "| X |   | X |   | X |   | X |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   | X |   | X |   | X |   | X |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "| X |   | X |   | X |   | X |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   | X |   | X |   | X |   | X |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "| X |   | X |   | X |   | X |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   | X |   | X |   | X |   | X |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "| X |   | X |   | X |   | X |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   | X |   | X |   | X |   | X |\n"
             "+---+---+---+---+---+---+---+---+\n";
  actual = print(whiteSquaresBB);
  cout << actual;
  ASSERT_EQ(expected, actual);

  expected = "+---+---+---+---+---+---+---+---+\n"
             "|   | X |   | X |   | X |   | X |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "| X |   | X |   | X |   | X |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   | X |   | X |   | X |   | X |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "| X |   | X |   | X |   | X |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   | X |   | X |   | X |   | X |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "| X |   | X |   | X |   | X |   |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "|   | X |   | X |   | X |   | X |\n"
             "+---+---+---+---+---+---+---+---+\n"
             "| X |   | X |   | X |   | X |   |\n"
             "+---+---+---+---+---+---+---+---+\n";
  actual = print(blackSquaresBB);
  cout << actual;
  ASSERT_EQ(expected, actual);
}

TEST(BitboardsTest, centerDistance) {
  NEWLINE;
  init();
  ASSERT_EQ(2, centerDistance[SQ_C2]);
  ASSERT_EQ(3, centerDistance[SQ_B8]);
  ASSERT_EQ(3, centerDistance[SQ_H1]);
  ASSERT_EQ(3, centerDistance[SQ_H7]);
}