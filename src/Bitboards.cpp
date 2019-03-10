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

#include <string>
#include <iostream>

#include "globals.h"
#include "Bitboards.h"

using namespace std;

int squareDistance[SQ_NONE][SQ_NONE];

namespace Bitboards {

  Bitboard squareBB[SQ_LENGTH];
  Bitboard squareDiagUpBB[SQ_LENGTH];
  Bitboard squareDiagDownBB[SQ_LENGTH];

  Bitboard movesRank[SQ_LENGTH][256];
  Bitboard movesFile[SQ_LENGTH][256];
  Bitboard movesDiagUp[SQ_LENGTH][256];
  Bitboard movesDiagDown[SQ_LENGTH][256];

  Bitboard pawnAttacks[COLOR_LENGTH][SQ_LENGTH];
  Bitboard pawnMoves[COLOR_LENGTH][SQ_LENGTH];

  Square indexMapR90[SQ_LENGTH];
  Square indexMapL90[SQ_LENGTH];
  Square indexMapR45[SQ_LENGTH];
  Square indexMapL45[SQ_LENGTH];

  uint8_t PopCnt16[1 << 16];

  /**
   * Prints a bitboard a an 8x8 matrix for output on a console
   */
  string print(Bitboard b) {
    string s = "+---+---+---+---+---+---+---+---+\n";
    for (Rank r = RANK_8; r >= RANK_1; --r) {
      for (File f = FILE_A; f <= FILE_H; ++f) {
        s += b & getSquare(f, r) ? "| X " : "|   ";
      }
      s += "|\n+---+---+---+---+---+---+---+---+\n";
    }
    return s;
  }

  /**
   * Prints a bitboard as a series of 0 and 1 grouped in 8 bits
   * beginning with the LSB (0) on the left and the MSB (63) on the right
   */
  string printFlat(Bitboard b) {
    string s;
    for (int i = 0; i < 64; i++) {
      if (i > 0 && i % 8 == 0) s += ".";
      s += b & (1L << i) ? "1" : "0";
    }
    s += " (" + to_string(b) + ")";
    return s;
  }

  /**
   * Rotates a Bitboard
   * @param b
   * @return rotated bitboard
   */
  Bitboard rotateR90(Bitboard b) {
    return rotate(b, rotateMapR90);
  }

  /**
   * Rotates a Bitboard
   * @param b
   * @return rotated bitboard
   */
  Bitboard rotateL90(Bitboard b) {
    return rotate(b, rotateMapL90);
  }

  /**
   * Rotates a Bitboard
   * @param b
   * @return rotated bitboard
   */
  Bitboard rotateR45(Bitboard b) {
    return rotate(b, rotateMapR45);
  }

  /**
   * Rotates a Bitboard
   * @param b
   * @return rotated bitboard
   */
  Bitboard rotateL45(Bitboard b) {
    return rotate(b, rotateMapL45);
  }

  /**
   * Rotates a Bitboard
   * @param b
   * @param rotMap - array with mapping for rotation
   * @return rotated bitboard
   */
  Bitboard rotate(Bitboard b, const int *rotMap) {
    Bitboard rotated = EMPTY_BB;
    for (Square sq = SQ_A1; sq < SQ_LENGTH; ++sq) {
      if ((b & Square(rotMap[sq])) != 0) rotated |= squareBB[sq];
    }
    return rotated;
  }

  /**
   * @return popcount16() counts the non-zero bits using SWAR-Popcount algorithm
   */
  unsigned popcount16(unsigned u) {
    u -= (u >> 1) & 0x5555U;
    u = ((u >> 2) & 0x3333U) + (u & 0x3333U);
    u = ((u >> 4) + u) & 0x0F0FU;
    return (u * 0x0101U) >> 8;
  }

  /**
   * Initializes various pre-computed bitboards
   */
  void init() {

    // pre-computes 16-bit population counter to use in popcount(64-bit)
    for (unsigned i = 0; i < (1 << 16); ++i)
      PopCnt16[i] = (uint8_t) popcount16(i);

    // all squares
    for (Square sq = SQ_A1; sq <= SQ_H8; ++sq) {

      // square bitboard
      squareBB[sq] = (1ULL << sq);

      // square diagonals
      if (DiagUpA8 & sq) squareDiagUpBB[sq] = DiagUpA8;
      else if (DiagUpA7 & sq) squareDiagUpBB[sq] = DiagUpA7;
      else if (DiagUpA6 & sq) squareDiagUpBB[sq] = DiagUpA6;
      else if (DiagUpA5 & sq) squareDiagUpBB[sq] = DiagUpA5;
      else if (DiagUpA4 & sq) squareDiagUpBB[sq] = DiagUpA4;
      else if (DiagUpA3 & sq) squareDiagUpBB[sq] = DiagUpA3;
      else if (DiagUpA2 & sq) squareDiagUpBB[sq] = DiagUpA2;
      else if (DiagUpA1 & sq) squareDiagUpBB[sq] = DiagUpA1;
      else if (DiagUpB1 & sq) squareDiagUpBB[sq] = DiagUpB1;
      else if (DiagUpC1 & sq) squareDiagUpBB[sq] = DiagUpC1;
      else if (DiagUpD1 & sq) squareDiagUpBB[sq] = DiagUpD1;
      else if (DiagUpE1 & sq) squareDiagUpBB[sq] = DiagUpE1;
      else if (DiagUpF1 & sq) squareDiagUpBB[sq] = DiagUpF1;
      else if (DiagUpG1 & sq) squareDiagUpBB[sq] = DiagUpG1;
      else if (DiagUpH1 & sq) squareDiagUpBB[sq] = DiagUpH1;
      if (DiagDownH8 & sq) squareDiagDownBB[sq] = DiagDownH8;
      else if (DiagDownH7 & sq) squareDiagDownBB[sq] = DiagDownH7;
      else if (DiagDownH6 & sq) squareDiagDownBB[sq] = DiagDownH6;
      else if (DiagDownH5 & sq) squareDiagDownBB[sq] = DiagDownH5;
      else if (DiagDownH4 & sq) squareDiagDownBB[sq] = DiagDownH4;
      else if (DiagDownH3 & sq) squareDiagDownBB[sq] = DiagDownH3;
      else if (DiagDownH2 & sq) squareDiagDownBB[sq] = DiagDownH2;
      else if (DiagDownH1 & sq) squareDiagDownBB[sq] = DiagDownH1;
      else if (DiagDownG1 & sq) squareDiagDownBB[sq] = DiagDownG1;
      else if (DiagDownF1 & sq) squareDiagDownBB[sq] = DiagDownF1;
      else if (DiagDownE1 & sq) squareDiagDownBB[sq] = DiagDownE1;
      else if (DiagDownD1 & sq) squareDiagDownBB[sq] = DiagDownD1;
      else if (DiagDownC1 & sq) squareDiagDownBB[sq] = DiagDownC1;
      else if (DiagDownB1 & sq) squareDiagDownBB[sq] = DiagDownB1;
      else if (DiagDownA1 & sq) squareDiagDownBB[sq] = DiagDownA1;
    }

    // distance between squares
    for (Square sq1 = SQ_A1; sq1 <= SQ_H8; ++sq1) {
      for (Square sq2 = SQ_A1; sq2 <= SQ_H8; ++sq2) {
        if (sq1 != sq2)
          squareDistance[sq1][sq2] = (int8_t) max(distance(fileOf(sq1), fileOf(sq2)),
                                                  distance(rankOf(sq1), rankOf(sq2)));
      }
    }

    // All sliding attacks with blockers - horizontal
    // Shamefully copied from Beowulf :)
    for (File file = FILE_A; file <= FILE_H; ++file) {
      for (int j = 0; j < 256; j++) {
        Bitboard mask = 0L;
        for (int x = file - 1; x >= 0; x--) {
          mask += (1L << x);
          if ((j & (1 << x)) != 0) break;
        }
        for (int x = file + 1; x < 8; x++) {
          mask += (1L << x);
          if ((j & (1 << x)) != 0) break;
        }
        for (Rank rank = RANK_1; rank <= RANK_8; ++rank) {
          movesRank[(rank * 8) + file][j] = mask << (rank * 8);
        }
      }
    }

    // All sliding attacks with blocker - vertical
    // Shamefully copied from Beowulf :)
    for (Rank rank = RANK_1; rank <= RANK_8; ++rank) {
      for (int j = 0; j < 256; j++) {
        Bitboard mask = 0;
        for (int x = 6 - rank; x >= 0; x--) {
          mask += (1L << (8 * (7 - x)));
          if ((j & (1 << x)) != 0) break;
        }
        for (int x = 8 - rank; x < 8; x++) {
          mask += (1L << (8 * (7 - x)));
          if ((j & (1 << x)) != 0) break;
        }
        for (File file = FILE_A; file <= FILE_H; ++file) {
          movesFile[(rank * 8) + file][j] = mask << file;
        }
      }
    }

    // All sliding attacks with blocker - up diag sliders
    // Shamefully copied from Beowulf :)
    for (Square square = SQ_A1; square <= SQ_H8; ++square) {
      File file = fileOf(square);
      Rank rank = rankOf(square);

      /* Get the far left hand square on this diagonal */
      Square diagstart = (Square) (square - 9 * (min((int) file, (int) rank)));
      File dsfile = fileOf(diagstart);
      int dl = lengthDiagUp[square];

      /* Loop through all possible occupations of this diagonal line */
      for (int sq = 0; sq < (1 << dl); sq++) {
        Bitboard mask = 0, mask2 = 0;

        /* Calculate possible target squares */
        for (int b1 = (file - dsfile) - 1; b1 >= 0; b1--) {
          mask += (ONE_BB << b1);
          if ((sq & (1 << b1)) != 0) break;
        }

        for (int b2 = (file - dsfile) + 1; b2 < dl; b2++) {
          mask += (ONE_BB << b2);
          if ((sq & (1 << b2)) != 0) break;
        }
        /* Rotate target squares back */
        for (int x = 0; x < dl; x++) mask2 += (((mask >> x) & 1) << (diagstart + (9 * x)));
        movesDiagUp[square][sq] = mask2;
      }
    }

    // All sliding attacks with blocker - down diag sliders
    // Shamefully copied from Beowulf :)
    for (Square square = SQ_A1; square <= SQ_H8; ++square) {
      File file = fileOf(square);
      Rank rank = rankOf(square);

      /* Get the far left hand square on this diagonal */
      Square diagstart = (Square) (7 * (min((int) file, (int) (7 - rank))) + square);
      File dsfile = fileOf(diagstart);
      int dl = lengthDiagDown[square];

      /* Loop through all possible occupations of this diagonal line */
      for (int j = 0; j < (1 << dl); j++) {
        Bitboard mask = 0, mask2 = 0;

        /* Calculate possible target squares */
        for (int x = (file - dsfile) - 1; x >= 0; x--) {
          mask += (ONE_BB << x);
          if ((j & (1 << x)) != 0) break;
        }

        for (int x = (file - dsfile) + 1; x < dl; x++) {
          mask += (ONE_BB << x);
          if ((j & (1 << x)) != 0) break;
        }

        /* Rotate the target line back onto the required diagonal */
        for (int x = 0; x < dl; x++) mask2 += (((mask >> x) & 1) << (diagstart - (7 * x)));

        movesDiagDown[square][j] = mask2;
      }
    }

    // pawn moves and attacks
    for (Square square = SQ_A1; square <= SQ_H8; ++square) {
      File file = fileOf(square);
      Rank rank = rankOf(square);

      // pawn attacks
      if (file > FILE_A) {
        if (square > SQ_H1) pawnAttacks[WHITE][square] |= (ONE_BB << square + NORTH_WEST);
        if (square < SQ_A8) pawnAttacks[BLACK][square] |= (1L << square + SOUTH_WEST);
      }
      if (file < FILE_H) {
        if (square > SQ_H1) pawnAttacks[WHITE][square] |= (1L << square + NORTH_EAST);
        if (square < SQ_A8) pawnAttacks[BLACK][square] |= (1L << square + SOUTH_EAST);
      }
      // pawn moves
      if (square > SQ_H1) pawnMoves[WHITE][square] |= (1L << square + NORTH);
      if (square < SQ_A8) pawnMoves[BLACK][square] |= (1L << square + SOUTH);
      // pawn double moves
      if (rank == RANK_2) pawnMoves[WHITE][square] |= (1L << square + NORTH + NORTH);
      if (rank == RANK_7) pawnMoves[BLACK][square] |= (1L << square + SOUTH + SOUTH);
    }

    // Reverse index to quickly calculate the index of a square in the rotated board
    for (Square square1 = SQ_A1; square1 < SQ_LENGTH; ++square1) {
      indexMapR90[rotateMapR90[square1]] = square1;
      indexMapL90[rotateMapL90[square1]] = square1;
      indexMapR45[rotateMapR45[square1]] = square1;
      indexMapL45[rotateMapL45[square1]] = square1;
    }

  }

  /**
   * Shifts a bitboard in the given direction.
   * @param d Direction
   * @param b Bitboard
   * @return shifted bitboard
   */
  Bitboard shift(Direction d, Bitboard b) {
    // move the bits and clear the left our right file
    // after the shift to erase bit jumping over
    switch (d) {
      case NORTH:
        return (b << 8);
      case EAST:
        return (b << 1) & ~FileABB;
      case SOUTH:
        return (b >> 8);
      case WEST:
        return (b >> 1) & ~FileHBB;
      case NORTH_EAST:
        return (b << 9);
      case SOUTH_EAST:
        return (b >> 7);
      case SOUTH_WEST:
        return (b >> 9);
      case NORTH_WEST:
        return (b << 7);
    }
    assert(false);
    return b;
  }

  /**
   * Bitboard for all possible horizontal moves on the rank of the square with
   * the rank content (blocking pieces) determined from the given pieces bitboard.
   */
  Bitboard getMovesRank(Square sq, Bitboard content) {
    // content = the pieces currently on the board and maybe blocking the moves
    // no rotation necessary for ranks - their squares are already in a row
    // shift to the least significant bit
    Bitboard contentIdx = content >> 8 * rankOf(sq);
    // retrieve all possible moves for this square with the current content
    // and mask with the first row to erase any other pieces
    return movesRank[sq][contentIdx & 255];
  }

  /**
   * Bitboard for all possible horizontal moves on the rank of the square with
   * the rank content (blocking pieces) determined from the given non-rotated
   * bitboard.
   */
  Bitboard getMovesFile(Square sq, Bitboard content) {
    // content = the pieces currently on the board and maybe blocking the moves
    // rotate the content of the board to get all file squares in a row
    Bitboard rotated = rotateL90(content);
    return getMovesFileR(sq, rotated);
  }

  /**
   * Bitboard for all possible horizontal moves on the rank of the square with
   * the rank content (blocking pieces) determined from the given L90 rotated
   * bitboard.
   */
  Bitboard getMovesFileR(Square sq, Bitboard rotated) {
    // shift to the first byte (to the right in Java)
    Bitboard contentIdx = rotated >> fileOf(sq) * 8;
    // retrieve all possible moves for this square with the current content
    // and mask with the first row to erase any other pieces not erased by shift
    return movesFile[sq][contentIdx & 255];
  }

  /**
  * Bitboard for all possible diagonal up moves of the square with
  * the content (blocking pieces) determined from the given non rotated bitboard.
  */
  Bitboard getMovesDiagUp(Square square, Bitboard content) {
    // content = the pieces currently on the board and maybe blocking the moves
    // rotate the content of the board to get all diagonals in a row
    // cout << Bitboards::printFlat(content) << endl;
    return getMovesDiagUpR(square, rotateR45(content));
  }

  /**
   * Bitboard for all possible diagonal up moves of the square with
   * the content (blocking pieces) determined from the given R45 rotated
   * bitboard.
   */
  Bitboard getMovesDiagUpR(Square sq, Bitboard rotated) {
    // shift the correct row to the first byte (to the right in Java)
    Bitboard shifted = rotated >> shiftsDiagUp[sq];
    // mask the content with the length of the diagonal to erase any other pieces
    // which have not been erased by the shift
    Bitboard contentMasked = shifted & lengthDiagUpMask(sq);
    // retrieve all possible moves for this square with the current content
    //    cout << Bitboards::printFlat(rotated) << endl;
    //    cout << Bitboards::printFlat(shifted) << endl;
    //    cout << Bitboards::printFlat(lengthDiagUpMask(sq)) << endl;
    //    cout << Bitboards::printFlat(contentMasked) << endl;
    return movesDiagUp[sq][contentMasked];
  }

  /**
   * Bitboard for all possible diagonal up moves of the square with
   * the content (blocking pieces) determined from the given non rotated bitboard.
   */
  Bitboard getMovesDiagDown(Square square, Bitboard content) {
    // content = the pieces currently on the board and maybe blocking the moves
    // rotate the content of the board to get all diagonals in a row
    // cout << Bitboards::printFlat(content) << endl;
    return getMovesDiagDownR(square, rotateL45(content));
  }

  /**
   * Bitboard for all possible diagonal up moves of the square with
   * the content (blocking pieces) determined from the given L45 rotated
   * bitboard.
   */
  Bitboard getMovesDiagDownR(Square sq, Bitboard rotated) {
    // shift the correct row to the first byte (to the right in Java)
    Bitboard shifted = rotated >> shiftsDiagDown[sq];
    // mask the content with the length of the diagonal to erase any other pieces
    // which have not been erased by the shift
    Bitboard contentMasked = shifted & lengthDiagDownMask(sq);
    // retrieve all possible moves for this square with the current content
    //    cout << Bitboards::printFlat(rotated) << endl;
    //    cout << Bitboards::printFlat(shifted) << endl;
    //    cout << Bitboards::printFlat(lengthDiagDownMask(sq)) << endl;
    //    cout << Bitboards::printFlat(contentMasked) << endl;
    return movesDiagDown[sq][contentMasked];
  }
}
