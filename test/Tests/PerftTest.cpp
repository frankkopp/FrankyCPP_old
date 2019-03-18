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

#include "../../src/Perft.h"

using namespace std;

using testing::Eq;

TEST(PerftTest, stdPerft) {
  Position::init();
  Bitboards::init();
  NEWLINE;
  
  MoveGenerator mg;
  Position     position;
  Perft p;

  cout << "Standard PERFT Test" << endl;
  cout << "==============================" << endl;

  // @formatter:off
  const u_int64_t results[10][6] = {
    //N                 Nodes            Captures              EP             Checks              Mates
    { 0,                 1ULL,               0ULL,           0ULL,              0ULL,              0ULL },
    { 1,                20ULL,               0ULL,           0ULL,              0ULL,              0ULL },
    { 2,               400ULL,               0ULL,           0ULL,              0ULL,              0ULL },
    { 3,             8'902ULL,              34ULL,           0ULL,             12ULL,              0ULL },
    { 4,           197'281ULL,           1'576ULL,           0ULL,            469ULL,              8ULL },
    { 5,         4'865'609ULL,          82'719ULL,         258ULL,         27'351ULL,            347ULL },
    { 6,       119'060'324ULL,       2'812'008ULL,       5'248ULL,        809'099ULL,         10'828ULL },
    { 7,     3'195'901'860ULL,     108'329'926ULL,     319'617ULL,     33'103'848ULL,        435'816ULL },
    { 8,    84'998'978'956ULL,   3'523'740'106ULL,   7'187'977ULL,    968'981'593ULL,      9'852'036ULL },
    { 9, 2'439'530'234'167ULL, 125'208'536'153ULL, 319'496'827ULL, 36'095'901'903ULL,    400'191'963ULL }
  };
  // @formatter:on

  int maxDepth = 8;

  for (int i = 1; i <= maxDepth; i++) {
    p.perft(i);
    ASSERT_EQ(results[i][1], p.getNodes());
    ASSERT_EQ(results[i][2], p.getCaptureCounter());
    ASSERT_EQ(results[i][3], p.getEnpassantCounter());
    ASSERT_EQ(results[i][4], p.getCheckCounter());
//    ASSERT_EQ(results[i][5], p.getCheckMateCounter());
  }
  cout << "==============================" << endl;

}
