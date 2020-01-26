/*
 * MIT License
 *
 * Copyright (c) 2020 Frank Kopp
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
#include "Logging.h"
#include "types.h"
#include "misc.h"
#include <Position.h>

using testing::Eq;


class MiscTest : public ::testing::Test {
public:
  static void SetUpTestSuite() {
    NEWLINE;
    INIT::init();
    NEWLINE;
  }

  std::shared_ptr<spdlog::logger> LOG = spdlog::get("Test_Logger");


protected:
  void SetUp() override {}
  void TearDown() override {}
};

TEST_F(MiscTest, moveFromSAN) {
  Position position;
  Move expected;
  Move actual;

  Logger::get().MAIN_LOG->set_level(spdlog::level::critical);

  expected = createMove("e2e4");
  actual = Misc::getMoveFromSAN(position, "e4");
  ASSERT_EQ(expected, actual);

  position = Position("r1bqk2r/ppp2ppp/2np1n2/2b1p3/2B1P3/1P1P1N2/P1P2PPP/RNBQK2R w KQkq - 0 6");

  // not a move on this position
  expected = MOVE_NONE;
  actual = Misc::getMoveFromSAN(position, "e4");
  ASSERT_EQ(expected, actual);

  // ambiguous
  expected = MOVE_NONE;
  actual = Misc::getMoveFromSAN(position, "d2");
  ASSERT_EQ(expected, actual);

  expected = createMove("d1d2");
  actual = Misc::getMoveFromSAN(position, "Qd2");
  ASSERT_EQ(expected, actual);

  expected = createMove("e1d2");
  actual = Misc::getMoveFromSAN(position, "Kd2");
  ASSERT_EQ(expected, actual);

  expected = createMove("c1d2");
  actual = Misc::getMoveFromSAN(position, "Bd2");
  ASSERT_EQ(expected, actual);

  position = Position("r1bqk2r/p1p2pp1/1pnp1n1p/2b1p3/2B1P2N/1P1P4/P1PN1PPP/R1BQK2R w KQkq - 0 8");

  // ambiguous
  expected = createMove("f2f3");
  actual = Misc::getMoveFromSAN(position, "f3");
  ASSERT_EQ(expected, actual);

  // ambiguous
  expected = MOVE_NONE;
  actual = Misc::getMoveFromSAN(position, "Nf3");
  ASSERT_EQ(expected, actual);

  // file disambiguation
  expected = createMove("d2f3");
  actual = Misc::getMoveFromSAN(position, "Ndf3");
  ASSERT_EQ(expected, actual);

  // file disambiguation
  expected = createMove("h4f3");
  actual = Misc::getMoveFromSAN(position, "Nhf3");
  ASSERT_EQ(expected, actual);

  position = Position("r3k2r/pbpq1pp1/1pnp1n1p/2b1pN2/2B1P3/1P1P1N2/P1P2PPP/R1BQK2R w KQkq - 4 10");

  // pawn
  expected = createMove("h2h4");
  actual = Misc::getMoveFromSAN(position, "h4");
  ASSERT_EQ(expected, actual);

  // ambiguous
  expected = MOVE_NONE;
  actual = Misc::getMoveFromSAN(position, "Nh4");
  ASSERT_EQ(expected, actual);

  // rank disambiguation
  expected = createMove("f3h4");
  actual = Misc::getMoveFromSAN(position, "N3h4");
  ASSERT_EQ(expected, actual);

  // rank disambiguation
  expected = createMove("f5h4");
  actual = Misc::getMoveFromSAN(position, "N5h4");
  ASSERT_EQ(expected, actual);

  // castling white king side
  expected = createMove<CASTLING>("e1g1");
  actual = Misc::getMoveFromSAN(position, "O-O");
  ASSERT_EQ(expected, actual);

  position = Position("r3k2r/1ppn3p/2q1q1n1/4P3/2q1Pp2/6R1/pbp2PPP/1R4K1 b kq e3");

  // promotion ambigous
  expected = MOVE_NONE;
  actual = Misc::getMoveFromSAN(position, "Qb1");
  ASSERT_EQ(expected, actual);

  // promotion
  expected = createMove<PROMOTION>("a2b1q");
  actual = Misc::getMoveFromSAN(position, "ab1=Q");
  ASSERT_EQ(expected, actual);

  // en passant
  expected = createMove<ENPASSANT>("f4e3");
  actual = Misc::getMoveFromSAN(position, "e3");
  ASSERT_EQ(expected, actual);

  // capture sign
  position = Position("7k/8/3p4/4N3/8/5p2/P7/1K2N3 w - -");
  expected = createMove("e5f3");
  actual = Misc::getMoveFromSAN(position, "N5xf3");
  ASSERT_EQ(expected, actual);

  // r7/2r1kpp1/1p6/pB1Pp1P1/Pbp1P3/2N2b1P/1PPK1P2/R6R b - - bm Bxh1; id "FRANKY-1 #11";
  position = Position("r7/2r1kpp1/1p6/pB1Pp1P1/Pbp1P3/2N2b1P/1PPK1P2/R6R b - -");
  expected = createMove("f3h1");
  actual = Misc::getMoveFromSAN(position, "Bxh1");
  ASSERT_EQ(expected, actual);

  position = Position("r2qr1k1/pb2bp1p/1pn1p1pB/8/2BP4/P1P2N2/4QPPP/3R1RK1 w - - 0 1");
  expected = createMove("d4d5");
  actual = Misc::getMoveFromSAN(position, "d5");
  ASSERT_EQ(expected, actual);

}

