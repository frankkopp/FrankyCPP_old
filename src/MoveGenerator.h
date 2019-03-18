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

#include "globals.h"
#include "Position.h"
#include "Bitboards.h"

using namespace std;

enum GenMode {
  GENZERO,
  GENCAP,
  GENNONCAP = GENCAP << 1,
  GENALL = GENCAP | GENNONCAP
};

class MoveGenerator {

public:

  MoveGenerator();

  /**
   * Generates pseudo moves for the next player. Does not check if king is left in check or
   * passes an attacked square when castling or has been in check before castling.
   *
   * @param genMode
   * @param position
   * @param moves - generated moves will be added to this list
   */
  vector<Move> generatePseudoLegalMoves(GenMode genMode, Position *position);

  /**
  * Generates legal moves for the next player.
  *
  * @param genMode
  * @param position
  * @param moves - generated moves will be added to this list
  */
  vector<Move> generateLegalMoves(GenMode genMode, Position *position);

private:

  FRIEND_TEST(MoveGenTest, pawnMoves);
  FRIEND_TEST(MoveGenTest, kingMoves);
  FRIEND_TEST(MoveGenTest, normalMoves);
  FRIEND_TEST(MoveGenTest, castlingMoves);

  /**
   * Generates pseudo pawn moves for the next player. Does not check if king is left in check
   * @param genMode
   * @param position
   * @param moves - generated moves will be added to this list
   */
  void generatePawnMoves(GenMode genMode, const Position *position, vector<Move> *moves);

  /**
   * Generates pseudo knight, bishop, rook and queen moves for the next player.
   * Does not check if king is left in check
   * @param genMode
   * @param position
   * @param moves - generated moves will be added to this list
   */
  void generateMoves(GenMode genMode, const Position *position, vector<Move> *moves);

  /**
   * Generates pseudo king moves for the next player. Does not check if king
   * lands on an attacked square.
   * @param genMode
   * @param position
   * @param moves - generated moves will be added to this list
   */
  void generateKingMoves(GenMode genMode, const Position *position, vector<Move> *moves);

  /**
   * Generates pseudo castling move for the next player. Does not check if king passes or lands on an
   * attacked square.
   * @param genMode
   * @param position
   * @param moves - generated moves will be added to this list
   */
  void generateCastling(GenMode genMode, Position *position, vector<Move> *moves);

};


#endif //FRANKYCPP_MOVEGENERATOR_H
