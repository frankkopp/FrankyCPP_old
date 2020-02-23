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
#include <optional>

/**
 * Synchronized FIFO queue based on std::queue and std::deque
 * @tparam T
 */
template<class T>
class Fifo {

  mutable std::mutex fifoLock;
  mutable std::condition_variable cv;

  std::queue<T, std::deque<T>> fifo;

  bool closedFlag = false;

public:
  Fifo() {
    LOG__TRACE(Logger::get().MAIN_LOG, "Constructor");
  }

  ~Fifo() = default;

  // copy
  Fifo(Fifo const &other) {
    LOG__TRACE(Logger::get().MAIN_LOG, "Copy constructor");
    std::scoped_lock lock{other.fifoLock};
    fifo = other.fifo;
  }

  // copy assignment
  Fifo &operator=(const Fifo &other) {
    LOG__TRACE(Logger::get().MAIN_LOG, "Copy assignment");
    std::scoped_lock lock(fifoLock, other.fifoLock);
    fifo = other.fifo;
    return *this;
  }

  // move
  Fifo(Fifo const &&other) {
    LOG__TRACE(Logger::get().MAIN_LOG, "Move constructor");
    std::scoped_lock lock{other.fifoLock};
    fifo = std::move(other.fifo);
  }

  // move assignment
  Fifo &operator=(const Fifo &&other) {
    LOG__TRACE(Logger::get().MAIN_LOG, "Move assignment");
    if (this != &other) {
      std::scoped_lock lock(fifoLock, other.fifoLock);
      fifo = std::move(other.fifo);
    }
    return *this;
  }

  void push(T &t) {
    {
      std::scoped_lock<std::mutex> lock{fifoLock};
      LOG__TRACE(Logger::get().MAIN_LOG, "Reference push");
      fifo.push(t);
    }
    cv.notify_one();
  }

  void push(T &&t) {
    {
      std::scoped_lock<std::mutex> lock{fifoLock};
      LOG__TRACE(Logger::get().MAIN_LOG, "Move push");
      fifo.push(std::move(t));
    }
    cv.notify_one();
  }

  /**
   * Retrieves an std::optional with the next item from the Fifo and removes
   * the item from the queue.
   * Returns an empty optional if called on empty list.
   */
  std::optional<T> pop() {
    std::scoped_lock<std::mutex> lock{fifoLock};
    if (fifo.empty()) return std::nullopt;
    LOG__TRACE(Logger::get().MAIN_LOG, "Value pop");
    std::optional<T> t{fifo.front()};
    fifo.pop();
    return t;
  }

  /**
   * Retrieves an std::optional with the next item from the Fifo and removes
   * the item from the queue.
   * Changes the given std::optional reference and returns it.
   * the optional will be an empty optional if called on empty list.
   */
  std::optional<T> pop(std::optional<T>  &t) {
    std::scoped_lock<std::mutex> lock{fifoLock};
    if (fifo.empty()) return std::nullopt;
    LOG__TRACE(Logger::get().MAIN_LOG, "Reference pop");
    t.emplace(fifo.front());
    fifo.pop();
    return t;
  }

  /**
   * Retrieves the next item from the Fifo . Blocks if the Fifo is empty and
   * waits until an item becomes available. Block can be canceled be calling
   * Fifo.close in which case this will return nullptr.
   */
  std::optional<T> pop_wait() {
    std::unique_lock<std::mutex> lock{fifoLock};
    if (closedFlag && fifo.empty()) return std::nullopt;
    LOG__TRACE(Logger::get().MAIN_LOG, "Pop wait value");
    cv.wait(lock, [this] { return !fifo.empty() || closedFlag; });
    if (fifo.empty()) return std::nullopt;
    std::optional<T> t{fifo.front()};
    fifo.pop();
    return t;
  }

  /**
   * Retrieves the next item from the Fifo. Blocks if the Fifo is empty and
   * waits until an item becomes available. Block can be canceled be calling
   * Fifo.close in which case this will return nullptr.
   */
  std::optional<T> pop_wait(std::optional<T>  &t) {
    std::unique_lock<std::mutex> lock{fifoLock};
    if (closedFlag && fifo.empty()) return std::nullopt;
    LOG__TRACE(Logger::get().MAIN_LOG, "Pop wait reference");
    cv.wait(lock, [this] { return !fifo.empty() || closedFlag; });
    if (fifo.empty()) return std::nullopt;
    t.emplace(fifo.front());
    fifo.pop();
    return t;
  }

  void close() {
    std::scoped_lock<std::mutex> lock{fifoLock};
    closedFlag = true;
    cv.notify_all();
  }

  void open() {
    std::scoped_lock<std::mutex> lock{fifoLock};
    closedFlag = false;
  }

  bool isClosed() {
    std::scoped_lock<std::mutex> lock{fifoLock};
    return closedFlag;
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
