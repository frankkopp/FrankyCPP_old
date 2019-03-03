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
#include <chrono>

#include "../../src/Bitboards.h"

struct space_out : std::numpunct<char> {
  char do_thousands_sep() const override { return '.'; } // separate with spaces
  std::string do_grouping() const override { return "\03"; } // groups of 1 digit
};

void test1();
void test2();

TEST(TimingTests, ArrayVsExp) {

  std::locale loc(std::cout.getloc(), new space_out);
  std::cout.imbue(loc);
  std::cout << "\n";

  int ITERATIONS = 1000000;

  auto start = std::chrono::high_resolution_clock::now();
  for (int j = 0; j < ITERATIONS; ++j) {
    test1();
  }
  auto finish = std::chrono::high_resolution_clock::now();

  std::cout << "Test 1: "
            << std::setprecision(2)
            << std::chrono::duration_cast<std::chrono::nanoseconds>(finish - start).count()
            << " ns\n";

  start = std::chrono::high_resolution_clock::now();
  for (int j = 0; j < ITERATIONS; ++j) {
    test2();
  }
  finish = std::chrono::high_resolution_clock::now();

  std::cout << "Test 2: "
            << std::setprecision(2)
            << std::chrono::duration_cast<std::chrono::nanoseconds>(finish - start).count()
            << " ns\n";

}

void test1() {
  for (Square i = SQ_A1; i <= SQ_H8; ++i) {
    std::string s = squareLabel(i);
  }
}

void test2() {
  for (Square i = SQ_A1; i <= SQ_H8; ++i) {

  }
}