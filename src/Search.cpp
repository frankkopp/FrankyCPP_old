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
  myColor = position.getNextPlayer();
  searchLimits = std::move(limits);

  // make sure we have a semaphore available
  searchSemaphore.release();

  // join() previous thread
  if (myThread.joinable()) myThread.join();
  stopSearchFlag = false;

  // start search in a separate thread
  LOG->debug("Starting search in separate thread.");
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

  LOG->debug("Search thread started.");

  // store the start time of the search
  startTime = lastUciUpdateTime = std::chrono::high_resolution_clock::now();

  // Initialize for new search
  lastSearchResult = SearchResult();
  softTimeLimit = hardTimeLimit = extraTime = 0;
  searchStats = SearchStats();

  // Initialize ply based data
  // Each depth in search gets it own global field to avoid object creation
  // during search.
  for (auto &moveGenerator : moveGenerators) {
    moveGenerator = MoveGenerator();
  }

  // search mode
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
  LOG->debug("Search thread ended.");
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
    LOG->debug("Time Management: {} soft: {:n} hard: {:n}",
               (searchLimits.timeControl ? "ON" : "OFF"),
               softTimeLimit, hardTimeLimit);
    LOG->debug("Start Depth: {} Max Depth: {}", iterationDepth, searchLimits.maxDepth);
    LOG->debug("Starting iterative deepening now...");
  };

  // check search requirements
  assert (!rootMoves.empty() && "No root moves to search");
  assert (currentBestRootMove != NOMOVE && "No initial best root move");
  assert (iterationDepth > 0 && "iterationDepth <= 0");

  // first iteration update before start
  sendUCIIterationEndInfo();

  // ###########################################
  // ### BEGIN Iterative Deepening
  do {
    assert (currentBestRootMove != NOMOVE && "No  best root move");

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

    sendUCIIterationEndInfo();

    // break on stop signal
    if (stopSearchFlag || softTimeLimitReached()) break;

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
  searchStats.lastSearchTime = std::chrono::duration_cast<std::chrono::milliseconds>(
    stopTime - startTime).count();

  // print result of the search
  if (LOG->should_log(spdlog::level::debug)) {
    LOG->debug("Search statistics: {}", searchStats.str());
    LOG->debug("Search Depth was {} ({})", searchStats.currentIterationDepth,
               searchStats.currentExtraSearchDepth);
    LOG->debug("Search took {}", fmt::format("{},{:03} sec",
                                             (searchStats.lastSearchTime % 1'000'000) /
                                             1'000,
                                             (searchStats.lastSearchTime % 1'000)));
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
  int i = 0;
  for (auto &move : rootMoves) {
    //LOG->trace("Root Move: {}", printMove(move));

    searchStats.currentRootMove = move;
    searchStats.currentRootMoveNumber = ++i;
    sendUCICurrentRootMove();

    value = searchMove(pPosition, depth, ROOT_PLY, move, true);

    if (stopConditions()) return VALUE_NONE; // value does not matter because of top flag
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

    if (stopConditions()) return VALUE_NONE; // value does not matter because of top flag
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
    searchStats.nodesVisited++;
    currentVariation.push_back(move);
    sendUCISearchUpdate();
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

inline bool Search::stopConditions() {
  return stopSearchFlag = (stopSearchFlag
                           || hardTimeLimitReached()
                           || (searchLimits.nodes
                               && searchStats.nodesVisited >= searchLimits.nodes));
}

void Search::configureTimeLimits() {
  // TODO: time management

  if (searchLimits.moveTime > 0) { // mode time per move
    softTimeLimit = hardTimeLimit = searchLimits.moveTime;
  }
  else { // remaining time - estimated time per move

    // retrieve time left from search mode
    assert ((searchLimits.whiteTime && searchLimits.blackTime) && "move times must be > 0");
    MilliSec timeLeft = (myColor == WHITE) ? searchLimits.whiteTime : searchLimits.blackTime;

    // Give some overhead time so that in games with very low available time we
    // do not run out of time
    timeLeft -= 1'000; // this should do

    // when we know the move to go (until next time control) use them otherwise assume 40
    int movesLeft = searchLimits.movesToGo > 0 ? searchLimits.movesToGo : 40;

    // when we have a time increase per move we estimate the additional time we should have
    if (myColor == WHITE) {
      timeLeft += 40 * searchLimits.whiteInc;
    }
    else { // we assume it is black to safe the extra check
      timeLeft += 40 * searchLimits.blackInc;
    }

    // for timed games with remaining time
    hardTimeLimit = (MilliSec) (timeLeft / movesLeft);
    softTimeLimit = (MilliSec) (hardTimeLimit * 0.8);
  }

  // limits for very short available time
  if (hardTimeLimit < 100) {
    addExtraTime(0.9);
  }
}

/**
 * Changes the time limit by the given factor and also sets the soft time limit
 * to 0.8 of the hard time limit.
 * Factor 1 is neutral. <1 shortens the time, >1 adds time<br/>
 * Example: factor 0.8 is 20% less time. Factor 1.2 is 20% additional time
 * Always calculated from the initial time budget.
 *
 * @param factor
 */
void Search::addExtraTime(double factor) {
  if (searchLimits.moveTime == 0) {
    extraTime += hardTimeLimit * (factor - 1);
    LOG->debug("Time added {:n} ms to {:n} ms",
               extraTime,
               hardTimeLimit + extraTime);
  }
}

/**
 * Soft time limit is used in iterative deepening to decide if an new depth should even be
 * started.
 *
 * @return true if soft time limit is reached, false otherwise
 */
bool Search::softTimeLimitReached() {
  if (!searchLimits.timeControl) return false;
  stopSearchFlag = elapsedTime() >= softTimeLimit + (extraTime * 0.8);
  if (stopSearchFlag) LOG->trace("Soft time limit reached!");
  return stopSearchFlag;
}

/**
 * Hard time limit is used to check time regularily in the search to stop the search when
 * time is out
 * TODO instead of checking this regulary we could use a timer thread to set stopSearch to true.
 *
 * @return true if hard time limit is reached, false otherwise
 */
bool Search::hardTimeLimitReached() {
  if (!searchLimits.timeControl) return false;
  stopSearchFlag = elapsedTime() >= hardTimeLimit + extraTime;
  if (stopSearchFlag) LOG->trace("Hard time limit reached!");
  return stopSearchFlag;
}

/**
 * @return the elapsed time in ms since the start of the search
 */
MilliSec Search::elapsedTime() {
  return elapsedTime(startTime);
}

/**
 * @param t time point since the elapsed time
 * @return the elapsed time from the start of the search to the given t
 */
MilliSec Search::elapsedTime(std::chrono::time_point<std::chrono::steady_clock> t) {
  return std::chrono::duration_cast<std::chrono::milliseconds>(
    std::chrono::high_resolution_clock::now() - t).count();
}

/**
 * Generates root moves and filters them according to the UCI searchmoves list
 * @param pPosition
 * @return UCI filtered root moves
 */
MoveList Search::generateRootMoves(Position *pPosition) {
  MoveList *legalMoves = moveGenerators[ROOT_PLY].generateLegalMoves(GENALL, pPosition);
  MoveList moveList;
  if (searchLimits.moves.empty()) { // if UCI searchmoves is empty then add all
    for (auto legalMove : *legalMoves) {
      moveList.push_back(legalMove);
    }
  }
  else { // only add if in the UCI searchmoves list
    for (auto legalMove : *legalMoves) {
      for (auto move : searchLimits.moves) {
        if (moveOf(move) == moveOf(legalMove)) {
          moveList.push_back(legalMove);
        }
      }
    }
  }
  return moveList;
}

void Search::sendUCIIterationEndInfo() {

  // DEBUG
  MoveList pvList = MoveList();
  pvList.push_back(currentBestRootMove);

  std::string infoString =
    fmt::format(
      "depth {:d} seldepth {:d} multipv 1 {:d} nodes {:d} nps {:d} time {:d} pv {:s}",
      searchStats.currentIterationDepth,
      searchStats.currentExtraSearchDepth,
      currentBestRootValue,
      searchStats.nodesVisited,
      getNps(),
      elapsedTime(),
      printMoveListUCI(pvList));

  if (pEngine == nullptr) LOG->warn("<no engine> >> {}", infoString);
  else
    pEngine->sendIterationEndInfo(searchStats.currentIterationDepth,
                                  searchStats.currentExtraSearchDepth,
                                  currentBestRootValue,
                                  searchStats.nodesVisited,
                                  getNps(),
                                  elapsedTime(),
                                  pvList);
}

void Search::sendUCICurrentRootMove() {
  std::string infoString =
    fmt::format(
      "currmove {:s} currmovenumber {:d}",
      printMove(searchStats.currentRootMove),
      searchStats.currentRootMoveNumber);

  if (pEngine == nullptr) LOG->warn("<no engine> >> {}", infoString);
  else
    pEngine->sendCurrentRootMove(searchStats.currentRootMove,
                                 searchStats.currentRootMoveNumber);
}

void Search::sendUCISearchUpdate() {
  if (elapsedTime(lastUciUpdateTime) > UCI_UPDATE_INTERVAL) {
    lastUciUpdateTime = std::chrono::high_resolution_clock::now();

    std::string infoString = fmt::format(
      "depth {:d} seldepth {:d} nodes {:d} nps {:d} time {:d} hashfull {:d}",
      searchStats.currentIterationDepth,
      searchStats.currentExtraSearchDepth,
      searchStats.nodesVisited,
      getNps(),
      elapsedTime(),
      0); // TODO

    //(int) (1000 * ((float) transpositionTable.getNumberOfEntries() / transpositionTable.getMaxEntries())));
    if (pEngine == nullptr) LOG->warn("<no engine>> {}", infoString);
    else
      pEngine->sendSearchUpdate(searchStats.currentIterationDepth,
                                searchStats.currentExtraSearchDepth,
                                searchStats.nodesVisited,
                                getNps(),
                                elapsedTime(),
                                0);

    infoString = fmt::format("currline {:s}",
                             printMoveListUCI(currentVariation));
    if (pEngine == nullptr) LOG->warn("<no engine> >> {}", infoString);
    else pEngine->sendCurrentLine(currentVariation);
  }
}

MilliSec Search::getNps() {
  return 1000 * searchStats.nodesVisited / (1 + elapsedTime());
}





