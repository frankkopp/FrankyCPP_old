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

#ifndef FRANKYCPP_BITBOARDS_H
#define FRANKYCPP_BITBOARDS_H

#include "globals.h"

namespace Bitboards {
  const void init();
  const std::string print(Bitboard b);
  const std::string printFlat(Bitboard b);
  const Bitboard shift(Direction d, Bitboard b);
}

constexpr Bitboard EMPTY_BB = Bitboard(0);
constexpr Bitboard ALL_BB = ~EMPTY_BB;
constexpr Bitboard ONE_BB = Bitboard(1);

constexpr Bitboard FileABB = 0x0101010101010101ULL;
constexpr Bitboard FileBBB = FileABB << 1;
constexpr Bitboard FileCBB = FileABB << 2;
constexpr Bitboard FileDBB = FileABB << 3;
constexpr Bitboard FileEBB = FileABB << 4;
constexpr Bitboard FileFBB = FileABB << 5;
constexpr Bitboard FileGBB = FileABB << 6;
constexpr Bitboard FileHBB = FileABB << 7;

constexpr Bitboard Rank1BB = 0xFF;
constexpr Bitboard Rank2BB = Rank1BB << (8 * 1);
constexpr Bitboard Rank3BB = Rank1BB << (8 * 2);
constexpr Bitboard Rank4BB = Rank1BB << (8 * 3);
constexpr Bitboard Rank5BB = Rank1BB << (8 * 4);
constexpr Bitboard Rank6BB = Rank1BB << (8 * 5);
constexpr Bitboard Rank7BB = Rank1BB << (8 * 6);
constexpr Bitboard Rank8BB = Rank1BB << (8 * 7);

static const Bitboard DiagUpA1 = 0b\
10000000\
01000000\
00100000\
00010000\
00001000\
00000100\
00000010\
00000001;
static const Bitboard DiagUpB1 = Bitboards::shift(EAST, DiagUpA1);
static const Bitboard DiagUpC1 = Bitboards::shift(EAST, DiagUpB1);
static const Bitboard DiagUpD1 = Bitboards::shift(EAST, DiagUpC1);
static const Bitboard DiagUpE1 = Bitboards::shift(EAST, DiagUpD1);
static const Bitboard DiagUpF1 = Bitboards::shift(EAST, DiagUpE1);
static const Bitboard DiagUpG1 = Bitboards::shift(EAST, DiagUpF1);
static const Bitboard DiagUpH1 = Bitboards::shift(EAST, DiagUpG1);
static const Bitboard DiagUpA2 = Bitboards::shift(NORTH, DiagUpA1);
static const Bitboard DiagUpA3 = Bitboards::shift(NORTH, DiagUpA2);
static const Bitboard DiagUpA4 = Bitboards::shift(NORTH, DiagUpA3);
static const Bitboard DiagUpA5 = Bitboards::shift(NORTH, DiagUpA4);
static const Bitboard DiagUpA6 = Bitboards::shift(NORTH, DiagUpA5);
static const Bitboard DiagUpA7 = Bitboards::shift(NORTH, DiagUpA6);
static const Bitboard DiagUpA8 = Bitboards::shift(NORTH, DiagUpA7);

static const Bitboard DiagDownH1 = 0b\
00000001\
00000010\
00000100\
00001000\
00010000\
00100000\
01000000\
10000000;
static const Bitboard DiagDownH2 = Bitboards::shift(NORTH, DiagDownH1);
static const Bitboard DiagDownH3 = Bitboards::shift(NORTH, DiagDownH2);
static const Bitboard DiagDownH4 = Bitboards::shift(NORTH, DiagDownH3);
static const Bitboard DiagDownH5 = Bitboards::shift(NORTH, DiagDownH4);
static const Bitboard DiagDownH6 = Bitboards::shift(NORTH, DiagDownH5);
static const Bitboard DiagDownH7 = Bitboards::shift(NORTH, DiagDownH6);
static const Bitboard DiagDownH8 = Bitboards::shift(NORTH, DiagDownH7);
static const Bitboard DiagDownG1 = Bitboards::shift(WEST, DiagDownH1);
static const Bitboard DiagDownF1 = Bitboards::shift(WEST, DiagDownG1);
static const Bitboard DiagDownE1 = Bitboards::shift(WEST, DiagDownF1);
static const Bitboard DiagDownD1 = Bitboards::shift(WEST, DiagDownE1);
static const Bitboard DiagDownC1 = Bitboards::shift(WEST, DiagDownD1);
static const Bitboard DiagDownB1 = Bitboards::shift(WEST, DiagDownC1);
static const Bitboard DiagDownA1 = Bitboards::shift(WEST, DiagDownB1);

extern Bitboard squareBB[SQ_NONE];
extern Bitboard squareDiagUp[SQ_NONE];
extern Bitboard squareDiagDown[SQ_NONE];

//// Operators for testing of Bitboards and Squares

inline Bitboard operator&(Bitboard b, Square s) {
  assert(s >= SQ_A1 && s <= SQ_H8);
  return b & squareBB[s];
}

inline Bitboard operator|(Bitboard b, Square s) {
  assert(s >= SQ_A1 && s <= SQ_H8);
  return b | squareBB[s];
}

inline Bitboard operator^(Bitboard b, Square s) {
  assert(s >= SQ_A1 && s <= SQ_H8);
  return b ^ squareBB[s];
}

inline Bitboard &operator|=(Bitboard &b, Square s) {
  assert(s >= SQ_A1 && s <= SQ_H8);
  return b |= squareBB[s];
}

inline Bitboard &operator^=(Bitboard &b, Square s) {
  assert(s >= SQ_A1 && s <= SQ_H8);
  return b ^= squareBB[s];
}

//// Get bitboards for ranks or files

inline Bitboard rankBB(Rank r) {
  return Rank1BB << (8 * r);
}

inline Bitboard rankBB(Square s) {
  return rankBB(rankOf(s));
}

inline Bitboard fileBB(File f) {
  return FileABB << f;
}

inline Bitboard fileBB(Square s) {
  return fileBB(fileOf(s));
}

/// popcount() counts the number of non-zero bits in a bitboard
inline int popcount(Bitboard b) {
  extern uint8_t PopCnt16[1 << 16];
  union { Bitboard bb; uint16_t u[4]; } v = { b };
  return PopCnt16[v.u[0]] + PopCnt16[v.u[1]] + PopCnt16[v.u[2]] + PopCnt16[v.u[3]];
}

/// lsb() and msb() return the least/most significant bit in a non-zero bitboard
inline Square lsb(Bitboard b) {
  assert(b);
  return Square(__builtin_ctzll(b));
}

/// lsb() and msb() return the least/most significant bit in a non-zero bitboard
inline Square msb(Bitboard b) {
  assert(b);
  return Square(63 ^ __builtin_clzll(b));
}

/// pop_lsb() finds and clears the least significant bit in a non-zero bitboard
 inline Square pop_lsb(Bitboard* b) {
  const Square s = lsb(*b);
  *b &= *b - 1;
  return s;
}


#endif //FRANKYCPP_BITBOARDS_H
