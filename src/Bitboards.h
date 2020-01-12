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

  extern Bitboard sqToFileBB[SQ_LENGTH];
  extern Bitboard sqToRankBB[SQ_LENGTH];
  extern Bitboard filesWestMask[SQ_LENGTH];
  extern Bitboard filesEastMask[SQ_LENGTH];
  extern Bitboard fileWestMask[SQ_LENGTH];
  extern Bitboard fileEastMask[SQ_LENGTH];
  extern Bitboard neighbourFilesMask[SQ_LENGTH];
  extern Bitboard ranksNorthMask[SQ_LENGTH];
  extern Bitboard ranksSouthMask[SQ_LENGTH];

  extern Bitboard rays[OR_LENGTH][SQ_LENGTH];

  extern Bitboard passedPawnMask[COLOR_LENGTH][SQ_LENGTH];

  extern Bitboard kingSideCastleMask[COLOR_LENGTH];
  extern Bitboard queenSideCastleMask[COLOR_LENGTH];

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

  constexpr Square rotateSquareR90(Square sq) { return indexMapR90[sq]; }
  constexpr Square rotateSquareL90(Square sq) { return indexMapL90[sq]; }
  constexpr Square rotateSquareR45 (Square sq) { return indexMapR45[sq]; }
  constexpr Square rotateSquareL45(Square sq) { return indexMapL45[sq]; }

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

  // Get bitboards for ranks or files
  constexpr Bitboard rankBB(int r) { return Rank1BB << (8 * r); }

  constexpr Bitboard rankBB(Rank r) { return Rank1BB << (8 * r); }

  // use pre-computed sqToRankBB[] array instead
  constexpr Bitboard rankBB(Square s) { return rankBB(rankOf(s)); }

  constexpr Bitboard fileBB(int f) { return FileABB << f; }

  constexpr Bitboard fileBB(File f) { return FileABB << f; }

  // use pre-computed sqToFileBB[] array instead
  constexpr Bitboard fileBB(Square s) { return fileBB(fileOf(s)); }

  /** popcount() counts the number of non-zero bits in a bitboard */
  constexpr int popcount(Bitboard b) {
#if defined(__GNUC__) // GCC, Clang, ICC
    return __builtin_popcountll(b);
#else // Compiler is not GCC
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

  /**
   * Used when no build in popcount is available for compiler.
   * @return popcount16() counts the non-zero bits using SWAR-Popcount algorithm
   */
  constexpr unsigned popcount16(unsigned u) {
    u -= (u >> 1U) & 0x5555U;
    u = ((u >> 2U) & 0x3333U) + (u & 0x3333U);
    u = ((u >> 4U) + u) & 0x0F0FU;
    return (u * 0x0101U) >> 8U;
  }

  /** lsb() and msb() return the least/most significant bit in a non-zero
   * bitboard */
  constexpr Square lsb(Bitboard b) {
    assert(b);        // __builtin_ctzll is undefined on 0
#if defined(__GNUC__) // GCC, Clang, ICC
    return Square(__builtin_ctzll(b));
#else // Compiler is not GCC
#error "Compiler not yet supported."
    // GTEST - nice examples
    // Windows:  _MSC_VER
    // _BitScanForward64/_BitScanReverse64
    // https://docs.microsoft.com/en-us/cpp/intrinsics/bitscanreverse-bitscanreverse64?view=vs-2019
#endif
  }

  /** lsb() and msb() return the least/most significant bit in a non-zero
   * bitboard */
  constexpr Square msb(Bitboard b) {
    assert(b);        // __builtin_clzll is undefined on 0
#if defined(__GNUC__) // GCC, Clang, ICC
    return Square(63 ^ __builtin_clzll(b));
#else // Compiler is not GCC
#error "Compiler not yet supported."
#endif
  }

  /** pop_lsb() finds and clears the least significant bit in a non-zero
   * bitboard */
  constexpr Square popLSB(Bitboard &b) {
    assert(b); // lsb is undefined on 0
    const Square s = lsb(b);
    b &= b - 1;
    return s;
  }
  constexpr void popLSB2(Bitboard &b, Square &sq) {
    assert(b); // lsb is undefined on 0
    sq = lsb(b);
    b &= b - 1;
  }

  /**
   * Shifts a bitboard in the given direction.
   * @param d Direction
   * @param b Bitboard
   * @return shifted bitboard
   */
  constexpr Bitboard shift(Direction d, Bitboard b) {
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
      return (b << 9) & ~FileABB;
    case SOUTH_EAST:
      return (b >> 7) & ~FileABB;
    case SOUTH_WEST:
      return (b >> 9) & ~FileHBB;
    case NORTH_WEST:
      return (b << 7) & ~FileHBB;
    }
  }

  /**
   * Rotates a Bitboard
   * @param b
   * @param rotMap - array with mapping for rotation
   * @return rotated bitboard
   */
  constexpr Bitboard rotate(Bitboard b, const int *rotMap) {
    Bitboard rotated = EMPTY_BB;
    for (Square sq = SQ_A1; sq < SQ_LENGTH; ++sq) {
      if ((b & Square(rotMap[sq])) != 0)
        rotated |= squareBB[sq];
    }
    return rotated;
  }

  /**
   * Rotates a Bitboard
   * @param b
   * @return rotated bitboard
   */
  constexpr Bitboard rotateR90(Bitboard b) { return rotate(b, rotateMapR90); }

  /**
   * Rotates a Bitboard
   * @param b
   * @return rotated bitboard
   */
  constexpr Bitboard rotateL90(Bitboard b) { return rotate(b, rotateMapL90); }

  /**
   * Rotates a Bitboard
   * @param b
   * @return rotated bitboard
   */
  constexpr Bitboard rotateR45(Bitboard b) { return rotate(b, rotateMapR45); }

  /**
   * Rotates a Bitboard
   * @param b
   * @return rotated bitboard
   */
  constexpr Bitboard rotateL45(Bitboard b) { return rotate(b, rotateMapL45); }

  /**
   * Bitboard for all possible horizontal moves on the rank of the square with
   * the rank content (blocking pieces) determined from the given pieces
   * bitboard.
   */
  constexpr Bitboard getMovesRank(Square sq, Bitboard content) {
    // content = the pieces currently on the board and maybe blocking the moves
    // no rotation necessary for ranks - their squares are already in a row
    // shift to the least significant bit
    const Bitboard contentIdx = content >> 8 * rankOf(sq);
    // retrieve all possible moves for this square with the current content
    // and mask with the first row to erase any other pieces
    return movesRank[sq][contentIdx & 255];
  }

  /**
   * Bitboard for all possible horizontal moves on the rank of the square with
   * the rank content (blocking pieces) determined from the given L90 rotated
   * bitboard.
   */
  constexpr Bitboard getMovesFileR(Square sq, Bitboard rotated) {
    // shift to the first byte (to the right in Java)
    const Bitboard contentIdx = rotated >> fileOf(sq) * 8;
    // retrieve all possible moves for this square with the current content
    // and mask with the first row to erase any other pieces not erased by shift
    return movesFile[sq][contentIdx & 255];
  }

  /**
   * Bitboard for all possible horizontal moves on the rank of the square with
   * the rank content (blocking pieces) determined from the given non-rotated
   * bitboard.
   */
  constexpr Bitboard getMovesFile(Square sq, Bitboard content) {
    // content = the pieces currently on the board and maybe blocking the moves
    // rotate the content of the board to get all file squares in a row
    return getMovesFileR(sq, rotateL90(content));
  }

  /**
   * Bitboard for all possible diagonal up moves of the square with
   * the content (blocking pieces) determined from the given R45 rotated
   * bitboard.
   */
  constexpr Bitboard getMovesDiagUpR(Square sq, Bitboard rotated) {
    // shift the correct row to the first byte (to the right in Java)
    const Bitboard shifted = rotated >> shiftsDiagUp[sq];
    // mask the content with the length of the diagonal to erase any other
    // pieces which have not been erased by the shift
    const Bitboard contentMasked = shifted & lengthDiagUpMask(sq);
    // retrieve all possible moves for this square with the current content
    return movesDiagUp[sq][contentMasked];
  }

  /**
   * Bitboard for all possible diagonal up moves of the square with
   * the content (blocking pieces) determined from the given non rotated
   * bitboard.
   */
  constexpr Bitboard getMovesDiagUp(Square square, Bitboard content) {
    // content = the pieces currently on the board and maybe blocking the moves
    // rotate the content of the board to get all diagonals in a row
    return getMovesDiagUpR(square, rotateR45(content));
  }

  /**
   * Bitboard for all possible diagonal up moves of the square with
   * the content (blocking pieces) determined from the given L45 rotated
   * bitboard.
   */
  constexpr Bitboard getMovesDiagDownR(Square sq, Bitboard rotated) {
    // shift the correct row to the first byte (to the right in Java)
    const Bitboard shifted = rotated >> shiftsDiagDown[sq];
    // mask the content with the length of the diagonal to erase any other
    // pieces which have not been erased by the shift
    const Bitboard contentMasked = shifted & lengthDiagDownMask(sq);
    // retrieve all possible moves for this square with the current content
    return movesDiagDown[sq][contentMasked];
  }

  /**
   * Bitboard for all possible diagonal up moves of the square with
   * the content (blocking pieces) determined from the given non rotated
   * bitboard.
   */
  constexpr Bitboard getMovesDiagDown(Square square, Bitboard content) {
    // content = the pieces currently on the board and maybe blocking the moves
    // rotate the content of the board to get all diagonals in a row
    return getMovesDiagDownR(square, rotateL45(content));
  }

  /**
   * Prints a bitboard a an 8x8 matrix for output on a console
   */
  inline std::string print(Bitboard b) {
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
  inline std::string printFlat(Bitboard b) {
    std::string s;
    for (int i = 0; i < 64; i++) {
      if (i > 0 && i % 8 == 0) s += ".";
      s += b & (1L << i) ? "1" : "0";
    }
    s += " (" + std::to_string(b) + ")";
    return s;
  }

} // namespace Bitboards

//// Operators for Squares as Bitboards
constexpr Bitboard operator&(const Square lhs, const Square rhs) {
  return Bitboards::squareBB[lhs] & Bitboards::squareBB[rhs];
}

constexpr Bitboard operator|(const Square lhs, const Square rhs) {
  return Bitboards::squareBB[lhs] | Bitboards::squareBB[rhs];
}

//// Operators for testing of Bitboards and Squares
constexpr Bitboard operator&(const Bitboard b, const Square s) {
  assert(s >= SQ_A1 && s <= SQ_H8);
  return b & Bitboards::squareBB[s];
}

constexpr Bitboard operator|(const Bitboard b, const Square s) {
  assert(s >= SQ_A1 && s <= SQ_H8);
  return b | Bitboards::squareBB[s];
}

constexpr Bitboard operator^(const Bitboard b, const Square s) {
  assert(s >= SQ_A1 && s <= SQ_H8);
  return b ^ Bitboards::squareBB[s];
}

constexpr Bitboard &operator|=(Bitboard &b, const Square s) {
  assert(s >= SQ_A1 && s <= SQ_H8);
  return b |= Bitboards::squareBB[s];
}

constexpr Bitboard &operator^=(Bitboard &b, const Square s) {
  assert(s >= SQ_A1 && s <= SQ_H8);
  return b ^= Bitboards::squareBB[s];
}

#endif //FRANKYCPP_BITBOARDS_H
