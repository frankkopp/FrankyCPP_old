/*
 * MIT License
 *
 * Copyright (c) 2019 Frank Kopp
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

#include "Evaluator.h"
#include "EvaluatorConfig.h"
#include "Bitboards.h"
#include "Position.h"

using namespace Bitboards;
using namespace EvaluatorConfig;


Evaluator::Evaluator() = default;

Value Evaluator::evaluate(const Position &position) {

  // Calculations are always from the view of the white player.

  // if not enough material on the board for a win then it is a draw
  if (position.checkInsufficientMaterial()) return VALUE_DRAW;

  // MATERIAL & POSITION
  int value =
    (USE_MATERIAL
     ? position.getMaterial(WHITE) -
       position.getMaterial(BLACK) : 0) * MATERIAL_WEIGHT
    + (USE_POSITION
       ? position.getPosValue(WHITE) -
         position.getPosValue(BLACK) : 0) * POSITION_WEIGHT;

  // TODO: Early exit if score is very high

  // evaluate pawns
  if (USE_PAWNEVAL) {
    // TODO: pawn cache?
    value += evaluatePawn<WHITE>(position) - evaluatePawn<BLACK>(position);
  }

  // evaluate pieces
  // @formatter:off
  value += evaluatePiece<WHITE, KNIGHT>(position) - evaluatePiece<BLACK, KNIGHT>(position);
  value += evaluatePiece<WHITE, BISHOP>(position) - evaluatePiece<BLACK, BISHOP>(position);
  value += evaluatePiece<WHITE, ROOK  >(position) - evaluatePiece<BLACK, ROOK  >(position);
  value += evaluatePiece<WHITE, QUEEN >(position) - evaluatePiece<BLACK, QUEEN >(position);
  // @formatter:on

  // evaluate king
  value += evaluateKing<WHITE>(position) - evaluateKing<BLACK>(position);

  // value is always from the view of the next player
  if (position.getNextPlayer() == BLACK) value *= -1;
  return static_cast<Value>(value);
}

template<Color C>
int Evaluator::evaluatePawn(const Position &position) {
  const Bitboard myPawns = position.getPieceBB(C, PAWN);
  const Bitboard oppPawns = position.getPieceBB(~C, PAWN);
  // evals inspired by Stockfish

  int value = 0;

  Bitboard isolated = EMPTY_BB;
  Bitboard doubled = EMPTY_BB; // both pawns are counted
  Bitboard passed = EMPTY_BB;
  Bitboard blocked = EMPTY_BB;
  Bitboard phalanx = EMPTY_BB; // both pawns are counted
  Bitboard supported = EMPTY_BB;

  // LOOP through all pawns of this color and type
  Bitboard pawns = myPawns;
  while (pawns) {
    const Square sq = popLSB(pawns);
    const Bitboard neighbours = myPawns & neighbourFilesMask[sq];
    // isolated pawns
    isolated |= neighbours ? EMPTY_BB : squareBB[sq];
    // doubled pawns - any other of my pawns on same file
    doubled |= ~squareBB[sq] & myPawns & fileBB(sq);
    // passed pawns - no opponent pawns in the area before me and no own pawn before me
    passed |= ((myPawns & fileBB(sq)) | oppPawns) & passedPawnMask[C][sq] ? EMPTY_BB : squareBB[sq];
    // blocked pawns
    blocked |= ((myPawns & fileBB(sq)) | oppPawns) & rays[C == WHITE ? N : S][sq] ? squareBB[sq] : EMPTY_BB;
    // pawns as neighbours in a row = phalanx
    phalanx |= myPawns & neighbours & rankBB(sq);
    // pawn as neighbours in the row forward = supported pawns
    supported |= myPawns & neighbours & rankBB(sq + (C == WHITE ? NORTH : SOUTH));
  }
  /*
  fprintln("{} isolated : {} {}\n{}", C == WHITE ? "WHITE" : "BLACK", popcount(isolated), printFlat(isolated), print(isolated));
  fprintln("{} doubled  : {} {}\n{}", C == WHITE ? "WHITE" : "BLACK", popcount(doubled), printFlat(doubled), print(doubled));
  fprintln("{} passed   : {} {}\n{}", C == WHITE ? "WHITE" : "BLACK", popcount(passed), printFlat(passed), print(passed));
  fprintln("{} blocked  : {} {}\n{}", C == WHITE ? "WHITE" : "BLACK", popcount(blocked), printFlat(blocked), print(blocked));
  fprintln("{} phalanx  : {} {}\n{}", C == WHITE ? "WHITE" : "BLACK", popcount(phalanx), printFlat(phalanx), print(phalanx));
  fprintln("{} supported: {} {}\n{}", C == WHITE ? "WHITE" : "BLACK", popcount(supported), printFlat(supported), print(supported));
  */
  // to mitigate rounding errors we scale the calculation and un-scale it again
  // at the end.
  const int scale = 100;
  const double gamePhaseFactor = position.getGamePhaseFactor() * scale;
  const double revGamePhaseFactor = 1.0 - gamePhaseFactor;
  value += popcount(isolated)
           * static_cast<int>((gamePhaseFactor * ISOLATED_PAWN_MID_WEIGHT)
                              * (revGamePhaseFactor * ISOLATED_PAWN_END_WEIGHT));
  value += (popcount(doubled)
            * static_cast<int>((gamePhaseFactor * DOUBLED_PAWN_MID_WEIGHT)
                               * (revGamePhaseFactor * DOUBLED_PAWN_END_WEIGHT))
           ) / 2; // doubled are counted twice
  value += popcount(passed)
           * static_cast<int>((gamePhaseFactor * PASSED_PAWN_MID_WEIGHT)
                              * (revGamePhaseFactor * PASSED_PAWN_END_WEIGHT));
  value += popcount(blocked)
           * static_cast<int>((gamePhaseFactor * BLOCKED_PAWN_MID_WEIGHT)
                              * (revGamePhaseFactor * BLOCKED_PAWN_END_WEIGHT));
  value += (popcount(phalanx)
            * static_cast<int>((gamePhaseFactor * PHALANX_PAWN_MID_WEIGHT)
                               * (revGamePhaseFactor * PHALANX_PAWN_END_WEIGHT))
           ) / 2; // phalanx are counted twice
  value += popcount(supported)
           * static_cast<int>((gamePhaseFactor * SUPPORTED_PAWN_MID_WEIGHT)
                              * (revGamePhaseFactor * SUPPORTED_PAWN_END_WEIGHT));
  value /= scale * scale;
  return value;
}

template<Color C, PieceType PT>
int Evaluator::evaluatePiece(const Position &position) {
  assert (PT != PAWN && PT != KING);
  const Bitboard occupiedBB = position.getOccupiedBB();
  const Bitboard myPiecesBB = position.getOccupiedBB(C);
  int mobility = 0;
  // LOOP through all piece of this color and type
  Bitboard pieces = position.getPieceBB(C, PT);
  while (pieces) {
    const Square fromSquare = Bitboards::popLSB(pieces);
    // TODO: ADD EVALS
    // MOBILITY
    if (USE_MOBILITY) {
      const Bitboard pseudoMoves = Bitboards::pseudoAttacks[PT][fromSquare];
      if (PT == KNIGHT) {
        // knights can't be blocked
        Bitboard moves = pseudoMoves & ~myPiecesBB;
        mobility += Bitboards::popcount(moves);
      }
      else { // sliding pieces
        Bitboard pseudoTo = pseudoMoves & ~myPiecesBB;
        while (pseudoTo) {
          const Square toSquare = Bitboards::popLSB(pseudoTo);
          if (USE_MOBILITY) {
            if (!(Bitboards::intermediateBB[fromSquare][toSquare] & occupiedBB)) {
              mobility += 1;
            }
          }
        }
      }
    }
  }
  return mobility * MOBILITY_WEIGHT;
}

template<Color C>
int Evaluator::evaluateKing(const Position &position) {
  return 0;
}

// explicitly instantiate all template definitions so other classes can see them
template int Evaluator::evaluatePiece<Color::WHITE, PieceType::KNIGHT>(const Position &position);
template int Evaluator::evaluatePiece<Color::WHITE, PieceType::BISHOP>(const Position &position);
template int Evaluator::evaluatePiece<Color::WHITE, PieceType::ROOK>(const Position &position);
template int Evaluator::evaluatePiece<Color::WHITE, PieceType::QUEEN>(const Position &position);
template int Evaluator::evaluatePiece<Color::WHITE, PieceType::KING>(const Position &position);
template int Evaluator::evaluatePiece<Color::BLACK, PieceType::KNIGHT>(const Position &position);
template int Evaluator::evaluatePiece<Color::BLACK, PieceType::BISHOP>(const Position &position);
template int Evaluator::evaluatePiece<Color::BLACK, PieceType::ROOK>(const Position &position);
template int Evaluator::evaluatePiece<Color::BLACK, PieceType::QUEEN>(const Position &position);
template int Evaluator::evaluatePiece<Color::BLACK, PieceType::KING>(const Position &position);

