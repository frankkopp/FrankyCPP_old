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

#ifndef FRANKYCPP_EVALUATORCONFIG_H
#define FRANKYCPP_EVALUATORCONFIG_H

struct EvaluatorConfig {

  bool USE_PAWN_TABLE = true;
  std::size_t PAWN_TABLE_SIZE = 1'048'576; // 2^20

  // set values for bonus > 0 and for penalty < 0

   int TEMPO = 30;

   bool USE_MATERIAL = true;
   int MATERIAL_WEIGHT = 1;

   bool USE_POSITION = true;
   int POSITION_WEIGHT = 1;

   bool USE_MOBILITY = true;
   int MOBILITY_WEIGHT = 2;

   bool USE_PAWNEVAL = true;
   int PAWNEVAL_WEIGHT = 1;
   int ISOLATED_PAWN_MID_WEIGHT = -5;
   int ISOLATED_PAWN_END_WEIGHT = -15;
   int DOUBLED_PAWN_MID_WEIGHT = -5;
   int DOUBLED_PAWN_END_WEIGHT = -15;
   int PASSED_PAWN_MID_WEIGHT = 15;
   int PASSED_PAWN_END_WEIGHT = 20;
   int BLOCKED_PAWN_MID_WEIGHT = -1;
   int BLOCKED_PAWN_END_WEIGHT = -10;
   int PHALANX_PAWN_MID_WEIGHT = 10;
   int PHALANX_PAWN_END_WEIGHT = 5;
   int SUPPORTED_PAWN_MID_WEIGHT = 5;
   int SUPPORTED_PAWN_END_WEIGHT = 5;

   bool USE_CHECK_BONUS = true;
   int CHECK_VALUE = 30;

   bool USE_PIECE_BONI = true;
   int BISHOP_PAIR = 30;
   int KNIGHT_PAIR = 10;
   int ROOK_PAIR = 15;

   bool USE_KING_CASTLE_SAFETY = true;
   int KING_CASTLE_SAFETY_WEIGHT = 1;
   int KING_SAFETY_PAWNSHIELD = 50;
   int TRAPPED_ROOK_PENALTY = -50;
   int TRAPPED_BISHOP_PENALTY = -50;

};

#endif //FRANKYCPP_EVALUATORCONFIG_H
