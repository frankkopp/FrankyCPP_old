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

#ifndef FRANKYCPP_SEARCHLIMITS_H
#define FRANKYCPP_SEARCHLIMITS_H

#include <chrono>
#include <ostream>

#include "datatypes.h"

typedef uint64_t Millisec;

class SearchLimits {

public:

  // defaults time control
  Millisec whiteTime;
  Millisec blackTime;
  Millisec whiteInc;
  Millisec blackInc;
  Millisec moveTime;
  int movesToGo;

  // extra limits
  int depth;
  long nodes;
  MoveList moves;

  // no time control
  int mate;
  bool ponder;
  bool infinite;
  bool perft;

  // state
  bool timeControl = false;
  int startDepth = 1;
  int maxDepth = MAX_PLY;

  // Constructor
  SearchLimits();
  SearchLimits(Millisec whiteTime,
               Millisec blackTime,
               Millisec whiteInc,
               Millisec blackInc,
               Millisec moveTime,
               int movesToGo,
               int depth,
               long nodes,
               const MoveList &moves,
               int mate,
               bool ponder,
               bool infinite,
               bool perft);

  // output
  friend std::ostream &operator<<(std::ostream &os, const SearchLimits &limits);

private:
  void setupLimits();

};


#endif //FRANKYCPP_SEARCHLIMITS_H
