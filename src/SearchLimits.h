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

#ifndef FRANKYCPP_SEARCHLIMITS_H
#define FRANKYCPP_SEARCHLIMITS_H

#include <chrono>
#include <ostream>
#include "types.h"

/**
 * Data class for search limits for a search run. Manages dependencies
 * automatically. E.g. sets timeControl() to true if times have been provided.
 */
class SearchLimits {

//  std::shared_ptr<spdlog::logger> LOG = spdlog::get("Search_Logger");

  // defaults time control
  MilliSec whiteTime = 0;
  MilliSec blackTime = 0;
  MilliSec whiteInc = 0;
  MilliSec blackInc = 0;
  MilliSec moveTime = 0;
  int movesToGo = 0;

  // extra limits
  Depth depth = DEPTH_NONE;
  uint64_t nodes = 0;
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

  MilliSec getWhiteTime() const {
    return whiteTime;
  }

  void setWhiteTime(MilliSec time) {
    whiteTime = time;
    setupLimits();
  }

  MilliSec getBlackTime() const {
    return blackTime;
  }

  void setBlackTime(MilliSec time) {
    blackTime = time;
    setupLimits();
  }

  MilliSec getWhiteInc() const {
    return whiteInc;
  }

  void setWhiteInc(MilliSec time) {
    whiteInc = time;
    setupLimits();
  }

  MilliSec getBlackInc() const {
    return blackInc;
  }

  void setBlackInc(MilliSec time) {
    blackInc = time;
    setupLimits();
  }

  MilliSec getMoveTime() const {
    return moveTime;
  }

  void setMoveTime(MilliSec time) {
    moveTime = time;
    setupLimits();
  }

  int getMovesToGo() const {
    return movesToGo;
  }

  void setMovesToGo(int m) {
    movesToGo = m;
    setupLimits();
  }

  Depth getDepth() const {
    return depth;
  }

  void setDepth(int d) {
    setDepth(static_cast<Depth>(d));
  }

  void setDepth(Depth d) {
    depth = d;
    setupLimits();
  }

  uint64_t getNodes() const {
    return nodes;
  }

  void setNodes(long n) {
    nodes = n;
    setupLimits();
  }

  const MoveList &getMoves() const {
    return moves;
  }

  void setMoves(const MoveList &moveList) {
    moves = moveList;
    setupLimits();
  }

  int getMate() const {
    return mate;
  }

  void setMate(int m) {
    mate = m;
    setupLimits();
  }

  bool isPonder() const {
    return ponder;
  }

  void setPonder(bool aBool) {
    ponder = aBool;
    setupLimits();
  }

  bool isInfinite() const {
    return infinite;
  }

  void setInfinite(bool aBool) {
    infinite = aBool;
    setupLimits();
  }

  bool isPerft() const {
    return perft;
  }

  void setPerft(bool aBool) {
    perft = aBool;
    setupLimits();
  }

  bool isTimeControl() const {
    return timeControl;
  }

  Depth getStartDepth() const {
    return startDepth;
  }

  void setStartDepth(int d) {
    setStartDepth(static_cast<Depth>(d));
  }

  void setStartDepth(Depth d) {
    startDepth = d;
    setupLimits();
  }

  Depth getMaxDepth() const {
    return maxDepth;
  }
  
};


#endif //FRANKYCPP_SEARCHLIMITS_H
