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

#include <algorithm>
#include <iostream>
#include <chrono>
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

void Search::startSearch(Position pos, SearchLimits *limits) {
  if (running) {
    LOG->error("Search already running");
    return;
  }

  // pos is a deep copy of the position parameter to not change
  // the original position given
  position = Position(pos);
  pSearchLimits = limits;
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
  LOG->info("Search started.");
  assert(running);
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
  for (int i = 0; i < MAX_SEARCH_DEPTH; i++) {
    moveGenerators[i] = MoveGenerator();
  }

  // search mode override
  if (PERFT || pSearchLimits->perft) {
    LOG->info("Search Mode: PERFT SEARCH ({})", pSearchLimits->maxDepth);
    PERFT = true;
  }

  LOG->info("Search Mode: {}", pSearchLimits->str());

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

SearchResult Search::iterativeDeepening(Position *pPosition) {
  SPDLOG_TRACE(LOG, "Iterative deepening start.");

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
  int depth = pSearchLimits->startDepth;

  // if time based game setup the soft and hard time limits
  if (pSearchLimits->timeControl) configureTimeLimits();

  // current search depth
  searchStats.currentSearchDepth = ROOT_PLY;
  searchStats.currentExtraSearchDepth = ROOT_PLY;

  // generate all legal root moves, and set pv move if we got one from TT
  MoveList rootMoves = generateRootMoves(&position);

  // PROTOTYPE / DEBUG
  return simulatedSearch(pPosition);
}

// DEBUG / PROTOTYPE
SearchResult Search::simulatedSearch(Position *pPosition) {
  LOG->debug("Generate debug move");
  MoveGenerator moveGenerator;
  LOG->debug("Generate position");
  LOG->debug("\n" + pPosition->str());
  LOG->debug("Generate legal moves");
  MoveList moves = moveGenerator.generateLegalMoves(GENALL, pPosition);
  LOG->debug("Legal Moves: {}", printMoveList(moves));

  for (int i = 0; i < 5; ++i) {
    LOG->debug("Search SIMULATION: {}", i);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    if (stopSearchFlag) break;
  }

  SearchResult searchResult;
  searchResult.bestMove = moves.front();

  return searchResult;
}

void Search::configureTimeLimits() {
  // TODO
}

MoveList Search::generateRootMoves(Position *pPosition) {
  MoveList legalMoves = moveGenerators[ROOT_PLY].generatePseudoLegalMoves(GENALL, pPosition);

  return MoveList();
}

