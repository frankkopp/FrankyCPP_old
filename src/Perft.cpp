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

#include <utility>
#include <chrono>
#include <iomanip>

#include "Perft.h"

Perft::Perft() {
  fen = START_POSITION_FEN;
}

Perft::Perft(const string &f) {
  fen = f;
}

void Perft::perft(int maxDepth) {
  perft(maxDepth, false);
}

void Perft::perft(int maxDepth, bool onDemand) {
  resetCounter();

  Position position(fen);
  MoveGenerator mg[MAX_PLY];
  ostringstream os;
  cout.imbue(digitLocale);
  os.imbue(digitLocale);
  os << setprecision(9);

  os << "Testing at depth " << maxDepth << endl;

  long result;
  auto start = std::chrono::high_resolution_clock::now();
  if (onDemand) result = miniMaxOD(maxDepth, &position, mg);
  else result = miniMax(maxDepth, &position, mg);
  auto finish = std::chrono::high_resolution_clock::now();
  long duration = std::chrono::duration_cast<std::chrono::milliseconds>(finish - start).count();

  nodes = result;

  os << "Leaf Nodes: " << nodes
     << " Captures: " << captureCounter
     << " EnPassant: " << enpassantCounter
     << " Checks: " << checkCounter
     << " Mates: " << checkMateCounter
     << endl;

  os << "Duration: " << duration << " ms" << endl;
  os << "NPS: " << ((result * 1e3) / duration) << " nps" << endl;

  cout << os.str();
}

void Perft::resetCounter() {
  nodes = 0;
  checkCounter = 0;
  checkMateCounter = 0;
  captureCounter = 0;
  enpassantCounter = 0;
}

long Perft::miniMax(int depth, Position *pPosition, MoveGenerator *pMg) {

  // Iterate over moves
  long totalNodes = 0L;

  //println(pPosition->str())
  
  // moves to search recursively
  MoveList moves = pMg[depth].generatePseudoLegalMoves(GENALL, pPosition);
  for (Move move : moves) {
    if (depth > 1) {
      pPosition->doMove(move);
      // only go into recursion if move was legal
      if (pPosition->isLegalPosition()) totalNodes += miniMax(depth - 1, pPosition, pMg);
      pPosition->undoMove();
    }
    else {
      const bool cap = pPosition->getPiece(getToSquare(move)) != PIECE_NONE;
      const bool ep = typeOf(move) == ENPASSANT;
      pPosition->doMove(move);
      if (pPosition->isLegalPosition()) {
        totalNodes++;
        if (ep) {
          enpassantCounter++;
          captureCounter++;
        }
        if (cap) captureCounter++;
        if (pPosition->hasCheck()) checkCounter++;
        if (pPosition->hasCheckMate()) checkMateCounter++;
      } 
      pPosition->undoMove();
    }
  }
  return totalNodes;
}

long Perft::miniMaxOD(int depth, Position *pPosition, MoveGenerator *pMg) {
  // Iterate over moves
  long totalNodes = 0L;

  //println(pPosition->str())

  // moves to search recursively
  Move move;
  while (true) {
    move = pMg[depth].getNextPseudoLegalMove(GENALL, pPosition);
    if (move == NOMOVE) break;
    // println(move);

    if (depth > 1) {
      pPosition->doMove(move);
      // only go into recursion if move was legal
      if (pPosition->isLegalPosition()) totalNodes += miniMaxOD(depth - 1, pPosition, pMg);
      pPosition->undoMove();
    }
    else {
      const bool cap = pPosition->getPiece(getToSquare(move)) != PIECE_NONE;
      const bool ep = typeOf(move) == ENPASSANT;
      pPosition->doMove(move);
      if (pPosition->isLegalPosition()) {
        totalNodes++;
        if (ep) {
          enpassantCounter++;
          captureCounter++;
        }
        if (cap) captureCounter++;
        if (pPosition->hasCheck()) checkCounter++;
        if (pPosition->hasCheckMate()) checkMateCounter++;
      }
      pPosition->undoMove();
    }
  }
    return totalNodes;
}




