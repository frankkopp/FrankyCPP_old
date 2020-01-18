/*
 * MIT License
 *
 * Copyright (c) 2020 Frank Kopp
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

#include <regex>
#include "misc.h"
#include "Logging.h"
#include "MoveGenerator.h"

namespace Misc {

  Move getMoveFromSAN(const Position &position, const std::string &sanMove) {

    static std::shared_ptr<spdlog::logger> LOG = spdlog::get("Main_Logger");

    // Regex for short move notation (SAN)
    std::regex regexPattern("([NBRQK])?([a-h])?([1-8])?x?([a-h][1-8]|O-O-O|O-O)(=([NBRQ]))?([+#])?");
    std::smatch matcher;

    // Match the target string
    if (!std::regex_match(sanMove, matcher, regexPattern)) {
      LOG__TRACE(LOG, "No match found");
      return MOVE_NONE;
    }

    // get the parts
    LOG__TRACE(LOG, "Match found");
    std::string pieceType = matcher.str(1);
    std::string disambFile = matcher.str(2);
    std::string disambRank = matcher.str(3);
    std::string toSquare = matcher.str(4);
    std::string promotion = matcher.str(6);
    std::string checkSign = matcher.str(7);
    LOG__TRACE(LOG, "Piece Type: {} File: {} Row: {} Target: {} Promotion: {} CheckSign: {}", pieceType, disambFile, disambRank, toSquare, promotion, checkSign);

    // Generate all legal moves and loop through them to search for a matching move
    Move moveFromSAN{MOVE_NONE};
    int movesFound = 0;
    MoveGenerator mg;
    const MoveList* legalMovesPtr = mg.generateLegalMoves<MoveGenerator::GENALL>(position);
    for (auto m : *legalMovesPtr) {
      m = moveOf(m);
      LOG__TRACE(LOG, "Move {}", printMoveVerbose(m));

      // castling move
      if (typeOf(m) == CASTLING) {
        const Square kingToSquare = getToSquare(m);
        std::string castlingString;
        switch (kingToSquare) {
          case SQ_G1: // white king side
          case SQ_G8: // black king side
            castlingString = "O-O";
            break;
          case SQ_C1: // white queen side
          case SQ_C8: // black queen side
            castlingString = "O-O-O";
            break;
          default:
            LOG__ERROR(LOG, "{}:{}:{} Move type CASTLING but wrong to square", __FILENAME__, __func__, __LINE__);
            continue;
        }
        if (castlingString == sanMove) {
          moveFromSAN = m;
          movesFound++;
          continue;
        }
      }

      // normal move
      const std::string &moveTarget = squareLabel(getToSquare(m));
      if (moveTarget == toSquare) {

        // Find out piece
        Piece movePiece = position.getPiece(getFromSquare(m));
        const std::string pieceTypeChar(1, pieceTypeToChar[typeOf(movePiece)]);
        LOG__TRACE(LOG, "Move PieceType is {}", pieceTypeChar);
        if (!pieceType.empty() && pieceTypeChar == pieceType) {
          LOG__TRACE(LOG, "PieceType is {}", pieceTypeChar);
        }
        else if (pieceType.empty() && typeOf(movePiece) == PAWN) {
          LOG__TRACE(LOG, "PieceType PAWN");
        }
        else {
          LOG__TRACE(LOG, "PieceType NO MATCH {} {}", pieceType, pieceTypeChar);
          continue;
        }

        // Disambiguation File
        if (!disambFile.empty()) {
          if (std::string(1, char('a' + fileOf(getFromSquare(m)))) == disambFile) {
            LOG__TRACE(LOG, "Disambiguation file found {}", disambFile);
          }
          else {
            LOG__TRACE(LOG, "No disambiguation file found");
            continue;
          }
        }

        // Disambiguation Rank
        if (!disambRank.empty()) {
          if (std::string(1, char('1' + rankOf(getFromSquare(m)))) == disambRank) {
            LOG__TRACE(LOG, "Disambiguation rank found {}", disambRank);
          }
          else {
            LOG__TRACE(LOG, "No disambiguation rank found");
            continue;
          }
        }

        // promotion
        if (!promotion.empty()) {
          if (std::string(1, pieceToChar[promotionType(m)]) == promotion) {
            LOG__TRACE(LOG, "Promotion to {} found", promotion);
          }
          else {
            LOG__TRACE(LOG, "No promotion found");
            continue;
          }
        }

        // we should have our move if we end up here
        moveFromSAN = m;
        movesFound++;
      }
    }
    // we should only have one move here
    if (movesFound > 1) {
      LOG__ERROR(LOG, "SAN move {} is ambiguous on {}!", sanMove, position.printFen());
    }
    else if (movesFound == 0 || !isMove(moveFromSAN)) {
      LOG__ERROR(LOG, "SAN move not valid! No such move at the current position: {} {}",
                 sanMove, position.printFen());
    }
    else {
      LOG__TRACE(LOG, "Found move {}", printMove(moveFromSAN));
      return moveFromSAN;
    }
    return MOVE_NONE;
  }

}