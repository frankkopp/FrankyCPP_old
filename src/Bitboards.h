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

#include "types.h"

namespace Bitboards {
  void init();

  std::string print(Bitboard b);
  std::string printFlat(Bitboard b);

  Bitboard shift(Direction d, Bitboard b);
  Bitboard rotateR90(Bitboard b);
  Bitboard rotateL90(Bitboard b);
  Bitboard rotateR45(Bitboard b);
  Bitboard rotateL45(Bitboard b);
  Bitboard rotate(Bitboard b, const int rotMap[SQ_LENGTH]);

  Bitboard getMovesRank(Square sq, Bitboard content);
  Bitboard getMovesFile(Square square, Bitboard content);
  Bitboard getMovesFileR(Square square, Bitboard rotated);
  Bitboard getMovesDiagUp(Square square, Bitboard content);
  Bitboard getMovesDiagUpR(Square sq, Bitboard rotated);
  Bitboard getMovesDiagDown(Square square, Bitboard content);
  Bitboard getMovesDiagDownR(Square square, Bitboard rotated);

  constexpr Bitboard EMPTY_BB = Bitboard(0U);
  constexpr Bitboard ALL_BB = ~EMPTY_BB;
  constexpr Bitboard ONE_BB = Bitboard(1U);

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

  constexpr Bitboard CastlingMask = (ONE_BB << SQ_E1)
                                      | (ONE_BB << SQ_A1)
                                      | (ONE_BB << SQ_H1)
                                      | (ONE_BB << SQ_E8)
                                      | (ONE_BB << SQ_A8)
                                      | (ONE_BB << SQ_H8);

  constexpr Bitboard promotionRank[COLOR_LENGTH] = { Rank8BB, Rank1BB };

  // pre-computed in Bitboards::init()
  extern Bitboard squareBB[SQ_LENGTH];
  extern Bitboard squareDiagUpBB[SQ_LENGTH];
  extern Bitboard squareDiagDownBB[SQ_LENGTH];
  extern Bitboard movesRank[SQ_LENGTH][256];
  extern Bitboard movesFile[SQ_LENGTH][256];
  extern Bitboard movesDiagUp[SQ_LENGTH][256];
  extern Bitboard movesDiagDown[SQ_LENGTH][256];
  extern Square indexMapR90[SQ_LENGTH];
  extern Square indexMapL90[SQ_LENGTH];
  extern Square indexMapR45[SQ_LENGTH];
  extern Square indexMapL45[SQ_LENGTH];

  extern Bitboard pawnAttacks[COLOR_LENGTH][SQ_LENGTH];
  extern Bitboard pawnMoves[COLOR_LENGTH][SQ_LENGTH];
  extern Bitboard pseudoAttacks[PT_LENGTH][SQ_LENGTH];

  extern Bitboard filesWestMask[SQ_LENGTH];
  extern Bitboard filesEastMask[SQ_LENGTH];
  extern Bitboard fileWestMask[SQ_LENGTH];
  extern Bitboard fileEastMask[SQ_LENGTH];
  extern Bitboard ranksNorthMask[SQ_LENGTH];
  extern Bitboard ranksSouthMask[SQ_LENGTH];

  extern Bitboard rays[OR_LENGTH][SQ_LENGTH];

  extern Bitboard passedPawnMask[COLOR_LENGTH][SQ_LENGTH];

  extern Bitboard whiteSquaresBB;
  extern Bitboard blackSquaresBB;

  extern Bitboard intermediateBB[SQ_LENGTH][SQ_LENGTH];

  extern int squareDistance[SQ_NONE][SQ_NONE];
  inline int distance(File f1, File f2) { return abs(f2 - f1); }
  inline int distance(Rank r1, Rank r2) { return abs(r2 - r1); }
  inline int distance(Square s1, Square s2) { return squareDistance[s1][s2]; }
  extern int centerDistance[SQ_LENGTH];

// @formatter:off

  constexpr int rotateMapR90[SQ_LENGTH] = {
    7, 15, 23, 31, 39, 47, 55, 63,
    6, 14, 22, 30, 38, 46, 54, 62,
    5, 13, 21, 29, 37, 45, 53, 61,
    4, 12, 20, 28, 36, 44, 52, 60,
    3, 11, 19, 27, 35, 43, 51, 59,
    2, 10, 18, 26, 34, 42, 50, 58,
    1, 9, 17, 25, 33, 41, 49, 57,
    0, 8, 16, 24, 32, 40, 48, 56
  };

  constexpr int rotateMapL90[SQ_LENGTH] = {
    56,	48,	40,	32,	24,	16,	8	, 0,
    57,	49,	41,	33,	25,	17,	9	, 1,
    58,	50,	42,	34,	26,	18,	10,	2,
    59,	51,	43,	35,	27,	19,	11,	3,
    60,	52,	44,	36,	28,	20,	12,	4,
    61,	53,	45,	37,	29,	21,	13,	5,
    62,	54,	46,	38,	30,	22,	14,	6,
    63,	55,	47,	39,	31,	23,	15,	7
  };

  constexpr int rotateMapR45[SQ_LENGTH] = {
     7,
     6,	15,
     5,	14,	23,
     4,	13,	22,	31,
     3,	12,	21,	30,	39,
     2,	11,	20,	29,	38,	47,
     1,	10,	19,	28,	37,	46,	55,
     0,	 9,	18,	27,	36,	45,	54,	63,
     8,	17,	26,	35,	44,	53,	62,
    16,	25,	34,	43,	52,	61,
    24,	33,	42,	51,	60,
    32,	41,	50,	59,
    40,	49,	58,
    48,	57,
    56
  };

  constexpr int rotateMapL45[SQ_LENGTH] = {
     0,
     8,	 1,
    16,	 9,	 2,
    24,	17,	10,	 3,
    32,	25,	18,	11,	 4,
    40,	33,	26,	19,	12,	 5,
    48,	41,	34,	27,	20,	13,	 6,
    56,	49,	42,	35,	28,	21,	14,	7,
    57,	50,	43,	36,	29,	22,	15,
    58,	51,	44,	37,	30,	23,
    59,	52,	45,	38,	31,
    60,	53,	46,	39,
    61,	54,	47,
    62,	55,
    63
  };

  inline Square rotateSquareR90(Square sq) { return indexMapR90[sq]; }
  inline Square rotateSquareL90(Square sq) { return indexMapL90[sq]; }
  inline Square rotateSquareR45 (Square sq) { return indexMapR45[sq]; }
  inline Square rotateSquareL45(Square sq) { return indexMapL45[sq]; }

  constexpr int lengthDiagUp[SQ_LENGTH] = {
    8, 7, 6, 5, 4, 3, 2, 1,
    7, 8, 7, 6, 5, 4, 3, 2,
    6, 7, 8, 7, 6, 5, 4, 3,
    5, 6, 7, 8, 7, 6, 5, 4,
    4, 5, 6, 7, 8, 7, 6, 5,
    3, 4, 5, 6, 7, 8, 7, 6,
    2, 3, 4, 5, 6, 7, 8, 7,
    1, 2, 3, 4, 5, 6, 7, 8
  };
  constexpr Bitboard lengthDiagUpMask(Square sq) {
    return (ONE_BB << lengthDiagUp[sq]) - 1;
  }

  constexpr int lengthDiagDown[SQ_LENGTH] = {
    1, 2, 3, 4, 5, 6, 7, 8,
    2, 3, 4, 5, 6, 7, 8, 7,
    3, 4, 5, 6, 7, 8, 7, 6,
    4, 5, 6, 7, 8, 7, 6, 5,
    5, 6, 7, 8, 7, 6, 5, 4,
    6, 7, 8, 7, 6, 5, 4, 3,
    7, 8, 7, 6, 5, 4, 3, 2,
    8, 7, 6, 5, 4, 3, 2, 1
  };

  constexpr Bitboard lengthDiagDownMask(Square sq) {
    return (ONE_BB << lengthDiagDown[sq]) - 1;
  }

  constexpr int shiftsDiagUp[SQ_LENGTH] = {
    28, 21, 15, 10,  6,  3,  1,  0,
    36, 28, 21, 15, 10,  6,  3,  1,
    43, 36, 28, 21, 15, 10,  6,  3,
    49, 43, 36, 28, 21, 15, 10,  6,
    54, 49, 43, 36, 28, 21, 15, 10,
    58, 54, 49, 43, 36, 28, 21, 15,
    61, 58, 54, 49, 43, 36, 28, 21,
    63, 61, 58, 54, 49, 43, 36, 28
  };

  constexpr int shiftsDiagDown[SQ_LENGTH] = {
     0,  1,  3,  6, 10, 15, 21, 28,
     1,  3,  6, 10, 15, 21, 28, 36,
     3,  6, 10, 15, 21, 28, 36, 43,
     6, 10, 15, 21, 28, 36, 43, 49,
    10, 15, 21, 28, 36, 43, 49, 54,
    15, 21, 28, 36, 43, 49, 54, 58,
    21, 28, 36, 43, 49, 54, 58, 61,
    28, 36, 43, 49, 54, 58, 61, 63
  };

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
// @formatter:on

  //// Get bitboards for ranks or files

  inline Bitboard rankBB(int r) {
    return Rank1BB << (8 * r);
  }

  inline Bitboard rankBB(Rank r) {
    return Rank1BB << (8 * r);
  }

  inline Bitboard rankBB(Square s) {
    return rankBB(rankOf(s));
  }

  inline Bitboard fileBB(int f) {
    return FileABB << f;
  }

  inline Bitboard fileBB(File f) {
    return FileABB << f;
  }

  inline Bitboard fileBB(Square s) {
    return fileBB(fileOf(s));
  }

  /// popcount() counts the number of non-zero bits in a bitboard
  inline int popcount(Bitboard b) {
#if defined(__GNUC__)  // GCC, Clang, ICC
    return __builtin_popcountll(b);
#else  // Compiler is not GCC
    // pre-computed table of population counter for 16-bit
    extern uint8_t PopCnt16[1 << 16];
    // nice trick to address 16-bit groups in a 64-bit int
    // @formatter:off
    union { Bitboard bb; uint16_t u[4]; } v = {b};
    // @formatter:on
    // adding all 16-bit population counters referenced in the 64-bit union
    return PopCnt16[v.u[0]] + PopCnt16[v.u[1]] + PopCnt16[v.u[2]] + PopCnt16[v.u[3]];
#endif
  }

/// lsb() and msb() return the least/most significant bit in a non-zero bitboard
  inline Square lsb(Bitboard b) {
    assert(b);
#if defined(__GNUC__)  // GCC, Clang, ICC
    return Square(__builtin_ctzll(b));
#else  // Compiler is not GCC
#error "Compiler not yet supported."
#endif
  }

/// lsb() and msb() return the least/most significant bit in a non-zero bitboard
  inline Square msb(Bitboard b) {
    assert(b);
#if defined(__GNUC__)  // GCC, Clang, ICC
    return Square(63 ^ __builtin_clzll(b));
#else  // Compiler is not GCC
#error "Compiler not yet supported."
#endif
  }

/// pop_lsb() finds and clears the least significant bit in a non-zero bitboard
  inline Square popLSB(Bitboard *b) {
    assert (*b);
    const Square s = lsb(*b);
    *b &= *b - 1;
    return s;
  }

}

//// Operators for testing of Bitboards and Squares

inline Bitboard operator&(Bitboard b, Square s) {
  assert(s >= SQ_A1 && s <= SQ_H8);
  return b & Bitboards::squareBB[s];
}

inline Bitboard operator|(Bitboard b, Square s) {
  assert(s >= SQ_A1 && s <= SQ_H8);
  return b | Bitboards::squareBB[s];
}

inline Bitboard operator^(Bitboard b, Square s) {
  assert(s >= SQ_A1 && s <= SQ_H8);
  return b ^ Bitboards::squareBB[s];
}

inline Bitboard &operator|=(Bitboard &b, Square s) {
  assert(s >= SQ_A1 && s <= SQ_H8);
  return b |= Bitboards::squareBB[s];
}

inline Bitboard &operator^=(Bitboard &b, Square s) {
  assert(s >= SQ_A1 && s <= SQ_H8);
  return b ^= Bitboards::squareBB[s];
}

#endif //FRANKYCPP_BITBOARDS_H
