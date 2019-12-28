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

#include "Evaluator.h"
#include "EvaluatorConfig.h"
#include "Position.h"

Evaluator::Evaluator() = default;

Value Evaluator::evaluate(const Position &position) {

  // Calculations are always from the view of the white player.

  // MATERIAL
  int matEval = EvaluatorConfig::USE_MATERIAL
                ? position.getMaterial(WHITE) -
                  position.getMaterial(BLACK) : 0;

  // POSITION
  int posEval = EvaluatorConfig::USE_POSITION
                ? position.getPosValue(WHITE) -
                  position.getPosValue(BLACK) : 0;

  auto value = Value(
    matEval * EvaluatorConfig::MATERIAL_WEIGHT
    + posEval * EvaluatorConfig::POSITION_WEIGHT
  );

  // value is always from the view of the next player
  if (position.getNextPlayer() == BLACK) value *= -1;

  return value;
}
