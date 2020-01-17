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
    std::regex regexPattern("([NBRQK])?([a-h])?([1-8])?([a-h][1-8]|O-O-O|O-O)(=([NBRQ]))?([+#])?");
    std::smatch matcher;

    // Match the target string
    if (!std::regex_match(sanMove, matcher, regexPattern)) {
      LOG__DEBUG(LOG, "No match found");
      return MOVE_NONE;
    }

    // get the parts
    LOG__DEBUG(LOG, "Match found");
    std::string piece = matcher.str(1);
    std::string disambFile = matcher.str(2);
    std::string disambRank = matcher.str(3);
    std::string toSquare = matcher.str(4);
    std::string promotion = matcher.str(6);
    std::string checkSign = matcher.str(7);
    LOG__DEBUG(LOG, "Piece: {} File: {} Row: {} Target: {} Promotion: {} CheckSign: {}", piece, disambFile, disambRank, toSquare, promotion, checkSign);

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
            LOG__ERROR(LOG, "{}:{}:{} Move type CASTLING but wrong to square", __FILE_NAME__, __func__, __LINE__);
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
        const std::string pieceChar(1, pieceToChar[movePiece]);
        if (!piece.empty() && pieceChar == piece) {
          LOG__DEBUG(LOG, "Piece is {}", pieceChar);
        }
        else if (piece == "") {
          LOG__DEBUG(LOG, "Piece PAWN");
        }
        else {
          LOG__DEBUG(LOG, "Piece NO MATCH");
          continue;
        }

        // Disambiguation File
        if (!disambFile.empty()) {
          if (std::string(1, char('a' + fileOf(getFromSquare(m)))) == disambFile) {
            LOG__DEBUG(LOG, "Disambiguation file found {}", disambFile);
          }
          else {
            LOG__DEBUG(LOG, "No disambiguation file found");
            continue;
          }
        }

        // Disambiguation Rank
        if (!disambRank.empty()) {
          if (std::string(1, char('1' + rankOf(getFromSquare(m)))) == disambRank) {
            LOG__DEBUG(LOG, "Disambiguation rank found {}", disambRank);
          }
          else {
            LOG__DEBUG(LOG, "No disambiguation rank found");
            continue;
          }
        }

        // promotion
        if (!promotion.empty()) {
          if (std::string(1, pieceToChar[promotionType(m)]) == promotion) {
            LOG__DEBUG(LOG, "Promotion to {} found", promotion);
          }
          else {
            LOG__DEBUG(LOG, "No promotion found");
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
      LOG__ERROR(LOG, "SAN move {} is ambiguous!", sanMove);
    }
    else if (movesFound == 0 || !isMove(moveFromSAN)) {
      LOG__ERROR(LOG, "SAN move not valid! No such move at the current position: " + sanMove);
    }
    else {
      LOG__DEBUG(LOG, "Found move {}", printMove(moveFromSAN));
      return moveFromSAN;
    }
    return MOVE_NONE;
  }

}