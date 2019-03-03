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

#include <string>

#include "globals.h"
#include "Bitboards.h"

Bitboard SquareBB[SQ_NONE];

/**
 * Prints a bitboard a an 8x8 matrix for output on a console
 */
const std::string
Bitboards::print(Bitboard b) {
  std::string s = "+---+---+---+---+---+---+---+---+\n";
  for (Rank r = RANK_8; r >= RANK_1; --r) {
    for (File f = FILE_A; f <= FILE_H; ++f) {
      s += b & getSquare(f, r) ? "| X " : "|   ";
    }
    s += "|\n+---+---+---+---+---+---+---+---+\n";
  }
  return s;
}

/**
 * Prints a bitboard as a series of 0 and 1 grouped in 8 bits
 * beginning with the LSB (0) on the left and the MSB (63) on the right
 */
const std::string
Bitboards::printFlat(Bitboard b) {
  std::string s;
  for (int i = 0; i < 64; i++) {
    if (i > 0 && i % 8 == 0) s += ".";
    s += b & (1L << i) ? "1" : "0";
  }
  s += " (" + std::to_string(b) + ")";
  return s;
}

/**
 * Initializes various pre-computed bitboards
 */
const void
Bitboards::init() {

  // all squares as an array
  for (Square s = SQ_A1; s <= SQ_H8; ++s) SquareBB[s] = (1ULL << s);



}

