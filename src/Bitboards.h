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

constexpr Bitboard DiagUpA1 = 0b\
10000000\
01000000\
00100000\
00010000\
00001000\
00000100\
00000010\
00000001;
constexpr Bitboard DiagUpB1 = (DiagUpA1 << 1) & ~FileABB; // shift EAST
constexpr Bitboard DiagUpC1 = (DiagUpB1 << 1) & ~FileABB;
constexpr Bitboard DiagUpD1 = (DiagUpC1 << 1) & ~FileABB;
constexpr Bitboard DiagUpE1 = (DiagUpD1 << 1) & ~FileABB;
constexpr Bitboard DiagUpF1 = (DiagUpE1 << 1) & ~FileABB;
constexpr Bitboard DiagUpG1 = (DiagUpF1 << 1) & ~FileABB;
constexpr Bitboard DiagUpH1 = (DiagUpG1 << 1) & ~FileABB;
constexpr Bitboard DiagUpA2 = DiagUpA1 << 8; // shift NORTH
constexpr Bitboard DiagUpA3 = DiagUpA2 << 8;
constexpr Bitboard DiagUpA4 = DiagUpA3 << 8;
constexpr Bitboard DiagUpA5 = DiagUpA4 << 8;
constexpr Bitboard DiagUpA6 = DiagUpA5 << 8;
constexpr Bitboard DiagUpA7 = DiagUpA6 << 8;
constexpr Bitboard DiagUpA8 = DiagUpA7 << 8;

constexpr Bitboard DiagDownH1 = 0b\
00000001\
00000010\
00000100\
00001000\
00010000\
00100000\
01000000\
10000000;
constexpr Bitboard DiagDownH2 = DiagDownH1 << 8; // shift NORTH
constexpr Bitboard DiagDownH3 = DiagDownH2 << 8;
constexpr Bitboard DiagDownH4 = DiagDownH3 << 8;
constexpr Bitboard DiagDownH5 = DiagDownH4 << 8;
constexpr Bitboard DiagDownH6 = DiagDownH5 << 8;
constexpr Bitboard DiagDownH7 = DiagDownH6 << 8;
constexpr Bitboard DiagDownH8 = DiagDownH7 << 8;
constexpr Bitboard DiagDownG1 = (DiagDownH1 >> 1) & ~FileHBB; // shift WEST
constexpr Bitboard DiagDownF1 = (DiagDownG1 >> 1) & ~FileHBB;
constexpr Bitboard DiagDownE1 = (DiagDownF1 >> 1) & ~FileHBB;
constexpr Bitboard DiagDownD1 = (DiagDownE1 >> 1) & ~FileHBB;
constexpr Bitboard DiagDownC1 = (DiagDownD1 >> 1) & ~FileHBB;
constexpr Bitboard DiagDownB1 = (DiagDownC1 >> 1) & ~FileHBB;
constexpr Bitboard DiagDownA1 = (DiagDownB1 >> 1) & ~FileHBB;

extern Bitboard squareBB[SQ_LENGTH];
extern Bitboard squareDiagUp[SQ_LENGTH];
extern Bitboard squareDiagDown[SQ_LENGTH];

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

inline int popcount2(Bitboard b) {
#if defined(__GNUC__)
  return __builtin_popcountll(b);
#else
  extern uint8_t PopCnt16[1 << 16];
  union { Bitboard bb; uint16_t u[4]; } v = { b };
  return PopCnt16[v.u[0]] + PopCnt16[v.u[1]] + PopCnt16[v.u[2]] + PopCnt16[v.u[3]];
#endif
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
