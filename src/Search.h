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
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
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

#include <thread>
#include <iostream>

#include "Semaphore.h"
#include "SearchLimits.h"

using namespace std;

class Engine;

class Search {

  Semaphore mySemaphore;
  thread myThread;

  Engine *pEngine;
  SearchLimits *pSearchLimits;

  // search state
  bool running = false;
  bool stopSearchFlag = false;

public:

  ////////////////////////////////////////////////
  ///// CONSTRUCTORS

  /** Default constructor creates a board with a back reference to the engine */
  Search(Engine *pEng);
  Search() = delete;
  ~Search() = default;

  /** starts the search in a separate thread with the given search limits */
  void startSearch(SearchLimits *limits);

  /** stops a running search gracefully - e.g. returns the best move found so far */
  void stopSearch();

  /** checks if the search is already running */
  bool isRunning();

private:
  void run();


};


#endif //FRANKYCPP_SEARCH_H
