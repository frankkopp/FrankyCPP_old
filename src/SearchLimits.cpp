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
    movesToGo(movesToGo), depth(depth), nodes(nodes),
    moves(std::move(moves)), mate(mate), ponder(ponder),
    infinite(infinite), perft(perft) {

  setupLimits();
}

void SearchLimits::setupLimits() {

  if (perft) {
    // no limits
    timeControl = false;
    startDepth = depth;
    // might be limited be depth as well
    maxDepth = depth > 0 ? depth : MAX_PLY;
  }
  else if (infinite) {
    // limited by depth only (identical to depth limit in this case)
    timeControl = false;
    startDepth = 1;
    // might be limited be depth as well
    maxDepth = depth > 0 ? depth : MAX_PLY;
  }
  else if (ponder) {
    // limits per depth only, start with 1
    timeControl = false;
    startDepth = 1;
    // might be limited be depth as well
    maxDepth = depth > 0 ? depth : MAX_PLY;
  }
  else if (mate) {
    // limits per mate depth and move time
    timeControl = moveTime != 0;
    startDepth = 1;
    // might be limited be depth as well
    maxDepth = depth > 0 ? depth : MAX_PLY;
  }
  else if (whiteTime && blackTime) {
    // normal game with time for each player
    timeControl = true;
    startDepth = 1;
    // might be limited be depth as well
    maxDepth = depth ? depth : MAX_PLY;
  }
  else if (moveTime) {
    // normal game with time for each move
    timeControl = true;
    startDepth = 1;
    // might be limited be depth as well
    maxDepth = depth ? depth : MAX_PLY;
  }
  else if (depth && !nodes) {
    // limited only by depth but still iterating
    timeControl = false;
    startDepth = 1;
    maxDepth = depth;
  }
  else if (nodes) {
    // limited only by the number of nodes visited
    timeControl = false;
    startDepth = 1;
    // might be limited be depth as well
    maxDepth = depth ? depth : MAX_PLY;
  }
  else { // invalid search mode - use default
    LOG->warn("SearchMode is invalid as no mode could be deducted from settings.");
    timeControl = false;
    startDepth = 1;
    maxDepth = 1;
    LOG->warn("SearchMode set to depth {}", maxDepth);
  }
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
