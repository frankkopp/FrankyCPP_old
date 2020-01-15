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

// included dependencies
#include <iostream>
#include <ostream>
#include <thread>

#include "gtest/gtest_prod.h"
#include "types.h"
#include "Logging.h"
#include "MoveGenerator.h"
#include "Semaphore.h"
#include "SearchStats.h"
#include "SearchLimits.h"
#include "Evaluator.h"

//#include "Position.h"

// forward declared dependencies
class Engine;
class TT;

struct SearchResult {
  Move bestMove = MOVE_NONE;
  Value bestMoveValue = VALUE_NONE;
  Move ponderMove = MOVE_NONE;
  int64_t time = 0;
  int depth = 0;
  int extraDepth = 0;

  std::string str() const {
    return "Best Move: " + printMove(bestMove) + " (" +
           std::to_string(bestMoveValue) + ") " +
           "Ponder Move: " + printMove(ponderMove) +
           " Depth: " + std::to_string(depth) + "/" +
           std::to_string(extraDepth);
  }
};

inline std::ostream &operator<<(std::ostream &os,
                                const SearchResult &searchResult) {
  os << searchResult.str();
  return os;
}

class Search {

  std::shared_ptr<spdlog::logger> LOG = spdlog::get("Search_Logger");

  // used to protect the transposition table from clearing and resizing during
  // search
  std::timed_mutex tt_lock;

  // UCI related
  constexpr static MilliSec UCI_UPDATE_INTERVAL = 1'000;
  MilliSec lastUciUpdateTime{};

  // thread control
  Semaphore initSemaphore;   // used to block while initializing thread
  Semaphore searchSemaphore; // used to block while searching
  std::thread myThread;

  // pointer to engine of available
  Engine* pEngine{nullptr};

  // search mode
  SearchLimits* searchLimitsPtr{nullptr};
  SearchStats searchStats;

  // search state
  std::atomic_bool _isRunning = false;
  std::atomic_bool _stopSearchFlag = false;
  std::atomic_bool _hasResult = false;

  // search result
  SearchResult lastSearchResult;

  // transposition table (singleton)
  TT *tt;

  // time check every x nodes
  // As time checks are expensive we only do them every x-th node.
  // As we loose precession in time keeping with this this must not be
  // too high.
  constexpr static uint64_t TIME_CHECK_FREQ = 0b1111'1111'1111'1111;

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

  // store current iteration depth to limit max quiescence depth
  Depth currentIterationDepth = static_cast<Depth>(0);

  // store the current principal variation
  MoveList pv[DEPTH_MAX]{};

  // prepared move generator instances for each depth
  MoveGenerator moveGenerators[DEPTH_MAX]{};

  // mate threat in ply revealed by null move search
  bool mateThreat[DEPTH_MAX]{};

  // Evaluator
  Evaluator evaluator;

  const char *getTest() { return "TEST"; };

public:
  // for code re-using through templating we use search types when calling
  // search()
  enum Search_Type { ROOT, NONROOT, QUIESCENCE, PERFT };

  // in PV we search the full window in NonPV we try a zero window first
  enum Node_Type { NonPV, PV };

  enum Do_Null : bool { No_Null_Move = false, Do_Null_Move = true };

  ////////////////////////////////////////////////
  ///// CONSTRUCTORS

  /** Default constructor creates a board with a back reference to the engine */
  Search();
  explicit Search(Engine *pEng);
  ~Search();
  // disallow copies and moves
  Search(Search const &) = delete;
  Search &operator=(const Search &) = delete;
  Search(Search const &&) = delete;
  Search &operator=(const Search &&) = delete;

  ////////////////////////////////////////////////
  ///// PUBLIC

  /** starts the search in a separate thread with the given search limits */
  void startSearch(const Position &position, SearchLimits &limits);

  /** Stops a running search gracefully - e.g. returns the best move found so
   * far */
  void stopSearch();

  /** checks if the search is already running */
  bool isRunning() const;

  /** signals if we have a result */
  bool hasResult() const;

  /** wait while searching */
  void waitWhileSearching();

  /** return search stats instance */
  inline const SearchStats &getSearchStats() const { return searchStats; }

  /** return the last search result */
  inline const SearchResult &getLastSearchResult() const {
    return lastSearchResult;
  };

  /** to signal the search that pondering was successful */
  void ponderhit();

  /** return current pv */
  const MoveList &getPV() const { return pv[PLY_ROOT]; };

  /** clears the hash table */
  void clearHash();

  /** resize the hash to the given value in MB */
  void setHashSize(int sizeInMB);

private:
  ////////////////////////////////////////////////
  ///// PRIVATE

  void run(Position position);

  SearchResult iterativeDeepening(Position &refPosition);

  template <Search_Type ST, Node_Type NT>
  Value search(Position &position, Depth depth, Ply ply, Value alpha,
               Value beta, Do_Null doNull);

  template <Search_Type ST> Move getMove(Position &position, int ply);

  template <Search_Type ST> bool checkDrawRepAnd50(Position &position) const;

  Value evaluate(Position &position);

  /**
   * Generates root moves and filters them according to the UCI searchmoves list
   * @param position
   * @return UCI filtered root moves
   */
  MoveList generateRootMoves(Position &refPosition);
  static bool rootMovesSort(Move m1, Move m2);
  static bool goodCapture(Position &refPosition, Move move);
  static void savePV(Move move, MoveList &src, MoveList &dest);

  /**
   * Retrieves the PV line from the transposition table in root search.
   *
   * @param position
   * @param depth
   * @param pvRoot
   */
  void getPVLine(Position &position, MoveList &pvRoot, Depth depth);

  void storeTT(Position &position, Value value, Value_Type ttType, Depth depth,
               Ply ply, Move move, bool mateThreat);

  /**
   * correct any mate values which are sent to TT so that
   * they are relative to the ply we are in
   * @param value
   * @param ply
   * @return
   */
  static Value valueToTT(Value value, Ply ply);

  /**
   * correct any mate values coming from the TT so that
   * they are relative to the ply we are in
   * @param value
   * @param ply
   * @return
   */
  static Value valueFromTT(Value value, Ply ply);

  void configureTimeLimits();
  inline bool stopConditions(bool shouldTimeCheck);

  /**
   * Changes the time limit by the given factor and also sets the soft time
   * limit to 0.8 of the hard time limit. Factor 1 is neutral. <1 shortens the
   * time, >1 adds time<br/> Example: factor 0.8 is 20% less time. Factor 1.2 is
   * 20% additional time Always calculated from the initial time budget.
   *
   * @param d
   */
  void addExtraTime(double d);

  /**
   * Time limit is used to check time regularly in the search to stop the search
   * when time is out
   * IDEA instead of checking this regularly we could use a
   *  timer thread to set stopSearch to true.
   *
   * @return true if hard time limit is reached, false otherwise
   */
  inline bool timeLimitReached();

  inline bool shouldTimeCheck() const;

  /**
   * @param t time point since the elapsed time
   * @return the elapsed time from the start of the search to the given t
   */
  static inline MilliSec elapsedTime(MilliSec t);

  /**
   * @param t1 Earlier time point
   * @param t2 Later time point
   * @return Duration between time points in milliseconds
   */
  static inline MilliSec elapsedTime(MilliSec t1, MilliSec t2);

  /**
   * Returns the current time in ms
   * @return current time
   */
  static inline MilliSec now();

  inline MilliSec getNps() const;

  void sendIterationEndInfoToEngine() const;
  void sendCurrentRootMoveToEngine() const;
  void sendSearchUpdateToEngine();
  void sendResultToEngine() const;

  FRIEND_TEST(SearchTest, goodCapture);
};

#endif // FRANKYCPP_SEARCH_H
