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
#include "Logging.h"
#include "types.h"

class SearchLimits {

  std::shared_ptr<spdlog::logger> LOG = spdlog::get("Search_Logger");

  // defaults time control
  MilliSec whiteTime = 0;
  MilliSec blackTime = 0;
  MilliSec whiteInc = 0;
  MilliSec blackInc = 0;
  MilliSec moveTime = 0;
  int movesToGo = 0;

  // extra limits
  Depth depth = DEPTH_NONE;
  long nodes = 0;
  MoveList moves{};

  // no time control
  int mate = 0;
  bool ponder = false;
  bool infinite = false;
  bool perft = false;

  // state
  bool timeControl = false;
  Depth startDepth = DEPTH_ONE;
  Depth maxDepth = DEPTH_MAX;
  
  void setupLimits();
  
public:
 
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
               MoveList moves,
               int mate,
               bool ponder,
               bool infinite,
               bool perft);

  // output
  std::string str() const;
  friend std::ostream &operator<<(std::ostream &os, const SearchLimits &limits);

  void ponderHit();
  void ponderStop();
  
  MilliSec getWhiteTime() const;
  void setWhiteTime(MilliSec time);
  MilliSec getBlackTime() const;
  void setBlackTime(MilliSec time);
  MilliSec getWhiteInc() const;
  void setWhiteInc(MilliSec time);
  MilliSec getBlackInc() const;
  void setBlackInc(MilliSec time);
  MilliSec getMoveTime() const;
  void setMoveTime(MilliSec time);
  int getMovesToGo() const;
  void setMovesToGo(int m);
  Depth getDepth() const;
  void setDepth(int d);
  void setDepth(Depth d);
  long getNodes() const;
  void setNodes(long n);
  const MoveList &getMoves() const;
  void setMoves(const MoveList &moveList);
  int getMate() const;
  void setMate(int m);
  bool isPonder() const;
  void setPonder(bool aBool);
  bool isInfinite() const;
  void setInfinite(bool aBool);
  bool isPerft() const;
  void setPerft(bool aBool);
  bool isTimeControl() const;
  Depth getStartDepth() const;
  void setStartDepth(int d);
  void setStartDepth(Depth d);
  Depth getMaxDepth() const;
  
};


#endif //FRANKYCPP_SEARCHLIMITS_H
