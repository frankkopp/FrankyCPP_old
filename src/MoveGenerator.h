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
#include "Logging.h"
#include "types.h"

// circle reference between Position and MoveGenerator - this make it possible
class Position;

class MoveGenerator {
  
  std::shared_ptr<spdlog::logger> LOG = spdlog::get("MoveGen_Logger");

  MoveList pseudoLegalMoves = MoveList(256);
  MoveList legalMoves = MoveList(256);
  MoveList onDemandMoves = MoveList(256);

  enum onDemandStage : int {
    OD_NEW, PV, OD1, OD2, OD3, OD4, OD5, OD6, OD7, OD8, OD_END
  };
  onDemandStage currentODStage = OD_NEW;
  Key currentIteratorKey{};
  
  MoveList::size_type maxNumberOfKiller = 2; // default
  MoveList killerMoves = MoveList(0);
  Move pvMove = MOVE_NONE;

public:

  MoveGenerator();
  ~MoveGenerator();

  enum GenMode {
    GENZERO,
    GENCAP,
    GENNONCAP,
    GENALL
  };

  /**
    * Generates pseudo moves for the next player. Does not check if king is left in check or
    * passes an attacked square when castling or has been in check before castling.
    * Disregards PV moves and Killer moves.
    * They need to be handled after the returned MoveList. Or just use the OnDemand
    * Generator.
    *
    * @param genMode
    * @param pPosition
    * @param moves - generated moves will be added to this list
    */
  template<GenMode GM>
  const MoveList* generatePseudoLegalMoves(const Position &position);

  /**
    * Generates legal moves for the next player. Disregards PV moves and Killer moves.
    * They need to be handled after the returned MoveList. Or just use the OnDemand
    * Generator.
    *
    * @param genMode
    * @param pPosition
    * @param moves - generated moves will be added to this list
    */
  template<GenMode GM>
  const MoveList* generateLegalMoves(Position  &position);

  /**
   * Returns the next move for the given position. Usually this would be used in a loop
   * during search. If the position changes this will restart at the first move.
   * If a PV move is set with <code>setPV(Move pv)</code> this will be returned first
   * and will not be returned at its normal place.
   * Killer moves will be played as soon as possible. As Killer moves are stored for
   * the whole ply a Killer move might not be valid for the current position. Therefore
   * we need to wait until they are generated by the phased move generation and then
   * push to the top of the list of the generation stage. 
   *
   * @param genMode
   * @param pPosition
   * @return
   */
  template<GenMode GM>
  Move getNextPseudoLegalMove(const Position &position);

  /**
   * Resets the move generator to start fresh.
   * Clears all lists (e.g. killers) and resets on demand iterator
   */
  void reset();

  /**
   * Resets the move on demand generator to start fresh.
   * Also deletes Killer and PV moves
   */
  void resetOnDemand();

  /**
   * This method checks if the position has at least one legal move. It will mainly be used to
   * determine mate and stale mate position. This method returns as quick as possible as it is
   * sufficient to have found at least one legal move to see that the position is not a mate
   * position. It only has to check all moves if it is indeed a mate position which in general is a
   * rare case.
   *
   * @return true if there is at least one legal move
   */
  static bool hasLegalMove(const Position &position);
  
  /**
   * Provides the on demand move generator with a new killer move which should be returned as soon
   * as possible when generating moves with the on demand generator. Also tells the move generator
   * how many killer moves to store/use as a maximum.
   */
  void storeKiller(Move move, int maxKillers);

  /**
   * Sets a PV move which should be returned first by the OnDemand MoveGenerator. 
   * @param move
   */
  void setPV(Move move);

private:

  FRIEND_TEST(MoveGenTest, pawnMoves);
  FRIEND_TEST(MoveGenTest, kingMoves);
  FRIEND_TEST(MoveGenTest, normalMoves);
  FRIEND_TEST(MoveGenTest, castlingMoves);
  FRIEND_TEST(MoveGenTest, storeKiller);
  

  /**
   * Generates pseudo pawn moves for the next player. Does not check if king is left in check
   * @param genMode
   * @param pPosition
   * @param pMoves - generated moves will be added to this list
   */
  template<GenMode GM>
  void generatePawnMoves(const Position &position, MoveList* const pMoves);

  /**
   * Generates pseudo knight, bishop, rook and queen moves for the next player.
   * Does not check if king is left in check
   * @param genMode
   * @param pPosition
   * @param pMoves - generated moves will be added to this list
   */
  template<GenMode GM>
  void generateMoves(const Position &position, MoveList* const pMoves);

  /**
   * Generates pseudo king moves for the next player. Does not check if king
   * lands on an attacked square.
   * @param genMode
   * @param pPosition
   * @param pMoves - generated moves will be added to this list
   */
  template<GenMode GM>
  void generateKingMoves(const Position &position, MoveList* const pMoves);

  /**
   * Generates pseudo castling move for the next player. Does not check if king passes or lands on an
   * attacked square.
   * @param genMode
   * @param pPosition
   * @param pMoves - generated moves will be added to this list
   */
  template<GenMode GM>
  void generateCastling(const Position &position, MoveList* const pMoves);
  
  void pushKiller(MoveList &list);

  void filterPV(MoveList &deque);
};

#endif //FRANKYCPP_MOVEGENERATOR_H
