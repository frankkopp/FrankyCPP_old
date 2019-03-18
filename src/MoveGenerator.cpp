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
#include "MoveGenerator.h"

using namespace std;
using namespace Bitboards;

////////////////////////////////////////////////
///// CONSTRUCTORS

MoveGenerator::MoveGenerator() = default;

////////////////////////////////////////////////
///// PUBLIC

vector<Move> MoveGenerator::generatePseudoLegalMoves(GenMode genMode, const Position *position) {
  vector<Move> moves;
  generatePawnMoves(genMode, position, &moves);
  generateCastling(genMode, position, &moves);
  generateMoves(genMode, position, &moves);
  generateKingMoves(genMode, position, &moves);
  return moves;
}

vector<Move>
MoveGenerator::generateLegalMoves(GenMode genMode, Position *position) {
  vector<Move> moves(generatePseudoLegalMoves(genMode, position));
  std::vector<Move> filteredMoves;
  for (Move m : moves) {
    if (position->isLegalMove(m)) filteredMoves.push_back(m);
  }
  moves = filteredMoves;
  return moves;
}

////////////////////////////////////////////////
///// PRIVATE

void
MoveGenerator::generatePawnMoves(GenMode genMode, const Position *position, vector<Move> *moves) {

  const Color nextPlayer = position->getNextPlayer();
  const Bitboard myPawns = position->getPieceBB(nextPlayer, PAWN);
  const Bitboard oppPieces = position->getOccupiedBB(~nextPlayer);

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
      moves->push_back(createMove<PROMOTION>(fromSquare, toSquare, QUEEN));
      moves->push_back(createMove<PROMOTION>(fromSquare, toSquare, ROOK));
      moves->push_back(createMove<PROMOTION>(fromSquare, toSquare, BISHOP));
      moves->push_back(createMove<PROMOTION>(fromSquare, toSquare, KNIGHT));
    }
    tmpCaptures &= ~promotionRank[nextPlayer];
    while (tmpCaptures) {
      const Square toSquare = popLSB(&tmpCaptures);
      const Square fromSquare = toSquare + pawnDir[~nextPlayer] + EAST;
      moves->push_back(createMove(fromSquare, toSquare));
    }

    // normal pawn captures to the east - promotions first
    tmpCaptures = Bitboards::shift(pawnDir[nextPlayer] + EAST, myPawns) & oppPieces;
    promCaptures = tmpCaptures & promotionRank[nextPlayer];
    while (promCaptures) {
      const Square toSquare = popLSB(&promCaptures);
      const Square fromSquare = toSquare + pawnDir[~nextPlayer] + WEST;
      moves->push_back(createMove<PROMOTION>(fromSquare, toSquare, QUEEN));
      moves->push_back(createMove<PROMOTION>(fromSquare, toSquare, ROOK));
      moves->push_back(createMove<PROMOTION>(fromSquare, toSquare, BISHOP));
      moves->push_back(createMove<PROMOTION>(fromSquare, toSquare, KNIGHT));
    }
    tmpCaptures &= ~promotionRank[nextPlayer];
    while (tmpCaptures) {
      const Square toSquare = popLSB(&tmpCaptures);
      const Square fromSquare = toSquare + pawnDir[~nextPlayer] + WEST;
      moves->push_back(createMove(fromSquare, toSquare));
    }

    // en passant captures
    const Square enPassantSquare = position->getEnPassantSquare();
    if (enPassantSquare != SQ_NONE) {
      // left
      const Bitboard shift1 = shift(pawnDir[~nextPlayer] + WEST, squareBB[enPassantSquare]);
      tmpCaptures = shift1 & myPawns;
      if (tmpCaptures) {
        Square sqx = lsb(tmpCaptures);
        moves->push_back(createMove<ENPASSANT>(sqx, sqx + pawnDir[nextPlayer] + EAST));
      }
      // right
      tmpCaptures = shift(pawnDir[~nextPlayer] + EAST, squareBB[enPassantSquare]) & myPawns;
      if (tmpCaptures) {
        Square sqx = lsb(tmpCaptures);
        moves->push_back(createMove<ENPASSANT>(sqx, sqx + pawnDir[nextPlayer] + WEST));
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
    Bitboard tmpMoves = shift(pawnDir[nextPlayer], myPawns) & ~position->getOccupiedBB();
    // pawns double - check step two to unoccupied squares
    Bitboard tmpMovesDouble = shift(pawnDir[nextPlayer],
                                    tmpMoves & (nextPlayer == WHITE ? Rank3BB : Rank6BB)) &
                              ~position->getOccupiedBB();

    // single pawn steps - promotions first
    Bitboard promMoves = tmpMoves & promotionRank[nextPlayer];
    while (promMoves) {
      const Square toSquare = popLSB(&promMoves);
      const Square fromSquare = toSquare + pawnDir[~nextPlayer];
      moves->push_back(createMove<PROMOTION>(fromSquare, toSquare, QUEEN));
      moves->push_back(createMove<PROMOTION>(fromSquare, toSquare, ROOK));
      moves->push_back(createMove<PROMOTION>(fromSquare, toSquare, BISHOP));
      moves->push_back(createMove<PROMOTION>(fromSquare, toSquare, KNIGHT));
    }
    // double pawn steps
    while (tmpMovesDouble) {
      const Square toSquare = popLSB(&tmpMovesDouble);
      moves->push_back(createMove(toSquare + 2 * pawnDir[~nextPlayer], toSquare));
    }
    // normal single pawn steps
    tmpMoves = tmpMoves & ~promotionRank[nextPlayer];
    while (tmpMoves) {
      const Square toSquare = popLSB(&tmpMoves);
      const Square fromSquare = toSquare + pawnDir[~nextPlayer];
      moves->push_back(createMove(fromSquare, toSquare));
    }
  }
}

void
MoveGenerator::generateKingMoves(GenMode genMode, const Position *position, vector<Move> *moves) {
  const Color nextPlayer = position->getNextPlayer();
  const Bitboard occupiedBB = position->getOccupiedBB();
  const Bitboard opponentBB = position->getOccupiedBB(~nextPlayer);

  Bitboard pieces = position->getPieceBB(nextPlayer, KING);
  assert(popcount(pieces) == 1 && "More than one king not allowed!");

  const Square fromSquare = popLSB(&pieces);
  const Bitboard pseudoMoves = pseudoAttacks[KING][fromSquare];

  // captures
  if (genMode & GENCAP) {
    Bitboard captures = pseudoMoves & opponentBB;
    while (captures) moves->push_back(createMove(fromSquare, popLSB(&captures)));
  }

  // non captures
  if (genMode & GENNONCAP) {
    Bitboard nonCaptures = pseudoMoves & ~occupiedBB;
    while (nonCaptures) moves->push_back(createMove(fromSquare, popLSB(&nonCaptures)));
  }
}

void
MoveGenerator::generateMoves(GenMode genMode, const Position *position, vector<Move> *moves) {
  const Color nextPlayer = position->getNextPlayer();
  const Bitboard occupiedBB = position->getOccupiedBB();
  const Bitboard opponentBB = position->getOccupiedBB(~nextPlayer);

  for (PieceType pt = KNIGHT; pt < PIECETYPE_NONE; ++pt) {
    Bitboard pieces = position->getPieceBB(nextPlayer, pt);

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
              moves->push_back(createMove(fromSquare, targetSquare));
            };
          }
          else { // king and knight cannot be blocked
            moves->push_back(createMove(fromSquare, targetSquare));
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
              moves->push_back(createMove(fromSquare, toSquare));
            };
          }
          else { // king and knight cannot be blocked
            moves->push_back(createMove(fromSquare, toSquare));
          }
        }
      }
    }
  }
}

void
MoveGenerator::generateCastling(GenMode genMode, const Position *position, vector<Move> *moves) {
  const Color nextPlayer = position->getNextPlayer();
  const Bitboard occupiedBB = position->getOccupiedBB();

  // castling - pseudo castling - we will not check if we are in check after the move
  // or if we have passed an attacked square with the king

  if ((genMode & GENNONCAP) && position->getCastlingRights()) {
    const CastlingRights cr = position->getCastlingRights();
    if (nextPlayer == WHITE) { // white
      if (cr == WHITE_OO) {
        const Square from = SQ_E1;
        const Square to = SQ_G1;
        const Square rookFrom = SQ_H1;
        // way is free
        if (!(intermediateBB[from][rookFrom] & occupiedBB)) {
          moves->push_back(createMove<CASTLING>(from, to));
        }
      }
      if (cr == WHITE_OOO) {
        const Square from = SQ_E1;
        const Square to = SQ_C1;
        const Square rookFrom = SQ_A1;
        // way is free
        if (!(intermediateBB[from][rookFrom] & occupiedBB)) {
          moves->push_back(createMove<CASTLING>(from, to));
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
          moves->push_back(createMove<CASTLING>(from, to));
        }
      }
      if (cr == BLACK_OOO) {
        const Square from = SQ_E8;
        const Square to = SQ_C8;
        const Square rookFrom = SQ_A8;
        // way is free
        if (!(intermediateBB[from][rookFrom] & occupiedBB)) {
          moves->push_back(createMove<CASTLING>(from, to));
        }
      }
    }
  }
}

