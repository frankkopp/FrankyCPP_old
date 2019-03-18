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

#ifndef FRANKYCPP_PERFT_H
#define FRANKYCPP_PERFT_H

#include <string>

#include "globals.h"
#include "Bitboards.h"
#include "Position.h"
#include "MoveGenerator.h"

using namespace std;

class Perft {

  long   nodes;
  long   checkCounter;
  long   checkMateCounter;
  long   captureCounter;
  long   enpassantCounter;
  string fen;

public:
  Perft();
  explicit Perft(const string &fen);
  void perft(int maxDepth);

  long getNodes() const { return nodes; }
  long getCaptureCounter() const { return captureCounter; }
  long getEnpassantCounter() const { return enpassantCounter; }
  long getCheckCounter() const { return checkCounter; }
  long getCheckMateCounter() const { return checkMateCounter; }

private:
  void resetCounter();
  long miniMax(int depth, Position position, MoveGenerator mg);
};


#endif //FRANKYCPP_PERFT_H
