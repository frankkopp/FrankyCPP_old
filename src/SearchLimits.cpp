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

#include "SearchLimits.h"

#include <utility>

SearchLimits::SearchLimits() = default;

SearchLimits::SearchLimits(MilliSec whiteTime,
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
                           bool perft)
  : whiteTime(whiteTime), blackTime(blackTime),
    whiteInc(whiteInc), blackInc(blackInc), moveTime(moveTime),
    movesToGo(movesToGo), depth(static_cast<Depth>(depth)), nodes(nodes),
    moves(std::move(moves)), mate(mate), ponder(ponder),
    infinite(infinite), perft(perft) {

  setupLimits();
}

void SearchLimits::setupLimits() {

  // the order of these statement also exclude contradictions
  // e.g. if perft is set nothing else will checked. 
  if (perft) {
    timeControl = false;
    startDepth = depth ? depth : DEPTH_ONE;;
    maxDepth = depth ? depth : DEPTH_ONE;
  }
  else if (infinite) {
    timeControl = false;
    startDepth = DEPTH_ONE;
    maxDepth = DEPTH_MAX;
  }
  else if (ponder) {
    timeControl = false;
    startDepth = DEPTH_ONE;
    maxDepth = DEPTH_MAX;
  }
  else if (mate) {
    // limits per mate depth and move time
    timeControl = moveTime != 0;
    startDepth = DEPTH_ONE;
    // might be limited by depth as well
    maxDepth = depth ? depth : DEPTH_MAX;
  }
  else if (whiteTime && blackTime) {
    // normal game with time for each player
    timeControl = true;
    startDepth = DEPTH_ONE;
    // might be limited by depth as well
    maxDepth = depth ? depth : DEPTH_MAX;
  }
  else if (moveTime) {
    // normal game with time for each move
    timeControl = true;
    startDepth = DEPTH_ONE;
    // might be limited by depth as well
    maxDepth = depth ? depth : DEPTH_MAX;
  }
  else if (depth && !nodes) {
    // limited only by depth but still iterating
    timeControl = false;
    startDepth = DEPTH_ONE;
    maxDepth = depth;
  }
  else if (nodes) {
    // limited only by the number of nodes visited
    timeControl = false;
    startDepth = DEPTH_ONE;
    // might be limited by depth as well
    maxDepth = depth ? depth : DEPTH_MAX;
  }
  else { // invalid search mode - use default
    LOG->warn("SearchMode is invalid as no mode could be deducted from settings.");
    timeControl = false;
    startDepth = DEPTH_ONE;
    maxDepth = DEPTH_ONE;
    LOG->warn("SearchMode set to depth {}", maxDepth);
  }
}

void SearchLimits::ponderHit() {
  ponder=false;
  setupLimits();
}

void SearchLimits::ponderStop() {
  ponder=false;
  setupLimits();
}

std::ostream &operator<<(std::ostream &os, const SearchLimits &limits) {
  os << limits.str();
  return os;
}

std::string SearchLimits::str() const {
  std::stringstream os;
  os << "whiteTime: " << whiteTime << " blackTime: " << blackTime << " whiteInc: "
     << whiteInc << " blackInc: " << blackInc << " moveTime: " << moveTime
     << " movesToGo: " << movesToGo << " depth: " << depth << " nodes: "
     << nodes << " moves: " << moves << " mate: " << mate << " ponder: "
     << ponder << " infinite: " << infinite << " perft: " << perft
     << " timeControl: " << timeControl << " startDepth: " << startDepth
     << " maxDepth: " << maxDepth;
  return os.str();
}

MilliSec SearchLimits::getWhiteTime() const {
  return whiteTime;
}

void SearchLimits::setWhiteTime(MilliSec time) {
  SearchLimits::whiteTime = time;
  setupLimits();
}

MilliSec SearchLimits::getBlackTime() const {
  return blackTime;
}

void SearchLimits::setBlackTime(MilliSec time) {
  SearchLimits::blackTime = time;
  setupLimits();
}

MilliSec SearchLimits::getWhiteInc() const {
  return whiteInc;
}

void SearchLimits::setWhiteInc(MilliSec time) {
  SearchLimits::whiteInc = time;
  setupLimits();
}

MilliSec SearchLimits::getBlackInc() const {
  return blackInc;
}

void SearchLimits::setBlackInc(MilliSec time) {
  SearchLimits::blackInc = time;
  setupLimits();
}

MilliSec SearchLimits::getMoveTime() const {
  return moveTime;
}

void SearchLimits::setMoveTime(MilliSec time) {
  SearchLimits::moveTime = time;
  setupLimits();
}

int SearchLimits::getMovesToGo() const {
  return movesToGo;
}

void SearchLimits::setMovesToGo(int m) {
  SearchLimits::movesToGo = m;
  setupLimits();
}

Depth SearchLimits::getDepth() const {
  return depth;
}

void SearchLimits::setDepth(int d) {
  setDepth(static_cast<Depth>(d));
}

void SearchLimits::setDepth(Depth d) {
  SearchLimits::depth = d;
  setupLimits();
}

long SearchLimits::getNodes() const {
  return nodes;
}

void SearchLimits::setNodes(long n) {
  SearchLimits::nodes = n;
  setupLimits();
}

const MoveList &SearchLimits::getMoves() const {
  return moves;
}

void SearchLimits::setMoves(const MoveList &moveList) {
  SearchLimits::moves = moveList;
  setupLimits();
}

int SearchLimits::getMate() const {
  return mate;
}

void SearchLimits::setMate(int m) {
  SearchLimits::mate = m;
  setupLimits();
}

bool SearchLimits::isPonder() const {
  return ponder;
}

void SearchLimits::setPonder(bool aBool) {
  SearchLimits::ponder = aBool;
  setupLimits();
}

bool SearchLimits::isInfinite() const {
  return infinite;
}

void SearchLimits::setInfinite(bool aBool) {
  SearchLimits::infinite = aBool;
  setupLimits();
}

bool SearchLimits::isPerft() const {
  return perft;
}

void SearchLimits::setPerft(bool aBool) {
  SearchLimits::perft = aBool;
  setupLimits();
}

bool SearchLimits::isTimeControl() const {
  return timeControl;
}

Depth SearchLimits::getStartDepth() const {
  return startDepth;
}

void SearchLimits::setStartDepth(int d) {
  setStartDepth(static_cast<Depth>(d));
}

void SearchLimits::setStartDepth(Depth d) {
  SearchLimits::startDepth = d;
  setupLimits();
}

Depth SearchLimits::getMaxDepth() const {
  return maxDepth;
}


