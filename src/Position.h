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

#include "../test/lib/googletest-master/googletest/include/gtest/gtest_prod.h"
#include "datatypes.h"
#include "Values.h"
#include "Bitboards.h"
#include "MoveGenerator.h"

// circle reference between Position and MoveGenerator - this make it possible
class MoveGenerator;

namespace Zobrist {
  // zobrist key for pieces - piece, board
  extern Key pieces[PIECE_LENGTH][SQ_LENGTH];
  extern Key castlingRights[CR_LENGTH];
  extern Key enPassantFile[FILE_LENGTH];
  extern Key nextPlayer;
}

/**
 * This class represents the chess board and its position.<br>
 * It uses a 8x8 piece board and bitboards, a stack for undo moves, zobrist keys for
 * transposition tables, piece lists, material and positional value counter.
 *
 * Can be created with any FEN notation and as a copy from another Position.
 */
class Position {

  ////////////////////////////////////////////////
  ///// FIELDS

  // Flag for boolean states with undetermined state
  enum Flag {
    FLAG_TBD, FLAG_FALSE, FLAG_TRUE
  };

  static const int MAX_HISTORY = 256;

  // history counter
  int historyCounter = 0;

  // The zobrist key to use as a hash key in transposition tables
  // The zobrist key will be updated incrementally every time one of the the state variables change.
  Key zobristKey{};
  Key zobristKey_History[MAX_HISTORY]{};

  // **********************************************************
  // Board State START ----------------------------------------
  // unique chess position (exception is 3-fold repetition
  // which is also not represented in a FEN string)

  // piece Board
  Piece board[SQ_LENGTH]{};

  // Castling rights
  CastlingRights castlingRights;
  CastlingRights castlingRights_History[MAX_HISTORY]{};

  // en passant field
  Square enPassantSquare = SQ_NONE;
  Square enPassantSquare_History[MAX_HISTORY]{};

  // half move clock - number of half moves since last capture
  int halfMoveClock = 0;
  int halfMoveClockHistory[MAX_HISTORY]{};
  // has no zobrist key

  // next player color
  Color nextPlayer = WHITE;

  // Board State END ------------------------------------------
  // **********************************************************

  // **********************************************************
  // Extended Board State -------------------------------------
  // not necessary for a unique position

  // special for king squares
  Square kingSquare[COLOR_LENGTH]{};

  // We can recreate the board through the last move - no need for history of board itself
  // with this we can also capture 3-fold repetition
  Move moveHistory[MAX_HISTORY]{};
  Piece fromPieceHistory[MAX_HISTORY]{};
  Piece capturedPieceHistory[MAX_HISTORY]{};

  // half move number - the actual half move number to determine the full move number
  int nextHalfMoveNumber = 1;

  // piece bitboards
  Bitboard piecesBB[COLOR_LENGTH][PT_LENGTH]{};

  // occupied bitboards with rotations
  Bitboard occupiedBB[COLOR_LENGTH]{};
  Bitboard occupiedBBR90[COLOR_LENGTH]{};
  Bitboard occupiedBBL90[COLOR_LENGTH]{};
  Bitboard occupiedBBR45[COLOR_LENGTH]{};
  Bitboard occupiedBBL45[COLOR_LENGTH]{};

  // Extended Board State END ---------------------------------
  // **********************************************************

  // Material value will always be up to date
  int material[COLOR_LENGTH]{};

  // Positional value will always be up to date
  int psqMidValue[COLOR_LENGTH]{};
  int psqEndValue[COLOR_LENGTH]{};

  // Game phase value
  int gamePhase{};

  // caches a hasCheck and hasMate Flag for the current position. Will be set after
  // a call to hasCheck() and reset to TBD every time a move is made or unmade.
  Flag hasCheckFlag = FLAG_TBD;
  Flag hasCheckFlagHistory[MAX_HISTORY]{};
  Flag hasMateFlag = FLAG_TBD;
  Flag hasMateFlagHistory[MAX_HISTORY]{};

  // To be able to check for mate (no more moves) we need to include
  // a move generator instance
  MoveGenerator mateCheckMG;

public:

  /**
   * Initialize static Position class - should be called once from main();
   */
  static void init();

  /**
   * Creates a standard board position and initializes it with standard chess setup.
   */
  Position();

  /**
   * Creates a standard board position and initializes it with a fen position
   *
   * @param fen
   */
  explicit Position(const char *fen);

  /**
   * Creates a standard board position and initializes it with a fen position
   *
   * @param fen
   */
  explicit Position(const std::string& fen);

  /**
   * Copy constructor - creates a copy of the given Position
   *
   * @param op
   */
  Position(const Position &op);

  /**
   * Destructor
   */
  ~Position();

  /**
  * Returns a String representation of the chess position of this Position as a FEN String.
  *
  * @return FEN String of this position
  */
  friend std::ostream &operator<<(std::ostream &os, Position &position);

  /**
   * @return string showing the position as 8x8 matrix with additional
   *    information about the object's state
   *
   */
  std::string str() const;

  /**
   * @return string showing the position as a 8x8 matrix
   */
  std::string printBoard() const;

  /**
   * Returns a String representation of the chess position of this Position as a FEN String.
   *
   * @return FEN String of this position
   */
  std::string printFen() const;

  /**
  * Commits a move to the board. Due to performance there is no check if this move is legal on the
  * current position. Legal check needs to be done beforehand. Usually the move will be
  * generated by
  * our MoveGenerator and therefore the move will be assumed legal anyway.
  *
  * @param move the move
  */
  void doMove(Move move);

  /**
   * Takes back the last move from the board
   */
  void undoMove();

  /**
  * Makes a null move. Essentially switches sides within same position.
  */
  void doNullMove();

  /**
   * Undo a null move. Essentially switches back sides within same position.
   */
  void undoNullMove();

  /**
   * This determines if the current position has a check against the next player.
   * The result is cached so that several calls to this for the same position have
   * no extra performance hit.
   *
   * @return true if current position has check for next player
   */
  bool hasCheck();

  /**
   * Tests for mate on this position. If true the next player has has no move and is in check.
   * Expensive test as all legal moves have to be generated.
   *
   * TODO: This is violating encapsulation as it's current implementation needs
   *  a move generator to generate all possible moves to see if there are any
   *  legal moves. The MoveGenerator also needs to know Position which leads to
   *  a circle reference.
   *
   * @return true if current position is mate for next player
   */
  bool hasCheckMate();

  /**
   * Checks if move is giving check to the opponent.
   * This method is faster than making the move and checking for legality and giving check.
   * Needs to be a valid move for the position otherwise will crash.
   * For performance reason we do not want to check validity here.
   * Does NOT check if the move itself is legal (leaves the own king in check)
   *
   * @param move
   * @return true if move is giving check to opponent
   */
  bool givesCheck(Move move);

  /**
  * This checks if a certain square is currently under attack by the player of the given color. It
  * does not matter who has the next move on this position. It also is not checking if the actual
  * attack can be done as a legal move. E.g. a pinned piece could not actually make a capture on
  * the square.
  *
  * @param attackedSquare
  * @param attackerColor
  * @return true if under attack
  */
  bool isAttacked(Square sq, Color byColor);

  /**
   * This checks if the  move is legal by checking if it leaves the king in check or if it
   * would pass an attacked square when castling.
   */
  bool isLegalMove(Move move);

  /**
   * This checks if the last move was legal by checking if it left the king in check or if
   * the king has passed an attacked square during castling
   */
  bool isLegalPosition();

  /**
   * The fifty-move rule if during the previous 50 moves no pawn has been moved and no capture has
   * been made, either player may claim a draw.
   *
   * @return true if during the previous 50 moves no pawn has been moved and no capture has been
   * made
   */
  bool check50MovesRule() { return halfMoveClock >= 100; };

  /**
  * Repetition of a position:.
  * To detect a 3-fold repetition the given position most occurr at least 2 times before:<br/>
  * <code>position.checkRepetitions(2)</code> checks for 3 fold-repetition
  * <p>
  * 3-fold repetition: This most commonly occurs when neither side is able to avoid
  * repeating moves without incurring a disadvantage. The three occurrences of the position need
  * not occur on consecutive moves for a claim to be valid. FIDE rules make no mention of perpetual
  * check; this is merely a specific type of draw by threefold repetition.
  *
  * @return true if this position has been played reps times before
  */
  bool checkRepetitions(int reps);

  /**
  * Determines the repetitions of a position.
  *
  * @return number of repetitions
  */
  int countRepetitions();

  /**
   * FIDE Draws - Evaluation might define some more draw values.
   *
   * @return true if neither side can win
   */
  bool checkInsufficientMaterial();

  /**
  * Returns the last move. Returns Move.NOMOVE if there is no last move.
  *
  * @return int representing a move
  */
  Move getLastMove() { return historyCounter > 0 ? moveHistory[historyCounter - 1] : NOMOVE; };


  ////////////////////////////////////////////////
  ///// GETTER / SETTER
  Piece getPiece(Square square) const { return board[square]; }
  Key getZobristKey() const { return zobristKey; }
  Color getNextPlayer() const { return nextPlayer; }
  Square getEnPassantSquare() const { return enPassantSquare; }
  Square getKingSquare(const Color color) const { return kingSquare[color]; };

  Bitboard getPieceBB(Color c, PieceType pt) const { return piecesBB[c][pt]; }
  Bitboard getOccupiedBB() const { return occupiedBB[WHITE] | occupiedBB[BLACK]; }
  Bitboard getOccupiedBB(Color c) const { return occupiedBB[c]; }
  Bitboard getOccupiedBBR90() const { return occupiedBBR90[WHITE] | occupiedBBR90[BLACK]; }
  Bitboard getOccupiedBBR90(Color c) const { return occupiedBBR90[c]; }
  Bitboard getOccupiedBBL90() const { return occupiedBBL90[WHITE] | occupiedBBL90[BLACK]; }
  Bitboard getOccupiedBBL90(Color c) const { return occupiedBBL90[c]; }
  Bitboard getOccupiedBBR45() const { return occupiedBBR45[WHITE] | occupiedBBR45[BLACK]; }
  Bitboard getOccupiedBBR45(Color c) const { return occupiedBBR45[c]; }
  Bitboard getOccupiedBBL45() const { return occupiedBBL45[WHITE] | occupiedBBL45[BLACK]; }
  Bitboard getOccupiedBBL45(Color c) const { return occupiedBBL45[c]; }

  int getMaterial(Color c) const { return material[c]; }
  int getMidPosValue(Color c) const { return psqMidValue[c]; }
  int getEndPosValue(Color c) const { return psqEndValue[c]; }
  int getPosValue(Color c) const {
    return static_cast<int>(
      getGamePhaseFactor() * psqMidValue[c] + (1 - getGamePhaseFactor()) * psqEndValue[c]);
  }
  int getGamePhase() const { return gamePhase; }
  float getGamePhaseFactor() const { return float(gamePhase) / GAME_PHASE_MAX; }
  CastlingRights getCastlingRights() const { return castlingRights; }
  int getHalfMoveClock() const { return halfMoveClock; }
  int getNextHalfMoveNumber() const { return nextHalfMoveNumber; }

private:

  ////////////////////////////////////////////////
  ///// FUNC

  FRIEND_TEST(PositionTest, PosValue);

  void initializeBoard();
  void setupBoard(const char *fen);
  void movePiece(Square from, Square to);
  void putPiece(Piece piece, Square square);
  Piece removePiece(Square square);
  void invalidateCastlingRights(Square from, Square to);
  void clearEnPassant();
};

#endif //FRANKYCPP_POSITION_H
