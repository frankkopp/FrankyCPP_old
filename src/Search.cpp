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

#include <iostream>
#include <chrono>
#include <utility>
#include "Search.h"
#include "Engine.h"

////////////////////////////////////////////////
///// CONSTRUCTORS

Search::Search() {
  pEngine = nullptr;
}

Search::Search(Engine *pEng) {
  pEngine = pEng;
}

Search::~Search() {
  // necessary to avoid err message: terminate called without an active exception
  if (myThread.joinable()) myThread.join();
}

////////////////////////////////////////////////
///// PUBLIC

void Search::startSearch(const Position &pos, SearchLimits limits) {
  if (running) {
    LOG->error("Search already running");
    return;
  }

  // pos is a deep copy of the position parameter to not change
  // the original position given
  position = pos;
  searchLimits = std::move(limits);
  stopSearchFlag = false;

  // make sure we have a semaphore available
  searchSemaphore.release();

  // join() previous thread
  if (myThread.joinable()) myThread.join();

  // start search in a separate thread
  LOG->info("Starting search in separate thread.");
  myThread = std::thread(&Search::run, this);

  // wait until thread is initialized before returning to caller
  initSemaphore.getOrWait();
  assert(running);
  LOG->info("Search started.");
}

void Search::stopSearch() {
  if (!running) return;
  LOG->info("Stopping search.");
  // set stop flag - search needs to check regularly and stop accordingly
  stopSearchFlag = true;
  // Wait for the thread to die
  if (myThread.joinable()) myThread.join();
  LOG->info("Search stopped.");
  assert(!running);
}

bool Search::isRunning() { return running; }

void Search::waitWhileSearching() {
  if (!running) return;
  searchSemaphore.getOrWait();
  searchSemaphore.release();
}

////////////////////////////////////////////////
///// PRIVATE

/**
 * Called when the new search thread is started.
 * Initializes the search.
 * Calls <code>iterativeDeepening()</code> when search is initialized.
 * <p>
 * After the search has stopped calls <code>Engine.sendResult(searchResult)</code>
 * to store/hand over the result. After storing the result the search is ended
 * and the thread terminated.<br>
 */
void Search::run() {

  // get the search lock
  searchSemaphore.getOrWait();
  running = true;

  LOG->info("Search thread started.");

  // Initialize for new search
  searchStats = SearchStats();

  // Initialize ply based data
  // Each depth in search gets it own global field to avoid object creation
  // during search.
  for (auto &moveGenerator : moveGenerators) {
    moveGenerator = MoveGenerator();
  }

  // search mode override
  if (searchLimits.perft) {
    LOG->info("Search Mode: PERFT SEARCH ({})", searchLimits.maxDepth);
  }
  if (searchLimits.infinite) {
    LOG->info("Search Mode: INFINITE SEARCH");
  }

  // initialization done
  initSemaphore.release();

  // start iterative deepening
  lastSearchResult = iterativeDeepening(&position);

  LOG->info("Search Result: {}", lastSearchResult.str());
  if (pEngine) {
    pEngine->sendResult(lastSearchResult.bestMove, lastSearchResult.ponderMove);
  }

  running = false;
  searchSemaphore.release();
  LOG->info("Search thread ended.");
}

/**
 * Generates root moves and calls search in a loop increasing depth
 * with each iteration.
 * <p>
 * Detects mate if started on a mate position.
 * @param pPosition
 * @return
 */
SearchResult Search::iterativeDeepening(Position *pPosition) {
  // store the start time of the search
  startTime = std::chrono::high_resolution_clock::now();

  // init best move and value
  currentBestRootMove = NOMOVE;
  currentBestRootValue = VALUE_NONE;

  // prepare search result
  SearchResult searchResult;

  // no legal root moves - game already ended!
  if (!moveGenerators[ROOT_PLY].hasLegalMove(&position)) {
    if (position.hasCheck()) searchResult.resultValue = -VALUE_CHECKMATE;
    else searchResult.resultValue = VALUE_DRAW;
    return searchResult;
  }

  // start iterationDepth from searchMode
  int iterationDepth = searchLimits.startDepth;

  // if time based game setup the soft and hard time limits
  if (searchLimits.timeControl) configureTimeLimits();

  // current search iterationDepth
  searchStats.currentSearchDepth = ROOT_PLY;
  searchStats.currentExtraSearchDepth = ROOT_PLY;

  // generate all legal root moves, and set pv move if we got one from TT
  rootMoves = generateRootMoves(&position);
  LOG->debug("Root moves: {}", printMoveList(rootMoves));
  // TODO: remove value from moves / reset to min?

  // make sure we have a temporary best move
  if (currentBestRootMove == NOMOVE) { // when using TT this will already be set
    currentBestRootMove = rootMoves.front();
  }

  // print search setup for debugging
  if (LOG->should_log(spdlog::level::debug)) {
    LOG->debug("Searching in position: {}", position.printFen());
    LOG->debug("Searching these moves: {}", printMoveList(rootMoves));
    LOG->debug("Search mode: {}", searchLimits.str());
    LOG->debug("Time Management: {} soft: {} hard: {}",
               (searchLimits.timeControl ? "ON" : "OFF"),
               "TBD", "TBD");
    LOG->debug("Start Depth: {} Max Depth: {}", iterationDepth, searchLimits.maxDepth);
    LOG->debug("Starting iterative deepening now...");
  };

  // check search requirements
  assert (!rootMoves.empty() && "No root moves to search");
  assert (currentBestRootMove != NOMOVE && "No initial best root move");
  assert (iterationDepth > 0 && "iterationDepth <= 0");

  // ###########################################
  // ### BEGIN Iterative Deepening
  do {
    assert (currentBestRootMove != NOMOVE && "No  best root move");

    LOG->trace("Searching depth {}", iterationDepth);

    searchStats.currentIterationDepth = iterationDepth;
    searchStats.bestMoveChanges = 0;
    searchStats.nodesVisited++; // root node is always first searched node

    // ###########################################
    // ### CALL SEARCH for iterationDepth
    searchRoot(pPosition, iterationDepth);
    // ###########################################

    // we can only use the value if there has not been a stop
    if (!stopSearchFlag) {
      sort(rootMoves.begin(), rootMoves.end());
    }

    // break on stop signal
    // TODO: time management
    if (stopSearchFlag) break;

  } while (++iterationDepth <= searchLimits.maxDepth);
  // ### ENDOF Iterative Deepening
  // ###########################################

  // update searchResult here
  searchResult.bestMove = currentBestRootMove;
  searchResult.resultValue = currentBestRootValue;
  searchResult.depth = searchStats.currentSearchDepth;
  searchResult.extraDepth = searchStats.currentExtraSearchDepth;

  // search is finished - stop timer
  stopTime = std::chrono::high_resolution_clock::now();
  searchStats.lastSearchTime = std::chrono::duration_cast<std::chrono::nanoseconds>(
    stopTime - startTime).count();

  // print result of the search
  if (LOG->should_log(spdlog::level::debug)) {
    LOG->debug("Search statistics: {}", searchStats.str());
    LOG->debug("Search Depth was {} ({})", searchStats.currentIterationDepth,
               searchStats.currentExtraSearchDepth);
    LOG->debug("Search took {}", fmt::format("{},{:09} sec",
                                             (searchStats.lastSearchTime % 1'000'000'000'000) /
                                             1'000'000'000,
                                             (searchStats.lastSearchTime % 1'000'000'000)));
  }

  return searchResult;
}

/**
 * Called for root position as root moves are generated separately.
 * @param pPosition
 * @param depth
 * @param ply
 * @return
 */
Value Search::searchRoot(Position *pPosition, const int depth) {

  // TODO: check draw by repetition or 50moves rule / necessary here???

  Value value = VALUE_NONE;
  for (auto &move : rootMoves) {
    LOG->trace("Root Move: {}", printMove(move));

    value = searchMove(pPosition, depth, ROOT_PLY, move, true);

    if (stopConditions()) {
      stopSearchFlag = true;
      return VALUE_NONE; // value does not matter because of top flag
    }
  }

  return value;
}

/**
 * Recursive search for all non root positions with on the fly move generation.
 * @param pPosition
 * @param depth
 * @param ply
 * @return
 */
Value Search::searchNonRoot(Position *pPosition, const int depth, const int ply) {

  // update current search depth stats
  searchStats.currentSearchDepth = std::max(searchStats.currentSearchDepth, ply);
  searchStats.currentExtraSearchDepth = std::max(searchStats.currentExtraSearchDepth, ply);

  // On leaf node call qsearch
  if (depth <= 0 || ply >= MAX_SEARCH_DEPTH - 1) {
    return qsearch(pPosition, ply);
  }

  // TODO: check draw by repetition or 50moves rule

  assert(ply != ROOT_PLY);
  Value value = VALUE_NONE;
  Move move = NOMOVE;
  moveGenerators[ply].resetOnDemand();
  while ((move = moveGenerators[ply].getNextPseudoLegalMove(GENALL, pPosition)) != NOMOVE) {

    value = searchMove(pPosition, depth, ply, move, false);

    if (stopConditions()) {
      stopSearchFlag = true;
      return VALUE_NONE; // value does not matter because of top flag
    }
  }
  return value;
}

/**
 * Called for each move of a position.
 * @param pPosition
 * @param depth
 * @param ply
 * @param move
 * @param isRoot
 * @return
 */
Value Search::searchMove(Position *pPosition, const int depth, const int ply, const Move &move,
                         const bool isRoot) {
  Value value = VALUE_NONE;
  pPosition->doMove(move);
  if (pPosition->isLegalPosition()) {
    currentVariation.push_back(move);
    value = searchNonRoot(&position, depth - 1, ply + 1);
    currentVariation.pop_back();
  }
  pPosition->undoMove();
  return value;
}

Value Search::qsearch(Position *pPosition, const int ply) {
  // update current search depth stats
  searchStats.currentExtraSearchDepth = std::max(searchStats.currentExtraSearchDepth, ply);

  // if PERFT or MAX SEARCH DEPTH reached return with eval to count all captures etc.
  if (searchLimits.perft || ply >= MAX_SEARCH_DEPTH - 1) return evaluate(pPosition, ply);

  // TODO quiscence search
  return evaluate(pPosition, ply);
}

Value Search::evaluate(Position *pPosition, const int ply) {
  //  LOG->trace("Evaluate {}", printMoveList(currentVariation));

  // count all leaf nodes evaluated
  searchStats.leafPositionsEvaluated++;

  // PERFT stats
  if (searchLimits.perft) {
    // TODO: more perft stats
    return VALUE_ONE;
  }

  // TODO: evaluation
  return VALUE_DRAW;
}

inline bool Search::stopConditions() const {
  // TODO: time management
  return stopSearchFlag
         || (searchLimits.nodes
             && searchStats.nodesVisited >= searchLimits.nodes);
}

void Search::configureTimeLimits() {
  // TODO: time management
}

/**
 * Generates root moves and filters them according to the UCI searchmoves list
 * @param pPosition
 * @return UCI filtered root moves
 */
MoveList Search::generateRootMoves(Position *pPosition) {
  MoveList *legalMoves = moveGenerators[ROOT_PLY].generatePseudoLegalMoves(GENALL, pPosition);
  MoveList rootMoves;
  if (searchLimits.moves.empty()) { // if UCI searchmoves is empty then add all
    for (auto legalMove : *legalMoves) {
      rootMoves.push_back(legalMove);
    }
  }
  else { // only add if in the UCI searchmoves list
    for (auto legalMove : *legalMoves) {
      for (auto move : searchLimits.moves) {
        if (moveOf(move) == moveOf(legalMove)) {
          rootMoves.push_back(legalMove);
        }
      }
    }
  }
  return rootMoves;
}




