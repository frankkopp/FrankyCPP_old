/*
 * MIT License
 *
 * Copyright (c) 2018-2020 Frank Kopp
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

#ifndef FRANKYCPP_FIFO_H
#define FRANKYCPP_FIFO_H

#include <queue>
#include <deque>
#include <mutex>
#include <condition_variable>

/**
 * Synchronized FIFO queue based on std::queue and std::deque
 * @tparam T
 */
template<class T>
class Fifo {

  mutable std::mutex fifoLock;
  mutable std::condition_variable cv;
  std::queue<T, std::deque<T>> fifo;

public:
  Fifo() {
    LOG__DEBUG(Logger::get().MAIN_LOG, "Constructor");
  }

  ~Fifo() = default;

  // copy
  Fifo(Fifo const &other) {
    LOG__DEBUG(Logger::get().MAIN_LOG, "Copy constructor");
    std::scoped_lock lock{other.fifoLock};
    fifo = other.fifo;
  }

  // copy assignment
  Fifo &operator=(const Fifo &other) {
    LOG__DEBUG(Logger::get().MAIN_LOG, "Copy assignment");
    std::scoped_lock lock(fifoLock, other.fifoLock);
    fifo = other.fifo;
    return *this;
  }

  // move
  Fifo(Fifo const &&other) {
    LOG__DEBUG(Logger::get().MAIN_LOG, "Move constructor");
    std::scoped_lock lock{other.fifoLock};
    fifo = std::move(other.fifo);
  }

  // move assignment
  Fifo &operator=(const Fifo &&other) {
    LOG__DEBUG(Logger::get().MAIN_LOG, "Move assignment");
    if (this != &other) {
      std::scoped_lock lock(fifoLock, other.fifoLock);
      fifo = std::move(other.fifo);
    }
    return *this;
  }

  void push(T &t) {
    LOG__DEBUG(Logger::get().MAIN_LOG, "Reference push");
    {
      std::scoped_lock<std::mutex> lock{fifoLock};
      fifo.push(t);
    }
    cv.notify_one();
  }

  void push(T &&t) {
    LOG__DEBUG(Logger::get().MAIN_LOG, "Move push");
    {
      std::scoped_lock<std::mutex> lock{fifoLock};
      fifo.push(std::move(t));
    }
    cv.notify_one();
  }

  T pop() {
    LOG__DEBUG(Logger::get().MAIN_LOG, "Value pop");
    std::scoped_lock<std::mutex> lock{fifoLock};
    auto t = fifo.front();
    fifo.pop();
    return t;
  }

  T pop(T &t) {
    LOG__DEBUG(Logger::get().MAIN_LOG, "Reference pop");
    std::scoped_lock<std::mutex> lock{fifoLock};
    t = fifo.front();
    fifo.pop();
    return t;
  }

  T pop_wait() {
    LOG__DEBUG(Logger::get().MAIN_LOG, "Pop wait value");
    std::unique_lock<std::mutex> lock{fifoLock};
    cv.wait(lock, [this] { return !fifo.empty(); });
    auto t = fifo.front();
    fifo.pop();
    return t;
  }

  T pop_wait(T &t) {
    LOG__DEBUG(Logger::get().MAIN_LOG, "Pop wait reference");
    std::unique_lock<std::mutex> lock{fifoLock};
    cv.wait(lock, [this] { return !fifo.empty(); });
    t = fifo.front();
    fifo.pop();
    return t;
  }

  bool empty() const {
    std::scoped_lock<std::mutex> lock{fifoLock};
    return fifo.empty();
  }

  std::size_t size() const {
    std::scoped_lock<std::mutex> lock{fifoLock};
    return fifo.size();
  }

};


#endif //FRANKYCPP_FIFO_H
