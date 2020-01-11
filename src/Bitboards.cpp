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

#include "Bitboards.h"

using namespace std;

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
  Bitboard pseudoAttacks[PT_LENGTH][SQ_LENGTH];

  Square indexMapR90[SQ_LENGTH];
  Square indexMapL90[SQ_LENGTH];
  Square indexMapR45[SQ_LENGTH];
  Square indexMapL45[SQ_LENGTH];

  Bitboard filesWestMask[SQ_LENGTH];
  Bitboard filesEastMask[SQ_LENGTH];
  Bitboard fileWestMask[SQ_LENGTH];
  Bitboard fileEastMask[SQ_LENGTH];
  Bitboard neighbourFilesMask[SQ_LENGTH];
  Bitboard ranksNorthMask[SQ_LENGTH];
  Bitboard ranksSouthMask[SQ_LENGTH];

  Bitboard rays[OR_LENGTH][SQ_LENGTH];

  Bitboard passedPawnMask[COLOR_LENGTH][SQ_LENGTH];

  Bitboard kingSideCastleMask[COLOR_LENGTH];
  Bitboard queenSideCastleMask[COLOR_LENGTH];

  Bitboard whiteSquaresBB;
  Bitboard blackSquaresBB;

  Bitboard intermediateBB[SQ_LENGTH][SQ_LENGTH];

  int squareDistance[SQ_NONE][SQ_NONE];
  int centerDistance[SQ_LENGTH];

  uint8_t PopCnt16[1 << 16];

  /**
   * Initializes various pre-computed bitboards
   */
  void init() {

    // pre-computes 16-bit population counter to use in popcount(64-bit)
    for (unsigned i = 0; i < (1U << 16U); ++i)
      PopCnt16[i] = (uint8_t) popcount16(i);

    // all squares
    for (Square sq = SQ_A1; sq <= SQ_H8; ++sq) {

      // square bitboard
      squareBB[sq] = ONE_BB << sq;

      // square diagonals
      if (DiagUpA8 & sq) { squareDiagUpBB[sq] = DiagUpA8; }
      else if (DiagUpA7 & sq) { squareDiagUpBB[sq] = DiagUpA7; }
      else if (DiagUpA6 & sq) { squareDiagUpBB[sq] = DiagUpA6; }
      else if (DiagUpA5 & sq) { squareDiagUpBB[sq] = DiagUpA5; }
      else if (DiagUpA4 & sq) { squareDiagUpBB[sq] = DiagUpA4; }
      else if (DiagUpA3 & sq) { squareDiagUpBB[sq] = DiagUpA3; }
      else if (DiagUpA2 & sq) { squareDiagUpBB[sq] = DiagUpA2; }
      else if (DiagUpA1 & sq) { squareDiagUpBB[sq] = DiagUpA1; }
      else if (DiagUpB1 & sq) { squareDiagUpBB[sq] = DiagUpB1; }
      else if (DiagUpC1 & sq) { squareDiagUpBB[sq] = DiagUpC1; }
      else if (DiagUpD1 & sq) { squareDiagUpBB[sq] = DiagUpD1; }
      else if (DiagUpE1 & sq) { squareDiagUpBB[sq] = DiagUpE1; }
      else if (DiagUpF1 & sq) { squareDiagUpBB[sq] = DiagUpF1; }
      else if (DiagUpG1 & sq) { squareDiagUpBB[sq] = DiagUpG1; }
      else if (DiagUpH1 & sq) squareDiagUpBB[sq] = DiagUpH1;
      if (DiagDownH8 & sq) { squareDiagDownBB[sq] = DiagDownH8; }
      else if (DiagDownH7 & sq) { squareDiagDownBB[sq] = DiagDownH7; }
      else if (DiagDownH6 & sq) { squareDiagDownBB[sq] = DiagDownH6; }
      else if (DiagDownH5 & sq) { squareDiagDownBB[sq] = DiagDownH5; }
      else if (DiagDownH4 & sq) { squareDiagDownBB[sq] = DiagDownH4; }
      else if (DiagDownH3 & sq) { squareDiagDownBB[sq] = DiagDownH3; }
      else if (DiagDownH2 & sq) { squareDiagDownBB[sq] = DiagDownH2; }
      else if (DiagDownH1 & sq) { squareDiagDownBB[sq] = DiagDownH1; }
      else if (DiagDownG1 & sq) { squareDiagDownBB[sq] = DiagDownG1; }
      else if (DiagDownF1 & sq) { squareDiagDownBB[sq] = DiagDownF1; }
      else if (DiagDownE1 & sq) { squareDiagDownBB[sq] = DiagDownE1; }
      else if (DiagDownD1 & sq) { squareDiagDownBB[sq] = DiagDownD1; }
      else if (DiagDownC1 & sq) { squareDiagDownBB[sq] = DiagDownC1; }
      else if (DiagDownB1 & sq) { squareDiagDownBB[sq] = DiagDownB1; }
      else if (DiagDownA1 & sq) squareDiagDownBB[sq] = DiagDownA1;
    }

    // distance between squares
    for (Square sq1 = SQ_A1; sq1 <= SQ_H8; ++sq1) {
      for (Square sq2 = SQ_A1; sq2 <= SQ_H8; ++sq2) {
        if (sq1 != sq2) {
          squareDistance[sq1][sq2] = (int8_t) max(distance(fileOf(sq1), fileOf(sq2)),
                                                  distance(rankOf(sq1), rankOf(sq2)));
        }
      }
    }

    // Reverse index to quickly calculate the index of a square in the rotated board
    for (Square square1 = SQ_A1; square1 < SQ_LENGTH; ++square1) {
      indexMapR90[rotateMapR90[square1]] = square1;
      indexMapL90[rotateMapL90[square1]] = square1;
      indexMapR45[rotateMapR45[square1]] = square1;
      indexMapL45[rotateMapL45[square1]] = square1;
    }

    /// Pre-compute attacks and moves on a an empty board (pseudo attacks)

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

    // pawn moves
    for (Square square = SQ_A1; square <= SQ_H8; ++square) {
      // pawn moves
      if (square > SQ_H1) pawnMoves[WHITE][square] |= ONE_BB << (square + NORTH);
      if (square < SQ_A8) pawnMoves[BLACK][square] |= ONE_BB << (square + SOUTH);
      // pawn double moves
      if (rankOf(square) == RANK_2) pawnMoves[WHITE][square] |= ONE_BB << (square + NORTH + NORTH);
      if (rankOf(square) == RANK_7) pawnMoves[BLACK][square] |= ONE_BB << (square + SOUTH + SOUTH);
    }

    // @formatter:off
    // steps for kings, pawns, knight for WHITE - negate to get BLACK
    int steps[][5] = { {},
                       { NORTH_WEST, NORTH, NORTH_EAST, EAST }, // king
                       { NORTH_WEST, NORTH_EAST }, // pawn
                       { WEST+NORTH_WEST,          // knight
                         EAST+NORTH_EAST,
                         NORTH+NORTH_WEST,
                         NORTH+NORTH_EAST }};
    // @formatter:on

    // knight and king attacks
    for (Color c = WHITE; c <= BLACK; ++c) {
      for (PieceType pt : {KING, PAWN, KNIGHT}) {
        for (Square s = SQ_A1; s <= SQ_H8; ++s) {
          for (int i = 0; steps[pt][i]; ++i) {
            Square to = s + Direction(c == WHITE ? steps[pt][i] : -steps[pt][i]);
            if (isSquare(to) && distance(s, to) < 3) {
              if (pt == PAWN) { pawnAttacks[c][s] |= to; }
              else { pseudoAttacks[pt][s] |= to; }
            }
          }
        }
      }
    }

    // bishop, rook, queen pseudo attacks
    for (Square square = SQ_A1; square <= SQ_H8; ++square) {
      pseudoAttacks[BISHOP][square] |= movesDiagUp[square][0];
      pseudoAttacks[BISHOP][square] |= movesDiagDown[square][0];
      pseudoAttacks[ROOK][square] |= movesFile[square][0];
      pseudoAttacks[ROOK][square] |= movesRank[square][0];
      pseudoAttacks[QUEEN][square] |= pseudoAttacks[BISHOP][square] | pseudoAttacks[ROOK][square];
    }

    // masks for files and ranks left, right, up and down from square
    for (Square square = SQ_A1; square <= SQ_H8; ++square) {
      int f = fileOf(square);
      int r = rankOf(square);
      for (int j = 0; j <= 7; j++) {
        // file masks
        if (j < f) filesWestMask[square] |= fileBB(j);
        if (7 - j > f) filesEastMask[square] |= fileBB(7 - j);
        // rank masks
        if (7 - j > r) ranksNorthMask[square] |= rankBB(7 - j);
        if (j < r) ranksSouthMask[square] |= rankBB(j);
      }
      if (f > 0) fileWestMask[square] = fileBB(f - 1);
      if (f < 7) fileEastMask[square] = fileBB(f + 1);
      neighbourFilesMask[square] = fileEastMask[square] | fileWestMask[square];
    }

    // rays
    for (Square square = SQ_A1; square < SQ_LENGTH; ++square) {
      rays[N][square] = pseudoAttacks[ROOK][square] & ranksNorthMask[square];
      rays[E][square] = pseudoAttacks[ROOK][square] & filesEastMask[square];
      rays[S][square] = pseudoAttacks[ROOK][square] & ranksSouthMask[square];
      rays[W][square] = pseudoAttacks[ROOK][square] & filesWestMask[square];

      rays[NW][square] =
        pseudoAttacks[BISHOP][square] & filesWestMask[square] & ranksNorthMask[square];
      rays[NE][square] =
        pseudoAttacks[BISHOP][square] & filesEastMask[square] & ranksNorthMask[square];
      rays[SE][square] =
        pseudoAttacks[BISHOP][square] & filesEastMask[square] & ranksSouthMask[square];
      rays[SW][square] =
        pseudoAttacks[BISHOP][square] & filesWestMask[square] & ranksSouthMask[square];
    }

    // mask for passed pawns
    // Pawn front line - all squares north of the square for white, south for black
    for (Square square = SQ_A1; square <= SQ_H8; ++square) {
      int f = fileOf(square);
      int r = rankOf(square);
      // white pawn - ignore that pawns can'*t be on all squares
      passedPawnMask[WHITE][square] |= rays[N][square];
      if (f < 7 && r < 7) passedPawnMask[WHITE][square] |= rays[N][square + EAST];
      if (f > 0 && r < 7) passedPawnMask[WHITE][square] |= rays[N][square + WEST];
      // black pawn - ignore that pawns can'*t be on all squares
      passedPawnMask[BLACK][square] |= rays[S][square];
      if (f < 7 && r > 0) passedPawnMask[BLACK][square] |= rays[S][square + EAST];
      if (f > 0 && r > 0) passedPawnMask[BLACK][square] |= rays[S][square + WEST];
    }

    // mask for intermediate squares in between two squares
    for (Square from = SQ_A1; from <= SQ_H8; ++from) {
      for (Square to = SQ_A1; to <= SQ_H8; ++to) {
        for (auto &ray : rays) {
          Bitboard toBB = squareBB[to];
          if (ray[from] & toBB) {
            intermediateBB[from][to] |= ray[from] & ~ray[to] & ~toBB;
          }
        }
      }
    }

    kingSideCastleMask[WHITE] = squareBB[SQ_F1] | squareBB[SQ_G1] | squareBB[SQ_H1];
    kingSideCastleMask[BLACK] = squareBB[SQ_F8] | squareBB[SQ_G8] | squareBB[SQ_H8];
    queenSideCastleMask[WHITE] = squareBB[SQ_D1] | squareBB[SQ_C1] | squareBB[SQ_B1] | squareBB[SQ_A1];
    queenSideCastleMask[BLACK] = squareBB[SQ_D8] | squareBB[SQ_C8] | squareBB[SQ_B8] | squareBB[SQ_A8];

    // masks for each square color (good for bishops vs bishops or pawns)
    Bitboard tmpW = EMPTY_BB;
    Bitboard tmpB = EMPTY_BB;
    for (Square square = SQ_A1; square <= SQ_H8; ++square) {
      int f = fileOf(square);
      int r = rankOf(square);
      if ((f + r) % 2 == 0) { tmpB |= 1L << square; }
      else { tmpW |= 1L << square; }
    }
    whiteSquaresBB = tmpW;
    blackSquaresBB = tmpB;

    // distances to center squares by quadrant
    for (Square square = SQ_A1; square <= SQ_H8; ++square) {
      // left upper quadrant
      if ((squareBB[square] & ranksNorthMask[27] & filesWestMask[36]) != 0) {
        centerDistance[square] = distance(square, SQ_D5);
      }
        // right upper quadrant
      else if ((squareBB[square] & ranksNorthMask[28] & filesEastMask[35]) != 0) {
        centerDistance[square] = distance(square, SQ_E5);
      }
        // left lower quadrant
      else if ((squareBB[square] & ranksSouthMask[35] & filesWestMask[28]) != 0) {
        centerDistance[square] = distance(square, SQ_D4);
      }
        // right lower quadrant
      else if ((squareBB[square] & ranksSouthMask[36] & filesEastMask[27]) != 0) {
        centerDistance[square] = distance(square, SQ_E4);
      }
    }
  }
 
}
