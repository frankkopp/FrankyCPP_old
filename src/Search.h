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

struct SearchResult {
public:
  Move bestMove = NOMOVE;
  Move ponderMove = NOMOVE;
  int64_t time = 0;
  int depth = 0;
  int extraDepth = 0;

  std::string str() const {
    return "Best Move: " + printMove(bestMove) + " (" + std::to_string(valueOf(bestMove)) + ") "
           + "Ponder Move: " + printMove(ponderMove) + " Depth: " + std::to_string(depth) + "/" +
           std::to_string(extraDepth);
  }
};

inline std::ostream &operator<<(std::ostream &os, const SearchResult &searchResult) {
  os << searchResult.str();
  return os;
}

using Clock = std::chrono::high_resolution_clock;
using TimePoint = std::chrono::time_point<Clock>;
using Duration = std::chrono::duration<MilliSec , std::milli>;

class Search {

  std::shared_ptr<spdlog::logger> LOG = spdlog::get("Search_Logger");

  // UCI related
  constexpr static Duration UCI_UPDATE_INTERVAL = Duration (1'000);
  TimePoint lastUciUpdateTime {};
  
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
  TimePoint startTime {};
  TimePoint stopTime {};
  Duration softTimeLimit {};
  Duration hardTimeLimit {};
  Duration extraTime {};

  // the color of the searching player
  Color myColor = NOCOLOR;

  // list of moves at the root
  MoveList rootMoves;

  // store the current variation
  MoveList currentVariation;

  // store the current principal variation
  MoveList pv[MAX_SEARCH_DEPTH]{};

  // prepared move generator instances for each depth
  MoveGenerator moveGenerators[MAX_SEARCH_DEPTH]{};

  // Evaluator
  Evaluator evaluator;

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

  void ponderhit();
private:
  ////////////////////////////////////////////////
  ///// PRIVATE

  void run();

  SearchResult iterativeDeepening(Position *pPosition);
  void searchRoot(Position *pPosition, const int depth);
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
  
  static void savePV(Move move, MoveList &src, MoveList &dest);

  bool hardTimeLimitReached();
  void sendUCIIterationEndInfo();
  void sendUCICurrentRootMove();
  void sendUCISearchUpdate();

  void sendUCIBestMove();
  MilliSec getNps();

  inline Duration elapsedTime();
  static inline Duration elapsedTime(TimePoint t);
  static inline Duration elapsedTime(TimePoint t1, TimePoint t2);
  static inline TimePoint now();
};

#endif // FRANKYCPP_SEARCH_H
