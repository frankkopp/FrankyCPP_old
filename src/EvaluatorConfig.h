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

namespace EvaluatorConfig {

  inline bool USE_MATERIAL = true;
  inline int MATERIAL_WEIGHT = 1;

  inline bool USE_POSITION = true;
  inline int POSITION_WEIGHT = 1;

  inline bool USE_MOBILITY = true;
  inline int MOBILITY_WEIGHT = 2;

  inline bool USE_PAWNEVAL = true;
  inline int PAWNEVAL_WEIGHT = 1;
  inline int ISOLATED_PAWN_MID_WEIGHT = -5;
  inline int ISOLATED_PAWN_END_WEIGHT = -15;
  inline int DOUBLED_PAWN_MID_WEIGHT = -5;
  inline int DOUBLED_PAWN_END_WEIGHT = -15;
  inline int PASSED_PAWN_MID_WEIGHT = 10;
  inline int PASSED_PAWN_END_WEIGHT = 20;
  inline int BLOCKED_PAWN_MID_WEIGHT = -1;
  inline int BLOCKED_PAWN_END_WEIGHT = -10;
  inline int PHALANX_PAWN_MID_WEIGHT = 10;
  inline int PHALANX_PAWN_END_WEIGHT = 5;
  inline int SUPPORTED_PAWN_MID_WEIGHT = 5;
  inline int SUPPORTED_PAWN_END_WEIGHT = 5;

};

#endif //FRANKYCPP_EVALUATORCONFIG_H
