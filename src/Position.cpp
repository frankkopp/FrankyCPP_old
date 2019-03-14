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
#include "Bitboards.h"

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
  this->hasCheck = op.hasCheck;
  this->hasMate = op.hasMate;
  // color dependent
  for (Color c = WHITE; c <= BLACK; ++c) {
    std::copy(op.piecesBB[c],
              op.piecesBB[c] + PT_LENGTH,
              this->piecesBB[c]);
    this->occupiedBB[c] = op.occupiedBB[c];
    this->occupiedBBR90[c] = op.occupiedBBR90[c];
    this->occupiedBBL90[c] = op.occupiedBBL90[c];
    this->occupiedBBR45[c] = op.occupiedBBR45[c];
    this->occupiedBBL45[c] = op.occupiedBBL45[c];

    this->material[c] = op.material[c];
    this->psqMGValue[c] = op.psqMGValue[c];
    this->psqEGValue[c] = op.psqEGValue[c];
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

////////////////////////////////////////////////
///// PUBLIC

void Position::doMove(Move move) {
  assert(isMove(move));
  assert(isSquare(fromSquare(move)));
  assert(isSquare(toSquare(move)));
  assert(getPiece(fromSquare(move)) != PIECE_NONE);

  const MoveType moveType = typeOf(move);

  const Square fromSq = fromSquare(move);
  const Piece fromPC = getPiece(fromSq);
  const PieceType fromPT = typeOf(fromPC);
  const Color myColor = colorOf(fromPC);
  assert (myColor == nextPlayer);

  const Square toSq = toSquare(move);
  const Piece targetPC = getPiece(toSq);

  const PieceType promotionPT = promotionType(move);

  // save state of board for undo
  moveHistory[historyCounter] = move;
  fromPieceHIstory[historyCounter] = fromPC;
  capturedPieceHIstory[historyCounter] = targetPC;
  castlingRights_History[historyCounter] = castlingRights;
  enPassantSquare_History[historyCounter] = enPassantSquare;
  halfMoveClockHistory[historyCounter] = halfMoveClock;
  zobristKey_History[historyCounter] = zobristKey;
  hasCheckFlagHistory[historyCounter] = hasCheck;
  hasMateFlagHistory[historyCounter] = hasMate;
  historyCounter++;

  // reset check and mate flag
  hasCheck = FLAG_TBD;
  hasMate = FLAG_TBD;

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
      } else if (fromPT == PAWN) {
        halfMoveClock = 0; // reset half move clock because of pawn move
        if (distance(fromSq, toSq) == 2) { // pawn double - set en passant
          // set new en passant target field - always one "behind" the toSquare
          enPassantSquare = toSq + pawnDir[~myColor];
          zobristKey = zobristKey ^ Zobrist::enPassantFile[fileOf(enPassantSquare)]; // in
        }
      } else halfMoveClock++;
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
      assert(getPiece(capSq) == Piece((~myColor * 8) + 1));
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
      movePiece(toSquare(move), fromSquare(move));
      if (capturedPieceHIstory[historyCounter] != PIECE_NONE) putPiece(
          capturedPieceHIstory[historyCounter], toSquare(move));
      break;

    case PROMOTION:
      removePiece(toSquare(move));
      putPiece(makePiece(nextPlayer, PAWN), fromSquare(move));
      if (capturedPieceHIstory[historyCounter] != PIECE_NONE) putPiece(
          capturedPieceHIstory[historyCounter], toSquare(move));
      break;

    case ENPASSANT:
      // ignore Zobrist Key as it will be restored via history
      movePiece(toSquare(move), fromSquare(move));
      putPiece(makePiece(~nextPlayer, PAWN), toSquare(move) + pawnDir[~nextPlayer]);
      break;

    case CASTLING:
      // ignore Zobrist Key as it will be restored via history
      // castling rights are restored via history
      switch (toSquare(move)) {
        case SQ_G1:
          movePiece(toSquare(move), fromSquare(move)); // King
          movePiece(SQ_F1, SQ_H1); // Rook
          break;
        case SQ_C1:
          movePiece(toSquare(move), fromSquare(move)); // King
          movePiece(SQ_D1, SQ_A1); // Rook
          break;
        case SQ_G8:
          movePiece(toSquare(move), fromSquare(move)); // King
          movePiece(SQ_F8, SQ_H8); // Rook
          break;
        case SQ_C8:
          movePiece(toSquare(move), fromSquare(move)); // King
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
  hasCheck = hasCheckFlagHistory[historyCounter];
  hasMate = hasMateFlagHistory[historyCounter];
}

////////////////////////////////////////////////
///// TO STRING

std::string Position::str() const {
  ostringstream output;
  output << printBoard();
  output << printFen() << endl;
  output << "Check: "
         << (hasCheck == FLAG_TBD ? "N/A" : hasCheck == FLAG_TRUE ? "Check" : "No check");
  output << " Check Mate: "
         << (hasMate == FLAG_TBD ? "N/A" : hasMate == FLAG_TRUE ? "Mate" : "No mate") << endl;
  output << "Gamephase: " << gamePhase << endl;
  output << "Material: white=" << material[WHITE] << " black=" << material[BLACK] << endl;
  output << "PosValue MG: white=" << psqMGValue[WHITE] << " black=" << psqMGValue[BLACK] << endl;
  output << "PosValue EG: white=" << psqEGValue[WHITE] << " black=" << psqEGValue[BLACK] << endl;
  return output.str();
}

std::string Position::printBoard() const {
  ostringstream output;
  output << "  +---+---+---+---+---+---+---+---+" << endl;
  for (Rank r = RANK_8; r >= RANK_1; --r) {
    output << (r + 1) << " |";
    for (File f = FILE_A; f <= FILE_H; ++f) {
      Piece pc = getPiece(getSquare(f, r));
      if (pc == PIECE_NONE) output << "   |";
      else output << " " << pieceToChar[pc] << " |";
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
  } else {
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

inline void Position::movePiece(Square from, Square to) {
  putPiece(removePiece(from), to);
}

inline void Position::putPiece(Piece piece, Square square) {
  // bitboards
  assert ((occupiedBB[colorOf(piece)] & square) == 0);
  occupiedBB[colorOf(piece)] |= square;
  assert ((piecesBB[colorOf(piece)][typeOf(piece)] & square) == 0);
  piecesBB[colorOf(piece)][typeOf(piece)] |= square;
  // piece list and zobrist
  assert (getPiece(square) == PIECE_NONE);
  board[square] = piece;
  zobristKey ^= Zobrist::pieces[piece][square];
  // material
  material[colorOf(piece)] += pieceValue[typeOf(piece)];
  // position value
  psqMGValue[colorOf(piece)] += Values::midGamePosValue[piece][square];
  psqEGValue[colorOf(piece)] += Values::endGamePosValue[piece][square];
  // game phase
  gamePhase += gamePhaseValue[typeOf(piece)];
}

inline Piece Position::removePiece(Square square) {
  Piece old = getPiece(square);
  //  cout << Bitboards::print(occupiedBB[colorOf(old)]) << endl;
  //  cout << Bitboards::print(occupiedBB[colorOf(old)] & square) << endl;

  // bitboards
  assert (occupiedBB[colorOf(old)] & square);
  occupiedBB[colorOf(old)] ^= square;
  assert (piecesBB[colorOf(old)][typeOf(old)] & square);
  piecesBB[colorOf(old)][typeOf(old)] ^= square;
  // piece list
  assert (getPiece(square) != PIECE_NONE);
  board[square] = PIECE_NONE;
  zobristKey ^= Zobrist::pieces[old][square];
  // material
  material[colorOf(old)] -= pieceValue[typeOf(old)];
  // position value
  psqMGValue[colorOf(old)] -= Values::midGamePosValue[old][square];
  psqEGValue[colorOf(old)] -= Values::endGamePosValue[old][square];
  // game phase
  gamePhase -= gamePhaseValue[typeOf(old)];
  return old;
}

void Position::invalidateCastlingRights(Square fromSq, Square toSq) {
  // check for castling rights invalidation
  if (castlingRights == WHITE_CASTLING) {
    if (fromSq == SQ_E1 || toSq == SQ_E1) {
      zobristKey ^= Zobrist::castlingRights[castlingRights]; // out
      castlingRights -= WHITE_CASTLING;
      zobristKey ^= Zobrist::castlingRights[castlingRights]; // in
    }
    if (fromSq == SQ_A1 || toSq == SQ_A1) {
      zobristKey ^= Zobrist::castlingRights[castlingRights]; // out
      castlingRights -= WHITE_OOO;
      zobristKey ^= Zobrist::castlingRights[castlingRights]; // in
    }
    if (fromSq == SQ_H1 || toSq == SQ_H1) {
      zobristKey ^= Zobrist::castlingRights[castlingRights]; // out
      castlingRights -= WHITE_OO;
      zobristKey ^= Zobrist::castlingRights[castlingRights]; // in
    }
  }
  if (castlingRights == BLACK_CASTLING) {
    if (fromSq == SQ_E8 || toSq == SQ_E8) {
      zobristKey ^= Zobrist::castlingRights[castlingRights]; // out
      castlingRights -= BLACK_CASTLING;
      zobristKey ^= Zobrist::castlingRights[castlingRights]; // in
    }
    if (fromSq == SQ_A8 || toSq == SQ_A8) {
      zobristKey ^= Zobrist::castlingRights[castlingRights]; // out
      castlingRights -= BLACK_OOO;
      zobristKey ^= Zobrist::castlingRights[castlingRights]; // in
    }
    if (fromSq == SQ_H8 || toSq == SQ_H8) {
      zobristKey ^= Zobrist::castlingRights[castlingRights]; // out
      castlingRights -= BLACK_OO;
      zobristKey ^= Zobrist::castlingRights[castlingRights]; // in
    }
  }
}

inline void Position::clearEnPassant() {
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
    material[color] = 0;
  }

  gamePhase = 0;
}

void Position::setupBoard(const char *fen) {
  initializeBoard();

  unsigned char token;
  unsigned long idx;
  Square currentSquare = SQ_A8;

  std::istringstream ss(fen);
  ss >> std::noskipws;

  // pieces
  while ((ss >> token) && !isspace(token)) {
    if (isdigit(token)) currentSquare += (token - '0') * EAST;
    else if (token == '/') currentSquare += 2 * SOUTH;
    else if ((idx = pieceToChar.find(token)) != string::npos) {
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
  if (ss >> token) {
    if (!(token == 'w' || token == 'b')) return; // malformed - ignore the rest
    if (token == 'b') {
      nextPlayer = BLACK;
      zobristKey ^= Zobrist::nextPlayer;
    }
  } else return; // end of line

  // skip space
  if (!(ss >> token)) return; // end of line

  // castling rights
  while ((ss >> token) && !isspace(token)) {
    if (token == '-') {
      // skip space
      if (!(ss >> token)) return; // end of line
      break;
    } else if (token == 'K') castlingRights += WHITE_OO;
    else if (token == 'Q') castlingRights += WHITE_OOO;
    else if (token == 'k') castlingRights += BLACK_OO;
    else if (token == 'q') castlingRights += BLACK_OOO;
  }
  zobristKey ^= Zobrist::castlingRights[castlingRights];

  // en passant
  if (ss >> token) {
    if (token != '-') {
      if (token >= 'a' && token <= 'h') {
        File f = File(token - 'a');
        if (!(ss >> token)) return; // malformed - ignore the rest
        if ((token >= '1' && token <= '8')) {
          Rank r = Rank(token - '1');
          enPassantSquare = getSquare(f, r);
          zobristKey ^= Zobrist::enPassantFile[f];
        } else return; // malformed - ignore the rest
      } else return; // malformed - ignore the rest
    } // no en passant
  } else return; // end of line

  // skip space
  if (!(ss >> token)) return; // end of line

  // half move clock (50 moves rules)
  ss >> skipws >> halfMoveClock;

  // game move number - to be converted into half move number (ply)
  ss >> skipws >> nextHalfMoveNumber;
  nextHalfMoveNumber = 2 * nextHalfMoveNumber - (nextPlayer == WHITE);

}
















