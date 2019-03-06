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

#include <iostream>
#include "Search.h"

using namespace std;

void Search::start() {

  cout << "Start Thread!\n";
  thread myThread(&Search::run, this);
  myThread.detach();

  cout << "Wait for Thread init!\n";
  mySemaphore.wait();

  cout << "Thread Started - return to caller\n";
}

void Search::run() {
  cout << "New Thread: Started!\n";

  for (int i = 0; i < 3; ++i) {
    cout << "Init SIM: " << i << endl;
    this_thread::sleep_for(chrono::seconds(1));
  }
  cout << "New Thread: Init done!\n";
  mySemaphore.notify();

  cout << "New Thread: Start work...!\n";
  for (int i = 0; i < 100; ++i) {
    cout << "Search SIM: " << i << endl;
    this_thread::sleep_for(std::chrono::seconds(1));
  }
  cout << "New Thread: Finished!\n";
}
