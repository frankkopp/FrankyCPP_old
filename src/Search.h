/*
 * MIT License
 *
 * Copyright (c) 2019 Frank Kopp
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
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

#ifndef FRANKYCPP_SEARCH_H
#define FRANKYCPP_SEARCH_H

// forward declared dependencies
class SearchLimits;
class Engine;

// included dependencies
#include <iostream>
#include <thread>
#include <chrono>
#include "logging.h"
#include "SearchStats.h"
#include "Semaphore.h"
#include "Position.h"

// Constants
static constexpr int MAX_SEARCH_DEPTH = 255;
static constexpr int ROOT_PLY = 0;

class SearchResult {
public:
  Move bestMove = NOMOVE;
  Move ponderMove = NOMOVE;
  Value resultValue = VALUE_NONE;
  int64_t time = -1;
  int depth = 0;
  int extraDepth = 0;

  std::string str() const {
    return "Best Move: " + printMove(bestMove) + " (" + std::to_string(resultValue) + ") "
           + "Ponder Move: " + printMove(ponderMove) + " Depth: " + std::to_string(depth) + "/" +
           std::to_string(extraDepth);
  }
};

inline std::ostream &operator<<(std::ostream &os, const SearchResult &searchResult) {
  os << searchResult.str();
  return os;
}

class Search {

  std::shared_ptr<spdlog::logger> LOG = spdlog::get("Search_Logger");

  // thread control
  Semaphore initSemaphore; // used to block while initializing thread
  Semaphore searchSemaphore; // used to block while searching
  std::thread myThread;
  Engine *pEngine{nullptr};

  // search mode
  SearchLimits *pSearchLimits{nullptr};
  SearchStats searchStats;

  // current position
  Position position;

  // search state
  bool running = false;
  bool stopSearchFlag = false;

  // search mode overrides
  bool PERFT = false;

  // search result
  SearchResult lastSearchResult;

  // search start time
  std::chrono::time_point<std::chrono::steady_clock> startTime;
  // current best move
  Move currentBestRootMove;

  Value currentBestRootValue;

  // prepared move generator instances for each depth
  MoveGenerator moveGenerators[MAX_SEARCH_DEPTH];

public:
  ////////////////////////////////////////////////
  ///// CONSTRUCTORS

  /** Default constructor creates a board with a back reference to the engine */
  Search();
  explicit Search(Engine *pEng);
  virtual ~Search();

  ////////////////////////////////////////////////
  ///// PUBLIC

  /** starts the search in a separate thread with the given search limits */
  void startSearch(Position position, SearchLimits *limits);

  /** Stops a running search gracefully - e.g. returns the best move found so far */
  void stopSearch();

  /** checks if the search is already running */
  bool isRunning();

  /** wait while searching */
  void waitWhileSearching();

private:
  ////////////////////////////////////////////////
  ///// PRIVATE

  void run();
  SearchResult simulatedSearch(Position *pPosition);
  SearchResult iterativeDeepening(Position *pPosition);
  void configureTimeLimits();
  MoveList generateRootMoves(Position *pPosition);
};

#endif // FRANKYCPP_SEARCH_H
