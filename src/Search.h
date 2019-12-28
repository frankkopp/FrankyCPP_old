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
#include "time.h"
#include "Logging.h"
#include "SearchStats.h"
#include "Semaphore.h"
#include "Position.h"
#include "SearchLimits.h"
#include "Evaluator.h"
#include "TT.h"

struct SearchResult {
public:
  Move bestMove = MOVE_NONE;
  Move ponderMove = MOVE_NONE;
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

class Search {
  
  std::shared_ptr<spdlog::logger> LOG = spdlog::get("Search_Logger");

  enum Search_Type {
    ROOT, NONROOT, QUIESCENCE
  };

  // UCI related
  constexpr static MilliSec UCI_UPDATE_INTERVAL = 1'000;

  MilliSec lastUciUpdateTime{};

  // thread control
  Semaphore initSemaphore; // used to block while initializing thread
  Semaphore searchSemaphore; // used to block while searching
  std::thread myThread;
  
  // pointer to engine of available
  Engine* pEngine{nullptr};
  
  // search mode
  SearchLimits searchLimits;
  SearchStats searchStats;
  
  // current position
  Position myPosition;
  
  // search state
  std::atomic_bool running = false;
  std::atomic_bool stopSearchFlag = false;
  
  // search result
  SearchResult lastSearchResult;
  
  // transposition table (singleton)
  TT tt;

private:
  
  // search start time
  MilliSec startTime{};
  MilliSec stopTime{};
  MilliSec timeLimit{};
  MilliSec extraTime{};
  
  // the color of the searching player
  Color myColor = NOCOLOR;
  
  // list of moves at the root
  MoveList rootMoves;
  MoveList::size_type currentMoveIndex = 0;
  
  // store the current variation
  MoveList currentVariation;
  
  // store the current principal variation
  MoveList pv[DEPTH_MAX]{};
  
  // prepared move generator instances for each depth
  MoveGenerator moveGenerators[DEPTH_MAX]{};
  
  // Evaluator
  Evaluator evaluator;

public:
  ////////////////////////////////////////////////
  ///// CONSTRUCTORS
  
  /** Default constructor creates a board with a back reference to the engine */
  Search();
  explicit Search(Engine* pEng);
  virtual ~Search();
  
  ////////////////////////////////////////////////
  ///// PUBLIC
  
  /** starts the search in a separate thread with the given search limits */
  void startSearch(const Position &position, SearchLimits &limits);
  
  /** Stops a running search gracefully - e.g. returns the best move found so far */
  void stopSearch();
  
  /** checks if the search is already running */
  bool isRunning() const;
  
  /** wait while searching */
  void waitWhileSearching();
  
  /** return search stats instance */
  inline const SearchStats &getSearchStats() const { return searchStats; }
  
  /** return the last search result */
  inline const SearchResult &getLastSearchResult() const { return lastSearchResult; };
  
  /** to signal the search that pondering was successful */
  void ponderhit();
  
  /** return current pv */
  const MoveList &getPV() const { return pv[PLY_ROOT]; };

  /** clears the hash table */
  void clearHash();
private:
  ////////////////////////////////////////////////
  ///// PRIVATE
  
  FRIEND_TEST(SearchTest, goodCapture);
  
  void run();
  
  SearchResult iterativeDeepening(Position &refPosition);
  
  template<Search_Type ST>
  Value search(Position &position, Depth depth, Ply ply, Value alpha, Value beta);
  template<Search_Type ST>
  Move getMove(Position &refPosition, int ply);
  template<Search_Type ST>
  bool checkDrawRepAnd50(Position &refPosition) const;
  Value evaluate(Position &position, Ply ply);
  
  MoveList generateRootMoves(Position &refPosition);
  static bool rootMovesSort(Move m1, Move m2);
  static bool goodCapture(Position &refPosition, Move move);
  static void savePV(Move move, MoveList &src, MoveList &dest);
  
  void storeTT(Position &position, Value value, TT::EntryType ttType, Depth depth, Move bestMove, bool mateThreat);
  
  void configureTimeLimits();
  inline bool stopConditions();
  void addExtraTime(const double d);
  inline bool timeLimitReached();
  static inline MilliSec elapsedTime(const MilliSec t);
  static inline MilliSec elapsedTime(const MilliSec t1, const MilliSec t2);
  static inline MilliSec now();
  inline MilliSec getNps() const;
  
  void sendIterationEndInfoToEngine() const;
  void sendCurrentRootMoveToEngine() const;
  void sendSearchUpdateToEngine();
  void sendResultToEngine() const;

};

#endif // FRANKYCPP_SEARCH_H
