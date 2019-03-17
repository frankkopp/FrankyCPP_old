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

using namespace std;
using namespace Bitboards;

MoveGenerator::MoveGenerator() = default;

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
      const Square fromSquare = toSquare + pawnDir[~nextPlayer] + EAST;
      moves->push_back(createMove(fromSquare, toSquare));
    }

    // en passant captures
    const Square enPassantSquare = position->getEnPassantSquare();
    if (enPassantSquare != SQ_NONE) {
      // left
      tmpCaptures = shift(pawnDir[~nextPlayer] + WEST, squareBB[enPassantSquare]) & myPawns;
      if (tmpCaptures) {
        Square sqx = lsb(tmpCaptures);
        moves->push_back(createMove<ENPASSANT>(sqx, sqx + pawnDir[nextPlayer] + WEST));
      } else {
        // right
        tmpCaptures = shift(pawnDir[~nextPlayer] + EAST, squareBB[enPassantSquare]) & myPawns;
        if (tmpCaptures) {
          Square sqx = lsb(tmpCaptures);
          moves->push_back(createMove<ENPASSANT>(sqx, sqx + pawnDir[nextPlayer] + WEST));
        }
      }
    }
  }

  // captures
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
    tmpMoves = tmpMoves & ~promotionRank[nextPlayer];
    while (tmpMoves) {
      const Square toSquare = popLSB(&tmpMoves);
      const Square fromSquare = toSquare + pawnDir[~nextPlayer];
      moves->push_back(createMove(fromSquare, toSquare));
    }
  }
}


