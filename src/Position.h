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

#ifndef FRANKYCPP_POSITION_H
#define FRANKYCPP_POSITION_H

#include <cstdint>
#include "globals.h"
#include "Random.h"
#include "Values.h"
#include "../test/lib/googletest-master/googletest/include/gtest/gtest_prod.h"

namespace Zobrist {
  // zobrist key for pieces - piece, board
  extern Key pieces[PIECE_LENGTH][SQ_LENGTH];
  extern Key castlingRights[CR_LENGTH];
  extern Key enPassantFile[FILE_LENGTH];
  extern Key nextPlayer;
}

class Position {

  // Flag for boolean states with undetermined state
  enum Flag {
    FLAG_FALSE, FLAG_TRUE, FLAG_TBD
  };

  // Random number generator
  static const Random rng;

public:
  static const u_int8_t GAME_PHASE_MAX = 24;

  Position();
  Position(const char *fen);
  Position(const Position &op);

  static void init();

private:

  static const int MAX_HISTORY = 256;

  // history counter
  int historyCounter = 0;

  /*
 * The zobrist key to use as a hash key in transposition tables
 * The zobrist key will be updated incrementally every time one of the the state variables change.
 */
  Key zobristKey;
  Key zobristKey_History[MAX_HISTORY];

  // **********************************************************
  // Board State START ----------------------------------------
  // unique chess position (exception is 3-fold repetition
  // which is also not represented in a FEN string)
  //
  // piece Board
  Piece board[SQ_LENGTH];

  // Castling rights
  CastlingRights castlingRights;
  CastlingRights castlingRights_History[MAX_HISTORY];

  // en passant field - if NOSQUARE then we do not have an en passant option
  Square enPassantSquare = SQ_NONE;
  Square enPassantSquare_History[MAX_HISTORY];

  // half move clock - number of half moves since last capture
  int halfMoveClock = 0;
  int halfMoveClockHistory[MAX_HISTORY];
  // has no zobrist key

  // next player color
  Color nextPlayer = WHITE;

  // Board State END ------------------------------------------
  // **********************************************************

  // **********************************************************
  // Extended Board State -------------------------------------
  // not necessary for a unique position

  // We can recreate the board through the last move - no need for history of board itself
  // with this we can also capture 3-fold repetition
  int moveHistory[MAX_HISTORY];

  // half move number - the actual half move number to determine the full move number
  int nextHalfMoveNumber = 1;

  // piece bitboards
  Bitboard piecesBB[COLOR_LENGTH][PT_LENGTH];

  // occupied bitboards with rotations
  Bitboard occupiedBB[COLOR_LENGTH];
  Bitboard occupiedBBR90[COLOR_LENGTH];
  Bitboard occupiedBBL90[COLOR_LENGTH];
  Bitboard occupiedBBR45[COLOR_LENGTH];
  Bitboard occupiedBBL45[COLOR_LENGTH];

  // Material value will always be up to date
  int material[COLOR_LENGTH];

  // Positional value will always be up to date
  int psqMGValue[COLOR_LENGTH];
  int psqEGValue[COLOR_LENGTH];

  // Game phase value
  int gamePhase;

  // caches a hasCheck and hasMate Flag for the current position. Will be set after
  // a call to hasCheck() and reset to TBD every time a move is made or unmade.
  Flag hasCheck = FLAG_TBD;
  Flag hasCheckFlagHistory[MAX_HISTORY];
  Flag hasMate = FLAG_TBD;
  Flag hasMateFlagHistory[MAX_HISTORY];

  // Extended Board State END ---------------------------------
  // **********************************************************

  void initializeBoard();
  void setupBoard(const char *fen);

public:
  friend std::ostream &operator<<(std::ostream &os, Position &position);

  std::string str() const;
  std::string printBoard() const;
  std::string printFen() const;

  void doMove(Move move);
  void undoMove();
  void doNullMove();
  void undoNullMove();

  ////////////////////////////////////////////////
  ///// GETTER / SETTER
  inline Piece getPiece(Square square) const { return board[square]; }
  inline Key getZobristKey() const { return zobristKey; }
  inline Color getNextPlayer() const { return nextPlayer; }
  inline Square getEnPassantSquare() const { return enPassantSquare; }
  inline Bitboard getPieceBB(Color c, PieceType pt) const { return piecesBB[c][pt]; }
  inline Bitboard getOccupiedBB(Color c) const { return occupiedBB[c]; }
  inline Bitboard getOccupiedBB() const { return occupiedBB[WHITE] | occupiedBB[BLACK]; }
  inline int getMaterial(Color c) const { return material[c]; }
  inline int getMgPosValue(Color c) const { return psqMGValue[c]; }
  inline int getEgPosValue(Color c) const { return psqEGValue[c]; }
  inline int getGamePhase() const { return gamePhase; }

private:
  FRIEND_TEST(PositionTest, PosValue);
  void movePiece(Square from, Square to);
  void putPiece(Piece piece, Square square);
  Piece removePiece(Square square);
  void invalidateCastlingRights(Square from, Square to);
  void clearEnPassant();
};

#endif //FRANKYCPP_POSITION_H
