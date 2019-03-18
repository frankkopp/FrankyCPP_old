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

#include "MoveGenerator.h"
#include "Position.h"

using namespace std;
using namespace Bitboards;

////////////////////////////////////////////////
///// CONSTRUCTORS

MoveGenerator::MoveGenerator() {
  pseudoLegalMoves.reserve(256);
  legalMoves.reserve(256);
}

////////////////////////////////////////////////
///// PUBLIC

// TODO:
//  Implement caching when position is known to avoid re-generation
//  Implement on demand (lazy) move gen

MoveList MoveGenerator::generatePseudoLegalMoves(GenMode genMode, Position *pPosition) {
  pseudoLegalMoves.clear();
  generatePawnMoves(genMode, pPosition, &pseudoLegalMoves);
  generateCastling(genMode, pPosition, &pseudoLegalMoves);
  generateMoves(genMode, pPosition, &pseudoLegalMoves);
  generateKingMoves(genMode, pPosition, &pseudoLegalMoves);
  return pseudoLegalMoves;
}

MoveList
MoveGenerator::generateLegalMoves(GenMode genMode, Position *pPosition) {
  legalMoves.clear();
  generatePseudoLegalMoves(genMode, pPosition);
  for (Move m : pseudoLegalMoves) if (pPosition->isLegalMove(m)) legalMoves.push_back(m);
  return legalMoves;
}

bool
MoveGenerator::hasLegalMove(Position *pPosition) {

  // find KING, PAWN, KNIGHT, BISHOP, ROOK, QUEEN move

  const Color nextPlayer = pPosition->getNextPlayer();
  const Bitboard occupiedBB = pPosition->getOccupiedBB();
  const Bitboard nextPlayerBB = pPosition->getOccupiedBB(nextPlayer);
  const Bitboard opponentBB = pPosition->getOccupiedBB(~nextPlayer);
  const Bitboard myPawns = pPosition->getPieceBB(nextPlayer, PAWN);

  // KING
  const Square kingSquare = pPosition->getKingSquare(nextPlayer);
  Bitboard tmpMoves = pseudoAttacks[KING][kingSquare] & ~nextPlayerBB;
  while (tmpMoves) {
    const Square toSquare = popLSB(&tmpMoves);
    if (pPosition->isLegalMove(createMove(kingSquare, toSquare))) return true;
  }

  // PAWN
  // normal pawn captures to the west - promotions first
  tmpMoves = Bitboards::shift(pawnDir[nextPlayer] + WEST, myPawns) & opponentBB;
  while (tmpMoves) {
    const Square toSquare = popLSB(&tmpMoves);
    const Square fromSquare = toSquare + pawnDir[~nextPlayer] + EAST;
    if (pPosition->isLegalMove(createMove(fromSquare, toSquare))) return true;
  }

  // normal pawn captures to the east - promotions first
  tmpMoves = Bitboards::shift(pawnDir[nextPlayer] + EAST, myPawns) & opponentBB;
  while (tmpMoves) {
    const Square toSquare = popLSB(&tmpMoves);
    const Square fromSquare = toSquare + pawnDir[~nextPlayer] + WEST;
    if (pPosition->isLegalMove(createMove(fromSquare, toSquare))) return true;
  }

  // pawns - check step one to unoccupied squares
  tmpMoves = shift(pawnDir[nextPlayer], myPawns) & ~pPosition->getOccupiedBB();
  while (tmpMoves) {
    const Square toSquare = popLSB(&tmpMoves);
    const Square fromSquare = toSquare + pawnDir[~nextPlayer];
    if (pPosition->isLegalMove(createMove(fromSquare, toSquare))) return true;
  }

  // en passant captures
  const Square enPassantSquare = pPosition->getEnPassantSquare();
  if (enPassantSquare != SQ_NONE) {
    // left
    tmpMoves = shift(pawnDir[~nextPlayer] + WEST, squareBB[enPassantSquare]) & myPawns;
    if (tmpMoves) {
      const Square fromSquare = lsb(tmpMoves);
      if (pPosition->isLegalMove(
        createMove<ENPASSANT>(fromSquare, fromSquare + pawnDir[nextPlayer] + EAST)))
        return true;
    }
    // right
    tmpMoves = shift(pawnDir[~nextPlayer] + EAST, squareBB[enPassantSquare]) & myPawns;
    if (tmpMoves) {
      const Square fromSquare = lsb(tmpMoves);
      if (pPosition->isLegalMove(
        createMove<ENPASSANT>(fromSquare, fromSquare + pawnDir[nextPlayer] + WEST))  )
        return true;
    }
  }

  // OFFICERS
  for (PieceType pt = KNIGHT; pt <= QUEEN; ++pt) {
    Bitboard pieces = pPosition->getPieceBB(nextPlayer, pt);

    while (pieces) {
      const Square fromSquare = popLSB(&pieces);
      const Bitboard pseudoMoves = pseudoAttacks[pt][fromSquare];

      Bitboard moves = pseudoMoves & ~nextPlayerBB;
      while (moves) {
        const Square toSquare = popLSB(&moves);
        
        if (pt > KNIGHT) { // sliding pieces
          if (!(intermediateBB[fromSquare][toSquare] & occupiedBB)) {
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

  // captures
  if (genMode & GENCAP) {

    /*
    This algorithm shifts the own pawn bitboard in the direction of pawn captures
    and ANDs it with the opponents pieces. With this we get all possible captures
    and can easily create the moves by using a loop over all captures and using
    the backward shift for the from-Square.
     */

    // normal pawn captures to the west - promotions first
    Bitboard tmpCaptures = Bitboards::shift(pawnDir[nextPlayer] + WEST, myPawns) & oppPieces;
    Bitboard promCaptures = tmpCaptures & promotionRank[nextPlayer];
    while (promCaptures) {
      const Square toSquare = popLSB(&promCaptures);
      const Square fromSquare = toSquare + pawnDir[~nextPlayer] + EAST;
      pMoves->push_back(createMove<PROMOTION>(fromSquare, toSquare, QUEEN));
      pMoves->push_back(createMove<PROMOTION>(fromSquare, toSquare, ROOK));
      pMoves->push_back(createMove<PROMOTION>(fromSquare, toSquare, BISHOP));
      pMoves->push_back(createMove<PROMOTION>(fromSquare, toSquare, KNIGHT));
    }
    tmpCaptures &= ~promotionRank[nextPlayer];
    while (tmpCaptures) {
      const Square toSquare = popLSB(&tmpCaptures);
      const Square fromSquare = toSquare + pawnDir[~nextPlayer] + EAST;
      pMoves->push_back(createMove(fromSquare, toSquare));
    }

    // normal pawn captures to the east - promotions first
    tmpCaptures = Bitboards::shift(pawnDir[nextPlayer] + EAST, myPawns) & oppPieces;
    promCaptures = tmpCaptures & promotionRank[nextPlayer];
    while (promCaptures) {
      const Square toSquare = popLSB(&promCaptures);
      const Square fromSquare = toSquare + pawnDir[~nextPlayer] + WEST;
      pMoves->push_back(createMove<PROMOTION>(fromSquare, toSquare, QUEEN));
      pMoves->push_back(createMove<PROMOTION>(fromSquare, toSquare, ROOK));
      pMoves->push_back(createMove<PROMOTION>(fromSquare, toSquare, BISHOP));
      pMoves->push_back(createMove<PROMOTION>(fromSquare, toSquare, KNIGHT));
    }
    tmpCaptures &= ~promotionRank[nextPlayer];
    while (tmpCaptures) {
      const Square toSquare = popLSB(&tmpCaptures);
      const Square fromSquare = toSquare + pawnDir[~nextPlayer] + WEST;
      pMoves->push_back(createMove(fromSquare, toSquare));
    }

    // en passant captures
    const Square enPassantSquare = pPosition->getEnPassantSquare();
    if (enPassantSquare != SQ_NONE) {
      // left
      tmpCaptures = shift(pawnDir[~nextPlayer] + WEST, squareBB[enPassantSquare]) & myPawns;
      if (tmpCaptures) {
        Square sqx = lsb(tmpCaptures);
        pMoves->push_back(createMove<ENPASSANT>(sqx, sqx + pawnDir[nextPlayer] + EAST));
      }
      // right
      tmpCaptures = shift(pawnDir[~nextPlayer] + EAST, squareBB[enPassantSquare]) & myPawns;
      if (tmpCaptures) {
        Square sqx = lsb(tmpCaptures);
        pMoves->push_back(createMove<ENPASSANT>(sqx, sqx + pawnDir[nextPlayer] + WEST));
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
    Bitboard tmpMoves = shift(pawnDir[nextPlayer], myPawns) & ~pPosition->getOccupiedBB();
    // pawns double - check step two to unoccupied squares
    Bitboard tmpMovesDouble = shift(pawnDir[nextPlayer],
                                    tmpMoves & (nextPlayer == WHITE ? Rank3BB : Rank6BB)) &
                              ~pPosition->getOccupiedBB();

    // single pawn steps - promotions first
    Bitboard promMoves = tmpMoves & promotionRank[nextPlayer];
    while (promMoves) {
      const Square toSquare = popLSB(&promMoves);
      const Square fromSquare = toSquare + pawnDir[~nextPlayer];
      pMoves->push_back(createMove<PROMOTION>(fromSquare, toSquare, QUEEN));
      pMoves->push_back(createMove<PROMOTION>(fromSquare, toSquare, ROOK));
      pMoves->push_back(createMove<PROMOTION>(fromSquare, toSquare, BISHOP));
      pMoves->push_back(createMove<PROMOTION>(fromSquare, toSquare, KNIGHT));
    }
    // double pawn steps
    while (tmpMovesDouble) {
      const Square toSquare = popLSB(&tmpMovesDouble);
      pMoves->push_back(createMove(toSquare + 2 * pawnDir[~nextPlayer], toSquare));
    }
    // normal single pawn steps
    tmpMoves = tmpMoves & ~promotionRank[nextPlayer];
    while (tmpMoves) {
      const Square toSquare = popLSB(&tmpMoves);
      const Square fromSquare = toSquare + pawnDir[~nextPlayer];
      pMoves->push_back(createMove(fromSquare, toSquare));
    }
  }
}

void
MoveGenerator::generateKingMoves(GenMode genMode, const Position *pPosition, MoveList *pMoves) {
  const Color nextPlayer = pPosition->getNextPlayer();
  const Bitboard occupiedBB = pPosition->getOccupiedBB();
  const Bitboard opponentBB = pPosition->getOccupiedBB(~nextPlayer);

  Bitboard pieces = pPosition->getPieceBB(nextPlayer, KING);
  assert(popcount(pieces) == 1 && "More than one king not allowed!");

  const Square fromSquare = popLSB(&pieces);
  const Bitboard pseudoMoves = pseudoAttacks[KING][fromSquare];

  // captures
  if (genMode & GENCAP) {
    Bitboard captures = pseudoMoves & opponentBB;
    while (captures) pMoves->push_back(createMove(fromSquare, popLSB(&captures)));
  }

  // non captures
  if (genMode & GENNONCAP) {
    Bitboard nonCaptures = pseudoMoves & ~occupiedBB;
    while (nonCaptures) pMoves->push_back(createMove(fromSquare, popLSB(&nonCaptures)));
  }
}

void
MoveGenerator::generateMoves(GenMode genMode, const Position *pPosition, MoveList *pMoves) {
  const Color nextPlayer = pPosition->getNextPlayer();
  const Bitboard occupiedBB = pPosition->getOccupiedBB();
  const Bitboard opponentBB = pPosition->getOccupiedBB(~nextPlayer);

  for (PieceType pt = KNIGHT; pt < PIECETYPE_NONE; ++pt) {
    Bitboard pieces = pPosition->getPieceBB(nextPlayer, pt);

    while (pieces) {
      const Square fromSquare = popLSB(&pieces);
      const Bitboard pseudoMoves = pseudoAttacks[pt][fromSquare];

      // captures
      if (genMode & GENCAP) {
        Bitboard captures = pseudoMoves & opponentBB;
        while (captures) {
          const Square targetSquare = popLSB(&captures);
          if (pt > KNIGHT) { // sliding pieces
            if (!(intermediateBB[fromSquare][targetSquare] & occupiedBB)) {
              pMoves->push_back(createMove(fromSquare, targetSquare));
            };
          }
          else { // king and knight cannot be blocked
            pMoves->push_back(createMove(fromSquare, targetSquare));
          }
        }
      }

      // non captures
      if (genMode & GENNONCAP) {
        Bitboard nonCaptures = pseudoMoves & ~occupiedBB;
        while (nonCaptures) {
          const Square toSquare = popLSB(&nonCaptures);
          if (pt > KNIGHT) { // sliding pieces
            if (!(intermediateBB[fromSquare][toSquare] & occupiedBB)) {
              pMoves->push_back(createMove(fromSquare, toSquare));
            };
          }
          else { // king and knight cannot be blocked
            pMoves->push_back(createMove(fromSquare, toSquare));
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
        const Square from = SQ_E1;
        const Square to = SQ_G1;
        const Square rookFrom = SQ_H1;
        // way is free
        if (!(intermediateBB[from][rookFrom] & occupiedBB)) {
          pMoves->push_back(createMove<CASTLING>(from, to));
        }
      }
      if (cr == WHITE_OOO) {
        const Square from = SQ_E1;
        const Square to = SQ_C1;
        const Square rookFrom = SQ_A1;
        // way is free
        if (!(intermediateBB[from][rookFrom] & occupiedBB)) {
          pMoves->push_back(createMove<CASTLING>(from, to));
        }
      }
    }
    else { // black
      if (cr == BLACK_OO) {
        const Square from = SQ_E8;
        const Square to = SQ_G8;
        const Square rookFrom = SQ_H8;
        // way is free
        if (!(intermediateBB[from][rookFrom] & occupiedBB)) {
          pMoves->push_back(createMove<CASTLING>(from, to));
        }
      }
      if (cr == BLACK_OOO) {
        const Square from = SQ_E8;
        const Square to = SQ_C8;
        const Square rookFrom = SQ_A8;
        // way is free
        if (!(intermediateBB[from][rookFrom] & occupiedBB)) {
          pMoves->push_back(createMove<CASTLING>(from, to));
        }
      }
    }
  }
}

