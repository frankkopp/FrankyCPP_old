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
class Engine;

// included dependencies
#include <iostream>
#include <thread>
#include <chrono>
#include "logging.h"
#include "SearchStats.h"
#include "Semaphore.h"
#include "Position.h"
#include "SearchLimits.h"
#include "Evaluator.h"

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

  // UCI related
  constexpr static MilliSec UCI_UPDATE_INTERVAL = 1'000;
  std::chrono::time_point<std::chrono::steady_clock> lastUciUpdateTime;
  
  // thread control
  Semaphore initSemaphore; // used to block while initializing thread
  Semaphore searchSemaphore; // used to block while searching
  std::thread myThread;

  // pointer to engine of available
  Engine *pEngine{nullptr};

  // search mode
  SearchLimits searchLimits;
  SearchStats searchStats;

  // current position
  Position position;

  // search state
  bool running = false;
  bool stopSearchFlag = false;

  // search result
  SearchResult lastSearchResult;

  // search start time
  std::chrono::time_point<std::chrono::steady_clock> startTime;
  std::chrono::time_point<std::chrono::steady_clock> stopTime;
  MilliSec softTimeLimit = 0;
  MilliSec hardTimeLimit = 0;
  MilliSec extraTime = 0;

  // the color of the searching player
  Color myColor = NOCOLOR;

  // list of moves at the root
  MoveList rootMoves;

  // current best move
  Move currentBestRootMove = NOMOVE;
  Value currentBestRootValue = VALUE_NONE;

  // store the current variation
  MoveList currentVariation;

  // prepared move generator instances for each depth
  MoveGenerator moveGenerators[MAX_SEARCH_DEPTH];

  // Evaluator
  Evaluator evaluator = Evaluator();

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
  void startSearch(const Position& position, SearchLimits limits);

  /** Stops a running search gracefully - e.g. returns the best move found so far */
  void stopSearch();

  /** checks if the search is already running */
  bool isRunning();

  /** wait while searching */
  void waitWhileSearching();

  /** return search stats instance */
  inline const SearchStats &getSearchStats() const { return searchStats; }

private:
  ////////////////////////////////////////////////
  ///// PRIVATE

  void run();

  SearchResult iterativeDeepening(Position *pPosition);
  Value searchRoot(Position *pPosition, int depth);
  Value searchNonRoot(Position *pPosition, int depth, int ply);
  Value searchMove(Position *pPosition, int depth, int ply, const Move &move,
                   bool isRoot);
  Value qsearch(Position *pPosition, int ply);
  Value evaluate(Position *position, int ply);

  MoveList generateRootMoves(Position *pPosition);
  void configureTimeLimits();
  inline bool stopConditions();
  void addExtraTime(double d);
  bool softTimeLimitReached();
  bool hardTimeLimitReached();
  MilliSec elapsedTime();
  MilliSec elapsedTime(std::chrono::time_point<std::chrono::steady_clock> t);
  void sendUCIIterationEndInfo();
  void sendUCICurrentRootMove();
  void sendUCISearchUpdate();
  MilliSec getNps();
};

#endif // FRANKYCPP_SEARCH_H
