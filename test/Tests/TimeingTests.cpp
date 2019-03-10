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
#include <vector>
#include <chrono>
#include <ostream>
#include <cstdarg>

#include "../../src/Bitboards.h"

using namespace std;

struct myLoc : std::numpunct<char> {
  char do_decimal_point() const override { return ','; }
  char do_thousands_sep() const override { return '.'; }
  std::string do_grouping() const override { return "\03"; }
};

locale loc(cout.getloc(), new myLoc);

void testTiming(ostringstream &os, int rounds, int iterations, int repetitions,
                const vector<void (*)()> &tests);

TEST(TimingTests, popcount) {
  NEWLINE;
  ostringstream os;

  //// TESTS START
  Bitboards::init();
  auto f1 = []() { int i = popcount(DiagUpA1); };
  auto f2 = []() { int i = popcount(DiagUpA1); };
  vector<void (*)()> tests;
  tests.push_back(f1);
  tests.push_back(f2);
  //// TESTS END

  testTiming(os, 5, 50, 10'000'000, tests);

  cout << os.str();
}

void
testTiming(ostringstream &os, int rounds, int iterations, int repetitions,
           const vector<void (*)()> &tests) {

  cout.imbue(loc);
  os.imbue(loc);
  os << setprecision(9);

  os << endl;
  os << "Starting timing test: rounds=" << rounds << " iterations=" << iterations << " repetitions="
     << repetitions << endl;
  os << "======================================================================" << endl;

  // rounds
  for (int round = 1; round <= rounds; ++round) {
    cout << "Round " << round << " of " << rounds << " timing tests." << endl;
    // tests
    int testNr = 1;
    for (auto f : tests) {
      // iterations
      unsigned long long sum = 0ULL;
      int i = 0;
      while (i++ < iterations) {
        // repetitions
        auto start = std::chrono::high_resolution_clock::now();
        for (int j = 0; j < repetitions; ++j) f();
        auto finish = std::chrono::high_resolution_clock::now();
        sum += std::chrono::duration_cast<std::chrono::nanoseconds>(finish - start).count();
      }
      auto avg = ((double) sum / iterations);
      os << "Round " << setw(2) << round << " Test " << setw(2) << testNr++ << ": " << setw(12) << avg
         << " ns" << " (" << setw(12) << (avg/1e9) << " sec)" << endl;
    }
    os << endl;
  }
}



