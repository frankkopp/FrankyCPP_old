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

#ifndef FRANKYCPP_TYPES_H
#define FRANKYCPP_TYPES_H

#include <string>
#include <cstdint>
#include <cassert>
#include <iostream>
#include <sstream>
#include <vector>
#include <deque>
#include "fmt/locale.h"

// convenience macros
#define NEWLINE std::cout << std::endl
#define printBB(bb) std::cout << Bitboards::print((bb)) << std::endl

// Global constants
constexpr const char* START_POSITION_FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

/** Max number of moves in a game to be used in arrays etc. */
constexpr int MAX_MOVES = 256;

/** Game phase is 24 when all officers are present. 0 when no officer is present */
constexpr int GAME_PHASE_MAX = 24;

/** 64 bit Key for zobrist etc. */
typedef uint64_t Key;

/** for time keeping */
typedef uint64_t MilliSec;

///////////////////////////////////
//// INITIALIZATION

namespace INIT {
  /** initializes Values, Bitboards, Position */
  extern void init();
}

///////////////////////////////////
//// DEPTH
enum Depth : uint8_t {
  DEPTH_NONE = 0,
  DEPTH_ONE = 1,
  DEPTH_FRONTIER = 1,
  DEPTH_MAX = 128
};

///////////////////////////////////
//// PLY
enum Ply : uint8_t {
  PLY_ROOT = 0,
  PLY_NONE = 0,
  PLY_MAX = DEPTH_MAX
};

///////////////////////////////////
//// COLOR
enum Color {
  WHITE = 0,
  BLACK = 1,
  NOCOLOR = 2,
  COLOR_LENGTH = 2
};

constexpr Color operator~(Color c) { return Color(c ^ BLACK); };

///////////////////////////////////
//// BITBOARD
typedef uint64_t Bitboard;

///////////////////////////////////
//// SQUARES
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
  SQ_NONE,
  SQ_LENGTH = 64
};
// @formatter:on

/** checks if this is a valid square (int >= 0 and <64 */
constexpr bool isSquare(Square s) { return s >= SQ_A1 && s <= SQ_H8; }

///////////////////////////////////
//// FILES
enum File : int {
  FILE_A, FILE_B, FILE_C, FILE_D, FILE_E, FILE_F, FILE_G, FILE_H, FILE_NONE,
  FILE_LENGTH = 9
};

/** returns the file of this square */
constexpr File fileOf(Square s) { return File(s & 7); }

///////////////////////////////////
//// RANKS
enum Rank : int {
  RANK_1, RANK_2, RANK_3, RANK_4, RANK_5, RANK_6, RANK_7, RANK_8, RANK_NONE,
  RANK_LENGTH = 9
};

/** returns the rank of this square */
constexpr Rank rankOf(Square s) { return Rank(s >> 3); }

/** returns the square of the intersection of file and rank */
constexpr Square getSquare(File f, Rank r) { return Square((r << 3) + f); }

/** returns a string representing the square (e.g. a1 or h8) */
inline std::string squareLabel(Square sq) {
  return std::string{char('a' + fileOf(sq)), char('1' + rankOf(sq))};
}

///////////////////////////////////
//// DIRECTION
enum Direction : int {
  NORTH = 8,
  EAST = 1,
  SOUTH = -NORTH,
  WEST = -EAST,

  NORTH_EAST = NORTH + EAST,
  SOUTH_EAST = SOUTH + EAST,
  SOUTH_WEST = SOUTH + WEST,
  NORTH_WEST = NORTH + WEST
};

/** return direction of pawns for the color */
constexpr Direction pawnDir[COLOR_LENGTH] = {NORTH, SOUTH};

/// Additional operators to add a Direction to a Square
constexpr Square operator+(Square s, Direction d) { return Square(int(s) + int(d)); }

constexpr Square operator-(Square s, Direction d) { return Square(int(s) - int(d)); }

constexpr Square &operator+=(Square &s, Direction d) { return s = s + d; }

constexpr Square &operator-=(Square &s, Direction d) { return s = s - d; }

///////////////////////////////////
//// ORIENTATION
enum Orientation : int {
  NW, N, NE, E, SE, S, SW, W,
  OR_LENGTH
};

///////////////////////////////////
//// PIECE TYPES
enum PieceType : int {
  PIECETYPE_NONE, KING, PAWN, KNIGHT, BISHOP, ROOK, QUEEN, PT_LENGTH = 7
  //             |non sliding ------ |sliding -----------
};

/** returns a char representing the piece type - "kpnbrq" */
constexpr const char* pieceTypeToChar = " kpnbrq";

/** returns a string representing the piece type */
constexpr const char* pieceTypeToString[] = {
  "NOPIECE",
  "KING",
  "PAWN",
  "KNIGHT",
  "BISHOP",
  "ROOK",
  "QUEEN"
};

/** Game phase values */
constexpr const int gamePhaseValue[] = {
  0, // no type
  0, // king
  0, // pawn
  1, // knight
  1, // bishop
  2, // rook
  4  // queen
};

///////////////////////////////////
//// PIECES
enum Piece : int {
  PIECE_NONE,
  WHITE_KING = 1, WHITE_PAWN, WHITE_KNIGHT, WHITE_BISHOP, WHITE_ROOK, WHITE_QUEEN,
  BLACK_KING = 9, BLACK_PAWN, BLACK_KNIGHT, BLACK_BISHOP, BLACK_ROOK, BLACK_QUEEN,
  PIECE_LENGTH = 16
};

/** returns a char representing the piece. Upper case letters for white, lower case for black */
constexpr const char* pieceToChar = " KPNBRQ  kpnbrq   ";

/** creates the piece given by color and piece type */
constexpr Piece makePiece(Color c, PieceType pt) { return Piece((c << 3) + pt); }

/** returns the color of the given piece */
constexpr Color colorOf(Piece p) { return Color(p >> 3); }

/** returns the piece type of the given piece */
constexpr PieceType typeOf(Piece p) { return PieceType(p & 7); }

///////////////////////////////////
//// VALUE
enum Value : int16_t {
  VALUE_ZERO = 0,
  VALUE_DRAW = VALUE_ZERO,
  VALUE_ONE = 1,
  VALUE_INF = 15000,
  VALUE_NONE = -VALUE_INF - VALUE_ONE,
  VALUE_MIN = -10000,
  VALUE_MAX = 10000,
  VALUE_CHECKMATE = VALUE_MAX,
  VALUE_CHECKMATE_THRESHOLD = VALUE_CHECKMATE - PLY_MAX,
};

inline std::ostream &operator<<(std::ostream &os, const Value v) {
  os << std::to_string(v);
  return os;
}

/** PieceType values */
constexpr const Value pieceTypeValue[] = {
  Value(0),    // no type
  Value(2000), // king
  Value(100),  // pawn
  Value(320),  // knight
  Value(330),  // bishop
  Value(500),  // rook
  Value(900),  // queen
};

/** returns the value of the given piece type */
constexpr Value valueOf(const PieceType pt) { return pieceTypeValue[pt]; }

/** returns the value of the given piece */
constexpr Value valueOf(const Piece p) { return pieceTypeValue[typeOf(p)]; }

/** Returns true if value is considered a checkmate */
inline bool isCheckMateValue(const Value value) {
  return abs(value) >= VALUE_CHECKMATE_THRESHOLD && abs(value) <= VALUE_CHECKMATE;
}

constexpr Value operator+(Value d1, Ply d2) {
  return static_cast<Value>(static_cast<int>(d1) + static_cast<int>(d2));
}

constexpr Value operator-(Value d1, Ply d2) {
  return static_cast<Value>(static_cast<int>(d1) - static_cast<int>(d2));
}

/** Returns a UCI compatible std::string for the score in cp or in mate in ply */
inline std::string printValue(const Value value) {
  // TODO add full protocol (lowerbound, upperbound, etc.)
  std::string scoreString;
  if (isCheckMateValue(value)) {
    scoreString = "mate ";
    scoreString += value < 0 ? "-" : "";
    scoreString += std::to_string((VALUE_CHECKMATE - std::abs(value) + 1) / 2);
  }
  else {
    scoreString = "cp " + std::to_string(value);
  }
  return scoreString;
}

///////////////////////////////////
//// VALUE TYPE
enum Value_Type : u_int8_t {
  TYPE_NONE = 0,
  // the node for the value was fully calculated and is exact
    TYPE_EXACT = 1,
  // the node for the value has NOT found a value > alpha so alpha is
  // upper bound (value could be worse)
    TYPE_ALPHA = 2,
  // the node for the value has found a refutation (value > beta( and has
  // been cut off. Value is a lower bound (could be better).
    TYPE_BETA = 3,
};


///////////////////////////////////
//// MOVE
enum Move : uint32_t {
  /** A move is basically a 32-bit int */
    MOVE_NONE = 0
};

/* @formatter:off
BITMAP 32-bit
|-Value ------------------------|-Move -------------------------|
3	3	2	2	2	2	2	2	2	2	2	2	1	1	1	1 | 1	1	1	1	1	1	0	0	0	0	0	0	0	0	0	0
1	0	9	8	7	6	5	4	3	2	1	0	9	8	7	6	| 5	4	3	2	1	0	9	8	7	6	5	4	3	2	1	0
--------------------------------|--------------------------------
																| 										1	1	1	1	1	1  to
																| 				1	1	1	1	1	1						   from
																| 		1	1												   promotion piece type (pt-2 > 0-3)
																| 1	1														   move type
1	1	1	1	1	1	1	1	1	1	1	1	1	1	1	1	| 															   move sort value
*/ // @formatter:on

namespace MoveShifts {
  constexpr int FROM_SHIFT = 6;
  constexpr int PROM_TYPE_SHIFT = 12;
  constexpr int TYPE_SHIFT = 14;
  constexpr int VALUE_SHIFT = 16;

  constexpr int SQUARE_MASK = 0x3F;
  constexpr int FROMTO_MASK = 0xFFF;
  constexpr int PROM_TYPE_MASK = 3 << PROM_TYPE_SHIFT;
  constexpr int MOVE_TYPE_MASK = 3 << TYPE_SHIFT;

  constexpr int MOVE_MASK = 0xFFFF;  // first 16-bit
  constexpr int VALUE_MASK = 0xFFFF << VALUE_SHIFT; // second 16-bit
}

///////////////////////////////////
//// MOVE TYPE
enum MoveType {
  NORMAL,
  PROMOTION = 1 << MoveShifts::TYPE_SHIFT,
  ENPASSANT = 2 << MoveShifts::TYPE_SHIFT,
  CASTLING = 3 << MoveShifts::TYPE_SHIFT
};

/** Creates a move of type NORMAL */
constexpr Move createMove(Square from, Square to) {
  return Move((from << MoveShifts::FROM_SHIFT) + to);
}

/** Creates a move of type NORMAL with the given value */
constexpr Move createMove(Square from, Square to, Value v) {
  return Move(
    (Value(v - VALUE_NONE) << MoveShifts::VALUE_SHIFT) + (from << MoveShifts::FROM_SHIFT) + to);
}

/** Creates a move of type T with optional promotion type */
template<MoveType T>
constexpr Move createMove(Square from, Square to, PieceType pt = KNIGHT) {
  assert(T == PROMOTION || pt == KNIGHT);
  assert(pt == KNIGHT || pt == QUEEN || pt == ROOK || pt == BISHOP);
  return Move(
    T + ((pt - KNIGHT) << MoveShifts::PROM_TYPE_SHIFT) + (from << MoveShifts::FROM_SHIFT) + to);
}

/** Creates a move of type T with optional promotion type and the given value */
template<MoveType T>
constexpr Move createMove(Square from, Square to, Value v, PieceType pt = KNIGHT) {
  assert(T == PROMOTION || pt == KNIGHT);
  assert(pt == KNIGHT || pt == QUEEN || pt == ROOK || pt == BISHOP);
  assert(v <= VALUE_INF && v >= VALUE_NONE);
  return Move((Value(v - VALUE_NONE) << MoveShifts::VALUE_SHIFT) + T +
              ((pt - KNIGHT) << MoveShifts::PROM_TYPE_SHIFT) +
              (from << MoveShifts::FROM_SHIFT) + to);
}

/** Creates a move of type T from an UCI string */
template<MoveType T = NORMAL>
Move createMove(const char* move) {
  std::istringstream iss(move);
  iss >> std::noskipws;
  unsigned char token = 0;
  Square from, to;

  // from
  if (iss >> token) {
    if (token >= 'a' && token <= 'h') {
      File f = File(token - 'a');
      if (!(iss >> token)) return MOVE_NONE; // malformed - ignore the rest
      if ((token >= '1' && token <= '8')) {
        Rank r = Rank(token - '1');
        from = getSquare(f, r);
      }
      else { return MOVE_NONE; } // malformed - ignore the rest
    }
    else { return MOVE_NONE; } // malformed - ignore the rest
  }
  else { return MOVE_NONE; } // malformed - ignore the rest

  // to
  if (iss >> token) {
    if (token >= 'a' && token <= 'h') {
      File f = File(token - 'a');
      if (!(iss >> token)) return MOVE_NONE; // malformed - ignore the rest
      if ((token >= '1' && token <= '8')) {
        Rank r = Rank(token - '1');
        to = getSquare(f, r);
      }
      else { return MOVE_NONE; } // malformed - ignore the rest
    }
    else { return MOVE_NONE; } // malformed - ignore the rest
  }
  else { return MOVE_NONE; } // malformed - ignore the rest

  // promotion
  if (T == PROMOTION) {
    if (iss >> token) {
      switch (token) {
        case 'n':
          return createMove<T>(from, to, KNIGHT);
        case 'b':
          return createMove<T>(from, to, BISHOP);
        case 'r':
          return createMove<T>(from, to, ROOK);
        case 'q':
          return createMove<T>(from, to, QUEEN);
        default:
          break;
      }
    }
    else { return MOVE_NONE; } // malformed - ignore the rest
  }

  return createMove<T>(from, to);
}

/** returns the square the move originates from */
constexpr Square getFromSquare(Move m) {
  return Square(m >> MoveShifts::FROM_SHIFT & MoveShifts::SQUARE_MASK);
}

/** returns the square the move goes to */
constexpr Square getToSquare(Move m) { return Square(m & MoveShifts::SQUARE_MASK); }

/** checks if this a valid move */
constexpr bool isMove(Move m) {
  const Square fromSquare = getFromSquare(m);
  const Square toSquare = getToSquare(m);
  return fromSquare >= SQ_A1
         && fromSquare <= SQ_H8
         && toSquare >= SQ_A1
         && toSquare <= SQ_H8
         && fromSquare != toSquare;
}

/** returns the type of the move */
constexpr MoveType typeOf(Move m) { return MoveType(m & MoveShifts::MOVE_TYPE_MASK); }

/** returns the promotion type of the move. This only makes sense if the move
 * actually is of type promotion. Otherwise it must be ignored */
constexpr PieceType promotionType(Move m) {
  return PieceType(((m & MoveShifts::PROM_TYPE_MASK) >> MoveShifts::PROM_TYPE_SHIFT) + KNIGHT);
}

/** returns the value of the move */
constexpr Value valueOf(Move m) {
  return Value(((m & MoveShifts::VALUE_MASK) >> MoveShifts::VALUE_SHIFT) + VALUE_NONE);
}

/** returns the move without value */
constexpr Move moveOf(Move m) { return Move(m & MoveShifts::MOVE_MASK); }


/** sets the value for the move. E.g. used by the move generator for move sorting */
constexpr void setValue(Move &m, Value v) {
  assert(v >= VALUE_NONE && v <= -VALUE_NONE);
  if (moveOf(m) == MOVE_NONE) return; // can't store a value on a MOVE_NONE
  // when saving a value to a move we shift value to a positive integer (0-VALUE_NONE) and
  // encode it into the move
  // for retrieving we then shift the value back to a range from VALUE_NONE to VALUE_INF
  m = Move((m & MoveShifts::MOVE_MASK) | (Value(v - VALUE_NONE) << MoveShifts::VALUE_SHIFT));
}

/** returns a short representation of the move as string (UCI protocal) */
inline std::string printMove(const Move move) {
  std::string promotion = "";
  if (moveOf(move) == MOVE_NONE) return "NOMOVE";
  if ((typeOf(move) == PROMOTION)) promotion = pieceTypeToChar[promotionType(move)];
  return squareLabel(getFromSquare(move)) + squareLabel(getToSquare(move)) + promotion;
}

/** returns a verbose representation of the move as string */
inline std::string printMoveVerbose(const Move move) {
  if (!move) return "NOMOVE " + std::to_string(move);
  std::string tp;
  std::string promPt;
  switch (typeOf(move)) {
    case NORMAL:
      tp = "NORMAL";
      break;
    case PROMOTION:
      promPt = pieceTypeToChar[promotionType(move)];
      tp = "PROMOTION";
      break;
    case ENPASSANT:
      tp = "ENPASSANT";
      break;
    case CASTLING:
      tp = "CASTLING";
      break;
  }
  return squareLabel(getFromSquare(move)) + squareLabel(getToSquare(move)) + promPt
         + " (" + tp + " " + std::to_string(valueOf(move)) + " " + std::to_string(move) + ")";
}

inline std::ostream &operator<<(std::ostream &os, const Move move) {
  os << printMove(move);
  return os;
}

///////////////////////////////////
//// MOVELIST

/** A collection of moves using a std::deque */
typedef std::deque<Move> MoveList;

inline std::string printMoveList(const MoveList &moveList) {
  std::ostringstream os;
  os << "MoveList: size=" << moveList.size() << " [";
  for (auto itr = moveList.begin(); itr != moveList.end(); ++itr) {
    os << *itr;
    if (itr != moveList.end() - 1) os << ", ";
  }
  os << "]";
  return os.str();
}

inline std::string printMoveListUCI(const MoveList &moveList) {
  std::ostringstream os;
  for (Move m : moveList) {
    os << m;
    if (m != moveList.back()) os << " ";
  }
  return os.str();
}

inline std::ostream &operator<<(std::ostream &os, const MoveList &moveList) {
  os << printMoveList(moveList);
  return os;
}

///////////////////////////////////
//// CASTLING

/** CastlingSide */
enum CastlingSide : int {
  KING_SIDE, QUEEN_SIDE, NO_SIDE
};

/** CastlingRight */
enum CastlingRights : unsigned int {
  // @formatter:off
  NO_CASTLING = 0,                                // 0000

  WHITE_OO,                                       // 0001
  WHITE_OOO = WHITE_OO << 1,                      // 0010
  WHITE_CASTLING = WHITE_OO | WHITE_OOO,          // 0011

  BLACK_OO = WHITE_OO << 2,                       // 0100
  BLACK_OOO = WHITE_OO << 3,                      // 1000
  BLACK_CASTLING = BLACK_OO | BLACK_OOO,          // 1100

  ANY_CASTLING = WHITE_CASTLING | BLACK_CASTLING, // 1111
  CR_LENGTH = 16
  // @formatter:on
};

constexpr CastlingRights operator|(Color c, CastlingSide s) {
  return CastlingRights(WHITE_OO << ((s == QUEEN_SIDE) + 2 * c));
}

constexpr CastlingRights operator-(CastlingRights cr1, CastlingRights cr2) {
  assert(cr1 & cr2);
  return CastlingRights(cr1 ^ cr2);
}

constexpr CastlingRights &operator-=(CastlingRights &cr1, CastlingRights cr2) {
  assert(cr1 & cr2);
  return cr1 = CastlingRights(cr1 ^ cr2);
}

constexpr CastlingRights operator+(CastlingRights cr1, CastlingRights cr2) {
  return CastlingRights(cr1 | cr2);
}

constexpr CastlingRights &operator+=(CastlingRights &cr1, CastlingRights cr2) {
  return cr1 = CastlingRights(cr1 | cr2);
}

constexpr bool operator==(CastlingRights cr1, CastlingRights cr2) {
  return (cr1 & cr2) || (cr1 == 0 && cr2 == 0);
}

constexpr bool operator!=(CastlingRights cr1, CastlingRights cr2) {
  return !(cr1 & cr2);
}

/**
 * OPERATORS
 */
#define ENABLE_BASE_OPERATORS_ON(T) \
constexpr T operator+(T d1, T d2) { return static_cast<T>(static_cast<int>(d1) + static_cast<int>(d2)); } \
constexpr T operator-(T d1, T d2) { return static_cast<T>(static_cast<int>(d1) - static_cast<int>(d2)); } \
constexpr T operator-(T d) { return static_cast<T>(-static_cast<int>(d)); } \
inline T& operator+=(T& d1, T d2) { return d1 = d1 + d2; } \
inline T& operator-=(T& d1, T d2) { return d1 = d1 - d2; }

#define ENABLE_INCR_OPERATORS_ON(T) \
inline T& operator++(T& d) { return d = static_cast<T>(static_cast<int>(d) + 1); } \
inline T& operator--(T& d) { return d = static_cast<T>(static_cast<int>(d) - 1); }

#define ENABLE_FULL_OPERATORS_ON(T) \
ENABLE_BASE_OPERATORS_ON(T) \
ENABLE_INCR_OPERATORS_ON(T) \
constexpr T operator+(int i, T d) { return static_cast<T>(i + static_cast<int>(d)); } \
constexpr T operator+(T d, int i) { return static_cast<T>(static_cast<int>(d) + i); } \
constexpr T operator-(int i, T d) { return static_cast<T>(i - static_cast<int>(d)); } \
constexpr T operator-(T d, int i) { return static_cast<T>(static_cast<int>(d) - i); } \
constexpr T operator*(int i, T d) { return static_cast<T>(i * static_cast<int>(d)); } \
constexpr T operator*(T d, int i) { return static_cast<T>(static_cast<int>(d) * i); } \
constexpr T operator/(T d, int i) { return static_cast<T>(static_cast<int>(d) / i); } \
constexpr int operator/(T d1, T d2) { return static_cast<int>(d1) / static_cast<int>(d2); } \
inline T& operator*=(T& d, int i) { return d = static_cast<T>(static_cast<int>(d) * i); } \
inline T& operator/=(T& d, int i) { return d = static_cast<T>(static_cast<int>(d) / i); }

ENABLE_FULL_OPERATORS_ON(Depth)

ENABLE_FULL_OPERATORS_ON(Ply)

ENABLE_FULL_OPERATORS_ON(Value)

ENABLE_FULL_OPERATORS_ON(Direction)

ENABLE_INCR_OPERATORS_ON(PieceType)

ENABLE_INCR_OPERATORS_ON(Piece)

ENABLE_INCR_OPERATORS_ON(Color)

ENABLE_INCR_OPERATORS_ON(Square)

ENABLE_INCR_OPERATORS_ON(File)

ENABLE_INCR_OPERATORS_ON(Rank)

ENABLE_INCR_OPERATORS_ON(CastlingRights)

#undef ENABLE_FULL_OPERATORS_ON
#undef ENABLE_INCR_OPERATORS_ON
#undef ENABLE_BASE_OPERATORS_ON

constexpr const char* boolStr(bool b) { return b ? "true" : "false"; };

constexpr const char* boolStr(int b) { return b ? "true" : "false"; };

inline bool to_bool(std::string str) {
  std::transform(str.begin(), str.end(), str.begin(), ::tolower);
  std::istringstream is(str);
  bool b;
  is >> std::boolalpha >> b;
  return b;
}

/**
  * Prints a 64-bit uint as a series of 0 and 1 grouped in 8 bits
  * beginning with the MSB (0) on the left and the LSB (63) on the right
  * @param b
  */
inline std::string printBitString(uint64_t b) {
  std::ostringstream os;
  os << std::bitset<64>(b);
  return os.str();
}

struct deLocaleDecimals : std::numpunct<char> {
  char do_decimal_point() const override { return ','; }

  char do_thousands_sep() const override { return '.'; }

  std::string do_grouping() const override { return "\03"; }
};

const std::locale deLocale(std::locale("de_DE.UTF-8"), new deLocaleDecimals);

#define println(s) std::cout << (s) << std::endl
#define fprint(...) std::cout << fmt::format(deLocale, __VA_ARGS__)
#define fprintln(...) fprint(__VA_ARGS__) << std::endl

#endif //FRANKYCPP_TYPES_H
