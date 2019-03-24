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

#include "../../src/datatypes.h"
#include "../../src/Values.h"
#include "../../src/Position.h"

using namespace std;
using namespace Values;
using testing::Eq;

TEST(EvaluationTest, posValue) {
  NEWLINE;
  Values::init();
  Position::init();
  Bitboards::init();

//  for (Piece pc = WHITE_KING; pc < PIECE_NONE; ++pc) {
//    for (Square sq = SQ_A1; sq <= SQ_H8; ++sq) {
//      for (int gp = GAME_PHASE_MAX; gp >= 0; gp--) {
//        cout << "Pc: " << pieceTypeToChar[pc] << " Sq: " << squareLabel(sq) << " Gp: " << gp << endl;
//        cout << "PRE=  = " << posValue[pc][sq][gp] << endl;
//      }
//    }
//  }

  for (int gp = GAME_PHASE_MAX; gp >= 0; gp--) {
    cout << "WHITE = " << posValue[WHITE_KING][SQ_E2][gp] << endl;
    cout << "BLACK = " << posValue[BLACK_KING][SQ_E7][gp] << endl << endl;
  }

  //  ASSERT_EQ(-30, midGamePosValue[WHITE_PAWN][SQ_D2]);
  //  ASSERT_EQ(-30, midGamePosValue[BLACK_PAWN][SQ_E7]);
  //
  //  ASSERT_EQ(-50, midGamePosValue[WHITE_KNIGHT][SQ_A8]);
  //  ASSERT_EQ(-50, midGamePosValue[BLACK_KNIGHT][SQ_H1]);
  //
  //  ASSERT_EQ(5, endGamePosValue[WHITE_QUEEN][SQ_E4]);
  //  ASSERT_EQ(5, endGamePosValue[BLACK_QUEEN][SQ_D5]);
  //
  //  ASSERT_EQ(50, midGamePosValue[WHITE_KING][SQ_G1]);
  //  ASSERT_EQ(50, midGamePosValue[BLACK_KING][SQ_G8]);

}
