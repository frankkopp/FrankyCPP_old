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

#ifndef FRANKYCPP_EVALUATOR_H
#define FRANKYCPP_EVALUATOR_H

#include "gtest/gtest_prod.h"
#include "Logging.h"
#include "types.h"

// forward declared dependencies
class Position;

class Evaluator {

  std::shared_ptr<spdlog::logger> LOG = spdlog::get("Eval_Logger");

  struct Score {
    int16_t midGameScore = 0;
    int16_t endGameScore = 0;
  };

public:
  Evaluator();

  Value evaluate(const Position &position);

private:

  template <Color C>
  int evaluatePawn(const Position &position);

  template <Color C>
  int evaluateKing(const Position &position);

  template <Color C, PieceType PT>
  int evaluatePiece(const Position &position);

  FRIEND_TEST(EvaluatorTest, evaluatePieceMobility);
  FRIEND_TEST(EvaluatorTest, evaluatePawn);

};




#endif //FRANKYCPP_EVALUATOR_H
