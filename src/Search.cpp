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

void Search::startSearch(const Position& pos, SearchLimits limits) {
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
  for (auto & moveGenerator : moveGenerators) {
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
  lastSearchResult = iterativeDeepening();

  LOG->info("Search Result: {}", lastSearchResult.str());
  if (pEngine) {
    pEngine->sendResult(lastSearchResult.bestMove, lastSearchResult.ponderMove);
  }

  running = false;
  searchSemaphore.release();
  LOG->info("Search thread ended.");
}

SearchResult Search::iterativeDeepening() {
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

  // start depth from searchMode
  int depth = searchLimits.startDepth;

  // if time based game setup the soft and hard time limits
  if (searchLimits.timeControl) configureTimeLimits();

  // current search depth
  searchStats.currentSearchDepth = ROOT_PLY;
  searchStats.currentExtraSearchDepth = ROOT_PLY;

  // generate all legal root moves, and set pv move if we got one from TT
  MoveList rootMoves = generateRootMoves(&position);
  LOG->debug("Root moves: {}", printMoveList(rootMoves));

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
    LOG->debug("Start Depth: {} Max Depth: {}", depth, searchLimits.maxDepth);
    LOG->debug("Starting iterative deepening now...");
  };

  // check search requirements
  assert (!rootMoves.empty() && "No root moves to search");
  assert (currentBestRootMove != NOMOVE && "No initial best root move");
  assert (depth > 0 && "depth <= 0");

  // ###########################################
  // ### BEGIN Iterative Deepening
  do {
    SPDLOG_TRACE(LOG, "Depth {} start", depth);
    assert (currentBestRootMove != NOMOVE && "No  best root move");

    searchStats.currentIterationDepth = depth;
    searchStats.bestMoveChanges = 0;
    searchStats.nodesVisited++; // root node is always first searched node

    // ###########################################
    // ### CALL SEARCH for depth
    // ###

    // ALPHA_BETA
    Value value = simulatedSearch(&position, depth);// search(position, depth, ROOT_PLY,alpha, beta, PV_NODE, DO_NULL);

    // ###
    // ###########################################

    // break on stop signal
    // TODO: time management
    if (stopSearchFlag) break;

    SPDLOG_TRACE(LOG, "Depth {} end", depth);
  } while (++depth <= searchLimits.maxDepth );
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
                               (searchStats.lastSearchTime % 1'000'000'000'000) / 1'000'000'000,
                               (searchStats.lastSearchTime % 1'000'000'000)));
  }

  return searchResult;
}

// DEBUG / PROTOTYPE
Value Search::simulatedSearch(Position *pPosition, int depth) {
  MoveGenerator moveGenerator;
  LOG->debug("\n" + pPosition->str());
  MoveList* moves = moveGenerator.generateLegalMoves(GENALL, pPosition);
  for (int i = 0; i < depth; ++i) {
    LOG->debug("Search SIMULATION: {}", i);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    if (stopSearchFlag) break;
  }
  SearchResult searchResult;
  searchResult.bestMove = moves->front();
  return VALUE_NONE;
}

void Search::configureTimeLimits() {
  // TODO
}

MoveList Search::generateRootMoves(Position *pPosition) {
  LOG->debug("Generating root moves");
  MoveList* legalMoves = moveGenerators[ROOT_PLY].generatePseudoLegalMoves(GENALL, pPosition);
  LOG->debug("All legal moves {}", printMoveList(*legalMoves));
  MoveList rootMoves;
  if (searchLimits.moves.empty()) {
    LOG->debug("Adding all legal moves.");
    for (auto legalMove : *legalMoves) {
      rootMoves.push_back(legalMove);
    }
  } else {
    LOG->debug("Adding UCI selected moves {}", printMoveList(searchLimits.moves));
    for (auto legalMove : *legalMoves) {
      for (auto move : searchLimits.moves) {
        if (moveOf(move) == moveOf(legalMove)) {
          LOG->debug("Adding move: {}", printMove(legalMove));
          rootMoves.push_back(legalMove);
        }
      }
    }
  }
  return rootMoves;
}

