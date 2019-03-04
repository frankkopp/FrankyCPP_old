/*
 * MIT License
 *
 * Copyright (c) 2018 Frank Kopp
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

#include "UCIProtocolHandler.h"

UCIProtocolHandler::UCIProtocolHandler() {
  std::cout << "Hello World!\n";
  isRunning = false;
}

UCIProtocolHandler::~UCIProtocolHandler() {
  std::cout << "Byebye!\n";
}

void UCIProtocolHandler::start() {
  std::cout << "Start Thread!\n";

  myThread = std::thread([this] { this->run(); });

  while (!isRunning) {
    std::this_thread::sleep_for(std::chrono::nanoseconds(1000));
  }

  std::cout << "Thread Started\n";

  if (myThread.get_id() == std::this_thread::get_id()) std::cout << "start: NEW THREAD" << std::endl;
  else std::cout << "start: OLD THREAD" << std::endl;

  myThread.join();
  std::cout << "Thread Ended\n";
  isRunning = false;
}

void UCIProtocolHandler::run() {
  std::cout << "New Thread: Started!\n";
  isRunning = true;
  if (myThread.get_id() == std::this_thread::get_id()) std::cout << "run: NEW THREAD" << std::endl;
  else std::cout << "run: OLD THREAD" << std::endl;
  std::this_thread::sleep_for(std::chrono::seconds(2));
  std::cout << "New Thread: Finished!\n";
}



