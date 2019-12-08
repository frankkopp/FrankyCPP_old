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

  // Initialize for new search
  searchStats = SearchStats();

  initSemaphore.release();
  LOG->info("Search thread started.");

  // search mode override
  if (PERFT || pSearchLimits->perft) {
    LOG->info("Search Mode: PERFT SEARCH ({})", pSearchLimits->maxDepth);
    PERFT = true;
  }

  // search mode info
  LOG->info("Search Mode: {}", pSearchLimits->str());


  // DEBUG / PROTOTYPE
  simulatedSearch();

  running = false;
  searchSemaphore.release();
  LOG->info("Search thread ended.");
}

// DEBUG / PROTOTYPE
void Search::simulatedSearch() {
  LOG->debug("Generate debug move");
  MoveGenerator moveGenerator;
  LOG->debug("Generate position");
  LOG->debug(position.str());
  LOG->debug("Generate legal moves");
  MoveList moves = moveGenerator.generateLegalMoves(GENALL, &position);
  LOG->debug("Legal Moves: {}", printMoveList(moves));

  for (int i = 0; i < 5; ++i) {
    LOG->debug("Search SIMULATION: {}", i);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    if (stopSearchFlag) break;
  }

  LOG->info("Sending move: {}", printMove(moves.front()));
  if (pEngine) {
    pEngine->sendInfo(printMoveList(moves));
    pEngine->sendResult(moves.front(), NOMOVE);
  }
}

