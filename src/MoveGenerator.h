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

#ifndef FRANKYCPP_MOVEGENERATOR_H
#define FRANKYCPP_MOVEGENERATOR_H

#include <vector>

#include "../test/lib/googletest-master/googletest/include/gtest/gtest_prod.h"
#include "datatypes.h"
#include "Bitboards.h"

// circle reference between Position and MoveGenerator - this make it possible
class Position;

enum GenMode {
  GENZERO,
  GENCAP,
  GENNONCAP = GENCAP << 1,
  GENALL = GENCAP | GENNONCAP
};

class MoveGenerator {

  MoveList pseudoLegalMoves;
  MoveList legalMoves;
  MoveList onDemandMoves;

  enum onDemandStage : int { OD_NEW, OD1, OD2, OD3, OD4, OD5, OD6, OD7, OD_END };
  onDemandStage currentODStage = OD_NEW;
  Key currentIteratorKey;

public:

  MoveGenerator();
  ~MoveGenerator();

  /**
   * Generates pseudo moves for the next player. Does not check if king is left in check or
   * passes an attacked square when castling or has been in check before castling.
   *
   * @param genMode
   * @param pPosition
   * @param moves - generated moves will be added to this list
   */
  MoveList generatePseudoLegalMoves(GenMode genMode, Position *pPosition);

  /**
  * Generates legal moves for the next player.
  *
  * @param genMode
  * @param pPosition
  * @param moves - generated moves will be added to this list
  */
  MoveList generateLegalMoves(GenMode genMode, Position *pPosition);

  /**
   * Returns the next move for the given position. Usually this would be used in a loop
   * during search. If the position changes this will restart at the first move.
   *
   * @param genMode
   * @param pPosition
   * @return
   */
  Move getNextPseudoLegalMove(GenMode genMode, Position *pPosition);

  /**
   * This method checks if the position has at least one legal move. It will mainly be used to
   * determine mate and stale mate position. This method returns as quick as possible as it is
   * sufficient to have found at least one legal move to see that the position is not a mate
   * position. It only has to check all moves if it is indeed a mate position which in general is a
   * rare case.
   *
   * @return true if there is at least one legal move
   */
  bool hasLegalMove(Position *pPosition);

private:

  FRIEND_TEST(MoveGenTest, pawnMoves);
  FRIEND_TEST(MoveGenTest, kingMoves);
  FRIEND_TEST(MoveGenTest, normalMoves);
  FRIEND_TEST(MoveGenTest, castlingMoves);

  /**
   * Generates pseudo pawn moves for the next player. Does not check if king is left in check
   * @param genMode
   * @param pPosition
   * @param pMoves - generated moves will be added to this list
   */
  void generatePawnMoves(GenMode genMode, const Position *pPosition, MoveList *pMoves);

  /**
   * Generates pseudo knight, bishop, rook and queen moves for the next player.
   * Does not check if king is left in check
   * @param genMode
   * @param pPosition
   * @param pMoves - generated moves will be added to this list
   */
  void generateMoves(GenMode genMode, const Position *pPosition, MoveList *pMoves);

  /**
   * Generates pseudo king moves for the next player. Does not check if king
   * lands on an attacked square.
   * @param genMode
   * @param pPosition
   * @param pMoves - generated moves will be added to this list
   */
  void generateKingMoves(GenMode genMode, const Position *pPosition, MoveList *pMoves);

  /**
   * Generates pseudo castling move for the next player. Does not check if king passes or lands on an
   * attacked square.
   * @param genMode
   * @param pPosition
   * @param pMoves - generated moves will be added to this list
   */
  void generateCastling(GenMode genMode, const Position *pPosition, MoveList *pMoves);

};

#endif //FRANKYCPP_MOVEGENERATOR_H
