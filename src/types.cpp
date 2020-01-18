/*
 * MIT License
 *
 * Copyright (c) 2018-2020 Frank Kopp
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
//
//#include <string>
//#include <bitset>
//#include <sstream>
//#include <algorithm>
//#include "types.h"
//
//std::string printBitString(uint64_t b) {
//  std::ostringstream os;
//  os << std::bitset<64>(b);
//  return os.str();
//}
//
//std::string printValue(const Value value) {
//  // TODO add full protocol (lowerbound, upperbound, etc.)
//  std::string scoreString;
//  if (isCheckMateValue(value)) {
//    scoreString = "mate ";
//    scoreString += value < 0 ? "-" : "";
//    scoreString += std::to_string((VALUE_CHECKMATE - std::abs(value) + 1) / 2);
//  }
//  else if (value == VALUE_NONE) {
//    scoreString = "N/A";
//  }
//  else {
//    scoreString = "cp " + std::to_string(value);
//  }
//  return scoreString;
//}
//
//
//template<MoveType T>
//Move createMove(const char* move) {
//  std::istringstream iss(move);
//  iss >> std::noskipws;
//  unsigned char token = 0;
//  Square from, to;
//
//  // from
//  if (iss >> token) {
//    if (token >= 'a' && token <= 'h') {
//      File f = File(token - 'a');
//      if (!(iss >> token)) return MOVE_NONE; // malformed - ignore the rest
//      if ((token >= '1' && token <= '8')) {
//        Rank r = Rank(token - '1');
//        from = getSquare(f, r);
//      }
//      else { return MOVE_NONE; } // malformed - ignore the rest
//    }
//    else { return MOVE_NONE; } // malformed - ignore the rest
//  }
//  else { return MOVE_NONE; } // malformed - ignore the rest
//
//  // to
//  if (iss >> token) {
//    if (token >= 'a' && token <= 'h') {
//      File f = File(token - 'a');
//      if (!(iss >> token)) return MOVE_NONE; // malformed - ignore the rest
//      if ((token >= '1' && token <= '8')) {
//        Rank r = Rank(token - '1');
//        to = getSquare(f, r);
//      }
//      else { return MOVE_NONE; } // malformed - ignore the rest
//    }
//    else { return MOVE_NONE; } // malformed - ignore the rest
//  }
//  else { return MOVE_NONE; } // malformed - ignore the rest
//
//  // promotion
//  if (T == PROMOTION) {
//    if (iss >> token) {
//      switch (token) {
//        case 'n':
//          return createMove<T>(from, to, KNIGHT);
//        case 'b':
//          return createMove<T>(from, to, BISHOP);
//        case 'r':
//          return createMove<T>(from, to, ROOK);
//        case 'q':
//          return createMove<T>(from, to, QUEEN);
//        case 'N':
//          return createMove<T>(from, to, KNIGHT);
//        case 'B':
//          return createMove<T>(from, to, BISHOP);
//        case 'R':
//          return createMove<T>(from, to, ROOK);
//        case 'Q':
//          return createMove<T>(from, to, QUEEN);
//        default:
//          break;
//      }
//    }
//    else { return MOVE_NONE; } // malformed - ignore the rest
//  }
//  return createMove<T>(from, to);
//}
//
//std::string printMove(const Move move) {
//  std::string promotion = "";
//  if (moveOf(move) == MOVE_NONE) return "NOMOVE";
//  if ((typeOf(move) == PROMOTION)) promotion = pieceTypeToChar[promotionType(move)];
//  return squareLabel(getFromSquare(move)) + squareLabel(getToSquare(move)) + promotion;
//}
//
//std::string printMoveVerbose(const Move move) {
//  if (!move) return "NOMOVE " + std::to_string(move);
//  std::string tp;
//  std::string promPt;
//  switch (typeOf(move)) {
//    case NORMAL:
//      tp = "NORMAL";
//      break;
//    case PROMOTION:
//      promPt = pieceTypeToChar[promotionType(move)];
//      tp = "PROMOTION";
//      break;
//    case ENPASSANT:
//      tp = "ENPASSANT";
//      break;
//    case CASTLING:
//      tp = "CASTLING";
//      break;
//  }
//  return squareLabel(getFromSquare(move)) + squareLabel(getToSquare(move)) + promPt
//         + " (" + tp + " " + std::to_string(valueOf(move)) + " " + std::to_string(move) + ")";
//}
//
//std::string printMoveList(const MoveList &moveList) {
//  std::ostringstream os;
//  os << "MoveList: size=" << moveList.size() << " [";
//  for (auto itr = moveList.begin(); itr != moveList.end(); ++itr) {
//    os << *itr;
//    if (itr != moveList.end() - 1) os << ", ";
//  }
//  os << "]";
//  return os.str();
//}
//
//
//std::string printMoveListUCI(const MoveList &moveList) {
//  std::ostringstream os;
//  for (Move m : moveList) {
//    os << m;
//    if (m != moveList.back()) os << " ";
//  }
//  return os.str();
//}
//
//bool to_bool(std::string str) {
//  std::transform(str.begin(), str.end(), str.begin(), ::tolower);
//  std::istringstream is(str);
//  bool b;
//  is >> std::boolalpha >> b;
//  return b;
//}
//
