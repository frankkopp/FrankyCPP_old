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
#include "logging.h"
#include "globals.h"

typedef uint64_t MilliSec;

class SearchLimits {

  std::shared_ptr<spdlog::logger> LOG = spdlog::get("Search_Logger");

public:

  // defaults time control
  MilliSec whiteTime;
  MilliSec blackTime;
  MilliSec whiteInc;
  MilliSec blackInc;
  MilliSec moveTime;
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
  SearchLimits(MilliSec whiteTime,
               MilliSec blackTime,
               MilliSec whiteInc,
               MilliSec blackInc,
               MilliSec moveTime,
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
