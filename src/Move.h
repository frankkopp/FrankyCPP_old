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

#ifndef FRANKYCPP_MOVE_H
#define FRANKYCPP_MOVE_H

/* @formatter:off
BITMAP 16-bit
0 0 0 0 0 0 0 0 0 0 1 1 1 1 1 1
0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5
--------------------------------
1 1 1 1 1 1                     to
            1 1 1 1 1 1         from
                        1 1     promotion piece type (pt-2 > 0-3)
                            1 1 move type
*/ // @formatter:on

// MOVE

static const int FROM_SHIFT = 6;
static const int PROM_TYPE_SHIFT = 12;
static const int TYPE_SHIFT = 14;
static const int MOVE_MASK = 0x3F;
static const int MOVES_MASK = 0xFFF;
static const int MOVE_TYPE_MASK = 3;
static const int PROM_TYPE_MASK = 3;

enum Move : int {
  NOMOVE
};

enum MoveType {
  NORMAL,
  PROMOTION = 1 << TYPE_SHIFT,
  ENPASSANT = 2 << TYPE_SHIFT,
  CASTLING = 3 << TYPE_SHIFT
};

template<MoveType T>
constexpr Move make(Square from, Square to, PieceType pt = KNIGHT) {
  return Move(T + ((pt - KNIGHT) << PROM_TYPE_SHIFT) + (from << FROM_SHIFT) + to);
}

constexpr Move makeMove(Square from, Square to) { return Move((from << FROM_SHIFT) + to); }
constexpr Square fromSquare(Move m) { return Square((m >> FROM_SHIFT) & MOVE_MASK); }
constexpr Square toSquare(Move m) { return Square(m & MOVE_MASK); }
constexpr bool isMove(Move m) { return fromSquare(m) != toSquare(m); }
constexpr int fromTo(Move m) { return m & MOVES_MASK; }
constexpr MoveType typeOf(Move m) { return MoveType(m & (MOVE_TYPE_MASK << TYPE_SHIFT)); }
// promotion type only makes sense if it actually is a promotion otherwise it must be ignored
constexpr PieceType promotionType(Move m) {
  return PieceType(((m >> PROM_TYPE_SHIFT) & PROM_TYPE_MASK) + KNIGHT);
}

// CASTLING

enum CastlingSide : int {
  KING_SIDE, QUEEN_SIDE, NO_SIDE
};

enum CastlingRight : int {
  NO_CASTLING,
  WHITE_OO,
  WHITE_OOO = WHITE_OO << 1,
  BLACK_OO = WHITE_OO << 2,
  BLACK_OOO = WHITE_OO << 3,

  WHITE_CASTLING = WHITE_OO | WHITE_OOO,
  BLACK_CASTLING = BLACK_OO | BLACK_OOO,
  ANY_CASTLING = WHITE_CASTLING | BLACK_CASTLING,

  CASTLING_RIGHT_NB = 16
};

constexpr CastlingRight operator|(Color c, CastlingSide s) {
  return CastlingRight(WHITE_OO << ((s == QUEEN_SIDE) + 2 * c));
}

constexpr CastlingRight operator-(CastlingRight cr1, CastlingRight cr2) {
  assert(cr1 & cr2);
  return CastlingRight(cr1 ^ cr2);
}

constexpr CastlingRight &operator-=(CastlingRight &cr1, CastlingRight cr2) {
  assert(cr1 & cr2);
  return cr1 = CastlingRight(cr1 ^ cr2);
}

constexpr CastlingRight operator+(CastlingRight cr1, CastlingRight cr2) {
  assert(!(cr1 & cr2));
  return CastlingRight(cr1 | cr2);
}

constexpr CastlingRight &operator+=(CastlingRight &cr1, CastlingRight cr2) {
  assert(!(cr1 & cr2));
  return cr1 = CastlingRight(cr1 | cr2);
}

constexpr bool operator==(CastlingRight cr1, CastlingRight cr2) {
  return cr1 & cr2;
}

constexpr bool operator!=(CastlingRight cr1, CastlingRight cr2) {
  return !(cr1 & cr2);
}

#endif //FRANKYCPP_MOVE_H
