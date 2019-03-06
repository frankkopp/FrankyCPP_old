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
    std::cout << Bitboards::print(SquareBB[i]) << std::endl;
  }

  std::cout << Bitboards::print(SquareBB[SQ_A1]) << std::endl;
  std::cout << Bitboards::printFlat(SquareBB[SQ_A1]) << std::endl;
  std::cout << Bitboards::print(SquareBB[SQ_H1]) << std::endl;
  std::cout << Bitboards::printFlat(SquareBB[SQ_H1]) << std::endl;
  std::cout << Bitboards::print(SquareBB[SQ_A8]) << std::endl;
  std::cout << Bitboards::printFlat(SquareBB[SQ_A8]) << std::endl;
  std::cout << Bitboards::print(SquareBB[SQ_H8]) << std::endl;
  std::cout << Bitboards::printFlat(SquareBB[SQ_H8]) << std::endl;

  std::cout << Bitboards::print(SquareBB[SQ_H8]) << std::endl;
  std::cout << Bitboards::printFlat(SquareBB[SQ_H8]) << std::endl;

  std::cout << Bitboards::print(ALL_BB) << std::endl;

}