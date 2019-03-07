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

/*
data types:
Color
Square
Bitboard
Move
MoveType
Piece
PieceType

Classes:
Engine
Position
TT
EvalCache
Search
Evaluation
MoveGen

UCIHandler
UCIOption
UCISearchMode
 */

#ifndef FRANKYCPP_GLOBALS_H
#define FRANKYCPP_GLOBALS_H

#include <stdint.h>
#include <assert.h>

// Global constants
constexpr int MAX_MOVES = 256;
constexpr int MAX_PLY = 128;

// Color type
enum Color {
  WHITE, BLACK, NOCOLOR
};

constexpr Color operator~(Color c) { return Color(c ^ BLACK); };

/** Bitboard */
typedef uint64_t Bitboard;

/** Squares */
// @formatter:off
enum Square : int {
  SQ_A1, SQ_B1, SQ_C1, SQ_D1, SQ_E1, SQ_F1, SQ_G1, SQ_H1,
  SQ_A2, SQ_B2, SQ_C2, SQ_D2, SQ_E2, SQ_F2, SQ_G2, SQ_H2,
  SQ_A3, SQ_B3, SQ_C3, SQ_D3, SQ_E3, SQ_F3, SQ_G3, SQ_H3,
  SQ_A4, SQ_B4, SQ_C4, SQ_D4, SQ_E4, SQ_F4, SQ_G4, SQ_H4,
  SQ_A5, SQ_B5, SQ_C5, SQ_D5, SQ_E5, SQ_F5, SQ_G5, SQ_H5,
  SQ_A6, SQ_B6, SQ_C6, SQ_D6, SQ_E6, SQ_F6, SQ_G6, SQ_H6,
  SQ_A7, SQ_B7, SQ_C7, SQ_D7, SQ_E7, SQ_F7, SQ_G7, SQ_H7,
  SQ_A8, SQ_B8, SQ_C8, SQ_D8, SQ_E8, SQ_F8, SQ_G8, SQ_H8,
  SQ_NONE
};
// @formatter:on

/** Files */
enum File : int {
  FILE_A, FILE_B, FILE_C, FILE_D, FILE_E, FILE_F, FILE_G, FILE_H, FILE_NONE
};

/** Ranks */
enum Rank : int {
  RANK_1, RANK_2, RANK_3, RANK_4, RANK_5, RANK_6, RANK_7, RANK_8, RANK_NONE
};

/** Direction */
enum Direction : int {
  NORTH =  8,
  EAST  =  1,
  SOUTH = -NORTH,
  WEST  = -EAST,

  NORTH_EAST = NORTH + EAST,
  SOUTH_EAST = SOUTH + EAST,
  SOUTH_WEST = SOUTH + WEST,
  NORTH_WEST = NORTH + WEST
};

/** PieceTypes */
enum PieceType : int {
  // non-sliding ---- sliding -----------
    KING, PAWN, KNIGHT, BISHOP, ROOK, QUEEN, PIECETYPE_NONE
};

const static int pieceValue[] = {
  2000, // king
  100,  // pawn
  320,  // knight
  330,  // bishop
  500,  // rook
  900,  // queen
  0     // notype
};

/** Pieces */
enum Piece : int {
  // 0x0 - 0x5
    WHITE_KING = 0, WHITE_PAWN, WHITE_KNIGHT, WHITE_BISHOP, WHITE_ROOK, WHITE_QUEEN,
  // 0x8 - 0XD
    BLACK_KING = 8, BLACK_PAWN, BLACK_KNIGHT, BLACK_BISHOP, BLACK_ROOK, BLACK_QUEEN,
  // 0x10
    PIECE_NONE = 16
};

/**
 * OPERATORS
 */
#define ENABLE_BASE_OPERATORS_ON(T)                                \
constexpr T operator+(T d1, T d2) { return T(int(d1) + int(d2)); } \
constexpr T operator-(T d1, T d2) { return T(int(d1) - int(d2)); } \
constexpr T operator-(T d) { return T(-int(d)); }                  \
inline T& operator+=(T& d1, T d2) { return d1 = d1 + d2; }         \
inline T& operator-=(T& d1, T d2) { return d1 = d1 - d2; }

#define ENABLE_INCR_OPERATORS_ON(T)                                \
inline T& operator++(T& d) { return d = T(int(d) + 1); }           \
inline T& operator--(T& d) { return d = T(int(d) - 1); }

#define ENABLE_FULL_OPERATORS_ON(T)                                \
ENABLE_BASE_OPERATORS_ON(T)                                        \
ENABLE_INCR_OPERATORS_ON(T)                                        \
constexpr T operator*(int i, T d) { return T(i * int(d)); }        \
constexpr T operator*(T d, int i) { return T(int(d) * i); }        \
constexpr T operator/(T d, int i) { return T(int(d) / i); }        \
constexpr int operator/(T d1, T d2) { return int(d1) / int(d2); }  \
inline T& operator*=(T& d, int i) { return d = T(int(d) * i); }    \
inline T& operator/=(T& d, int i) { return d = T(int(d) / i); }

//ENABLE_FULL_OPERATORS_ON(Value)
//ENABLE_FULL_OPERATORS_ON(Depth)
ENABLE_FULL_OPERATORS_ON(Direction)

ENABLE_INCR_OPERATORS_ON(PieceType)
ENABLE_INCR_OPERATORS_ON(Piece)
ENABLE_INCR_OPERATORS_ON(Color)
ENABLE_INCR_OPERATORS_ON(Square)
ENABLE_INCR_OPERATORS_ON(File)
ENABLE_INCR_OPERATORS_ON(Rank)

//ENABLE_BASE_OPERATORS_ON(Score)

#undef ENABLE_FULL_OPERATORS_ON
#undef ENABLE_INCR_OPERATORS_ON
#undef ENABLE_BASE_OPERATORS_ON

/**
 * constant expressions
 */
constexpr bool isSquare(Square s) { return s >= SQ_A1 && s <= SQ_H8; }
constexpr Square getSquare(File f, Rank r) { return Square((r << 3) + f); }
constexpr File fileOf(Square s) { return File(s & 7); }
constexpr File rankOf(Square s) { return File(s >> 3); }
constexpr Piece makePiece(Color c, PieceType pt) { return Piece((c << 3) + pt); }
constexpr Color colorOf(Piece p) { return Color(p >> 3); }
constexpr PieceType typeOf(Piece p) { return PieceType(p & 7); }

inline std::string squareLabel(Square sq) {
  return std::string{char('a' + fileOf(sq)), char('1' + rankOf(sq))};
}

#include "Move.h"

#endif //FRANKYCPP_GLOBALS_H
