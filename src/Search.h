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
#include <iosfwd>
#include <ostream>
#include <thread>
#include <atomic>
#include "types.h"
#include "Semaphore.h"
#include "SearchStats.h"
#include "SearchLimits.h"
#include "MoveGenerator.h"
#include "gtest/gtest_prod.h"

// forward declared dependencies
class Engine;
class TT;
class Evaluator;
class MoveGenerator;

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

//  std::shared_ptr<spdlog::logger> LOG = spdlog::get("Search_Logger");

  // used to protect the transposition table from clearing and resizing during
  // search
  std::timed_mutex tt_lock;

  // UCI related
  constexpr static MilliSec UCI_UPDATE_INTERVAL = 500;
  MilliSec lastUciUpdateTime{};

  // thread control
  Semaphore initSemaphore;   // used to block while initializing thread
  Semaphore searchSemaphore; // used to block while searching
  std::thread searchThread;
  std::thread timerThread;

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
  std::unique_ptr<Evaluator> pEvaluator;

public:
  // for code re-using through templating we use search types when calling
  // search()
  enum Search_Type { ROOT, NONROOT, QUIESCENCE, PERFT };

  // in PV we search the full window in NonPV we try a zero window first
  enum Node_Type { NonPV, PV };

  // If this is true we are allowed to do a NULL move in this ply. This is to
  // avoid recursive null move searches or other prunings which do not make sense
  // in a null move search
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

  /**
   * Called after starting the search in a new thread. Configures the search
   * and eventually calls iterativeDeepening. After the search it takes the
   * result to send it to the UCI engine.
   * @param position
   */
  void run(Position position);

  /**
   * The iterative search function searches through each consecutive depth until
   * time is up or some other criteria (number of nodes searched, etc.) is met.
   */
  SearchResult iterativeDeepening(Position &refPosition);

  /**
   * The main search function. The templating distinguishes between
   * search in the root node, normal non root nodes and quiscence nodes.
   * Also takes care of the special PERFT case.
   * Template parameter node type will distinguish between PV and NonPV nodes
   * for PVS search.
   */
  template <Search_Type ST, Node_Type NT>
  Value search(Position &position, Depth depth, Ply ply, Value alpha,
               Value beta, Do_Null doNull);

  /**
   * Returns the next move from the move generator depending on the search type.
   * In the root node the next root move, in non root the next move from the
   * on demand move generator and in quiescence the next capturing move also
   * from the on demand move generator.
   */
  template <Search_Type ST>
  Move getMove(Position &position, int ply);

  /**
   * Returns true if we either have a 3-fold repetition ot have more than 100 reversible moves.
   * OBS: In quiescence search we only check for a single repetition
   */
  template <Search_Type ST>
  bool checkDrawRepAnd50(Position &position) const;

  /**
   * Calculates an evaluation value for the given position
   */
  Value evaluate(Position &position);

  /**
   * Generates root moves and filters them according to the UCI searchmoves list
   */
  MoveList generateRootMoves(Position &refPosition);

  /**
   * Sort the root moves according to their numerical value.
   * As root moves have their calculated value stored as the upper 16-bit
   * of the 32-bit int this will sort them very quickly.
   */
  static bool rootMovesSort(Move m1, Move m2);

  /**
   * Used to filter out only valuable captures in quiescence search.
   * Will be replaced by SEE in the future.
   */
  static bool goodCapture(Position &position, Move move);

  /**
   * Stores the best move of a ply search (a move which raised alpha) into the
   * principal variation list. As we have such a list for each depth we can
   * add them up to get a list of the overall best variation at the root node.
   */
  static void savePV(Move move, MoveList &src, MoveList &dest);

  /**
   * Retrieves the PV line from the transposition table in root search.
   */
  void getPVLine(Position &position, MoveList &pvRoot, Depth depth);

  /**
   * Stores search result of a node to the transposition table
   */
  void storeTT(Position &position, Value value, Value_Type ttType, Depth depth,
               Ply ply, Move move, bool mateThreat);

  /**
   * correct any mate values which are sent to TT so that
   * they are relative to the ply we are in
   */
  static Value valueToTT(Value value, Ply ply);

  /**
   * correct any mate values coming from the TT so that
   * they are relative to the ply we are in
   */
  static Value valueFromTT(Value value, Ply ply);

  /**
   * configures the timeLimit value from the given searchLimits
   */
  void configureTimeLimits();

  /**
   * checks if search should stop
   */
  inline bool stopConditions();

  /**
   * Changes the time limit by the given factor and also sets the soft time
   * limit to 0.8 of the hard time limit. Factor 1 is neutral. <1 shortens the
   * time, >1 adds time<br/> Example: factor 0.8 is 20% less time. Factor 1.2 is
   * 20% additional time Always calculated from the initial time budget.
   */
  void addExtraTime(double d);

  /**
   * Time limit is used to check time regularly in the search to stop the search
   * when time is out
   * @return true if hard time limit is reached, false otherwise
   */
  inline bool timeLimitReached();

  /**
   * Starts a thread which waits for the timeLimit + extraTime amount
   * of time and then sets the stopSearchFlag to true;
   */
  void startTimer();

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

  /**
   * Returns the current nodes per second value
   */
  inline MilliSec getNps() const;

  /** these are sending information to the UCI protocol */
  void sendIterationEndInfoToEngine() const;
  void sendCurrentRootMoveToEngine() const;
  void sendSearchUpdateToEngine();
  void sendResultToEngine() const;
  void sendStringToEngine(const std::string& anyString) const;

  FRIEND_TEST(SearchTest, goodCapture);
  FRIEND_TEST(SearchTest, timerTest);

};

#endif // FRANKYCPP_SEARCH_H
