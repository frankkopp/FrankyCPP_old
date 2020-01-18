/*
 * MIT License
 *
 * Copyright (c) 2018-2020 Frank Kopp
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

////////////////////////////////////////////////
///// CONSTRUCTORS

MoveGenerator::MoveGenerator() = default;
MoveGenerator::~MoveGenerator() = default;

////////////////////////////////////////////////
///// PUBLIC


template<MoveGenerator::GenMode GM>
const MoveList* MoveGenerator::generatePseudoLegalMoves(const Position &position) {
  pseudoLegalMoves.clear();
  generatePawnMoves<GM>(position, &pseudoLegalMoves);
  generateCastling<GM>(position, &pseudoLegalMoves);
  generateMoves<GM>(position, &pseudoLegalMoves);
  generateKingMoves<GM>(position, &pseudoLegalMoves);
  stable_sort(pseudoLegalMoves.begin(), pseudoLegalMoves.end());
  return &pseudoLegalMoves;
}


template<MoveGenerator::GenMode GM>
const MoveList* MoveGenerator::generateLegalMoves(const Position &position) {
  legalMoves.clear();
  generatePseudoLegalMoves<GM>(position);
  for (Move m : pseudoLegalMoves) if (position.isLegalMove(m)) legalMoves.push_back(m);
  return &legalMoves;
}

template<MoveGenerator::GenMode GM>
Move MoveGenerator::getNextPseudoLegalMove(const Position &position) {
  // if the position changes during iteration the iteration will be reset and
  // generation will be restart with the new position.
  if (position.getZobristKey() != currentIteratorKey) {
    onDemandMoves.clear();
    currentODStage = OD_NEW;
    currentIteratorKey = position.getZobristKey();
  }

  bool pvIsCapture = false;

  /*
   * If the list is currently empty and we have not generated all moves yet
   * generate the next batch until we have new moves or all moves are generated
   * and there are no more moves to generate
   */
  while (onDemandMoves.empty() && currentODStage < OD_END) {
    switch (currentODStage) {
      case OD_NEW:
        currentODStage = PV;
        // fall through
      case PV:
        /*
         * If a pvMove is set we return it first and filter it out with each
         * successive move gen stage.
         */
        if (pvMove) {
          pvIsCapture = position.isCapturingMove(pvMove);
          if (GM == GENALL|| (GM == GENCAP && pvIsCapture) || (GM == GENNONCAP && !pvIsCapture)) {
            onDemandMoves.push_back(pvMove);
          }
        }
        currentODStage = OD1;
        break;
      case OD1: // capture
        generatePawnMoves<GENCAP>(position, &onDemandMoves);
        if (pvMove && pvIsCapture) filterPV(onDemandMoves);
        stable_sort(onDemandMoves.begin(), onDemandMoves.end());
        currentODStage = OD2;
        break;
      case OD2:
        generateMoves<GENCAP>(position, &onDemandMoves);
        if (pvMove && pvIsCapture) filterPV(onDemandMoves);
        stable_sort(onDemandMoves.begin(), onDemandMoves.end());
        currentODStage = OD3;
        break;
      case OD3:
        generateKingMoves<GENCAP>(position, &onDemandMoves);
        if (pvMove && pvIsCapture) filterPV(onDemandMoves);
        stable_sort(onDemandMoves.begin(), onDemandMoves.end());
        if (GM & GENNONCAP) { currentODStage = OD5; }
        else { currentODStage = OD_END; }
        break;
      case OD4:
      case OD5: // non capture
        generatePawnMoves<GENNONCAP>(position, &onDemandMoves);
        if (pvMove && !pvIsCapture) filterPV(onDemandMoves);
        stable_sort(onDemandMoves.begin(), onDemandMoves.end());
        /*
         * When killer moves are set we push them to the top of the list
         * after sorting in each stage.
         * Killer may only be returned if they actually are valid moves
         * in this position which we can't know as Killers are stored
         * for the whole ply. Obviously checking if the killer move is valid
         * is expensive (part of a whole move generation) so we only re-sort
         * them to the top once they are actually generated.
         */
        pushKiller(onDemandMoves);
        currentODStage = OD6;
        break;
      case OD6:
        generateCastling<GENNONCAP>(position, &onDemandMoves);
        if (pvMove && !pvIsCapture) filterPV(onDemandMoves);
        stable_sort(onDemandMoves.begin(), onDemandMoves.end());
        pushKiller(onDemandMoves);
        currentODStage = OD7;
        break;
      case OD7:
        generateMoves<GENNONCAP>(position, &onDemandMoves);
        if (pvMove && !pvIsCapture) filterPV(onDemandMoves);
        stable_sort(onDemandMoves.begin(), onDemandMoves.end());
        pushKiller(onDemandMoves);
        currentODStage = OD8;
        break;
      case OD8:
        generateKingMoves<GENNONCAP>(position, &onDemandMoves);
        if (pvMove && !pvIsCapture) filterPV(onDemandMoves);
        stable_sort(onDemandMoves.begin(), onDemandMoves.end());
        pushKiller(onDemandMoves);
        currentODStage = OD_END;
        break;
      case OD_END:
        break;
    }
  }
  // return a move and delete it form the list
  if (onDemandMoves.empty()) {
    return MOVE_NONE;
  }
  else {
    const Move move = onDemandMoves.front();
    onDemandMoves.pop_front();
    return move;
  }
}

void MoveGenerator::reset() {
  pseudoLegalMoves.clear();
  legalMoves.clear();
  onDemandMoves.clear();
  currentODStage = OD_NEW;
  currentIteratorKey = 0;
  pvMove = MOVE_NONE;
  killerMoves.clear();
}

void MoveGenerator::resetOnDemand() {
  onDemandMoves.clear();
  currentODStage = OD_NEW;
  currentIteratorKey = 0;
  pvMove = MOVE_NONE;
  killerMoves.clear();
}

void MoveGenerator::storeKiller(const Move move, const int maxKillers) {
  maxNumberOfKiller = maxKillers;
  // only store if not already in list
  if (std::find(killerMoves.begin(), killerMoves.end(), move) == killerMoves.end()) {
    killerMoves.push_front(move);
    if (killerMoves.size() > maxNumberOfKiller) killerMoves.pop_back();
  }
}

inline void MoveGenerator::pushKiller(MoveList &list) {
  for (auto k : killerMoves) {
    // Find the move in the list. If move not found ignore killer.
    // Otherwise move element to the front. 
    const auto element = std::find(list.begin(), list.end(), k);
    if (element != list.end()) {
      const Move tmp = *element;
      list.erase(element);
      list.push_front(tmp);
    }
  }
}

inline void MoveGenerator::filterPV(MoveList &moveList) {
  moveList.erase(std::remove_if(moveList.begin(), moveList.end(), [&](Move m) {
    return (moveOf(m) == pvMove);
  }), moveList.end());
}

void MoveGenerator::setPV(Move move) {
  pvMove = moveOf(move);
}

bool MoveGenerator::hasLegalMove(const Position &position) {

  /*
  To determine if we have at least one legal move we only have to find
  one legal move. We search for any KING, PAWN, KNIGHT, BISHOP, ROOK, QUEEN move
  and return immediately if we found one.
  The order of our search is from approx. the most likely to the least likely
  */

  const Color nextPlayer = position.getNextPlayer();
  const Bitboard occupiedBB = position.getOccupiedBB();
  const Bitboard nextPlayerBB = position.getOccupiedBB(nextPlayer);
  const Bitboard opponentBB = position.getOccupiedBB(~nextPlayer);
  const Bitboard myPawns = position.getPieceBB(nextPlayer, PAWN);

  // KING
  const Square kingSquare = position.getKingSquare(nextPlayer);
  Bitboard tmpMoves = Bitboards::pseudoAttacks[KING][kingSquare] & ~nextPlayerBB;
  while (tmpMoves) {
    const Square toSquare = Bitboards::popLSB(tmpMoves);
    if (position.isLegalMove(createMove(kingSquare, toSquare))) return true;
  }

  // PAWN
  // normal pawn captures to the west - promotions first
  tmpMoves = Bitboards::shift(pawnDir[nextPlayer] + WEST, myPawns) & opponentBB;
  while (tmpMoves) {
    const Square toSquare = Bitboards::popLSB(tmpMoves);
    const Square fromSquare = toSquare + pawnDir[~nextPlayer] + EAST;
    if (position.isLegalMove(createMove(fromSquare, toSquare))) return true;
  }

  // normal pawn captures to the east - promotions first
  tmpMoves = Bitboards::shift(pawnDir[nextPlayer] + EAST, myPawns) & opponentBB;
  while (tmpMoves) {
    const Square toSquare = Bitboards::popLSB(tmpMoves);
    const Square fromSquare = toSquare + pawnDir[~nextPlayer] + WEST;
    if (position.isLegalMove(createMove(fromSquare, toSquare))) return true;
  }

  // pawn pushes - check step one to unoccupied squares
  tmpMoves = Bitboards::shift(pawnDir[nextPlayer], myPawns) & ~position.getOccupiedBB();
  // double pawn steps
  Bitboard tmpMoves2 = Bitboards::shift(pawnDir[nextPlayer], tmpMoves & (nextPlayer == WHITE
                                                                         ? Bitboards::Rank3BB
                                                                         : Bitboards::Rank6BB)) &
                       ~position.getOccupiedBB();
  while (tmpMoves) {
    const Square toSquare = Bitboards::popLSB(tmpMoves);
    const Square fromSquare = toSquare + pawnDir[~nextPlayer];
    if (position.isLegalMove(createMove(fromSquare, toSquare))) return true;
  }
  while (tmpMoves2) {
    const Square toSquare = Bitboards::popLSB(tmpMoves2);
    const Square fromSquare = toSquare + pawnDir[~nextPlayer] + pawnDir[~nextPlayer];
    if (position.isLegalMove(createMove(fromSquare, toSquare))) return true;
  }

  // en passant captures
  const Square enPassantSquare = position.getEnPassantSquare();
  if (enPassantSquare != SQ_NONE) {
    // left
    tmpMoves =
      Bitboards::shift(pawnDir[~nextPlayer] + WEST, Bitboards::squareBB[enPassantSquare]) & myPawns;
    if (tmpMoves) {
      const Square fromSquare = Bitboards::lsb(tmpMoves);
      if (position.isLegalMove(
        createMove<ENPASSANT>(fromSquare, fromSquare + pawnDir[nextPlayer] + EAST))) {
        return true;
      }
    }
    // right
    tmpMoves =
      Bitboards::shift(pawnDir[~nextPlayer] + EAST, Bitboards::squareBB[enPassantSquare]) & myPawns;
    if (tmpMoves) {
      const Square fromSquare = Bitboards::lsb(tmpMoves);
      if (position.isLegalMove(
        createMove<ENPASSANT>(fromSquare, fromSquare + pawnDir[nextPlayer] + WEST))) {
        return true;
      }
    }
  }

  // OFFICERS
  for (PieceType pt = KNIGHT; pt <= QUEEN; ++pt) {
    Bitboard pieces = position.getPieceBB(nextPlayer, pt);
    while (pieces) {
      const Square fromSquare = Bitboards::popLSB(pieces);
      const Bitboard pseudoMoves = Bitboards::pseudoAttacks[pt][fromSquare];

      Bitboard moves = pseudoMoves & ~nextPlayerBB;
      while (moves) {
        const Square toSquare = Bitboards::popLSB(moves);

        if (pt > KNIGHT) { // sliding pieces
          if (!(Bitboards::intermediateBB[fromSquare][toSquare] & occupiedBB)) {
            if (position.isLegalMove(createMove(fromSquare, toSquare))) return true;
          }
        }
        else { // knight cannot be blocked
          if (position.isLegalMove(createMove(fromSquare, toSquare))) return true;
        }
      }
    }
  }

  // no move found
  return false;
}

bool MoveGenerator::validateMove(const Position &position, const Move move) {
  const Move moveOf1 = moveOf(move);
  if (!moveOf1) return false;
  const MoveList* lm = generateLegalMoves<GENALL>(position);
  return std::find_if(lm->begin(), lm->end(), [&](
    Move m) { return (moveOf1 == moveOf(m)); }) != lm->end();
}

////////////////////////////////////////////////
///// PRIVATE

template<MoveGenerator::GenMode GM>
void MoveGenerator::generatePawnMoves(const Position &position, MoveList* const pMoves) {

  const Color nextPlayer = position.getNextPlayer();
  const Bitboard myPawns = position.getPieceBB(nextPlayer, PAWN);
  const Bitboard oppPieces = position.getOccupiedBB(~nextPlayer);

  const Piece piece = makePiece(nextPlayer, PAWN);
  const int gamePhase = position.getGamePhase();

  // captures
  if (GM == GENCAP || GM == GENALL) {

    /*
    This algorithm shifts the own pawn bitboard in the direction of pawn captures
    and ANDs it with the opponents pieces. With this we get all possible captures
    and can easily create the moves by using a loop over all captures and using
    the backward shift for the from-Square.
    All moves get stable_sort values so that stable_sort order should be:
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
        const Square toSquare = Bitboards::popLSB(promCaptures);
        const Square fromSquare = toSquare + pawnDir[~nextPlayer] - dir;
        // value is the delta of values from the two pieces involved minus the promotion value
        const Value value =
          valueOf(position.getPiece(fromSquare)) - valueOf(position.getPiece(toSquare)) -
          Values::posValue[piece][toSquare][gamePhase];
        pMoves->push_back(
          createMove<PROMOTION>(fromSquare, toSquare, value - valueOf(QUEEN), QUEEN));
        pMoves->push_back(createMove<PROMOTION>(fromSquare, toSquare,
                                                value - valueOf(ROOK) + static_cast<Value>(2000),
                                                ROOK));
        pMoves->push_back(createMove<PROMOTION>(fromSquare, toSquare,
                                                value - valueOf(BISHOP) + static_cast<Value>(2000),
                                                BISHOP));
        pMoves->push_back(
          createMove<PROMOTION>(fromSquare, toSquare, value - valueOf(KNIGHT), KNIGHT));
      }
      tmpCaptures &= ~Bitboards::promotionRank[nextPlayer];
      while (tmpCaptures) {
        const Square toSquare = Bitboards::popLSB(tmpCaptures);
        const Square fromSquare = toSquare + pawnDir[~nextPlayer] - dir;
        // value is the delta of values from the two pieces involved
        const Value value =
          valueOf(position.getPiece(fromSquare)) - valueOf(position.getPiece(toSquare)) -
          Values::posValue[piece][toSquare][gamePhase];
        pMoves->push_back(createMove(fromSquare, toSquare, value));
      }
    }

    // en passant captures
    const Square enPassantSquare = position.getEnPassantSquare();
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
  if (GM == GENNONCAP || GM == GENALL) {

    /*
    Move my pawns forward one step and keep all on not occupied squares
    Move pawns now on rank 3 (rank 6) another square forward to check for pawn doubles.
    Loop over pawns remaining on unoccupied squares and add moves.
     */

    // pawns - check step one to unoccupied squares
    Bitboard tmpMoves = Bitboards::shift(pawnDir[nextPlayer], myPawns) & ~position.getOccupiedBB();
    // pawns double - check step two to unoccupied squares
    Bitboard tmpMovesDouble = Bitboards::shift(pawnDir[nextPlayer], tmpMoves & (nextPlayer == WHITE
                                                                                ? Bitboards::Rank3BB
                                                                                : Bitboards::Rank6BB)) &
                              ~position.getOccupiedBB();

    // single pawn steps - promotions first
    Bitboard promMoves = tmpMoves & Bitboards::promotionRank[nextPlayer];
    while (promMoves) {
      const Square toSquare = Bitboards::popLSB(promMoves);
      const Square fromSquare = toSquare + pawnDir[~nextPlayer];
      // value is done manually for stable_sorting of queen prom first, then knight and others
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
      const Square toSquare = Bitboards::popLSB(tmpMovesDouble);
      // value is the positional value of the piece at this gamephase
      const Value value1 = Values::posValue[piece][toSquare][gamePhase];
      const auto value = static_cast<Value>(10000) - value1;
      pMoves->push_back(createMove(toSquare + 2 * pawnDir[~nextPlayer], toSquare, value));
    }
    // normal single pawn steps
    tmpMoves = tmpMoves & ~Bitboards::promotionRank[nextPlayer];
    while (tmpMoves) {
      const Square toSquare = Bitboards::popLSB(tmpMoves);
      const Square fromSquare = toSquare + pawnDir[~nextPlayer];
      // value is the positional value of the piece at this gamephase
      const Value value = static_cast<Value>(10000) - Values::posValue[piece][toSquare][gamePhase];
      pMoves->push_back(createMove(fromSquare, toSquare, value));
    }
  }
}

template<MoveGenerator::GenMode GM>
void MoveGenerator::generateKingMoves(const Position &position, MoveList* const pMoves) {
  const Color nextPlayer = position.getNextPlayer();
  const Bitboard occupiedBB = position.getOccupiedBB();
  const Bitboard opponentBB = position.getOccupiedBB(~nextPlayer);

  const Piece piece = makePiece(nextPlayer, KING);
  const int gamePhase = position.getGamePhase();


  Bitboard pieces = position.getPieceBB(nextPlayer, KING);
  assert(Bitboards::popcount(pieces) == 1 && "More than one king not allowed!");

  const Square fromSquare = Bitboards::popLSB(pieces);
  const Bitboard pseudoMoves = Bitboards::pseudoAttacks[KING][fromSquare];

  // captures
  if (GM == GENCAP || GM == GENALL) {
    Bitboard captures = pseudoMoves & opponentBB;
    while (captures) {
      const Square toSquare = Bitboards::popLSB(captures);
      // value is the positional value of the piece at this gamephase minus the
      // value of the captured piece
      const Value value =
        Values::posValue[piece][toSquare][gamePhase] - valueOf(position.getPiece(toSquare)) -
        Values::posValue[piece][toSquare][gamePhase];
      pMoves->push_back(createMove(fromSquare, toSquare, value));
    }
  }

  // non captures
  if (GM == GENNONCAP || GM == GENALL) {
    Bitboard nonCaptures = pseudoMoves & ~occupiedBB;
    while (nonCaptures) {
      const Square toSquare = Bitboards::popLSB(nonCaptures);
      // value is the positional value of the piece at this gamephase
      const Value value = static_cast<Value>(10000) - Values::posValue[piece][toSquare][gamePhase];
      pMoves->push_back(createMove(fromSquare, toSquare, value));
    }
  }
}

template<MoveGenerator::GenMode GM>
void MoveGenerator::generateMoves(const Position &position, MoveList* const pMoves) {
  const Color nextPlayer = position.getNextPlayer();
  const Bitboard occupiedBB = position.getOccupiedBB();
  const Bitboard opponentBB = position.getOccupiedBB(~nextPlayer);
  const int gamePhase = position.getGamePhase();

  for (PieceType pt = KNIGHT; pt <= QUEEN; ++pt) {
    Bitboard pieces = position.getPieceBB(nextPlayer, pt);
    const Piece piece = makePiece(nextPlayer, pt);

    while (pieces) {
      const Square fromSquare = Bitboards::popLSB(pieces);
      const Bitboard pseudoMoves = Bitboards::pseudoAttacks[pt][fromSquare];

      // captures
      if (GM == GENCAP || GM == GENALL) {
        Bitboard captures = pseudoMoves & opponentBB;
        while (captures) {
          const Square toSquare = Bitboards::popLSB(captures);
          if (pt > KNIGHT) { // sliding pieces
            if (!(Bitboards::intermediateBB[fromSquare][toSquare] & occupiedBB)) {
              // value is the delta of values from the two pieces involved
              const Value value =
                valueOf(position.getPiece(fromSquare)) - valueOf(position.getPiece(toSquare)) -
                Values::posValue[piece][toSquare][gamePhase];
              pMoves->push_back(createMove(fromSquare, toSquare, value));
            }
          }
          else { // king and knight cannot be blocked
            // value is the delta of values from the two pieces involved
            const Value value =
              valueOf(position.getPiece(fromSquare)) - valueOf(position.getPiece(toSquare)) -
              Values::posValue[piece][toSquare][gamePhase];
            pMoves->push_back(createMove(fromSquare, toSquare, value));
          }
        }
      }

      // non captures
      if (GM == GENNONCAP || GM == GENALL) {
        Bitboard nonCaptures = pseudoMoves & ~occupiedBB;
        while (nonCaptures) {
          const Square toSquare = Bitboards::popLSB(nonCaptures);
          if (pt > KNIGHT) { // sliding pieces
            if (!(Bitboards::intermediateBB[fromSquare][toSquare] & occupiedBB)) {
              // value is the positional value of the piece at this gamephase
              const Value value =
                static_cast<Value>(10000) - Values::posValue[piece][toSquare][gamePhase];
              pMoves->push_back(createMove(fromSquare, toSquare, value));
            }
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

template<MoveGenerator::GenMode GM>
void MoveGenerator::generateCastling(const Position &position, MoveList* const pMoves) {
  const Color nextPlayer = position.getNextPlayer();
  const Bitboard occupiedBB = position.getOccupiedBB();

  // castling - pseudo castling - we will not check if we are in check after the move
  // or if we have passed an attacked square with the king or if the king has been in check

  if ((GM == GENNONCAP || GM == GENALL) && position.getCastlingRights()) {

    const CastlingRights cr = position.getCastlingRights();
    if (nextPlayer == WHITE) { // white
      if (cr == WHITE_OO) {
        assert(position.getKingSquare(WHITE) == SQ_E1);
        assert(position.getPiece(SQ_H1) == WHITE_ROOK);
        // way is free
        if (!(Bitboards::intermediateBB[SQ_E1][SQ_H1] & occupiedBB)) {
          pMoves->push_back(createMove<CASTLING>(SQ_E1, SQ_G1, static_cast<Value>(9500)));
        }
      }
      if (cr == WHITE_OOO) {
        assert(position.getKingSquare(WHITE) == SQ_E1);
        assert(position.getPiece(SQ_A1) == WHITE_ROOK);
        // way is free
        if (!(Bitboards::intermediateBB[SQ_E1][SQ_A1] & occupiedBB)) {
          pMoves->push_back(createMove<CASTLING>(SQ_E1, SQ_C1, static_cast<Value>(9500)));
        }
      }
    }
    else { // black
      if (cr == BLACK_OO) {
        assert(position.getKingSquare(BLACK) == SQ_E8);
        assert(position.getPiece(SQ_H8) == BLACK_ROOK);
        // way is free
        if (!(Bitboards::intermediateBB[SQ_E8][SQ_H8] & occupiedBB)) {
          pMoves->push_back(createMove<CASTLING>(SQ_E8, SQ_G8, static_cast<Value>(9500)));
        }
      }
      if (cr == BLACK_OOO) {
        assert(position.getKingSquare(BLACK) == SQ_E8);
        assert(position.getPiece(SQ_A8) == BLACK_ROOK);
        // way is free
        if (!(Bitboards::intermediateBB[SQ_E8][SQ_A8] & occupiedBB)) {
          pMoves->push_back(createMove<CASTLING>(SQ_E8, SQ_C8, static_cast<Value>(9500)));
        }
      }
    }
  }
}

// @formatter:off

// explicitly instantiate all template definitions so other classes can see them
template const MoveList* MoveGenerator::generatePseudoLegalMoves<MoveGenerator::GENCAP>(const Position &position);
template const MoveList* MoveGenerator::generatePseudoLegalMoves<MoveGenerator::GENNONCAP>(const Position &position);
template const MoveList* MoveGenerator::generatePseudoLegalMoves<MoveGenerator::GENALL>(const Position &position);

template const MoveList* MoveGenerator::generateLegalMoves<MoveGenerator::GENCAP>(const Position &position);
template const MoveList* MoveGenerator::generateLegalMoves<MoveGenerator::GENNONCAP>(const Position &position);
template const MoveList* MoveGenerator::generateLegalMoves<MoveGenerator::GENALL>(const Position &position);

template Move MoveGenerator::getNextPseudoLegalMove<MoveGenerator::GENCAP>(const Position &position);
template Move MoveGenerator::getNextPseudoLegalMove<MoveGenerator::GENNONCAP>(const Position &position);
template Move MoveGenerator::getNextPseudoLegalMove<MoveGenerator::GENALL>(const Position &position);

template void MoveGenerator::generatePawnMoves<MoveGenerator::GENCAP>(const Position &position, MoveList* const pMoves);
template void MoveGenerator::generatePawnMoves<MoveGenerator::GENNONCAP>(const Position &position, MoveList* const pMoves);
template void MoveGenerator::generatePawnMoves<MoveGenerator::GENALL>(const Position &position, MoveList* const pMoves);

template void MoveGenerator::generateMoves<MoveGenerator::GENCAP>(const Position &position, MoveList* const pMoves);
template void MoveGenerator::generateMoves<MoveGenerator::GENNONCAP>(const Position &position, MoveList* const pMoves);
template void MoveGenerator::generateMoves<MoveGenerator::GENALL>(const Position &position, MoveList* const pMoves);

template void MoveGenerator::generateKingMoves<MoveGenerator::GENCAP>(const Position &position, MoveList* const pMoves);
template void MoveGenerator::generateKingMoves<MoveGenerator::GENNONCAP>(const Position &position, MoveList* const pMoves);
template void MoveGenerator::generateKingMoves<MoveGenerator::GENALL>(const Position &position, MoveList* const pMoves);

template void MoveGenerator::generateCastling<MoveGenerator::GENCAP>(const Position &position, MoveList* const pMoves);
template void MoveGenerator::generateCastling<MoveGenerator::GENNONCAP>(const Position &position, MoveList* const pMoves);
template void MoveGenerator::generateCastling<MoveGenerator::GENALL>(const Position &position, MoveList* const pMoves);

// @formatter:on
