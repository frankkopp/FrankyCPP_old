/*
 * MIT License
 *
 * Copyright (c) 2019 Frank Kopp
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

#include <utility>
#include "../../src/logging.h"
#include "../../src/Values.h"

using testing::Eq;

class ValuesTest : public ::testing::Test {
public:

  static void SetUpTestSuite() {
    NEWLINE;
    LOGGING::init();
    INIT::init();
    NEWLINE;

    spdlog::set_level(spdlog::level::trace);
  }

  std::shared_ptr<spdlog::logger> LOG = spdlog::get("Test_Logger");

protected:

  void SetUp() override {}
  void TearDown() override {}

};

TEST_F(ValuesTest, basic) {
  ASSERT_EQ(25, Values::posMidValue[WHITE_PAWN][SQ_E4]);
  ASSERT_EQ(-30, Values::posMidValue[WHITE_KNIGHT][SQ_H3]);
  ASSERT_EQ(5, Values::posMidValue[WHITE_BISHOP][SQ_G2]);
  ASSERT_EQ(-15, Values::posMidValue[WHITE_ROOK][SQ_H1]);
  ASSERT_EQ(2, Values::posMidValue[WHITE_QUEEN][SQ_E5]);
  ASSERT_EQ(50, Values::posMidValue[WHITE_KING][SQ_G1]);

  ASSERT_EQ(25, Values::posMidValue[BLACK_PAWN][SQ_E5]);
  ASSERT_EQ(-30, Values::posMidValue[BLACK_KNIGHT][SQ_A6]);
  ASSERT_EQ(5, Values::posMidValue[BLACK_BISHOP][SQ_B7]);
  ASSERT_EQ(-15, Values::posMidValue[BLACK_ROOK][SQ_A8]);
  ASSERT_EQ(2, Values::posMidValue[BLACK_QUEEN][SQ_D4]);
  ASSERT_EQ(50, Values::posMidValue[BLACK_KING][SQ_G8]);

  ASSERT_EQ(50, Values::posEndValue[WHITE_PAWN][SQ_E7]);
  ASSERT_EQ(-30, Values::posEndValue[WHITE_KNIGHT][SQ_H3]);
  ASSERT_EQ(0, Values::posEndValue[WHITE_BISHOP][SQ_G2]);
  ASSERT_EQ(5, Values::posEndValue[WHITE_ROOK][SQ_H8]);
  ASSERT_EQ(5, Values::posEndValue[WHITE_QUEEN][SQ_E5]);
  ASSERT_EQ(-30, Values::posEndValue[WHITE_KING][SQ_G1]);

  ASSERT_EQ(50, Values::posEndValue[BLACK_PAWN][SQ_E2]);
  ASSERT_EQ(-30, Values::posEndValue[BLACK_KNIGHT][SQ_A6]);
  ASSERT_EQ(0, Values::posEndValue[BLACK_BISHOP][SQ_B7]);
  ASSERT_EQ(5, Values::posEndValue[BLACK_ROOK][SQ_A1]);
  ASSERT_EQ(5, Values::posEndValue[BLACK_QUEEN][SQ_D4]);
  ASSERT_EQ(-30, Values::posEndValue[BLACK_KING][SQ_G8]);

  const Value value = Values::posMidValue[WHITE_PAWN][SQ_A4];
  const Value value1 = Values::posValue[WHITE_PAWN][SQ_A4][GAME_PHASE_MAX];
  ASSERT_EQ(value, value1);
  const Value value2 = Values::posEndValue[WHITE_PAWN][SQ_A4];
  const Value value3 = Values::posValue[WHITE_PAWN][SQ_A4][0];
  ASSERT_EQ(value2, value3);

}