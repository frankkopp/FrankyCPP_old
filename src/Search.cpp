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

using namespace std;

////////////////////////////////////////////////
///// CONSTRUCTORS

Search::Search(Engine *pEng) { pEngine = pEng; }

Search::~Search() {
  // necessary to avoid err message: terminate called without an active exception
  if (myThread.joinable()) myThread.join();
}

////////////////////////////////////////////////
///// PUBLIC

void Search::startSearch(SearchLimits *limits) {
  if (running) {
    cerr << "Search already running" << endl;
    return;
  }
  pSearchLimits = limits;
  // make sure we have a semaphore available
  searchSemaphore.release();
  // join() previous thread
  if (myThread.joinable()) myThread.join();
  // start search in a separate thread
  myThread = thread(&Search::run, this);
  //myThread.detach();
  // wait until thread is initialized before returning to caller
  initSemaphore.getOrWait();
  assert(running);
}

void Search::stopSearch() {
  if (!running)
    return;
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
  stopSearchFlag = false;
  running = true;

  // DEBUG / PROTOTYPE
  cout << "New Thread: Started!\n";
    this_thread::sleep_for(chrono::seconds(1));
  cout << "New Thread: Init done!\n";
  initSemaphore.release();

  cout << "New Thread: Start work...!\n";
  for (int i = 0; i < 2; ++i) {
    cout << "Search SIMULATION: " << i << endl;
    this_thread::sleep_for(std::chrono::seconds(1));
    if (stopSearchFlag) break;
  }

  cout << "Generate debug move\n";
  MoveGenerator moveGenerator;
  cout << "Generate position\n";
  cout << pEngine->getPosition()->str() << endl;
  Position myPosition(*pEngine->getPosition());
  cout << myPosition.str() << endl;
  cout << pEngine->getPosition() << endl;
  cout << &myPosition << endl;
  cout << "Generate legal moves\n";
  MoveList moves = moveGenerator.generateLegalMoves(GENALL, &myPosition);
  cout << "Legal Moves: " << moves << endl;
  cout << "Send move\n";
  ostringstream ss;
  ss << moves;
  pEngine->sendInfo(ss.str());
  pEngine->sendResult(moves.front(), NOMOVE);

  // DEBUG / PROTOTYPE

  running = false;
  cout << "New Thread: Finished!\n";
  searchSemaphore.release();
}

