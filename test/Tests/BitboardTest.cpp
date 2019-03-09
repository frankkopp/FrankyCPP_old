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

using testing::Eq;

TEST(BitboardsTest, print) {

  // TODO do some asserts to really test

  Bitboards::init();

  std::cout << "\n";

  std::cout << Bitboards::print(EMPTY_BB) << std::endl;
  std::cout << Bitboards::printFlat(EMPTY_BB) << std::endl;
  std::cout << Bitboards::print(ALL_BB) << std::endl;
  std::cout << Bitboards::printFlat(ALL_BB) << std::endl;

  for (Square i = SQ_A1; i <= SQ_H8; ++i) {
    std::cout << squareLabel(i) << std::endl;
    std::cout << Bitboards::print(squareBB[i]) << std::endl;
  }

  std::cout << Bitboards::print(squareBB[SQ_A1]) << std::endl;
  std::cout << Bitboards::printFlat(squareBB[SQ_A1]) << std::endl;
  std::cout << Bitboards::print(squareBB[SQ_H1]) << std::endl;
  std::cout << Bitboards::printFlat(squareBB[SQ_H1]) << std::endl;
  std::cout << Bitboards::print(squareBB[SQ_A8]) << std::endl;
  std::cout << Bitboards::printFlat(squareBB[SQ_A8]) << std::endl;
  std::cout << Bitboards::print(squareBB[SQ_H8]) << std::endl;
  std::cout << Bitboards::printFlat(squareBB[SQ_H8]) << std::endl;

  std::cout << Bitboards::print(squareBB[SQ_H8]) << std::endl;
  std::cout << Bitboards::printFlat(squareBB[SQ_H8]) << std::endl;

  std::cout << Bitboards::print(ALL_BB) << std::endl;
}


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

  ASSERT_EQ(DiagUpA1, squareDiagUp[SQ_A1]);
  ASSERT_EQ(DiagUpA1, squareDiagUp[SQ_C3]);
  ASSERT_EQ(DiagUpA1, squareDiagUp[SQ_G7]);
  ASSERT_EQ(DiagUpA1, squareDiagUp[SQ_H8]);

  ASSERT_EQ(DiagDownH1, squareDiagDown[SQ_A8]);
  ASSERT_EQ(DiagDownH1, squareDiagDown[SQ_C6]);
  ASSERT_EQ(DiagDownH1, squareDiagDown[SQ_G2]);
  ASSERT_EQ(DiagDownH1, squareDiagDown[SQ_H1]);
}


TEST(BitboardsTest, bitScans) {
  Bitboards::init();

  ASSERT_EQ(1, popcount(squareBB[SQ_D3]));
  ASSERT_EQ(2, popcount(squareBB[SQ_D3] | squareBB[SQ_H2]));
  ASSERT_EQ(8, popcount(DiagUpA1));

  ASSERT_EQ(19, lsb(squareBB[SQ_D3]));
  ASSERT_EQ(19, msb(squareBB[SQ_D3]));



}
