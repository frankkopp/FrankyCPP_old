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

uint8_t PopCnt16[1 << 16];
Bitboard squareBB[SQ_NONE];
Bitboard squareDiagUp[SQ_NONE];
Bitboard squareDiagDown[SQ_NONE];

int8_t squareDistance[SQ_NONE][SQ_NONE];

namespace Bitboards {

/**
 * Prints a bitboard a an 8x8 matrix for output on a console
 */
  const std::string print(Bitboard b) {
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
  const std::string printFlat(Bitboard b) {
    std::string s;
    for (int i = 0; i < 64; i++) {
      if (i > 0 && i % 8 == 0) s += ".";
      s += b & (1L << i) ? "1" : "0";
    }
    s += " (" + std::to_string(b) + ")";
    return s;
  }


/// popcount16() counts the non-zero bits using SWAR-Popcount algorithm
  unsigned popcount16(unsigned u) {
    u -= (u >> 1) & 0x5555U;
    u = ((u >> 2) & 0x3333U) + (u & 0x3333U);
    u = ((u >> 4) + u) & 0x0F0FU;
    return (u * 0x0101U) >> 8;
  }

/**
 * Initializes various pre-computed bitboards
 */
  const void init() {

    for (unsigned i = 0; i < (1 << 16); ++i)
      PopCnt16[i] = (uint8_t) popcount16(i);

    // all squares
    for (Square sq = SQ_A1; sq <= SQ_H8; ++sq) {

      // square bitboard
      squareBB[sq] = (1ULL << sq);

      // square diagonals
      if (DiagUpA8 & sq) squareDiagUp[sq] = DiagUpA8;
      else if (DiagUpA7 & sq) squareDiagUp[sq] = DiagUpA7;
      else if (DiagUpA6 & sq) squareDiagUp[sq] = DiagUpA6;
      else if (DiagUpA5 & sq) squareDiagUp[sq] = DiagUpA5;
      else if (DiagUpA4 & sq) squareDiagUp[sq] = DiagUpA4;
      else if (DiagUpA3 & sq) squareDiagUp[sq] = DiagUpA3;
      else if (DiagUpA2 & sq) squareDiagUp[sq] = DiagUpA2;
      else if (DiagUpA1 & sq) squareDiagUp[sq] = DiagUpA1;
      else if (DiagUpB1 & sq) squareDiagUp[sq] = DiagUpB1;
      else if (DiagUpC1 & sq) squareDiagUp[sq] = DiagUpC1;
      else if (DiagUpD1 & sq) squareDiagUp[sq] = DiagUpD1;
      else if (DiagUpE1 & sq) squareDiagUp[sq] = DiagUpE1;
      else if (DiagUpF1 & sq) squareDiagUp[sq] = DiagUpF1;
      else if (DiagUpG1 & sq) squareDiagUp[sq] = DiagUpG1;
      else if (DiagUpH1 & sq) squareDiagUp[sq] = DiagUpH1;
      if (DiagDownH8 & sq) squareDiagDown[sq] = DiagDownH8;
      else if (DiagDownH7 & sq) squareDiagDown[sq] = DiagDownH7;
      else if (DiagDownH6 & sq) squareDiagDown[sq] = DiagDownH6;
      else if (DiagDownH5 & sq) squareDiagDown[sq] = DiagDownH5;
      else if (DiagDownH4 & sq) squareDiagDown[sq] = DiagDownH4;
      else if (DiagDownH3 & sq) squareDiagDown[sq] = DiagDownH3;
      else if (DiagDownH2 & sq) squareDiagDown[sq] = DiagDownH2;
      else if (DiagDownH1 & sq) squareDiagDown[sq] = DiagDownH1;
      else if (DiagDownG1 & sq) squareDiagDown[sq] = DiagDownG1;
      else if (DiagDownF1 & sq) squareDiagDown[sq] = DiagDownF1;
      else if (DiagDownE1 & sq) squareDiagDown[sq] = DiagDownE1;
      else if (DiagDownD1 & sq) squareDiagDown[sq] = DiagDownD1;
      else if (DiagDownC1 & sq) squareDiagDown[sq] = DiagDownC1;
      else if (DiagDownB1 & sq) squareDiagDown[sq] = DiagDownB1;
      else if (DiagDownA1 & sq) squareDiagDown[sq] = DiagDownA1;
    }

    // distance between squares
    for (Square sq1 = SQ_A1; sq1 <= SQ_H8; ++sq1) {
      for (Square sq2 = SQ_A1; sq2 <= SQ_H8; ++sq2) {
        if (sq1 != sq2)
          squareDistance[sq1][sq2] = (int8_t) std::max(distance(fileOf(sq1), fileOf(sq2)),
                                                       distance(rankOf(sq1), rankOf(sq2)));
      }
    }

  }

  const Bitboard shift(Direction d, Bitboard b) {
    // move the bits and clear the left our right file
    // after the shift to erase bit jumping over
    switch (d) {
      case NORTH:
        return (b << 8);
      case EAST:
        return (b << 1) & ~FileABB;
      case SOUTH:
        return (b >> 8);
      case WEST:
        return (b >> 1) & ~FileHBB;
      case NORTH_EAST:
        return (b << 9);
      case SOUTH_EAST:
        return (b >> 7);
      case SOUTH_WEST:
        return (b >> 9);
      case NORTH_WEST:
        return (b << 7);
    }
    assert(false);
    return b;
  }
}
