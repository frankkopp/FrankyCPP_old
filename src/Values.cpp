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

#include "Values.h"

namespace Values {
  int midGamePosValue[PIECE_LENGTH][SQ_LENGTH];
  int endGamePosValue[PIECE_LENGTH][SQ_LENGTH];

  void init() {
    for (Piece pc = WHITE_KING; pc < PIECE_NONE; ++pc) {
      for (Square sq = SQ_A1; sq <= SQ_H8; ++sq) {
        switch (pc) {
          case WHITE_KING:
            midGamePosValue[pc][sq] = kingMidGame[63 - sq];
            endGamePosValue[pc][sq] = kingEndGame[63 - sq];
            break;
          case WHITE_PAWN:
            midGamePosValue[pc][sq] = pawnsMidGame[63 - sq];
            endGamePosValue[pc][sq] = pawnsEndGame[63 - sq];
            break;
          case WHITE_KNIGHT:
            midGamePosValue[pc][sq] = knightMidGame[63 - sq];
            endGamePosValue[pc][sq] = knightEndGame[63 - sq];
            break;
          case WHITE_BISHOP:
            midGamePosValue[pc][sq] = bishopMidGame[63 - sq];
            endGamePosValue[pc][sq] = bishopEndGame[63 - sq];
            break;
          case WHITE_ROOK:
            midGamePosValue[pc][sq] = rookMidGame[63 - sq];
            endGamePosValue[pc][sq] = rookEndGame[63 - sq];
            break;
          case WHITE_QUEEN:
            midGamePosValue[pc][sq] = queenMidGame[63 - sq];
            endGamePosValue[pc][sq] = queenEndGame[63 - sq];
            break;
          case BLACK_KING:
            midGamePosValue[pc][sq] = kingMidGame[sq];
            endGamePosValue[pc][sq] = kingEndGame[sq];
            break;
          case BLACK_PAWN:
            midGamePosValue[pc][sq] = pawnsMidGame[sq];
            endGamePosValue[pc][sq] = pawnsEndGame[sq];
            break;
          case BLACK_KNIGHT:
            midGamePosValue[pc][sq] = knightMidGame[sq];
            endGamePosValue[pc][sq] = knightEndGame[sq];
            break;
          case BLACK_BISHOP:
            midGamePosValue[pc][sq] = bishopMidGame[sq];
            endGamePosValue[pc][sq] = bishopEndGame[sq];
            break;
          case BLACK_ROOK:
            midGamePosValue[pc][sq] = rookMidGame[sq];
            endGamePosValue[pc][sq] = rookEndGame[sq];
            break;
          case BLACK_QUEEN:
            midGamePosValue[pc][sq] = queenMidGame[sq];
            endGamePosValue[pc][sq] = queenEndGame[sq];
            break;
          case PIECE_NONE:
            break;
        }
      }
    }
  }
}