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
  resetCounter();

  Position position(fen);
  MoveGenerator mg;

  long result;
  auto start = std::chrono::high_resolution_clock::now();
  result = miniMax(maxDepth, position, mg);
  auto finish = std::chrono::high_resolution_clock::now();
  long duration = std::chrono::duration_cast<std::chrono::milliseconds>(finish - start).count();

  nodes = result;

  ostringstream os;
  cout.imbue(digitLocale);
  os.imbue(digitLocale);
  os << setprecision(9);

  os << "Testing at depth " << maxDepth << endl;
  os << "Leaf Nodes: " << nodes
     << " Captures: " << captureCounter
     << " EnPassant: " << enpassantCounter
     << " Checks: " << checkCounter
     << " Mates: " << checkMateCounter
     << endl;

  os << "Duration: " << duration << " ms" << endl;
  os << "NPS: " << ((result * 1e3) / duration) << " nps" << endl;

  cout << os.str() << endl;
}

void Perft::resetCounter() {
  nodes = 0;
  checkCounter = 0;
  checkMateCounter = 0;
  captureCounter = 0;
  enpassantCounter = 0;
}

long Perft::miniMax(int depth, Position position, MoveGenerator mg) {

  // Iterate over moves
  long totalNodes = 0L;

  // moves to search recursively
  vector<Move> moves = mg.generatePseudoLegalMoves(GENALL, &position);
  for (Move move : moves) {
    if (depth > 1) {
      position.doMove(move);
      // only go into recursion if move was legal
      if (position.isLegalPosition()) totalNodes += miniMax(depth - 1, position, mg);
      position.undoMove();
    }
    else {
      const bool cap = position.getPiece(getToSquare(move)) != PIECE_NONE;
      const bool ep = typeOf(move) == ENPASSANT;
      position.doMove(move);
      if (position.isLegalPosition()) {
        totalNodes++;
        if (ep) {
          enpassantCounter++;
          captureCounter++;
        }
        if (cap) captureCounter++;
        if (position.hasCheck()) checkCounter++;
        if (position.hasCheckMate()) checkMateCounter++;
      } 
      position.undoMove();
    }
  }
  return totalNodes;
}



