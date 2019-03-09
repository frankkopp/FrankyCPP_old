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
///// PUBLIC

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

std::string Position::str() {
  ostringstream output;
  output << printBoard();
  output << printFen() << endl;
  output << (hasCheck == 1 ? "Has check!" : "No check");
  if (hasCheck == 1 && hasMate == 1) output << "Has check mate!" << endl;
  else output << endl;
  output << "Material: white=" << material[WHITE] << " black=" << material[BLACK] << endl;
  return output.str();
}

std::string Position::printBoard() {
  ostringstream output;
  output << "  +---+---+---+---+---+---+---+---+" << endl;
  for (Rank r = RANK_8; r >= RANK_1; --r) {
    output << (r + 1) << " |";
    for (File f = FILE_A; f <= FILE_H; ++f) {
      Piece pc = board[getSquare(f, r)];
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

std::string Position::printFen() {
  ostringstream fen;

  // pieces
  for (Rank r = RANK_8; r >= RANK_1; --r) {
    int emptySquares = 0;
    for (File f = FILE_A; f <= FILE_H; ++f) {
      Piece pc = board[getSquare(f, r)];

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

void Position::movePiece(Square from, Square to) {
  putPiece(removePiece(from), to);
}

void Position::putPiece(Piece piece, Square square) {
  // piece list and zobrist
  assert (board[square] == PIECE_NONE);
  board[square] = piece;
  zobristKey ^= Zobrist::pieces[piece][square];
  // bitboards
  assert ((occupiedBB[colorOf(piece)] & square) == 0);
  occupiedBB[colorOf(piece)] |= square;
  assert ((piecesBB[colorOf(piece)][typeOf(piece)] & square) == 0);
  piecesBB[colorOf(piece)][typeOf(piece)] |= square;
  // material
  material[colorOf(piece)] += pieceValue[typeOf(piece)];
  // TODO position value
  // game phase
  // TODO game phase
}

Piece Position::removePiece(Square square) {
  // piece list
  assert (board[square] != PIECE_NONE);
  Piece old = board[square];
  board[square] = PIECE_NONE;
  zobristKey ^= Zobrist::pieces[old][square];
  // bitboards
  assert (occupiedBB[colorOf(old)] & square);
  occupiedBB[colorOf(old)] ^= square;
  assert (piecesBB[colorOf(old)][typeOf(old)] & square);
  piecesBB[colorOf(old)][typeOf(old)] ^= square;
  // material
  material[colorOf(old)] -= pieceValue[typeOf(old)];
  // TODO position value
  // game phase
  // TODO game phase
  return old;
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
    occupiedBB[color] = EMPTY_BB;
    occupiedBBR90[color] = EMPTY_BB;
    occupiedBBL90[color] = EMPTY_BB;
    occupiedBBR45[color] = EMPTY_BB;
    occupiedBBL45[color] = EMPTY_BB;
    std::fill_n(&piecesBB[color][0], sizeof(piecesBB[color]), EMPTY_BB);
    material[color] = 0;
  }

  gamePhase = GAME_PHASE_MAX;
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
    if (token == '-') break;
    else if (token == 'K') castlingRights += WHITE_OO;
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

////////////////////////////////////////////////
///// GETTER / SETTER

Key Position::getZobristKey() const {
  return zobristKey;
}

Color Position::getNextPlayer() const {
  return nextPlayer;
}

Square Position::getEnPassantSquare() const {
  return enPassantSquare;
}

Bitboard Position::getPieceBB(Color c, PieceType pt) const {
  return piecesBB[c][pt];
}

Bitboard Position::getOccupiedBB(Color c) const {
  return occupiedBB[c];
}

int Position::getMaterial(Color c) const {
  return material[c];
}













