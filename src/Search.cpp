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

#include "Engine.h"
#include "Search.h"

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
    std::cerr << "Search already running" << std::endl;
    return;
  }

  // // pos is a deep copy of the position parameter to not change
  // the original position given
  position = pos;
  pSearchLimits = limits;
  stopSearchFlag = false;

  // make sure we have a semaphore available
  searchSemaphore.release();
  // join() previous thread
  if (myThread.joinable()) myThread.join();
  // start search in a separate thread
  myThread = std::thread(&Search::run, this);
  //myThread.detach();
  // wait until thread is initialized before returning to caller
  initSemaphore.getOrWait();
  assert(running);
}

void Search::stopSearch() {
  if (!running) return;
  // set stop flag - search needs to check regularly and stop accordingly
  stopSearchFlag = true;
  // Wait for the thread to die
  if (myThread.joinable()) myThread.join();
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


  // DEBUG / PROTOTYPE

  for (int i = 0; i < 2; ++i) {
    std::cout << "Init SIMULATION: " << i << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
  std::cout << "New Thread: Init done!\n";
  initSemaphore.release();

  std::cout << "New Thread: Start work...!\n";
  for (int i = 0; i < 5; ++i) {
    std::cout << "Search SIMULATION: " << i << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(1));
    if (stopSearchFlag) break;
  }

  std::cout << "Generate debug move\n";
  MoveGenerator moveGenerator;
  std::cout << "Generate position\n";
  std::cout << position.str() << std::endl;
  std::cout << "Generate legal moves\n";
  MoveList moves = moveGenerator.generateLegalMoves(GENALL, &position);
  std::cout << "Legal Moves: " << moves << std::endl;
  std::cout << "Send move\n";
  std::ostringstream ss;
  ss << moves;
  if (pEngine) {
    pEngine->sendInfo(ss.str());
    pEngine->sendResult(moves.front(), NOMOVE);
  }

  // DEBUG / PROTOTYPE

  running = false;
  searchSemaphore.release();
}

