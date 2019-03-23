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
  Position position;
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
    { 7,     3'195'901'860ULL,     108'329'926ULL,     319'617ULL,     33'103'848ULL,        435'767ULL },
    { 8,    84'998'978'956ULL,   3'523'740'106ULL,   7'187'977ULL,    968'981'593ULL,      9'852'036ULL },
    { 9, 2'439'530'234'167ULL, 125'208'536'153ULL, 319'496'827ULL, 36'095'901'903ULL,    400'191'963ULL }
  };
  // @formatter:on

  int maxDepth = 6;

  for (int i = 1; i <= maxDepth; i++) {
    p.perft(i);
    NEWLINE
    ASSERT_EQ(results[i][1], p.getNodes());
    ASSERT_EQ(results[i][2], p.getCaptureCounter());
    ASSERT_EQ(results[i][3], p.getEnpassantCounter());
    ASSERT_EQ(results[i][4], p.getCheckCounter());
    ASSERT_EQ(results[i][5], p.getCheckMateCounter());
  }
  cout << "==============================" << endl;

}

TEST(PerftTest, kiwiPetePerft) {
  Position::init();
  Bitboards::init();
  NEWLINE;

  MoveGenerator mg;
  Position position;

  cout << "Kiwipete PERFT Test" << endl;
  cout << "==============================" << endl;

  Perft p("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -");

  // @formatter:off
  const u_int64_t results[6][6] = {
    //N  Nodes      Captures EP     Checks  Mates
    { 0, 0,         0,       0,     0,      0},
    { 1, 48,        8,       0,     0,      0},
    { 2, 2039,      351,     1,     3,      0},
    { 3, 97862,     17102,   45,    993,    1},
    { 4, 4085603,   757163,  1929,  25523,  43},
    { 5, 193690690, 35043416,73365, 3309887,30171},
  };
  // @formatter:on

  int maxDepth = 5;

  for (int i = 1; i <= maxDepth; i++) {
    p.perft(i);
    NEWLINE
    ASSERT_EQ(results[i][1], p.getNodes());
    ASSERT_EQ(results[i][2], p.getCaptureCounter());
    ASSERT_EQ(results[i][3], p.getEnpassantCounter());
    ASSERT_EQ(results[i][4], p.getCheckCounter());
    ASSERT_EQ(results[i][5], p.getCheckMateCounter());
  }
  cout << "==============================" << endl;

}

TEST(PerftTest, pos3Perft) {
  Position::init();
  Bitboards::init();
  NEWLINE;

  MoveGenerator mg;
  Position position;

  cout << "Pos3 PERFT Test" << endl;
  cout << "==============================" << endl;

  Perft p("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - -");

  // @formatter:off
  const u_int64_t results[8][6] = {
          //N  Nodes      Captures EP     Checks   Mates
          { 0, 0,         0,       0,     0,       0},
          { 1, 14,        1,       0,     2,       0},
          { 2, 191,       14,      0,     10,      0},
          { 3, 2812,      209,     2,     267,     0},
          { 4, 43238,     3348,    123,   1680,    17},
          { 5, 674624,    52051,   1165,  52950,   0},
          { 6, 11030083,  940350,  33325, 452473,  2733},
          { 7, 178633661, 14519036,294874,12797406,87}
  };
  // @formatter:on

  int maxDepth = 6;

  for (int i = 1; i <= maxDepth; i++) {
    p.perft(i);
    NEWLINE
    ASSERT_EQ(results[i][1], p.getNodes());
    ASSERT_EQ(results[i][2], p.getCaptureCounter());
    ASSERT_EQ(results[i][3], p.getEnpassantCounter());
    ASSERT_EQ(results[i][4], p.getCheckCounter());
    ASSERT_EQ(results[i][5], p.getCheckMateCounter());
  }
  cout << "==============================" << endl;

}

TEST(PerftTest, pos4Perft) {
  Position::init();
  Bitboards::init();
  NEWLINE;

  MoveGenerator mg;
  Position position;

  cout << "Pos4 PERFT Test" << endl;
  cout << "==============================" << endl;

  Perft p("r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1");

  // @formatter:off
  const u_int64_t results[7][6] = {
          //N  Nodes      Captures EP     Checks   Mates
          { 0, 0,         0,         0,     0,        0},
          { 1, 6,         0,         0,     0,        0},
          { 2, 264,       87,        0,     10,       0},
          { 3, 9467,      1021,      4,     38,       22},
          { 4, 422333,    131393,    0,     15492,    5},
          { 5, 15833292,  2046173,   6512,  200568,   50562},
          { 6, 706045033, 210369132, 212,   26973664, 81076}
  };
  // @formatter:on

  int maxDepth = 5;

  for (int i = 1; i <= maxDepth; i++) {
    p.perft(i);
    NEWLINE
    ASSERT_EQ(results[i][1], p.getNodes());
    ASSERT_EQ(results[i][2], p.getCaptureCounter());
    ASSERT_EQ(results[i][3], p.getEnpassantCounter());
    ASSERT_EQ(results[i][4], p.getCheckCounter());
    ASSERT_EQ(results[i][5], p.getCheckMateCounter());
  }
  cout << "==============================" << endl;

  cout << "Pos4 Mirrored PERFT Test" << endl;
  cout << "==============================" << endl;

  Perft p2("r2q1rk1/pP1p2pp/Q4n2/bbp1p3/Np6/1B3NBn/pPPP1PPP/R3K2R b KQ - 0 1");

  // @formatter:off
  const u_int64_t results2[7][6] = {
          //N  Nodes      Captures EP     Checks   Mates
          { 0, 0,         0,         0,     0,        0},
          { 1, 6,         0,         0,     0,        0},
          { 2, 264,       87,        0,     10,       0},
          { 3, 9467,      1021,      4,     38,       22},
          { 4, 422333,    131393,    0,     15492,    5},
          { 5, 15833292,  2046173,   6512,  200568,   50562},
          { 6, 706045033, 210369132, 212,   26973664, 81076}
  };
  // @formatter:on

  for (int i = 1; i <= maxDepth; i++) {
    p2.perft(i);
    NEWLINE
    ASSERT_EQ(results2[i][1], p2.getNodes());
    ASSERT_EQ(results2[i][2], p2.getCaptureCounter());
    ASSERT_EQ(results2[i][3], p2.getEnpassantCounter());
    ASSERT_EQ(results2[i][4], p2.getCheckCounter());
    ASSERT_EQ(results2[i][5], p2.getCheckMateCounter());
  }
  cout << "==============================" << endl;
}


TEST(PerftTest, pos5Perft) {
  Position::init();
  Bitboards::init();
  NEWLINE;

  MoveGenerator mg;
  Position position;

  cout << "Pos5 PERFT Test" << endl;
  cout << "==============================" << endl;

  Perft p("rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ -");

  // @formatter:off
  const u_int64_t results[6][6] = {
          //N  Nodes      Captures EP     Checks   Mates
          { 0, 0       },
          { 1, 44       },
          { 2, 1486     },
          { 3, 62379    },
          { 4, 2103487  },
          { 5, 89941194 }
  };
  // @formatter:on

  int maxDepth = 5;

  for (int i = 1; i <= maxDepth; i++) {
    p.perft(i);
    NEWLINE
    ASSERT_EQ(results[i][1], p.getNodes());
  }
  cout << "==============================" << endl;

}

/**
  * Perft Test
  *
  * *  TalkChess PERFT Tests (by Martin Sedlak)
  *  * //--Illegal ep move #1
  *  * 3k4/3p4/8/K1P4r/8/8/8/8 b - - 0 1; perft 6 = 1134888
  *  * //--Illegal ep move #2
  *  * 8/8/4k3/8/2p5/8/B2P2K1/8 w - - 0 1; perft 6 = 1015133
  *  * //--EP Capture Checks Opponent
  *  * 8/8/1k6/2b5/2pP4/8/5K2/8 b - d3 0 1; perft 6 = 1440467
  *  * //--Short Castling Gives Check
  *  * 5k2/8/8/8/8/8/8/4K2R w K - 0 1; perft 6 = 661072
  *  * //--Long Castling Gives Check
  *  * 3k4/8/8/8/8/8/8/R3K3 w Q - 0 1; perft 6 = 803711
  *  * //--Castle Rights
  *  * r3k2r/1b4bq/8/8/8/8/7B/R3K2R w KQkq - 0 1; perft 4 = 1274206
  *  * //--Castling Prevented
  *  * r3k2r/8/3Q4/8/8/5q2/8/R3K2R b KQkq - 0 1; perft 4 = 1720476
  *  * //--Promote out of Check
  *  * 2K2r2/4P3/8/8/8/8/8/3k4 w - - 0 1; perft 6 = 3821001
  *  * //--Discovered Check
  *  * 8/8/1P2K3/8/2n5/1q6/8/5k2 b - - 0 1; perft 5 = 1004658
  *  * //--Promote to give check
  *  * 4k3/1P6/8/8/8/8/K7/8 w - - 0 1; perft 6 = 217342
  *  * //--Under Promote to give check
  *  * 8/P1k5/K7/8/8/8/8/8 w - - 0 1; perft 6 = 92683
  *  * //--Self Stalemate
  *  * K1k5/8/P7/8/8/8/8/8 w - - 0 1; perft 6 = 2217
  *  * //--Stalemate & Checkmate
  *  * 8/k1P5/8/1K6/8/8/8/8 w - - 0 1; perft 7 = 567584
  *  * //--Stalemate & Checkmate
  *  * 8/8/2k5/5q2/5n2/8/5K2/8 b - - 0 1; perft 4 = 23527
  */
void variousPerftTests(const string &s, int depth, int result);
TEST(PerftTest, Various) {
  Position::init();
  Bitboards::init();
  NEWLINE;

  variousPerftTests("3k4/3p4/8/K1P4r/8/8/8/8 b - - 0 1", 6, 1134888);
  variousPerftTests("8/8/4k3/8/2p5/8/B2P2K1/8 w - - 0 1", 6, 1015133);
  variousPerftTests("8/8/1k6/2b5/2pP4/8/5K2/8 b - d3 0 1", 6, 1440467);
  variousPerftTests("5k2/8/8/8/8/8/8/4K2R w K - 0 1", 6, 661072);
  variousPerftTests("3k4/8/8/8/8/8/8/R3K3 w Q - 0 1", 6, 803711);
  variousPerftTests("r3k2r/1b4bq/8/8/8/8/7B/R3K2R w KQkq - 0 1", 4, 1274206);
  variousPerftTests("r3k2r/8/3Q4/8/8/5q2/8/R3K2R b KQkq - 0 1", 4, 1720476);
  variousPerftTests("2K2r2/4P3/8/8/8/8/8/3k4 w - - 0 1", 6, 3821001);
  variousPerftTests("8/8/1P2K3/8/2n5/1q6/8/5k2 b - - 0 1", 5, 1004658);
  variousPerftTests("4k3/1P6/8/8/8/8/K7/8 w - - 0 1", 6, 217342);
  variousPerftTests("8/P1k5/K7/8/8/8/8/8 w - - 0 1", 6, 92683);
  variousPerftTests("K1k5/8/P7/8/8/8/8/8 w - - 0 1", 6, 2217);
  variousPerftTests("8/k1P5/8/1K6/8/8/8/8 w - - 0 1", 7, 567584);
  variousPerftTests("8/8/2k5/5q2/5n2/8/5K2/8 b - - 0 1", 4, 23527);
  variousPerftTests("3k4/3p4/8/K1P4r/8/8/8/8 b - - 0 1", 6, 1134888);
  // promotions
  variousPerftTests("n1n5/PPPk4/8/8/8/8/4Kppp/5N1N b - - 0 1", 1, 24);
  variousPerftTests("n1n5/PPPk4/8/8/8/8/4Kppp/5N1N b - - 0 1", 2, 496);
  variousPerftTests("n1n5/PPPk4/8/8/8/8/4Kppp/5N1N b - - 0 1", 3, 9483);
  variousPerftTests("n1n5/PPPk4/8/8/8/8/4Kppp/5N1N b - - 0 1", 4, 182838);
  variousPerftTests("n1n5/PPPk4/8/8/8/8/4Kppp/5N1N b - - 0 1", 5, 3605103);
  variousPerftTests("n1n5/PPPk4/8/8/8/8/4Kppp/5N1N b - - 0 1", 6, 71179139);

}

void variousPerftTests(const string &s, int depth, int result) {
  println("Various PERFT Tests");
  println("==============================");
  println(s);
  println("Expected Result: " + to_string(result));
  MoveGenerator mg;
  Position position;
  Perft p(s);
  p.perft(depth);
  println("Actual Result: " + to_string(p.getNodes()));
  ASSERT_EQ(result, p.getNodes());
  println("==============================\n");
}
