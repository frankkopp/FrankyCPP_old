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

#include <iostream>
#include <sstream>

#include "Position.h"
#include "MoveGenerator.h"

using namespace std;
using namespace Bitboards;

Key Zobrist::pieces[PIECE_LENGTH][SQ_LENGTH];
Key Zobrist::castlingRights[CR_LENGTH];
Key Zobrist::enPassantFile[FILE_LENGTH];
Key Zobrist::nextPlayer;

////////////////////////////////////////////////
///// STATIC

void Position::init() {

  // Zobrist Key initialization
  Random random(1070372);
  for (Piece pc = WHITE_KING; pc < PIECE_LENGTH; ++pc) {
    for (Square sq = SQ_A1; sq < SQ_LENGTH; ++sq) {
      Zobrist::pieces[pc][sq] = random.rand<Key>();
    }
  }
  for (CastlingRights cr = NO_CASTLING; cr <= ANY_CASTLING; ++cr) {
    Zobrist::castlingRights[cr] = random.rand<Key>();
  }
  for (File f = FILE_A; f <= FILE_H; ++f) {
    Zobrist::enPassantFile[f] = random.rand<Key>();
  }
  Zobrist::nextPlayer = random.rand<Key>();

}

////////////////////////////////////////////////
///// CONSTRUCTORS

/** Default constructor creates a board with standard start setup */
Position::Position() : Position(START_POSITION_FEN) {};

/** Creates a board with setup from the given fen */
Position::Position(std::string fen) : Position(fen.c_str()) {};

/** Creates a board with setup from the given fen */
Position::Position(const char *fen) {
  setupBoard(fen);
}

/** Copy constructor creates a board as a deep copy from the given board */
Position::Position(const Position &op) {
  // piece list
  std::copy(op.board, op.board + SQ_LENGTH, this->board);

  // game state
  this->zobristKey = op.zobristKey;
  this->castlingRights = op.castlingRights;
  this->enPassantSquare = op.enPassantSquare;
  this->nextPlayer = op.nextPlayer;
  this->halfMoveClock = op.halfMoveClock;
  this->nextHalfMoveNumber = op.nextHalfMoveNumber;
  this->gamePhase = op.gamePhase;
  this->hasCheckFlag = op.hasCheckFlag;
  this->hasMateFlag = op.hasMateFlag;
  // color dependent
  for (Color c = WHITE; c <= BLACK; ++c) {
    std::copy(op.piecesBB[c],
              op.piecesBB[c] + PT_LENGTH,
              this->piecesBB[c]);
    this->kingSquare[c] = op.kingSquare[c];
    this->occupiedBB[c] = op.occupiedBB[c];
    this->occupiedBBR90[c] = op.occupiedBBR90[c];
    this->occupiedBBL90[c] = op.occupiedBBL90[c];
    this->occupiedBBR45[c] = op.occupiedBBR45[c];
    this->occupiedBBL45[c] = op.occupiedBBL45[c];

    this->material[c] = op.material[c];
    this->psqMidValue[c] = op.psqMidValue[c];
  }
  // necessary history b/o move repetition
  this->historyCounter = op.historyCounter;
  std::copy(op.moveHistory, op.moveHistory + MAX_HISTORY, this->moveHistory);
  // history - maybe be not necessary - without we only loose undo move
  std::copy(op.zobristKey_History, op.zobristKey_History + MAX_HISTORY, this->zobristKey_History);
  std::copy(op.castlingRights_History, op.castlingRights_History + MAX_HISTORY,
            this->castlingRights_History);
  std::copy(op.enPassantSquare_History, op.enPassantSquare_History + MAX_HISTORY,
            this->enPassantSquare_History);
  std::copy(op.halfMoveClockHistory, op.halfMoveClockHistory + MAX_HISTORY,
            this->halfMoveClockHistory);
  std::copy(op.hasCheckFlagHistory, op.hasCheckFlagHistory + MAX_HISTORY,
            this->hasCheckFlagHistory);
  std::copy(op.hasMateFlagHistory, op.hasMateFlagHistory + MAX_HISTORY, this->hasMateFlagHistory);
}

Position::~Position() {
}

////////////////////////////////////////////////
///// PUBLIC

void Position::doMove(Move move) {
  assert(isMove(move));
  assert(isSquare(getFromSquare(move)));
  assert(isSquare(getToSquare(move)));
  assert(getPiece(getFromSquare(move)) != PIECE_NONE);

  const MoveType moveType = typeOf(move);

  const Square fromSq = getFromSquare(move);
  const Piece fromPC = getPiece(fromSq);
  const PieceType fromPT = typeOf(fromPC);
  const Color myColor = colorOf(fromPC);
  assert (myColor == nextPlayer);

  const Square toSq = getToSquare(move);
  const Piece targetPC = getPiece(toSq);

  const PieceType promotionPT = promotionType(move);

  // save state of board for undo
  moveHistory[historyCounter] = move;
  fromPieceHistory[historyCounter] = fromPC;
  capturedPieceHistory[historyCounter] = targetPC;
  castlingRights_History[historyCounter] = castlingRights;
  enPassantSquare_History[historyCounter] = enPassantSquare;
  halfMoveClockHistory[historyCounter] = halfMoveClock;
  zobristKey_History[historyCounter] = zobristKey;
  hasCheckFlagHistory[historyCounter] = hasCheckFlag;
  hasMateFlagHistory[historyCounter] = hasMateFlag;
  historyCounter++;

  // reset check and mate flag
  hasCheckFlag = FLAG_TBD;
  hasMateFlag = FLAG_TBD;

  // do move
  switch (moveType) {

    case NORMAL:
      if (castlingRights && (CastlingMask & fromSq || CastlingMask & toSq)) {
        invalidateCastlingRights(fromSq, toSq);
      }
      clearEnPassant();
      if (targetPC != PIECE_NONE) { // capture
        removePiece(toSq);
        halfMoveClock = 0; // reset half move clock because of capture
      }
      else if (fromPT == PAWN) {
        halfMoveClock = 0; // reset half move clock because of pawn move
        if (distance(fromSq, toSq) == 2) { // pawn double - set en passant
          // set new en passant target field - always one "behind" the toSquare
          enPassantSquare = toSq + pawnDir[~myColor];
          zobristKey = zobristKey ^ Zobrist::enPassantFile[fileOf(enPassantSquare)]; // in
        }
      }
      else halfMoveClock++;
      movePiece(fromSq, toSq);
      break;

    case PROMOTION:
      assert(fromPC == makePiece(myColor, PAWN));
      assert(rankOf(toSq) == (myColor == WHITE ? RANK_8 : RANK_1));
      if (targetPC != PIECE_NONE) removePiece(toSq); // capture
      if (castlingRights && (CastlingMask & fromSq || CastlingMask & toSq)) {
        invalidateCastlingRights(fromSq, toSq);
      }
      removePiece(fromSq);
      putPiece(makePiece(myColor, promotionPT), toSq);
      clearEnPassant();
      halfMoveClock = 0; // reset half move clock because of pawn move
      break;

    case ENPASSANT: {
      assert(fromPC == makePiece(myColor, PAWN));
      assert(enPassantSquare != SQ_NONE);
      Square capSq(toSq + pawnDir[~myColor]);
      assert(getPiece(capSq) == makePiece(~myColor, PAWN));
      removePiece(capSq);
      movePiece(fromSq, toSq);
      clearEnPassant();
      halfMoveClock = 0; // reset half move clock because of pawn move
      break;
    }

    case CASTLING:
      assert(fromPC == makePiece(myColor, KING));
      switch (toSq) {
        case SQ_G1:
          assert (castlingRights == WHITE_OO);
          assert (fromSq == SQ_E1);
          assert (getPiece(SQ_E1) == WHITE_KING);
          assert (getPiece(SQ_H1) == WHITE_ROOK);
          assert (!(getOccupiedBB() & intermediateBB[SQ_E1][SQ_H1]));
          movePiece(fromSq, toSq); // King
          movePiece(SQ_H1, SQ_F1); // Rook
          zobristKey ^= Zobrist::castlingRights[castlingRights]; // out
          // clear both by first OR then XOR
          castlingRights += WHITE_CASTLING;
          castlingRights -= WHITE_CASTLING;
          zobristKey ^= Zobrist::castlingRights[castlingRights]; // in;
          break;
        case SQ_C1:
          assert (castlingRights == WHITE_OOO);
          assert (fromSq == SQ_E1);
          assert (getPiece(SQ_E1) == WHITE_KING);
          assert (getPiece(SQ_A1) == WHITE_ROOK);
          assert (!(getOccupiedBB() & intermediateBB[SQ_E1][SQ_A1]));
          movePiece(fromSq, toSq); // King
          movePiece(SQ_A1, SQ_D1); // Rook
          zobristKey ^= Zobrist::castlingRights[castlingRights]; // out
          castlingRights += WHITE_CASTLING;
          castlingRights -= WHITE_CASTLING;
          zobristKey ^= Zobrist::castlingRights[castlingRights]; // in
          break;
        case SQ_G8:
          assert (castlingRights == BLACK_OO);
          assert (fromSq == SQ_E8);
          assert (getPiece(SQ_E8) == BLACK_KING);
          assert (getPiece(SQ_H8) == BLACK_ROOK);
          assert (!(getOccupiedBB() & intermediateBB[SQ_E8][SQ_H8]));
          movePiece(fromSq, toSq); // King
          movePiece(SQ_H8, SQ_F8); // Rook
          zobristKey ^= Zobrist::castlingRights[castlingRights]; // out
          castlingRights += BLACK_CASTLING;
          castlingRights -= BLACK_CASTLING;
          zobristKey ^= Zobrist::castlingRights[castlingRights]; // in
          break;
        case SQ_C8:
          assert (castlingRights == BLACK_OOO);
          assert (fromSq == SQ_E8);
          assert (getPiece(SQ_E8) == BLACK_KING);
          assert (getPiece(SQ_A8) == BLACK_ROOK);
          assert (!(getOccupiedBB() & intermediateBB[SQ_E8][SQ_A8]));
          movePiece(fromSq, toSq); // King
          movePiece(SQ_A8, SQ_D8); // Rook
          zobristKey ^= Zobrist::castlingRights[castlingRights]; // out
          castlingRights += BLACK_CASTLING;
          castlingRights -= BLACK_CASTLING;
          zobristKey ^= Zobrist::castlingRights[castlingRights]; // in
          break;
        default:
          throw std::invalid_argument("Invalid castle move!");
      }
      clearEnPassant();
      halfMoveClock++;
      break;
  }

  // update halfMoveNumber
  nextHalfMoveNumber++;

  // change color (active player)
  nextPlayer = ~nextPlayer;
  zobristKey = this->zobristKey ^ Zobrist::nextPlayer;
}

void Position::undoMove() {
  assert(historyCounter > 0);

  // Restore state part 1
  historyCounter--;
  nextHalfMoveNumber--;
  nextPlayer = ~nextPlayer;
  const Move move = moveHistory[historyCounter];

  // undo piece move / restore board
  switch (typeOf(move)) {

    case NORMAL:
      movePiece(getToSquare(move), getFromSquare(move));
      if (capturedPieceHistory[historyCounter] != PIECE_NONE)
        putPiece(
          capturedPieceHistory[historyCounter], getToSquare(move));
      break;

    case PROMOTION:
      removePiece(getToSquare(move));
      putPiece(makePiece(nextPlayer, PAWN), getFromSquare(move));
      if (capturedPieceHistory[historyCounter] != PIECE_NONE)
        putPiece(
          capturedPieceHistory[historyCounter], getToSquare(move));
      break;

    case ENPASSANT:
      // ignore Zobrist Key as it will be restored via history
      movePiece(getToSquare(move), getFromSquare(move));
      putPiece(makePiece(~nextPlayer, PAWN), getToSquare(move) + pawnDir[~nextPlayer]);
      break;

    case CASTLING:
      // ignore Zobrist Key as it will be restored via history
      // castling rights are restored via history
      switch (getToSquare(move)) {
        case SQ_G1:
          movePiece(getToSquare(move), getFromSquare(move)); // King
          movePiece(SQ_F1, SQ_H1); // Rook
          break;
        case SQ_C1:
          movePiece(getToSquare(move), getFromSquare(move)); // King
          movePiece(SQ_D1, SQ_A1); // Rook
          break;
        case SQ_G8:
          movePiece(getToSquare(move), getFromSquare(move)); // King
          movePiece(SQ_F8, SQ_H8); // Rook
          break;
        case SQ_C8:
          movePiece(getToSquare(move), getFromSquare(move)); // King
          movePiece(SQ_D8, SQ_A8); // Rook
          break;
        default:
          throw std::invalid_argument("Invalid castle move!");
      }
      break;
  }

  // restore state part 2
  castlingRights = castlingRights_History[historyCounter];
  enPassantSquare = enPassantSquare_History[historyCounter];
  halfMoveClock = halfMoveClockHistory[historyCounter];
  zobristKey = zobristKey_History[historyCounter];
  hasCheckFlag = hasCheckFlagHistory[historyCounter];
  hasMateFlag = hasMateFlagHistory[historyCounter];
}

void Position::doNullMove() {
  // save state of board for undo
  moveHistory[historyCounter] = NOMOVE;
  fromPieceHistory[historyCounter] = PIECE_NONE;
  capturedPieceHistory[historyCounter] = PIECE_NONE;
  castlingRights_History[historyCounter] = castlingRights;
  enPassantSquare_History[historyCounter] = enPassantSquare;
  halfMoveClockHistory[historyCounter] = halfMoveClock;
  zobristKey_History[historyCounter] = zobristKey;
  hasCheckFlagHistory[historyCounter] = hasCheckFlag;
  hasMateFlagHistory[historyCounter] = hasMateFlag;
  historyCounter++;

  // reset check and mate flag
  hasCheckFlag = FLAG_TBD;
  hasMateFlag = FLAG_TBD;

  // clear en passant
  if (enPassantSquare != SQ_NONE) {
    zobristKey = zobristKey ^ Zobrist::enPassantFile[fileOf(enPassantSquare)]; // out
    enPassantSquare = SQ_NONE;
  }

  // update halfMoveNumber
  nextHalfMoveNumber++;

  // change color (active player)
  nextPlayer = ~nextPlayer;
  zobristKey = zobristKey ^ Zobrist::nextPlayer;
}

void Position::undoNullMove() {
  // Restore state part 1
  historyCounter--;
  nextHalfMoveNumber--;
  nextPlayer = ~nextPlayer;
  castlingRights = castlingRights_History[historyCounter];
  enPassantSquare = enPassantSquare_History[historyCounter];
  halfMoveClock = halfMoveClockHistory[historyCounter];
  zobristKey = zobristKey_History[historyCounter];
  hasCheckFlag = hasCheckFlagHistory[historyCounter];
  hasMateFlag = hasMateFlagHistory[historyCounter];
}

bool Position::isAttacked(Square attackedSquare, Color attackerColor) {
  assert(attackedSquare != SQ_NONE);
  assert(attackerColor != NOCOLOR);

  // check pawns
  if (Bitboards::pawnAttacks[~attackerColor][attackedSquare] & piecesBB[attackerColor][PAWN])
    return true;

  // check knights
  if (Bitboards::pseudoAttacks[KNIGHT][attackedSquare] & piecesBB[attackerColor][KNIGHT])
    return true;

  // check king
  if ((Bitboards::pseudoAttacks[KING][attackedSquare] & piecesBB[attackerColor][KING]))
    return true;

  // Sliding
  // rooks and queens
  if (((Bitboards::pseudoAttacks[ROOK][attackedSquare] & piecesBB[attackerColor][ROOK])
       || ((Bitboards::pseudoAttacks[QUEEN][attackedSquare] & piecesBB[attackerColor][QUEEN])))

      && ((Bitboards::getMovesRank(attackedSquare, getOccupiedBB())
           | Bitboards::getMovesFileR(attackedSquare, getOccupiedBBL90())) &
          (piecesBB[attackerColor][ROOK] | piecesBB[attackerColor][QUEEN])))
    return true;

  // bishop and queens
  if (((Bitboards::pseudoAttacks[BISHOP][attackedSquare] & piecesBB[attackerColor][BISHOP])
       || ((Bitboards::pseudoAttacks[QUEEN][attackedSquare] & piecesBB[attackerColor][QUEEN])))

      && ((Bitboards::getMovesDiagUpR(attackedSquare, getOccupiedBBR45())
           | Bitboards::getMovesDiagDownR(attackedSquare, getOccupiedBBL45()))
          & (piecesBB[attackerColor][BISHOP] | piecesBB[attackerColor][QUEEN])))
    return true;

  // check en passant
  if (enPassantSquare != SQ_NONE) {
    // white is attacker
    if (attackerColor == WHITE
        // black is target
        && getPiece(enPassantSquare + SOUTH) == BLACK_PAWN
        // this is indeed the en passant attacked square
        && enPassantSquare + SOUTH == attackedSquare) {
      // left
      Square sq = attackedSquare + WEST;
      if (distance(attackedSquare, sq) == 1 && getPiece(sq) == WHITE_PAWN) return true;
      // right
      sq = attackedSquare + EAST;
      return distance(attackedSquare, sq) == 1 && getPiece(sq) == WHITE_PAWN;
    }
    // black is attacker (assume not noColor)
    else if (attackerColor == BLACK
             // white is target
             && getPiece(enPassantSquare + NORTH) == WHITE_PAWN
             // this is indeed the en passant attacked square
             && enPassantSquare + NORTH == attackedSquare) {
      // attack from left
      Square sq = attackedSquare + WEST;
      if (distance(attackedSquare, sq) == 1 && getPiece(sq) == BLACK_PAWN) return true;
      // right
      sq = attackedSquare + EAST;
      return distance(attackedSquare, sq) == 1 && getPiece(sq) == BLACK_PAWN;
    }
  }
  return false;
}

bool Position::isLegalMove(Move move) {
  // king is not allowed to pass a square which is attacked by opponent
  if (typeOf(move) == CASTLING) {
    switch (getToSquare(move)) {
      case SQ_G1:
        if (isAttacked(SQ_E1, ~nextPlayer)) return false;
        if (isAttacked(SQ_F1, ~nextPlayer)) return false;
        break;
      case SQ_C1:
        if (isAttacked(SQ_E1, ~nextPlayer)) return false;
        if (isAttacked(SQ_D1, ~nextPlayer)) return false;
        break;
      case SQ_G8:
        if (isAttacked(SQ_E8, ~nextPlayer)) return false;
        if (isAttacked(SQ_F8, ~nextPlayer)) return false;
        break;
      case SQ_C8:
        if (isAttacked(SQ_E8, ~nextPlayer)) return false;
        if (isAttacked(SQ_D8, ~nextPlayer)) return false;
        break;
      default:
        break;
    }
  }
  // make the move on the position
  // TODO: can we make this more efficient??
  doMove(move);
  // check if the move leaves the king in check
  if (!isAttacked(kingSquare[~nextPlayer], nextPlayer)) {
    undoMove();
    return true;
  }
  undoMove();
  return false;
}

bool Position::isLegalPosition() {
  if (historyCounter > 0) {
    Move lastMove = moveHistory[historyCounter - 1];
    if (typeOf(lastMove) == CASTLING) {
      // king is not allowed to pass a square which is attacked by opponent
      switch (getToSquare(lastMove)) {
        case SQ_G1:
          if (isAttacked(SQ_E1, nextPlayer)) return false;
          if (isAttacked(SQ_F1, nextPlayer)) return false;
          break;
        case SQ_C1:
          if (isAttacked(SQ_E1, nextPlayer)) return false;
          if (isAttacked(SQ_D1, nextPlayer)) return false;
          break;
        case SQ_G8:
          if (isAttacked(SQ_E8, nextPlayer)) return false;
          if (isAttacked(SQ_F8, nextPlayer)) return false;
          break;
        case SQ_C8:
          if (isAttacked(SQ_E8, nextPlayer)) return false;
          if (isAttacked(SQ_D8, nextPlayer)) return false;
          break;
        default:
          break;
      }
    }
  }
  return !isAttacked(kingSquare[~nextPlayer], nextPlayer);
}

bool Position::hasCheck() {
  if (hasCheckFlag != FLAG_TBD) return (hasCheckFlag == FLAG_TRUE);
  const bool check = isAttacked(kingSquare[nextPlayer], ~nextPlayer);
  hasCheckFlag = check ? FLAG_TRUE : FLAG_FALSE;
  return check;
}

bool Position::hasCheckMate() {
  if (!hasCheck()) return false;
  if (hasMateFlag != FLAG_TBD) return (hasMateFlag == FLAG_TRUE);
  const bool hasLegalMove = mateCheckMG.hasLegalMove(this);
   if (!hasLegalMove) {
    hasMateFlag = FLAG_TRUE;
    return true;
  }
  hasMateFlag = FLAG_FALSE;
  return false;
}

bool Position::checkRepetitions(int reps) {
  /*
   [0]     3185849660387886977 << 1st
   [1]     447745478729458041
   [2]     3230145143131659788
   [3]     491763876012767476
   [4]     3185849660387886977 << 2nd
   [5]     447745478729458041
   [6]     3230145143131659788
   [7]     491763876012767476  <<< history
   [8]     3185849660387886977 <<< 3rd REPETITION from current zobrist
    */
  int counter = 0;
  int i = historyCounter - 2;
  int lastHalfMove = halfMoveClock;
  while (i >= 0) {
    // every time the half move clock gets reset (non reversible position) there
    // can't be any more repetition of positions before this position
    if (halfMoveClockHistory[i] >= lastHalfMove) break;
    else lastHalfMove = halfMoveClockHistory[i];
    if (zobristKey == zobristKey_History[i]) counter++;
    if (counter >= reps) return true;
    i -= 2;
  }
  return false;
}

int Position::countRepetitions() {
  int counter = 0;
  int i = historyCounter - 2;
  int lastHalfMove = halfMoveClock;
  while (i >= 0) {
    // every time the half move clock gets reset (non reversible position) there
    // can't be any more repetition of positions before this position
    if (halfMoveClockHistory[i] >= lastHalfMove) break;
    else lastHalfMove = halfMoveClockHistory[i];
    if (zobristKey == zobristKey_History[i]) counter++;
    i -= 2;
  }
  return counter;
}

bool Position::checkInsufficientMaterial() {
  // TODO optimize??

  /*
    * both sides have a bare king
    * one side has a king and a minor piece against a bare king
    * one side has two knights against the bare king
    * both sides have a king and a bishop, the bishops being the same color
    */
  if (popcount(piecesBB[WHITE][PAWN]) == 0 && popcount(piecesBB[BLACK][PAWN]) == 0
      && popcount(piecesBB[WHITE][ROOK]) == 0 && popcount(piecesBB[BLACK][ROOK]) == 0
      && popcount(piecesBB[WHITE][QUEEN]) == 0 && popcount(piecesBB[BLACK][QUEEN]) == 0) {

    // white king bare KK*
    if (popcount(piecesBB[WHITE][KNIGHT]) == 0 && popcount(piecesBB[WHITE][BISHOP]) == 0) {

      // both kings bare KK, KKN, KKNN
      if (popcount(piecesBB[BLACK][KNIGHT]) <= 2 && popcount(piecesBB[BLACK][BISHOP]) == 0) {
        return true;
      }

      // KKB
      return popcount(piecesBB[BLACK][KNIGHT]) == 0 && popcount(piecesBB[BLACK][BISHOP]) == 1;

    }
      // only black king bare K*K
    else if (popcount(piecesBB[BLACK][KNIGHT]) == 0 && popcount(piecesBB[BLACK][BISHOP]) == 0) {

      // both kings bare KK, KNK, KNNK
      if (popcount(piecesBB[WHITE][KNIGHT]) <= 2 && popcount(piecesBB[WHITE][BISHOP]) == 0) {
        return true;
      }

      // KBK
      return popcount(piecesBB[BLACK][KNIGHT]) == 0 && popcount(piecesBB[BLACK][BISHOP]) == 1;
    }

      // KBKB - B same field color
    else if (popcount(piecesBB[BLACK][KNIGHT]) == 0 && popcount(piecesBB[BLACK][BISHOP]) == 1
             && popcount(piecesBB[WHITE][KNIGHT]) == 0 && popcount(piecesBB[WHITE][BISHOP]) == 1) {

      // bishops on the same square color
      return (
        ((whiteSquaresBB & piecesBB[WHITE][BISHOP]) && (whiteSquaresBB & piecesBB[BLACK][BISHOP]))
        ||
        ((blackSquaresBB & piecesBB[WHITE][BISHOP]) && (blackSquaresBB & piecesBB[BLACK][BISHOP])));
    }
  }
  return false;
}

bool Position::givesCheck(Move move) {

  // opponents king square
  const Bitboard kingBB = piecesBB[~nextPlayer][KING];
  const Square kingSquare = lsb(kingBB);
  // fromSquare
  const Square fromSquare = getFromSquare(move);
  // target square
  Square toSquare = getToSquare(move);
  // the moving piece
  Piece fromPc = getPiece(fromSquare);
  PieceType fromPt = typeOf(fromPc);
  // the square captured by en passant capture
  Square epTargetSquare = SQ_NONE;

  // promotion moves - use new piece type
  const MoveType moveType = typeOf(move);
  if (moveType == PROMOTION) {
    fromPt = promotionType(move);
  }
    // Castling
  else if (moveType == CASTLING) {
    // set the target square to the rook square and
    // piece type to ROOK. King can't give check
    // also no revealed check possible in castling
    fromPt = ROOK;
    switch (toSquare) {
      case SQ_G1: // white king side castle
        toSquare = SQ_F1;
        break;
      case SQ_C1: // white queen side castle
        toSquare = SQ_D1;
        break;
      case SQ_G8: // black king side castle
        toSquare = SQ_F8;
        break;
      case SQ_C8: // black queen side castle
        toSquare = SQ_D8;
        break;
      default:
        break;
    }
  }
    // en passant
  else if (moveType == ENPASSANT) {
    epTargetSquare = toSquare + pawnDir[~colorOf(fromPc)];
  }

  // queen can be rook or bishop
  if (fromPt == QUEEN) {
    // if queen on same rank or same file then she acts like rook
    // otherwise like bishop
    if (rankOf(toSquare) == rankOf(kingSquare) || fileOf(toSquare) == fileOf(kingSquare)) {
      fromPt = ROOK;
    }
    else {
      fromPt = BISHOP;
    }
  }

  // get all pieces to check occupied intermediate squares
  Bitboard allOccupiedBitboard = getOccupiedBB();
  Bitboard intermediate, boardAfterMove;

  // #########################################################################
  // Direct checks
  // #########################################################################
  switch (fromPt) {

    case PAWN:
      // normal pawn direct chess include en passant captures
      if (Bitboards::pawnAttacks[colorOf(fromPc)][toSquare] & kingSquare) return true;
      else break;

    case KNIGHT:
      if (Bitboards::pseudoAttacks[KNIGHT][toSquare] & kingSquare) return true;
      else break;

    case ROOK:
      // is attack even possible
      if (Bitboards::pseudoAttacks[ROOK][toSquare] & kingSquare) {
        // squares in between attacker and king
        intermediate = Bitboards::intermediateBB[toSquare][kingSquare];
        // adapt board by moving the piece on the bitboard
        assert(allOccupiedBitboard & fromSquare);
        boardAfterMove = allOccupiedBitboard ^ fromSquare;
        boardAfterMove |= toSquare;
        // if squares in between are not occupied then it is a check
        if ((intermediate & boardAfterMove) == 0) return true;
      }
      break;
    case BISHOP:
      // is attack even possible
      if (Bitboards::pseudoAttacks[BISHOP][toSquare] & kingSquare) {
        // squares in between attacker and king
        intermediate = Bitboards::intermediateBB[toSquare][kingSquare];
        // adapt board by moving the piece on the bitboard
        assert (allOccupiedBitboard & fromSquare);
        boardAfterMove = allOccupiedBitboard ^ fromSquare;
        boardAfterMove |= toSquare;
        // if squares in between are not occupied then it is a check
        if ((intermediate & boardAfterMove) == 0) return true;
      }
      break;
    default:
      break;
  }

  // #########################################################################
  // Revealed checks
  // #########################################################################

  // we only need to check for rook, bishop and queens
  // knight and pawn attacks can't be revealed
  // exception is en passant where the captured piece can reveal check
  // check all directions and slide until invalid
  const bool isEnPassant = (moveType == ENPASSANT);

  // rooks
  // Check if there are any rooks on possible attack squares
  Bitboard rooks = piecesBB[colorOf(fromPc)][ROOK];
  if (Bitboards::pseudoAttacks[ROOK][kingSquare] & rooks) {
    // iterate over all pieces
    while (rooks) {
      const Square sq = popLSB(&rooks);
      // if the square is not reachable from the piece's square we can skip this
      if ((Bitboards::pseudoAttacks[ROOK][sq] & kingSquare) == 0) continue;
      // if there are no occupied squares between the piece square and the
      // target square we have a check
      intermediate = Bitboards::intermediateBB[sq][kingSquare];
      // adapt board by moving the piece on the bitboard
      assert (allOccupiedBitboard & fromSquare);
      boardAfterMove = allOccupiedBitboard ^ fromSquare;
      boardAfterMove |= toSquare;
      if (isEnPassant) boardAfterMove ^= epTargetSquare;
      // if squares in between are not occupied then it is a check
      if ((intermediate & boardAfterMove) == 0) return true;
    }
  }

  // Check if there are any bishops on possible attack squares
  Bitboard bishops = piecesBB[colorOf(fromPc)][BISHOP];
  if ((Bitboards::pseudoAttacks[BISHOP][kingSquare] & bishops)) {
    // iterate over all pieces
    while (bishops) {
      const Square sq = popLSB(&bishops);
      // if the square is not reachable from the piece's square we can skip this
      if ((Bitboards::pseudoAttacks[BISHOP][sq] & kingSquare) == 0) continue;
      // if there are no occupied squares between the piece square and the
      // target square we have a check
      intermediate = Bitboards::intermediateBB[sq][kingSquare];
      // adapt board by moving the piece on the bitboard
      assert (allOccupiedBitboard & fromSquare);
      boardAfterMove = allOccupiedBitboard ^ fromSquare;
      boardAfterMove |= toSquare;
      if (isEnPassant) boardAfterMove ^= epTargetSquare;
      // if squares in between are not occupied then it is a check
      if ((intermediate & boardAfterMove) == 0) return true;
    }
  }

  // Check if there are any bishops on possible attack squares
  Bitboard queens = piecesBB[colorOf(fromPc)][QUEEN];
  if ((Bitboards::pseudoAttacks[QUEEN][kingSquare] & queens)) {
    // iterate over all pieces
    while (queens) {
      const Square sq = popLSB(&queens);
      // if the square is not reachable from the piece's square we can skip this
      if ((Bitboards::pseudoAttacks[QUEEN][sq] & kingSquare) == 0) continue;
      // if there are no occupied squares between the piece square and the
      // target square we have a check
      intermediate = Bitboards::intermediateBB[sq][kingSquare];
      // adapt board by moving the piece on the bitboard
      assert (allOccupiedBitboard & fromSquare);
      boardAfterMove = allOccupiedBitboard ^ fromSquare;
      boardAfterMove |= toSquare;
      if (isEnPassant) boardAfterMove ^= epTargetSquare;
      // if squares in between are not occupied then it is a check
      if ((intermediate & boardAfterMove) == 0) return true;
    }
  }

  // we did not find a check
  return false;
}

////////////////////////////////////////////////
///// TO STRING

std::string Position::str() const {
  ostringstream output;
  output << printBoard();
  output << printFen() << endl;
  output << "Check: "
         << (hasCheckFlag == FLAG_TBD ? "N/A" : hasCheckFlag == FLAG_TRUE ? "Check" : "No check");
  output << " Check Mate: "
         << (hasMateFlag == FLAG_TBD ? "N/A" : hasMateFlag == FLAG_TRUE ? "Mate" : "No mate")
         << endl;
  output << "Gamephase: " << gamePhase << endl;
  output << "Material: white=" << material[WHITE] << " black=" << material[BLACK] << endl;
  output << "PosValue: white=" << psqMidValue[WHITE] << " black=" << psqMidValue[BLACK] << endl;
  return output.str();
}

std::string Position::printBoard() const {
  const std::string ptc = "KONBRQ  k*nbrq   ";
  ostringstream output;
  output << "  +---+---+---+---+---+---+---+---+" << endl;
  for (Rank r = RANK_8; r >= RANK_1; --r) {
    output << (r + 1) << " |";
    for (File f = FILE_A; f <= FILE_H; ++f) {
      Piece pc = getPiece(getSquare(f, r));
      if (pc == PIECE_NONE) output << "   |";
      else output << " " << ptc[pc] << " |";
    }
    output << endl;
    output << "  +---+---+---+---+---+---+---+---+" << endl;
  }
  output << "   ";
  for (File f = FILE_A; f <= FILE_H; ++f) {
    output << " " << char('A' + f) << "  ";
  }
  output << endl << endl;
  return output.str();
}

std::string Position::printFen() const {
  ostringstream fen;

  // pieces
  for (Rank r = RANK_8; r >= RANK_1; --r) {
    int emptySquares = 0;
    for (File f = FILE_A; f <= FILE_H; ++f) {
      Piece pc = getPiece(getSquare(f, r));

      if (pc == PIECE_NONE) emptySquares++;
      else {
        if (emptySquares) {
          fen << to_string(emptySquares);
          emptySquares = 0;
        }
        fen << pieceToChar[pc];
      }
    }
    if (emptySquares) {
      fen << to_string(emptySquares);
    }
    if (r > RANK_1) fen << "/";
  }

  // next player
  fen << (nextPlayer ? " b " : " w ");

  // castling
  if (castlingRights == NO_CASTLING) fen << "-";
  else {
    if (castlingRights & WHITE_OO) fen << "K";
    if (castlingRights & WHITE_OOO) fen << "Q";
    if (castlingRights & BLACK_OO) fen << "k";
    if (castlingRights & BLACK_OOO) fen << "q";
  }

  // en passant
  if (enPassantSquare != SQ_NONE) {
    fen << " " << squareLabel(enPassantSquare) << " ";
  }
  else {
    fen << " - ";
  }

  // half move clock
  fen << halfMoveClock << " ";

  // full move number
  fen << ((nextHalfMoveNumber + 1) / 2);

  return fen.str();
}

std::ostream &operator<<(std::ostream &os, Position &position) {
  os << position.printFen();
  return os;
}

////////////////////////////////////////////////
///// PRIVATE

void Position::movePiece(Square from, Square to) {
  putPiece(removePiece(from), to);
}

void Position::putPiece(Piece piece, Square square) {
  const PieceType pieceType = typeOf(piece);
  const Color color = colorOf(piece);

  // bitboards
  assert ((piecesBB[color][pieceType] & square) == 0);
  piecesBB[color][pieceType] |= square;
  assert ((occupiedBB[color] & square) == 0);
  occupiedBB[color] |= square;
  // pre-rotated bb / expensive - ~30% hit
  occupiedBBR90[color] |= rotateSquareR90(square);
  occupiedBBL90[color] |= rotateSquareL90(square);
  occupiedBBR45[color] |= rotateSquareR45(square);
  occupiedBBL45[color] |= rotateSquareL45(square);

  // piece board
  assert (getPiece(square) == PIECE_NONE);
  board[square] = piece;
  if (pieceType == KING) kingSquare[color] = square;

  // zobrist
  zobristKey ^= Zobrist::pieces[piece][square];
  // game phase
  gamePhase = min(GAME_PHASE_MAX, gamePhase + gamePhaseValue[pieceType]);
  // material
  material[color] += pieceTypeValue[pieceType];
  // position value
  psqMidValue[color] += Values::posMidValue[piece][square];
  psqEndValue[color] += Values::posEndValue[piece][square];
}

Piece Position::removePiece(Square square) {
  const Piece old = getPiece(square);
  const Color color = colorOf(old);
  const PieceType pieceType = typeOf(old);

  // piece board
  assert (piecesBB[color][pieceType] & square);
  piecesBB[color][pieceType] ^= square;

  // bitboards
  assert (occupiedBB[color] & square);
  occupiedBB[color] ^= square;
  // pre-rotated bb / expensive - ~30% hit
  occupiedBBR90[color] ^= rotateSquareR90(square);
  occupiedBBL90[color] ^= rotateSquareL90(square);
  occupiedBBR45[color] ^= rotateSquareR45(square);
  occupiedBBL45[color] ^= rotateSquareL45(square);

  // piece list
  assert (getPiece(square) != PIECE_NONE);
  board[square] = PIECE_NONE;
  zobristKey ^= Zobrist::pieces[old][square];
  // game phase
  gamePhase = max(0, gamePhase - gamePhaseValue[pieceType]);
  // material
  material[color] -= pieceTypeValue[pieceType];
  // position value
  psqMidValue[color] -= Values::posMidValue[old][square];
  psqEndValue[color] -= Values::posEndValue[old][square];
  return old;
}

void Position::invalidateCastlingRights(Square fromSq, Square toSq) {
  // check for castling rights invalidation
  if (castlingRights & WHITE_CASTLING) {
    if (fromSq == SQ_E1 || toSq == SQ_E1) {
      zobristKey ^= Zobrist::castlingRights[castlingRights]; // out
      castlingRights += WHITE_CASTLING; // set both to delete both
      castlingRights -= WHITE_CASTLING;
      zobristKey ^= Zobrist::castlingRights[castlingRights]; // in
    }
    if (castlingRights == WHITE_OO && (fromSq == SQ_H1 || toSq == SQ_H1)) {
      zobristKey ^= Zobrist::castlingRights[castlingRights]; // out
      castlingRights -= WHITE_OO;
      zobristKey ^= Zobrist::castlingRights[castlingRights]; // in
    }
    if (castlingRights == WHITE_OOO && (fromSq == SQ_A1 || toSq == SQ_A1)) {
      zobristKey ^= Zobrist::castlingRights[castlingRights]; // out
      castlingRights -= WHITE_OOO;
      zobristKey ^= Zobrist::castlingRights[castlingRights]; // in
    }
  }
  if (castlingRights & BLACK_CASTLING) {
    if (fromSq == SQ_E8 || toSq == SQ_E8) {
      zobristKey ^= Zobrist::castlingRights[castlingRights]; // out
      castlingRights += BLACK_CASTLING; // set both to delete both
      castlingRights -= BLACK_CASTLING;
      zobristKey ^= Zobrist::castlingRights[castlingRights]; // in
    }
    if (castlingRights == BLACK_OOO && (fromSq == SQ_A8 || toSq == SQ_A8)) {
      zobristKey ^= Zobrist::castlingRights[castlingRights]; // out
      castlingRights -= BLACK_OOO;
      zobristKey ^= Zobrist::castlingRights[castlingRights]; // in
    }
    if (castlingRights == BLACK_OO && (fromSq == SQ_H8 || toSq == SQ_H8)) {
      zobristKey ^= Zobrist::castlingRights[castlingRights]; // out
      castlingRights -= BLACK_OO;
      zobristKey ^= Zobrist::castlingRights[castlingRights]; // in
    }
  }
}

void Position::clearEnPassant() {
  if (enPassantSquare != SQ_NONE) {
    zobristKey = zobristKey ^ Zobrist::enPassantFile[fileOf(enPassantSquare)]; // out
    enPassantSquare = SQ_NONE;
  }
}

void Position::initializeBoard() {

  std::fill_n(&board[0], sizeof(board), PIECE_NONE);

  castlingRights = NO_CASTLING;
  std::fill_n(&castlingRights_History[0], sizeof(castlingRights_History), NO_CASTLING);

  enPassantSquare = SQ_NONE;
  std::fill_n(&enPassantSquare_History[0], sizeof(enPassantSquare_History), SQ_NONE);

  halfMoveClock = 0;
  std::fill_n(&halfMoveClockHistory[0], sizeof(halfMoveClockHistory), SQ_NONE);

  nextPlayer = WHITE;

  for (Color color = WHITE; color <= BLACK; ++color) { // foreach color
    occupiedBB[color] = Bitboards::EMPTY_BB;
    occupiedBBR90[color] = Bitboards::EMPTY_BB;
    occupiedBBL90[color] = Bitboards::EMPTY_BB;
    occupiedBBR45[color] = Bitboards::EMPTY_BB;
    occupiedBBL45[color] = Bitboards::EMPTY_BB;
    std::fill_n(&piecesBB[color][0], sizeof(piecesBB[color]), Bitboards::EMPTY_BB);
    kingSquare[color] = SQ_NONE;
    material[color] = 0;
    psqMidValue[color] = 0;
  }

  hasCheckFlag = FLAG_TBD;
  hasMateFlag = FLAG_TBD;
  gamePhase = 0;
}

void Position::setupBoard(const char *fen) {
  initializeBoard();

  unsigned char token;
  unsigned long idx;
  Square currentSquare = SQ_A8;

  std::istringstream iss(fen);
  iss >> std::noskipws;

  // pieces
  while ((iss >> token) && !isspace(token)) {
    if (isdigit(token)) currentSquare += (token - '0') * EAST;
    else if (token == '/') currentSquare += 2 * SOUTH;
    else if ((idx = string(pieceToChar).find(token)) != string::npos) {
      putPiece(Piece(idx), currentSquare);
      ++currentSquare;
    }
  }

  // defaults in case fen is shortened
  nextPlayer = WHITE;
  castlingRights = NO_CASTLING;
  enPassantSquare = SQ_NONE;
  halfMoveClock = 0;
  nextHalfMoveNumber = 1;

  // next player
  if (iss >> token) {
    if (!(token == 'w' || token == 'b')) return; // malformed - ignore the rest
    if (token == 'b') {
      nextPlayer = BLACK;
      zobristKey ^= Zobrist::nextPlayer;
    }
  }
  else return; // end of line

  // skip space
  if (!(iss >> token)) return; // end of line

  // castling rights
  while ((iss >> token) && !isspace(token)) {
    if (token == '-') {
      // skip space
      if (!(iss >> token)) return; // end of line
      break;
    }
    else if (token == 'K') castlingRights += WHITE_OO;
    else if (token == 'Q') castlingRights += WHITE_OOO;
    else if (token == 'k') castlingRights += BLACK_OO;
    else if (token == 'q') castlingRights += BLACK_OOO;
  }
  zobristKey ^= Zobrist::castlingRights[castlingRights];

  // en passant
  if (iss >> token) {
    if (token != '-') {
      if (token >= 'a' && token <= 'h') {
        File f = File(token - 'a');
        if (!(iss >> token)) return; // malformed - ignore the rest
        if ((token >= '1' && token <= '8')) {
          Rank r = Rank(token - '1');
          enPassantSquare = getSquare(f, r);
          zobristKey ^= Zobrist::enPassantFile[f];
        }
        else return; // malformed - ignore the rest
      }
      else return; // malformed - ignore the rest
    } // no en passant
  }
  else return; // end of line

  // skip space
  if (!(iss >> token)) return; // end of line

  // half move clock (50 moves rules)
  iss >> skipws >> halfMoveClock;

  // game move number - to be converted into half move number (ply)
  iss >> skipws >> nextHalfMoveNumber;
  nextHalfMoveNumber = 2 * nextHalfMoveNumber - (nextPlayer == WHITE);

}






















