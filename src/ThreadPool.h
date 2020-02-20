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

#ifndef FRANKYCPP_THREADPOOL_H
#define FRANKYCPP_THREADPOOL_H

#include <condition_variable>
#include <functional>
#include <iostream>
#include <future>
#include <vector>
#include <thread>
#include <queue>

class ThreadPool {
public:
  using Task = std::function<void()>;

  explicit ThreadPool(std::size_t numThreads);

  ~ThreadPool() { stop(); }

  template<class T>
  auto enqueue(T task) -> std::future<decltype(task())> {
    auto wrapper = std::make_shared<std::packaged_task<decltype(task())()>>(std::move(task));
    { // lock block
      std::unique_lock<std::mutex> lock{mEventMutex};
      mTasks.emplace([=] {
        (*wrapper)();
      });
    }
    mEventVar.notify_one();
    return wrapper->get_future();
  }

  auto openTasks() { return mTasks.size(); }

private:
  std::vector<std::thread> mThreads;
  std::condition_variable mEventVar;
  std::mutex mEventMutex;
  bool mStopping = false;
  std::queue<Task> mTasks;

  void start(std::size_t numThreads);
  void stop() noexcept;

};

#endif //FRANKYCPP_THREADPOOL_H
