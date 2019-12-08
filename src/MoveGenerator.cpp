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

#include <algorithm>

#include "Bitboards.h"
#include "Values.h"
#include "Position.h"
#include "MoveGenerator.h"

// TODO
//  - PV move
//  - Killer Moves

////////////////////////////////////////////////
///// CONSTRUCTORS

MoveGenerator::MoveGenerator() = default;
MoveGenerator::~MoveGenerator() = default;

////////////////////////////////////////////////
///// PUBLIC

MoveList*
MoveGenerator::generatePseudoLegalMoves(GenMode genMode, Position *pPosition) {
  pseudoLegalMoves.clear();
  generatePawnMoves(genMode, pPosition, &pseudoLegalMoves);
  generateCastling(genMode, pPosition, &pseudoLegalMoves);
  generateMoves(genMode, pPosition, &pseudoLegalMoves);
  generateKingMoves(genMode, pPosition, &pseudoLegalMoves);
  sort(pseudoLegalMoves.begin(), pseudoLegalMoves.end());
  return &pseudoLegalMoves;
}

MoveList*
MoveGenerator::generateLegalMoves(GenMode genMode, Position *pPosition) {
  legalMoves.clear();
  generatePseudoLegalMoves(genMode, pPosition);
  for (Move m : pseudoLegalMoves) if (pPosition->isLegalMove(m)) legalMoves.push_back(m);
  return &legalMoves;
}

Move
MoveGenerator::getNextPseudoLegalMove(GenMode genMode, Position *pPosition) {
  // if the position changes during iteration the iteration will be reset and
  // generation will be restart with the new position.
  if (pPosition->getZobristKey() != currentIteratorKey) {
    onDemandMoves.clear();
    currentODStage = OD_NEW;
    currentIteratorKey = pPosition->getZobristKey();
  }

  /*
   * If the list is currently empty and we have not generated all moves yet
   * generate the next batch until we have new moves or all moves are generated
   * and there are no more moves to generate
   */
  while (onDemandMoves.empty() && currentODStage < OD_END) {
    switch (currentODStage) {
      case OD_NEW:
        currentODStage = OD1;
        // fall through
      case OD1:
        generatePawnMoves(GENCAP, pPosition, &onDemandMoves);
        sort(onDemandMoves.begin(), onDemandMoves.end());
        currentODStage = OD2;
        break;
      case OD2:
        generateMoves(GENCAP, pPosition, &onDemandMoves);
        sort(onDemandMoves.begin(), onDemandMoves.end());
        currentODStage = OD3;
        break;
      case OD3:
        generateKingMoves(GENCAP, pPosition, &onDemandMoves);
        sort(onDemandMoves.begin(), onDemandMoves.end());
        if (genMode & GENNONCAP) currentODStage = OD4;
        else currentODStage = OD_END;
        break;
      case OD4:
        generatePawnMoves(GENNONCAP, pPosition, &onDemandMoves);
        sort(onDemandMoves.begin(), onDemandMoves.end());
        currentODStage = OD5;
        break;
      case OD5:
        generateCastling(GENNONCAP, pPosition, &onDemandMoves);
        sort(onDemandMoves.begin(), onDemandMoves.end());
        currentODStage = OD6;
        break;
      case OD6:
        generateMoves(GENNONCAP, pPosition, &onDemandMoves);
        sort(onDemandMoves.begin(), onDemandMoves.end());
        currentODStage = OD7;
        break;
      case OD7:
        generateKingMoves(GENNONCAP, pPosition, &onDemandMoves);
        sort(onDemandMoves.begin(), onDemandMoves.end());
        currentODStage = OD_END;
        break;
      case OD_END:
        break;
    }
  }
  // return a move and delete it form the list
  if (onDemandMoves.empty()) return NOMOVE;
  else {
    Move move = onDemandMoves.front();
    onDemandMoves.pop_front();
    return move;
  }
}

void
MoveGenerator::resetOnDemand() {
  onDemandMoves.clear();
  currentODStage = OD_NEW;
  currentIteratorKey = 0;
}

bool
MoveGenerator::hasLegalMove(Position *pPosition) {

  /*
  To determine if we have at least one legal move we only have to find
  on legal move. We search for any KING, PAWN, KNIGHT, BISHOP, ROOK, QUEEN move
  and return immediately if we found one.
  The order of our search is from approx. the most likely to the least likely
  */

  const Color nextPlayer = pPosition->getNextPlayer();
  const Bitboard occupiedBB = pPosition->getOccupiedBB();
  const Bitboard nextPlayerBB = pPosition->getOccupiedBB(nextPlayer);
  const Bitboard opponentBB = pPosition->getOccupiedBB(~nextPlayer);
  const Bitboard myPawns = pPosition->getPieceBB(nextPlayer, PAWN);

  // KING
  const Square kingSquare = pPosition->getKingSquare(nextPlayer);
  Bitboard tmpMoves = Bitboards::pseudoAttacks[KING][kingSquare] & ~nextPlayerBB;
  while (tmpMoves) {
    const Square toSquare = Bitboards::popLSB(&tmpMoves);
    if (pPosition->isLegalMove(createMove(kingSquare, toSquare))) return true;
  }

  // PAWN
  // normal pawn captures to the west - promotions first
  tmpMoves = Bitboards::shift(pawnDir[nextPlayer] + WEST, myPawns) & opponentBB;
  while (tmpMoves) {
    const Square toSquare = Bitboards::popLSB(&tmpMoves);
    const Square fromSquare = toSquare + pawnDir[~nextPlayer] + EAST;
    if (pPosition->isLegalMove(createMove(fromSquare, toSquare))) return true;
  }

  // normal pawn captures to the east - promotions first
  tmpMoves = Bitboards::shift(pawnDir[nextPlayer] + EAST, myPawns) & opponentBB;
  while (tmpMoves) {
    const Square toSquare = Bitboards::popLSB(&tmpMoves);
    const Square fromSquare = toSquare + pawnDir[~nextPlayer] + WEST;
    if (pPosition->isLegalMove(createMove(fromSquare, toSquare))) return true;
  }

  // pawn pushes - check step one to unoccupied squares
  tmpMoves = Bitboards::shift(pawnDir[nextPlayer], myPawns) & ~pPosition->getOccupiedBB();
  // double pawn steps
  Bitboard tmpMoves2 = Bitboards::shift(pawnDir[nextPlayer],
                                        tmpMoves &
                                        (nextPlayer == WHITE ? Bitboards::Rank3BB
                                                             : Bitboards::Rank6BB)) &
                       ~pPosition->getOccupiedBB();
  while (tmpMoves) {
    const Square toSquare = Bitboards::popLSB(&tmpMoves);
    const Square fromSquare = toSquare + pawnDir[~nextPlayer];
    if (pPosition->isLegalMove(createMove(fromSquare, toSquare))) return true;
  }
  while (tmpMoves2) {
    const Square toSquare = Bitboards::popLSB(&tmpMoves2);
    const Square fromSquare = toSquare + pawnDir[~nextPlayer] + pawnDir[~nextPlayer];
    if (pPosition->isLegalMove(createMove(fromSquare, toSquare))) return true;
  }

  // en passant captures
  const Square enPassantSquare = pPosition->getEnPassantSquare();
  if (enPassantSquare != SQ_NONE) {
    // left
    tmpMoves =
      Bitboards::shift(pawnDir[~nextPlayer] + WEST, Bitboards::squareBB[enPassantSquare]) & myPawns;
    if (tmpMoves) {
      const Square fromSquare = Bitboards::lsb(tmpMoves);
      if (pPosition->isLegalMove(
        createMove<ENPASSANT>(fromSquare, fromSquare + pawnDir[nextPlayer] + EAST)))
        return true;
    }
    // right
    tmpMoves =
      Bitboards::shift(pawnDir[~nextPlayer] + EAST, Bitboards::squareBB[enPassantSquare]) & myPawns;
    if (tmpMoves) {
      const Square fromSquare = Bitboards::lsb(tmpMoves);
      if (pPosition->isLegalMove(
        createMove<ENPASSANT>(fromSquare, fromSquare + pawnDir[nextPlayer] + WEST)))
        return true;
    }
  }

  // OFFICERS
  for (PieceType pt = KNIGHT; pt <= QUEEN; ++pt) {
    Bitboard pieces = pPosition->getPieceBB(nextPlayer, pt);

    while (pieces) {
      const Square fromSquare = Bitboards::popLSB(&pieces);
      const Bitboard pseudoMoves = Bitboards::pseudoAttacks[pt][fromSquare];

      Bitboard moves = pseudoMoves & ~nextPlayerBB;
      while (moves) {
        const Square toSquare = Bitboards::popLSB(&moves);

        if (pt > KNIGHT) { // sliding pieces
          if (!(Bitboards::intermediateBB[fromSquare][toSquare] & occupiedBB)) {
            if (pPosition->isLegalMove(createMove(fromSquare, toSquare))) return true;
          }
        }
        else { // knight cannot be blocked
          if (pPosition->isLegalMove(createMove(fromSquare, toSquare))) return true;
        }
      }
    }
  }

  // no move found
  return false;
}

////////////////////////////////////////////////
///// PRIVATE

void
MoveGenerator::generatePawnMoves(GenMode genMode, const Position *pPosition, MoveList *pMoves) {

  const Color nextPlayer = pPosition->getNextPlayer();
  const Bitboard myPawns = pPosition->getPieceBB(nextPlayer, PAWN);
  const Bitboard oppPieces = pPosition->getOccupiedBB(~nextPlayer);

  const Piece piece = makePiece(nextPlayer, PAWN);
  const int gamePhase = pPosition->getGamePhase();

  // captures
  if (genMode & GENCAP) {

    /*
    This algorithm shifts the own pawn bitboard in the direction of pawn captures
    and ANDs it with the opponents pieces. With this we get all possible captures
    and can easily create the moves by using a loop over all captures and using
    the backward shift for the from-Square.
    All moves get sort values so that sort order should be:
     captures: most value victim least value attacker - promotion piece value
     non captures: killer (TBD), promotions, castling, normal moves (position value)
    */

    Bitboard tmpCaptures;
    Bitboard promCaptures;

    for (Direction dir : {WEST, EAST}) {
      // normal pawn captures - promotions first
      tmpCaptures = Bitboards::shift(pawnDir[nextPlayer] + dir, myPawns) & oppPieces;
      promCaptures = tmpCaptures & Bitboards::promotionRank[nextPlayer];
      while (promCaptures) {
        const Square toSquare = Bitboards::popLSB(&promCaptures);
        const Square fromSquare = toSquare + pawnDir[~nextPlayer] - dir;
        // value is the delta of values from the two pieces involved minus the promotion value
        const Value value =
          valueOf(pPosition->getPiece(fromSquare)) - valueOf(pPosition->getPiece(toSquare));
        pMoves->push_back(
          createMove<PROMOTION>(fromSquare, toSquare, value - valueOf(QUEEN), QUEEN));
        pMoves->push_back(
          createMove<PROMOTION>(fromSquare, toSquare,
                                value - valueOf(ROOK) + static_cast<Value>(2000), ROOK));
        pMoves->push_back(
          createMove<PROMOTION>(fromSquare, toSquare,
                                value - valueOf(BISHOP) + static_cast<Value>(2000), BISHOP));
        pMoves->push_back(
          createMove<PROMOTION>(fromSquare, toSquare, value - valueOf(KNIGHT), KNIGHT));
      }
      tmpCaptures &= ~Bitboards::promotionRank[nextPlayer];
      while (tmpCaptures) {
        const Square toSquare = Bitboards::popLSB(&tmpCaptures);
        const Square fromSquare = toSquare + pawnDir[~nextPlayer] - dir;
        // value is the delta of values from the two pieces involved
        const Value value =
          valueOf(pPosition->getPiece(fromSquare)) - valueOf(pPosition->getPiece(toSquare));
        pMoves->push_back(createMove(fromSquare, toSquare, value));
      }
    }

    // en passant captures
    const Square enPassantSquare = pPosition->getEnPassantSquare();
    if (enPassantSquare != SQ_NONE) {
      for (Direction dir : {WEST, EAST}) {
        tmpCaptures =
          Bitboards::shift(pawnDir[~nextPlayer] + dir, Bitboards::squareBB[enPassantSquare]) &
          myPawns;
        if (tmpCaptures) {
          Square fromSquare = Bitboards::lsb(tmpCaptures);
          Square toSquare = fromSquare + pawnDir[nextPlayer] - dir;
          // value is the positional value of the piece at this gamephase
          const Value value = Values::posValue[piece][toSquare][gamePhase];
          pMoves->push_back(createMove<ENPASSANT>(fromSquare, toSquare, value));
        }
      }
    }
  }

  // non captures
  if (genMode & GENNONCAP) {

    /*
    Move my pawns forward one step and keep all on not occupied squares
    Move pawns now on rank 3 (rank 6) another square forward to check for pawn doubles.
    Loop over pawns remaining on unoccupied squares and add moves.
     */

    // pawns - check step one to unoccupied squares
    Bitboard tmpMoves =
      Bitboards::shift(pawnDir[nextPlayer], myPawns) & ~pPosition->getOccupiedBB();
    // pawns double - check step two to unoccupied squares
    Bitboard tmpMovesDouble = Bitboards::shift(pawnDir[nextPlayer],
                                    tmpMoves & (nextPlayer == WHITE ? Bitboards::Rank3BB
                                                                    : Bitboards::Rank6BB)) &
                              ~pPosition->getOccupiedBB();

    // single pawn steps - promotions first
    Bitboard promMoves = tmpMoves & Bitboards::promotionRank[nextPlayer];
    while (promMoves) {
      const Square toSquare = Bitboards::popLSB(&promMoves);
      const Square fromSquare = toSquare + pawnDir[~nextPlayer];
      // value is done manually for sorting of queen prom first, then knight and others
      pMoves->push_back(
        createMove<PROMOTION>(fromSquare, toSquare, static_cast<Value>(9000), QUEEN));
      pMoves->push_back(
        createMove<PROMOTION>(fromSquare, toSquare, static_cast<Value>(10900), ROOK));
      pMoves->push_back(
        createMove<PROMOTION>(fromSquare, toSquare, static_cast<Value>(10900), BISHOP));
      pMoves->push_back(
        createMove<PROMOTION>(fromSquare, toSquare, static_cast<Value>(9100), KNIGHT));
    }
    // double pawn steps
    while (tmpMovesDouble) {
      const Square toSquare = Bitboards::popLSB(&tmpMovesDouble);
      // value is the positional value of the piece at this gamephase
      const Value value = static_cast<Value>(10000) - Values::posValue[piece][toSquare][gamePhase];
      pMoves->push_back(
        createMove(toSquare + 2 * pawnDir[~nextPlayer], toSquare, value));
    }
    // normal single pawn steps
    tmpMoves = tmpMoves & ~Bitboards::promotionRank[nextPlayer];
    while (tmpMoves) {
      const Square toSquare = Bitboards::popLSB(&tmpMoves);
      const Square fromSquare = toSquare + pawnDir[~nextPlayer];
      // value is the positional value of the piece at this gamephase
      const Value value = static_cast<Value>(10000) - Values::posValue[piece][toSquare][gamePhase];
      pMoves->push_back(createMove(fromSquare, toSquare, value));
    }
  }
}

void
MoveGenerator::generateKingMoves(GenMode genMode, const Position *pPosition, MoveList *pMoves) {
  const Color nextPlayer = pPosition->getNextPlayer();
  const Bitboard occupiedBB = pPosition->getOccupiedBB();
  const Bitboard opponentBB = pPosition->getOccupiedBB(~nextPlayer);

  const Piece piece = makePiece(nextPlayer, KING);
  const int gamePhase = pPosition->getGamePhase();


  Bitboard pieces = pPosition->getPieceBB(nextPlayer, KING);
  assert(Bitboards::popcount(pieces) == 1 && "More than one king not allowed!");

  const Square fromSquare = Bitboards::popLSB(&pieces);
  const Bitboard pseudoMoves = Bitboards::pseudoAttacks[KING][fromSquare];

  // captures
  if (genMode & GENCAP) {
    Bitboard captures = pseudoMoves & opponentBB;
    while (captures) {
      const Square toSquare = Bitboards::popLSB(&captures);
      // value is the positional value of the piece at this gamephase minus the
      // value of the captured piece
      const Value value = Values::posValue[piece][toSquare][gamePhase]
                          - valueOf(pPosition->getPiece(toSquare));
      pMoves->push_back(createMove(fromSquare, toSquare, value));
    }
  }

  // non captures
  if (genMode & GENNONCAP) {
    Bitboard nonCaptures = pseudoMoves & ~occupiedBB;
    while (nonCaptures) {
      const Square toSquare = Bitboards::popLSB(&nonCaptures);
      // value is the positional value of the piece at this gamephase
      const Value value = static_cast<Value>(10000) - Values::posValue[piece][toSquare][gamePhase];
      pMoves->push_back(createMove(fromSquare, toSquare, value));
    }
  }
}

void
MoveGenerator::generateMoves(GenMode genMode, const Position *pPosition, MoveList *pMoves) {
  const Color nextPlayer = pPosition->getNextPlayer();
  const Bitboard occupiedBB = pPosition->getOccupiedBB();
  const Bitboard opponentBB = pPosition->getOccupiedBB(~nextPlayer);
  const int gamePhase = pPosition->getGamePhase();

  for (PieceType pt = KNIGHT; pt < PIECETYPE_NONE; ++pt) {
    Bitboard pieces = pPosition->getPieceBB(nextPlayer, pt);
    const Piece piece = makePiece(nextPlayer, pt);

    while (pieces) {
      const Square fromSquare = Bitboards::popLSB(&pieces);
      const Bitboard pseudoMoves = Bitboards::pseudoAttacks[pt][fromSquare];

      // captures
      if (genMode & GENCAP) {
        Bitboard captures = pseudoMoves & opponentBB;
        while (captures) {
          const Square targetSquare = Bitboards::popLSB(&captures);
          if (pt > KNIGHT) { // sliding pieces
            if (!(Bitboards::intermediateBB[fromSquare][targetSquare] & occupiedBB)) {
              // value is the delta of values from the two pieces involved
              const Value value = valueOf(pPosition->getPiece(fromSquare))
                                  - valueOf(pPosition->getPiece(targetSquare));
              pMoves->push_back(createMove(fromSquare, targetSquare, value));
            };
          }
          else { // king and knight cannot be blocked
            // value is the delta of values from the two pieces involved
            const Value value = valueOf(pPosition->getPiece(fromSquare))
                                - valueOf(pPosition->getPiece(targetSquare));
            pMoves->push_back(createMove(fromSquare, targetSquare, value));
          }
        }
      }

      // non captures
      if (genMode & GENNONCAP) {
        Bitboard nonCaptures = pseudoMoves & ~occupiedBB;
        while (nonCaptures) {
          const Square toSquare = Bitboards::popLSB(&nonCaptures);
          if (pt > KNIGHT) { // sliding pieces
            if (!(Bitboards::intermediateBB[fromSquare][toSquare] & occupiedBB)) {
              // value is the positional value of the piece at this gamephase
              const Value value =
                static_cast<Value>(10000) - Values::posValue[piece][toSquare][gamePhase];
              pMoves->push_back(createMove(fromSquare, toSquare, value));
            };
          }
          else { // king and knight cannot be blocked
            // value is the positional value of the piece at this gamephase
            const Value value =
              static_cast<Value>(10000) - Values::posValue[piece][toSquare][gamePhase];
            pMoves->push_back(createMove(fromSquare, toSquare, value));
          }
        }
      }
    }
  }
}

void
MoveGenerator::generateCastling(GenMode genMode, const Position *pPosition, MoveList *pMoves) {
  const Color nextPlayer = pPosition->getNextPlayer();
  const Bitboard occupiedBB = pPosition->getOccupiedBB();

  // castling - pseudo castling - we will not check if we are in check after the move
  // or if we have passed an attacked square with the king or if the king has been in check

  if ((genMode & GENNONCAP) && pPosition->getCastlingRights()) {

    const CastlingRights cr = pPosition->getCastlingRights();
    if (nextPlayer == WHITE) { // white
      if (cr == WHITE_OO) {
        assert(pPosition->getKingSquare(WHITE) == SQ_E1);
        assert(pPosition->getPiece(SQ_H1) == WHITE_ROOK);
        // way is free
        if (!(Bitboards::intermediateBB[SQ_E1][SQ_H1] & occupiedBB)) {
          pMoves->push_back(createMove<CASTLING>(SQ_E1, SQ_G1, static_cast<Value>(9500)));
        }
      }
      if (cr == WHITE_OOO) {
        assert(pPosition->getKingSquare(WHITE) == SQ_E1);
        assert(pPosition->getPiece(SQ_A1) == WHITE_ROOK);
        // way is free
        if (!(Bitboards::intermediateBB[SQ_E1][SQ_A1] & occupiedBB)) {
          pMoves->push_back(createMove<CASTLING>(SQ_E1, SQ_C1, static_cast<Value>(9500)));
        }
      }
    }
    else { // black
      if (cr == BLACK_OO) {
        assert(pPosition->getKingSquare(BLACK) == SQ_E8);
        assert(pPosition->getPiece(SQ_H8) == BLACK_ROOK);
        // way is free
        if (!(Bitboards::intermediateBB[SQ_E8][SQ_H8] & occupiedBB)) {
          pMoves->push_back(createMove<CASTLING>(SQ_E8, SQ_G8, static_cast<Value>(9500)));
        }
      }
      if (cr == BLACK_OOO) {
        assert(pPosition->getKingSquare(BLACK) == SQ_E8);
        assert(pPosition->getPiece(SQ_A8) == BLACK_ROOK);
        // way is free
        if (!(Bitboards::intermediateBB[SQ_E8][SQ_A8] & occupiedBB)) {
          pMoves->push_back(createMove<CASTLING>(SQ_E8, SQ_C8, static_cast<Value>(9500)));
        }
      }
    }
  }
}

