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

#include "Attacks.h"
#include "Bitboards.h"
#include "Logging.h"
#include <iostream>


Value Attacks::see(Position& position, Move move) {
  ASSERT_START
  // capturing moves only
  if (!position.isCapturingMove(move)) {
    LOG__ERROR(Logger::get().SEARCH_LOG, "move send to SEE should be capturing {:<30s} {}", printMoveVerbose(move), position.printFen());
  }
  // non capturing moves are not evaluated in see
  if (valueOf(position.getPiece(getFromSquare(move))) + 50 < valueOf(position.getPiece(getToSquare(move)))) {
    LOG__ERROR(Logger::get().SEARCH_LOG, "move send to SEE should be capturing high to low value {:<30s} {}", printMoveVerbose(move), position.printFen());
  }
  ASSERT_END

  // en passants are ignored in a sense that it will be winning
  // capture and therefore should lead to no cut-offs when using see()
  if (typeOf(move) == ENPASSANT) return Value{ 100 };

  // prepare short array to store the captures - max 32 pieces
  std::array<Value, 32> gain{};

  int          ply        = 0;
  const Square toSquare   = getToSquare(move);
  Square       fromSquare = getFromSquare(move);
  Piece        movedPiece = position.getPiece(fromSquare);
  Color        nextPlayer = position.getNextPlayer();

  // get a bitboard of all occupied squares to remove single pieces later
  // to reveal hidden attacks (x-ray)
  Bitboard occupiedBitboard = position.getOccupiedBB();

  // get all attacks to the square as a bitboard
  Bitboard remainingAttacks = attacksTo(position, toSquare, WHITE)
                              | attacksTo(position, toSquare, BLACK);

  LOG__TRACE(Logger::get().SEARCH_LOG, "Determine gain for {} {}", position.printFen(), printMove(move));

  // initial value of the first capture
  Value capturedValue = valueOf(position.getPiece(toSquare));
  gain[ply]           = capturedValue;
  LOG__TRACE(Logger::get().SEARCH_LOG, "gain[{}] = {} | {}", ply, printValue(gain[ply]), printMove(move));

  // loop through all remaining attacks/captures
  do {
    ply++;                    // next depth
    nextPlayer = ~nextPlayer; // change side

    // speculative store, if defended
    gain[ply] = (typeOf(move) == PROMOTION
                     ? valueOf(promotionType(move)) - valueOf(PAWN)
                     : valueOf(movedPiece))
                - gain[ply - 1];
    LOG__TRACE(Logger::get().SEARCH_LOG, "gain[{}] = {} | {}", ply, printValue(gain[ply]), printMove(createMove(fromSquare, toSquare)));

    // pruning if defended - will not change final see score
    if (std::max(-gain[ply - 1], gain[ply]) < 0) break;

    remainingAttacks ^= fromSquare; // reset bit in set to traverse
    occupiedBitboard ^= fromSquare; // reset bit in temporary occupancy (for x-Rays)

    // reevaluate attacks to reveal attacks after removing the moving piece
    remainingAttacks |= revealedAttacks(position, toSquare, occupiedBitboard, WHITE)
                        | revealedAttacks(position, toSquare, occupiedBitboard, BLACK);

    // determine next capture
    fromSquare = getLeastValuablePiece(position, remainingAttacks, nextPlayer);
    movedPiece = position.getPiece(fromSquare);

  } while (fromSquare != SQ_NONE);

  while (--ply) {
    LOG__TRACE(Logger::get().SEARCH_LOG, "gain[{}] = -max({}, {}) = {}", ply, -gain[ply - 1], gain[ply], -std::max(-gain[ply - 1], gain[ply]));
    gain[ply - 1] = -std::max(-gain[ply - 1], gain[ply]);
  }

  return gain[0];
}

/**
 * Determine all attacks for SEE. EnPassant is not included as this is not
 * relevant for SEE as the move preceding enpassant is always non capturing.
 * @return Bitboard with all squares where color is attacking from
 */
Bitboard Attacks::attacksTo(Position& position, Square square, Color color) {
  // Non sliding attacks
  return (Bitboards::pawnAttacks[~color][square] & position.getPieceBB(color, PAWN))
         | (Bitboards::pseudoAttacks[KNIGHT][square] & position.getPieceBB(color, KNIGHT))
         | (Bitboards::pawnAttacks[~color][square] & position.getPieceBB(color, PAWN))
         // Sliding rooks and queens
         | ((Bitboards::getMovesRank(square, position.getOccupiedBB())
             | Bitboards::getMovesFileR(square, position.getOccupiedBBL90()))
            & (position.getPieceBB(color, ROOK) | (position.getPieceBB(color, QUEEN))))
         // Sliding bishops and queens
         | ((Bitboards::getMovesDiagUpR(square, position.getOccupiedBBR45())
             | Bitboards::getMovesDiagDownR(square, position.getOccupiedBBL45()))
            & (position.getPieceBB(color, BISHOP) | (position.getPieceBB(color, QUEEN))));
}

/**
 * Returns sliding attacks after a piece has been removed to reveal new attacks.
 * It is only necessary to look at slider pieces as only their attacks can be revealed
 */
Bitboard Attacks::revealedAttacks(Position& position, Square square,
                                  Bitboard occupiedBitboard, Color color) {

  return (((Bitboards::getMovesRank(square, occupiedBitboard)
            | Bitboards::getMovesFileR(square, Bitboards::rotateL90(occupiedBitboard)))
           & (position.getPieceBB(color, ROOK) | (position.getPieceBB(color, QUEEN))))

          | ((Bitboards::getMovesDiagUpR(square, occupiedBitboard)
              | Bitboards::getMovesDiagDownR(square, Bitboards::rotateL45(occupiedBitboard)))
             & (position.getPieceBB(color, BISHOP) | (position.getPieceBB(color, QUEEN)))))

         & occupiedBitboard;
}

/**
 * Returns a square with the least valuable attacker. When several of same
 * type are available it uses the least significant bit of the bitboard.
 */
Square Attacks::getLeastValuablePiece(Position& position, Bitboard bitboard, Color color) {

  // check all piece types with increasing value
  if ((bitboard & position.getPieceBB(color, PAWN)) != 0)
    return Bitboards::lsb(bitboard & position.getPieceBB(color, PAWN));

  if ((bitboard & position.getPieceBB(color, KNIGHT)) != 0)
    return Bitboards::lsb(bitboard & position.getPieceBB(color, KNIGHT));

  if ((bitboard & position.getPieceBB(color, BISHOP)) != 0)
    return Bitboards::lsb(bitboard & position.getPieceBB(color, BISHOP));

  if ((bitboard & position.getPieceBB(color, ROOK)) != 0)
    return Bitboards::lsb(bitboard & position.getPieceBB(color, ROOK));

  if ((bitboard & position.getPieceBB(color, QUEEN)) != 0)
    return Bitboards::lsb(bitboard & position.getPieceBB(color, QUEEN));

  if ((bitboard & position.getPieceBB(color, KING)) != 0)
    return Bitboards::lsb(bitboard & position.getPieceBB(color, KING));

  return SQ_NONE;
}
