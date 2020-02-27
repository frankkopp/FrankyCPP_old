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

#ifndef FRANKYCPP_ATTACKS_H
#define FRANKYCPP_ATTACKS_H

#include "Position.h"

class Attacks {

public:
  /**
   * Evaluates the SEE score for the given move which has not been made on the position
   * yet.<br/>
   * En-passant captures will always return a score of +100 and should therefore not be
   * cut-off.<br/>
   * Credit: https://www.chessprogramming.org/SEE_-_The_Swap_Algorithm
   *
   * Returns a value for the static exchange. The given move must be a capturing
   * move otherwise returns Value(0) .
   */
  static Value see(Position& position, Move move);

  static Bitboard attacksTo(Position& position, Square square, Color color);
  static Bitboard revealedAttacks(Position& position, Square square, Bitboard occupiedBitboard, Color color);
  static Square   getLeastValuablePiece(Position& position, Bitboard bitboard, Color color);
};


#endif //FRANKYCPP_ATTACKS_H
