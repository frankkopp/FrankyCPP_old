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
#include "Bitboards.h"
#include "Position.h"

using namespace Bitboards;

Evaluator::Evaluator(){
  resizePawnTable(config.PAWN_TABLE_SIZE);
}

Evaluator::Evaluator(std::size_t pawnEvalCacheSize) {
  resizePawnTable(pawnEvalCacheSize);
}

void Evaluator::resizePawnTable(std::size_t size) {
  if (config.USE_PAWN_TABLE) {
    pawnTable.resize(size);
    LOG__INFO(LOG, "Evaluator pawn table of size {:.2F} MB created with {:n} entries",
              static_cast<double>(sizeof(Entry) * config.PAWN_TABLE_SIZE) / (1024 * 1024), config.PAWN_TABLE_SIZE);
  }
}

Value Evaluator::evaluate(const Position &position) {
  LOG__TRACE(LOG, "Start eval on {}", position.printFen());

  // Calculations are always from the view of the white player.

  // if not enough material on the board for a win then it is a draw
  if (position.checkInsufficientMaterial()) {
    LOG__TRACE(LOG, "Eval: DRAW for insufficient material on {}", position.printFen());
    return VALUE_DRAW;
  }

  int value = 0;

  // MATERIAL & POSITION
  value = (config.USE_MATERIAL
           ? position.getMaterial(WHITE) -
             position.getMaterial(BLACK) : 0) * config.MATERIAL_WEIGHT;
  LOG__TRACE(LOG, "Eval value after material: {}", value);

  value += (config.USE_POSITION
            ? position.getPosValue(WHITE) -
              position.getPosValue(BLACK) : 0) * config.POSITION_WEIGHT;
  LOG__TRACE(LOG, "Eval value after position: {}", value);


  // TODO: Early exit if score is very high

  // evaluate pawns
  if (config.USE_PAWNEVAL) {
    value += pawnEval(position);
  }
  LOG__TRACE(LOG, "Eval value after pawns: {}", value);


  // evaluate pieces                                                         @formatter:off
  value += evaluatePiece<WHITE, KNIGHT>(position) - evaluatePiece<BLACK, KNIGHT>(position);
  value += evaluatePiece<WHITE, BISHOP>(position) - evaluatePiece<BLACK, BISHOP>(position);
  value += evaluatePiece<WHITE, ROOK  >(position) - evaluatePiece<BLACK, ROOK  >(position);
  value += evaluatePiece<WHITE, QUEEN >(position) - evaluatePiece<BLACK, QUEEN >(position);
  // @formatter:on
  LOG__TRACE(LOG, "Eval value after pieces: {}", value);


  // evaluate king
  value += evaluateKing<WHITE>(position) - evaluateKing<BLACK>(position);
  LOG__TRACE(LOG, "Eval value after king: {}", value);


  // CHECK Bonus: Giving check or being in check has value as it forces evasion moves
  if (config.USE_CHECK_BONUS) {
    value += position.isAttacked(position.getKingSquare(BLACK), WHITE) ? config.CHECK_VALUE : 0;
    value -= position.isAttacked(position.getKingSquare(WHITE), BLACK) ? config.CHECK_VALUE : 0;
  }
  LOG__TRACE(LOG, "Eval value after check bonus: {}", value);

  // value is always from the view of the next player
  if (position.getNextPlayer() == BLACK) value *= -1;

  // TEMPO Bonus for the side to move (helps with evaluation alternation -
  // less difference between side which makes aspiration search faster
  // (not empirically tested)
  value += config.TEMPO;
  LOG__TRACE(LOG, "Eval value after tempo and player adjust: {}", value);

  return static_cast<Value>(value);
}

int Evaluator::pawnEval(const Position &position) {
  const double gamePhaseFactor = position.getGamePhaseFactor();
  const double revGamePhaseFactor = 1.0 - gamePhaseFactor;
  const Bitboard pawnsBitboard = position.getPieceBB(WHITE, PAWN) | position.getPieceBB(BLACK, PAWN);
  Entry* entryPtr;
  int value = 0;

  // Get a pointer to a value entry for pawns
  // Either from the cache or from the default entry.
  if (config.USE_PAWN_TABLE) {
    entryPtr = &pawnTable[getTableIndex(pawnsBitboard)];
    LOG__TRACE(LOG, "Using pawn table on {}", pawnsBitboard);
  }
  else {
    // reset the default entry every time it is used
    defaultEntry = {0, 0, 0};
    entryPtr = &defaultEntry;
    LOG__TRACE(LOG, "Not using pawn table.");
  }
  // we have an entry here - either from the cache or the default entry

  if (entryPtr->pawnBitboard != 0 && entryPtr->pawnBitboard == pawnsBitboard) {
    cacheHits++;
    LOG__TRACE(LOG, "Found cache hit: {}", entryPtr->str());
  }
  else {
    cacheMisses++;
    // we did not find an entry or have the default entry
    if (config.USE_PAWN_TABLE) {
      if (entryPtr->pawnBitboard == 0) {
        cacheEntries++;
      }
      else {
        cacheReplace++;
      }
      // replace entry in cache by overwriting the key (=pawns bitboard)
      entryPtr->pawnBitboard = pawnsBitboard;
      entryPtr->midvalue = 0;
      entryPtr->endvalue = 0;
    }
    // entry values will be overwritten in evaluatePawns and stored in cache or the default entry
    evaluatePawns(position, entryPtr);
    LOG__TRACE(LOG, "Cache miss or no cache. Created cache entry: {}", entryPtr->str());
    LOG__TRACE(LOG, "{:s}", pawnTableStats());
  }

  // we have found a matching entry - add the respective mid and end values
  // for the current game phase
  value += static_cast<int>(entryPtr->midvalue * gamePhaseFactor + entryPtr->endvalue * revGamePhaseFactor);
  LOG__TRACE(LOG, "Game phase adjusted pawn eval results in {} (midvalue={}, endvalue={}, weight={})",
             value * config.PAWNEVAL_WEIGHT,
             entryPtr->midvalue * gamePhaseFactor,
             entryPtr->endvalue * revGamePhaseFactor,
             config.PAWNEVAL_WEIGHT);

  return value * config.PAWNEVAL_WEIGHT;
}

void Evaluator::evaluatePawns(const Position &position, Entry* const entry) {

  // compiler will likely unroll this
  for (Color color = WHITE; color <= BLACK; ++color) {
    const Bitboard myPawns = position.getPieceBB(color, PAWN);
    const Bitboard oppPawns = position.getPieceBB(~color, PAWN);
    // evals inspired by Stockfish

    int midvalue = 0;
    int endvalue = 0;

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
      passed |= ((myPawns & fileBB(sq)) | oppPawns) & passedPawnMask[color][sq] ? EMPTY_BB : squareBB[sq];
      // blocked pawns
      blocked |= ((myPawns & fileBB(sq)) | oppPawns) & rays[color == WHITE ? N : S][sq] ? squareBB[sq] : EMPTY_BB;
      // pawns as neighbours in a row = phalanx
      phalanx |= myPawns & neighbours & rankBB(sq);
      // pawn as neighbours in the row forward = supported pawns
      supported |= myPawns & neighbours & rankBB(sq + (color == WHITE ? NORTH : SOUTH));
    }

    /*
    fprintln("{} isolated : {} {}\n{}", !color ? "WHITE" : "BLACK", popcount(isolated), printFlat(isolated), print(isolated));
    fprintln("{} doubled  : {} {}\n{}", !color ? "WHITE" : "BLACK", popcount(doubled), printFlat(doubled), print(doubled));
    fprintln("{} passed   : {} {}\n{}", !color ? "WHITE" : "BLACK", popcount(passed), printFlat(passed), print(passed));
    fprintln("{} blocked  : {} {}\n{}", !color ? "WHITE" : "BLACK", popcount(blocked), printFlat(blocked), print(blocked));
    fprintln("{} phalanx  : {} {}\n{}", !color ? "WHITE" : "BLACK", popcount(phalanx), printFlat(phalanx), print(phalanx));
    fprintln("{} supported: {} {}\n{}", !color ? "WHITE" : "BLACK", popcount(supported), printFlat(supported), print(supported));
    */

    // @formatter:off
    midvalue +=  popcount(isolated)  * config.ISOLATED_PAWN_MID_WEIGHT ;
    endvalue +=  popcount(isolated)  * config.ISOLATED_PAWN_END_WEIGHT ;
    midvalue += (popcount(doubled)   * config.DOUBLED_PAWN_MID_WEIGHT  )/2;
    endvalue += (popcount(doubled)   * config.DOUBLED_PAWN_END_WEIGHT  )/2;
    midvalue +=  popcount(passed)    * config.PASSED_PAWN_MID_WEIGHT   ;
    endvalue +=  popcount(passed)    * config.PASSED_PAWN_END_WEIGHT   ;
    midvalue +=  popcount(blocked)   * config.BLOCKED_PAWN_MID_WEIGHT  ;
    endvalue +=  popcount(blocked)   * config.BLOCKED_PAWN_END_WEIGHT  ;
    midvalue += (popcount(phalanx)   * config.PHALANX_PAWN_MID_WEIGHT  )/2;
    endvalue += (popcount(phalanx)   * config.PHALANX_PAWN_END_WEIGHT  )/2;
    midvalue +=  popcount(supported) * config.SUPPORTED_PAWN_MID_WEIGHT;
    endvalue +=  popcount(supported) * config.SUPPORTED_PAWN_END_WEIGHT;
    // @formatter:on

    if (color == WHITE) {
      entry->midvalue += midvalue;
      entry->endvalue += endvalue;
    }
    else {
      entry->midvalue -= midvalue;
      entry->endvalue -= endvalue;
    }
    LOG__TRACE(LOG, "Raw pawn eval for {} results midvalue = {} and endvalue = {}",
               color ? "BLACK" : "WHITE", midvalue, endvalue);
  }
}

template<Color C, PieceType PT>
int Evaluator::evaluatePiece(const Position &position) {
  assert (PT != PAWN && PT != KING);

  int value = 0;

  // get all pieces of type PT from color C
  Bitboard pieces = position.getPieceBB(C, PT);

  if (config.USE_PIECE_BONI) {
    // bonus/malus for bishop pair
    if (PT == BISHOP && popcount(pieces) >= 2) value += config.BISHOP_PAIR;
    // bonus/malus for knight pair
    if (PT == KNIGHT && popcount(pieces) >= 2) value += config.KNIGHT_PAIR;
    // bonus/malus for rook pair
    if (PT == ROOK && popcount(pieces) >= 2) value += config.ROOK_PAIR;
  }

  // LOOP through all pieces of this color and type
  while (pieces) {
    const Square fromSquare = Bitboards::popLSB(pieces);
    // MOBILITY
    if (config.USE_MOBILITY) {
      value += mobility<C, PT>(position, fromSquare);
    }
    if (config.USE_PIECE_BONI) {
      // trapped bishops
      const Bitboard myPawns = position.getPieceBB(C, PAWN);
      if (PT == BISHOP && fromSquare == (C ? SQ_C8 : SQ_C1)) {
        value += myPawns & ((C ? SQ_B7 : SQ_B2) | (C ? SQ_D7 : SQ_D2)) ? config.TRAPPED_BISHOP_PENALTY : 0;
      }
      if (PT == BISHOP && fromSquare == (C ? SQ_F1 : SQ_F8)) {
        value += myPawns & ((C ? SQ_E7 : SQ_E2) | (C ? SQ_G7 : SQ_G2)) ? config.TRAPPED_BISHOP_PENALTY : 0;
      }
    }
  }

  LOG__TRACE(LOG, "Raw piece eval for {} {:6} results in value = {}",
             C ? "BLACK" : "WHITE", pieceTypeToString[PT], value);
  return value;
}

template<Color C, PieceType PT>
inline int Evaluator::mobility(const Position &position, const Square sq) {
  const Bitboard occupiedBB = position.getOccupiedBB();
  const Bitboard myPiecesBB = position.getOccupiedBB(C);
  const Bitboard pseudoMoves = Bitboards::pseudoAttacks[PT][sq];
  int mobility = 0;
  if (PT == KNIGHT) {
    // knights can't be blocked
    Bitboard moves = pseudoMoves & ~myPiecesBB;
    mobility += Bitboards::popcount(moves);
  }
  else { // sliding pieces
    Bitboard pseudoTo = pseudoMoves & ~myPiecesBB;
    while (pseudoTo) {
      const Square toSquare = Bitboards::popLSB(pseudoTo);
      if (config.USE_MOBILITY) {
        if (!(Bitboards::intermediateBB[sq][toSquare] & occupiedBB)) {
          mobility += 1;
        }
      }
    }
  }
  return mobility * config.MOBILITY_WEIGHT;
}

template<Color C>
int Evaluator::evaluateKing(const Position &position) {

  int value = 0;

  // king castle safety - skip in endgame
  if (config.USE_KING_CASTLE_SAFETY) {
    value += kingCastleSafety<C>(position);
  }

  LOG__TRACE(LOG, "Raw piece eval for {} {:6} results in value = {}", C ? "BLACK" : "WHITE",
             pieceTypeToString[KING], value);
  return value;
}

template<Color C>
int Evaluator::kingCastleSafety(const Position &position) {
  const Bitboard myRooks = position.getPieceBB(C, ROOK);
  const Bitboard myPawns = position.getPieceBB(C, PAWN);
  const Square kingSquare = position.getKingSquare(C);

  int value = 0;

  // king in king side castle
  if (kingSideCastleMask[C] & kingSquare) {
    // castle wall
    if (squareBB[C ? SQ_F7 : SQ_F2] & myPawns
        && (squareBB[C ? SQ_G7 : SQ_G2] | squareBB[C ? SQ_G6 : SQ_G3]) & myPawns
        && (squareBB[C ? SQ_H7 : SQ_H2] | squareBB[C ? SQ_H6 : SQ_H3] | squareBB[C ? SQ_H5 : SQ_H4]) & myPawns
      ) {
      value += static_cast<int>(config.KING_SAFETY_PAWNSHIELD * position.getGamePhaseFactor());
      // trapped rook
      if (myRooks & rays[E][kingSquare]) {
        value += config.TRAPPED_ROOK_PENALTY;
      }
    }
  }
    // king in queen side castle
  else if (queenSideCastleMask[C] & kingSquare) {
    // castle wall
    if (squareBB[C ? SQ_D7 : SQ_D2] & myPawns
        && squareBB[C ? SQ_B7 : SQ_B2] & myPawns
        && (squareBB[C ? SQ_C7 : SQ_C2] | squareBB[C ? SQ_C6 : SQ_C3]) & myPawns
        && (squareBB[C ? SQ_A7 : SQ_A2] | squareBB[C ? SQ_A6 : SQ_A3] | squareBB[C ? SQ_A5 : SQ_A4]) & myPawns
      ) {
      value += static_cast<int>(config.KING_SAFETY_PAWNSHIELD * position.getGamePhaseFactor());
      // trapped rook
      if (myRooks & rays[W][kingSquare]) {
        value += config.TRAPPED_ROOK_PENALTY;
      }
    }
  }

  return value * config.KING_CASTLE_SAFETY_WEIGHT;
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

