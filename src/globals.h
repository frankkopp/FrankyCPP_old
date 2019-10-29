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

// #define NDEBUG

#ifndef FRANKYCPP_GLOBALS_H
#define FRANKYCPP_GLOBALS_H

#include <string>
#include <cstdint>
#include <cassert>
#include <iostream>
#include <sstream>
#include <vector>
#include <deque>

// convenience macros
#define NEWLINE std::cout << std::endl
#define printBB(bb) std::cout << Bitboards::print((bb)) << std::endl
#define println(s) std::cout << (s) << std::endl

// Global constants
inline const char *START_POSITION_FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

/** Max number of moves in a game to be used in arrays etc. */
inline const int MAX_MOVES = 256;

/** Max number of search depths */
inline const int MAX_PLY = 128;

/** Game phase is 24 when all officers are present. 0 when no officer is present */
inline const int GAME_PHASE_MAX = 24;

/** 64 bit Key for zobrist etc. */
typedef uint64_t Key;

///////////////////////////////////
//// INITIALIZATION

namespace INIT {
  /** initializes Values, Bitboards, Position */
  extern void init();
}

///////////////////////////////////
//// COLOR

enum Color {
  WHITE, BLACK, NOCOLOR,
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
const static Direction pawnDir[COLOR_LENGTH] = {NORTH, SOUTH};

/** Orientation */
enum Orientation : int {
  NW, N, NE, E, SE, S, SW, W,
  OR_LENGTH
};

/// Additional operators to add a Direction to a Square
constexpr Square operator+(Square s, Direction d) { return Square(int(s) + int(d)); }
constexpr Square operator-(Square s, Direction d) { return Square(int(s) - int(d)); }
constexpr Square &operator+=(Square &s, Direction d) { return s = s + d; }
constexpr Square &operator-=(Square &s, Direction d) { return s = s - d; }

///////////////////////////////////
//// PIECE TYPES

enum PieceType : int {
  KING, PAWN, KNIGHT, BISHOP, ROOK, QUEEN, PIECETYPE_NONE, PT_LENGTH = 7
  // non sliding ---  sliding -----------
};

/** returns a char representing the piece type - "kpnbrq" */
inline const char *pieceTypeToChar = "kpnbrq";

/** Game phase values */
inline const int gamePhaseValue[] = {
  0, // king
  0,  // pawn
  1,  // knight
  1,  // bishop
  2,  // rook
  4,  // queen
  0   // notype
};

/** Pieces */
enum Piece : int {
  WHITE_KING = 0, WHITE_PAWN, WHITE_KNIGHT, WHITE_BISHOP, WHITE_ROOK, WHITE_QUEEN, // 0x0 - 0x5
  BLACK_KING = 8, BLACK_PAWN, BLACK_KNIGHT, BLACK_BISHOP, BLACK_ROOK, BLACK_QUEEN, // 0x8 - 0XD
  PIECE_NONE = 16, // 0x10
  PIECE_LENGTH = 16
};

/** returns a char representing the piece. Upper case letters for white, lower case for black */
inline const char *pieceToChar = "KPNBRQ  kpnbrq   ";

/** creates the piece given by color and piece type */
constexpr Piece makePiece(Color c, PieceType pt) { return Piece((c << 3) + pt); }
/** returns the color of the given piece */
constexpr Color colorOf(Piece p) { return Color(p >> 3); }
/** returns the piece type of the given piece */
constexpr PieceType typeOf(Piece p) { return PieceType(p & 7); }

///////////////////////////////////
//// VALUE

enum Value : int {
  VALUE_DRAW = 0,
  VALUE_INF = 15000,
  VALUE_NONE = -VALUE_INF - 1,
  VALUE_MIN = -10000,
  VALUE_MAX = 10000,
  VALUE_CHECKMATE = VALUE_MAX,
  VALUE_CHECKMATE_THRESHOLD = VALUE_CHECKMATE - MAX_PLY,
};

inline std::ostream &operator<<(std::ostream &os, const Value &v) {
  os << std::to_string(v);
  return os;
}

/** PieceType values */
inline const Value pieceTypeValue[] = {
  Value(2000), // king
  Value(100),  // pawn
  Value(320),  // knight
  Value(330),  // bishop
  Value(500),  // rook
  Value(900),  // queen
  Value(0)     // notype
};

/** returns the value of the given piece type */
inline Value valueOf(const PieceType pt) { return pieceTypeValue[pt]; }
/** returns the value of the given piece */
inline Value valueOf(const Piece p) { return pieceTypeValue[typeOf(p)]; }

///////////////////////////////////
//// MOVE

/* @formatter:off
BITMAP 32-bit
0 0 0 0 0 0 0 0 0 0 1 1 1 1 1 1 1 1 1 1 2 2 2 2 2 2 2 2 2 2 3 3
0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
---------------------------------------------------------------
1 1 1 1 1 1                                                     to
            1 1 1 1 1 1                                         from
                        1 1                                     promotion piece type (pt-2 > 0-3)
                            1 1                                 move type
                                1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 move sort value
*/ // @formatter:on

inline const int FROM_SHIFT = 6;
inline const int PROM_TYPE_SHIFT = 12;
inline const int TYPE_SHIFT = 14;
inline const int VALUE_SHIFT = 16;

inline const int SQUARE_MASK = 0x3F;
inline const int FROMTO_MASK = 0xFFF;
inline const int PROM_TYPE_MASK = 3 << PROM_TYPE_SHIFT;
inline const int MOVE_TYPE_MASK = 3 << TYPE_SHIFT;

inline const int MOVE_MASK = 0xFFFF;  // first 16-bit
inline const int VALUE_MASK = 0xFFFF << VALUE_SHIFT; // second 16-bit

/** A move is basically a 32-bit int */
enum Move : u_int32_t {
  NOMOVE
};

/** MoveType */
enum MoveType {
  NORMAL,
  PROMOTION = 1 << TYPE_SHIFT,
  ENPASSANT = 2 << TYPE_SHIFT,
  CASTLING = 3 << TYPE_SHIFT
};

/** Creates a move of type NORMAL */
constexpr Move createMove(Square from, Square to) { return Move((from << FROM_SHIFT) + to); }

/** Creates a move of type NORMAL with the given value */
constexpr Move createMove(Square from, Square to, Value v) {
  return Move((Value(v - VALUE_NONE) << VALUE_SHIFT) + (from << FROM_SHIFT) + to);
}

/** Creates a move of type T with optional promotion type */
template<MoveType T>
constexpr Move createMove(Square from, Square to, PieceType pt = KNIGHT) {
  assert(T == PROMOTION || pt == KNIGHT);
  assert(pt == KNIGHT || pt == QUEEN || pt == ROOK || pt == BISHOP);
  return Move(T + ((pt - KNIGHT) << PROM_TYPE_SHIFT) + (from << FROM_SHIFT) + to);
}

/** Creates a move of type T with optional promotion type and the given value */
template<MoveType T>
constexpr Move createMove(Square from, Square to, Value v, PieceType pt = KNIGHT) {
  assert(T == PROMOTION || pt == KNIGHT);
  assert(pt == KNIGHT || pt == QUEEN || pt == ROOK || pt == BISHOP);
  assert(v <= VALUE_INF && v >= VALUE_NONE);
  return Move((Value(v - VALUE_NONE) << VALUE_SHIFT) + T + ((pt - KNIGHT) << PROM_TYPE_SHIFT) +
              (from << FROM_SHIFT) + to);
}

/** Creates a move of type T from an UCI string */
template<MoveType T = NORMAL>
Move createMove(const char *move) {
  std::istringstream iss(move);
  iss >> std::noskipws;
  unsigned char token = 0;
  Square from, to;

  // from
  if (iss >> token) {
    if (token >= 'a' && token <= 'h') {
      File f = File(token - 'a');
      if (!(iss >> token)) return NOMOVE; // malformed - ignore the rest
      if ((token >= '1' && token <= '8')) {
        Rank r = Rank(token - '1');
        from = getSquare(f, r);
      }
      else return NOMOVE; // malformed - ignore the rest
    }
    else return NOMOVE; // malformed - ignore the rest
  }
  else return NOMOVE; // malformed - ignore the rest

  // to
  if (iss >> token) {
    if (token >= 'a' && token <= 'h') {
      File f = File(token - 'a');
      if (!(iss >> token)) return NOMOVE; // malformed - ignore the rest
      if ((token >= '1' && token <= '8')) {
        Rank r = Rank(token - '1');
        to = getSquare(f, r);
      }
      else return NOMOVE; // malformed - ignore the rest
    }
    else return NOMOVE; // malformed - ignore the rest
  }
  else return NOMOVE; // malformed - ignore the rest

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
    else return NOMOVE; // malformed - ignore the rest
  }

  return createMove<T>(from, to);
}

/** returns the square the move originates from */
constexpr Square getFromSquare(Move m) { return Square((m >> FROM_SHIFT) & SQUARE_MASK); }
/** returns the square the move goes to */
constexpr Square getToSquare(Move m) { return Square(m & SQUARE_MASK); }
/** checks if this a valid move */
constexpr bool isMove(Move m) {
  Square fromSquare = getFromSquare(m);
  Square toSquare = getToSquare(m);
  return fromSquare >= SQ_A1
         && fromSquare <= SQ_H8
         && toSquare >= SQ_A1
         && toSquare <= SQ_H8
         && fromSquare != toSquare;
}
/** returns the type of the move */
constexpr MoveType typeOf(Move m) { return MoveType(m & MOVE_TYPE_MASK); }
/** returns the promotion type of the move. This only makes sense if the move
 * actually is of type promotion. Otherwise it must be ignored */
constexpr PieceType promotionType(Move m) {
  return PieceType(((m & PROM_TYPE_MASK) >> PROM_TYPE_SHIFT) + KNIGHT);
}
/** sets the value for the move. E.g. used by the move generator for move sorting */
constexpr void setValue(Move &m, Value v) {
  assert(v <= VALUE_INF && v >= VALUE_NONE);
  // when saving a value to a move we shift value to a positive integer (0-VALUE_NONE) and
  // encode it into the move
  // for retrieving we then shift the value back to a range from VALUE_NONE to VALUE_INF
  m = Move((m & MOVE_MASK) | (Value(v - VALUE_NONE) << VALUE_SHIFT));
}
/** returns the value of the move */
constexpr Value valueOf(Move m) { return Value(((m & VALUE_MASK) >> VALUE_SHIFT) + VALUE_NONE); }

/** returns a short representation of the move as string */
inline std::string printMove(const Move &move) {
  std::string promotion = "";
  if ((typeOf(move) == PROMOTION)) promotion = pieceTypeToChar[promotionType(move)];
  return squareLabel(getFromSquare(move)) + squareLabel(getToSquare(move)) + promotion;
}

inline std::ostream &operator<<(std::ostream &os, const Move &move) {
  os << printMove(move);
  return os;
}

/** returns a verbose representation of the move as string */
inline std::string printMoveVerbose(const Move &move) {
  if (move == NOMOVE) return "NOMOVE";
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
         + " (" + tp + ") (" + std::to_string(valueOf(move)) + ")";
}


///////////////////////////////////
//// MOVELIST

/** A collection of moves using a std::deque */
typedef std::deque<Move> MoveList;

inline std::ostream &operator<<(std::ostream &os, const MoveList &moveList) {
  os << "MoveList: size=" << moveList.size() << " [";
  for (Move m : moveList) {
     os << m;
     if (m != moveList.back()) os << ", ";
  }
  os << "]" << std::endl;
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
#define ENABLE_BASE_OPERATORS_ON(T)                                \
constexpr T operator+(T d1, T d2) { return T(int(d1) + int(d2)); } \
constexpr T operator-(T d1, T d2) { return T(int(d1) - int(d2)); } \
constexpr T operator-(T d) { return T(-int(d)); }                  \
inline T& operator+=(T& d1, T d2) { return d1 = d1 + d2; }         \
inline T& operator-=(T& d1, T d2) { return d1 = d1 - d2; }

#define ENABLE_INCR_OPERATORS_ON(T)                      \
inline T& operator++(T& d) { return d = T(int(d) + 1); } \
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

ENABLE_FULL_OPERATORS_ON(Value)
//ENABLE_FULL_OPERATORS_ON(Depth)
ENABLE_FULL_OPERATORS_ON(Direction)

ENABLE_INCR_OPERATORS_ON(PieceType)
ENABLE_INCR_OPERATORS_ON(Piece)
ENABLE_INCR_OPERATORS_ON(Color)
ENABLE_INCR_OPERATORS_ON(Square)
ENABLE_INCR_OPERATORS_ON(File)
ENABLE_INCR_OPERATORS_ON(Rank)
ENABLE_INCR_OPERATORS_ON(CastlingRights)

//ENABLE_BASE_OPERATORS_ON(Score)

#undef ENABLE_FULL_OPERATORS_ON
#undef ENABLE_INCR_OPERATORS_ON
#undef ENABLE_BASE_OPERATORS_ON

constexpr const char *boolStr(bool b) { return b ? "true" : "false"; };
constexpr const char *boolStr(int b) { return b ? "true" : "false"; };

struct myLocale : std::numpunct<char> {
  char do_decimal_point() const override { return ','; }
  char do_thousands_sep() const override { return '.'; }
  std::string do_grouping() const override { return "\03"; }
};

const std::locale digitLocale(std::cout.getloc(), new myLocale);

#endif //FRANKYCPP_GLOBALS_H
