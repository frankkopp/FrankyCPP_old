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

#include <string>
#include <random>
#include <chrono>
#include "types.h"
#include "Logging.h"
#include "Fifo.h"
#include <gtest/gtest.h>

using testing::Eq;

class FifoTest : public ::testing::Test {
public:
  static void SetUpTestSuite() {
    NEWLINE;
    INIT::init();
    NEWLINE;
    Logger::get().TEST_LOG->set_level(spdlog::level::debug);
    Logger::get().MAIN_LOG->set_level(spdlog::level::warn);
  }

protected:
  void SetUp() override {}
  void TearDown() override {}

  std::random_device rd;
  std::uniform_int_distribution<unsigned long long> randomU64;

};

TEST_F(FifoTest, construct) {

  Fifo<std::string> fifo1;
  for (int i = 0; i < 1'000; ++i) {
    fifo1.push(std::to_string(randomU64(rd)));
  }
  LOG__INFO(Logger::get().TEST_LOG, "Entries in fifo: {:n}", fifo1.size());
  EXPECT_EQ(1'000, fifo1.size());

  Fifo<std::string> fifo2(fifo1);
  LOG__INFO(Logger::get().TEST_LOG, "Copied constructed new fifo2: {:n}", fifo2.size());
  EXPECT_EQ(1'000, fifo2.size());

  Fifo<std::string> fifo3 = fifo2;
  LOG__INFO(Logger::get().TEST_LOG, "Copied constructed new fifo2: {:n}", fifo3.size());
  EXPECT_EQ(1'000, fifo3.size());

  Fifo<std::string> fifo4 = Fifo<std::string>{};
  LOG__INFO(Logger::get().TEST_LOG, "Constructed fifo4: {:n}", fifo4.size());
  EXPECT_EQ(0, fifo4.size());

  fifo4 = fifo1;
  LOG__INFO(Logger::get().TEST_LOG, "Copied fifo1 into fifo4: {:n}", fifo4.size());
  EXPECT_EQ(1'000, fifo4.size());

  Fifo<std::string> fifo5(std::move(Fifo<std::string>{}));
  LOG__INFO(Logger::get().TEST_LOG, "Move constructed fifo5: {:n}", fifo5.size());
  EXPECT_EQ(0, fifo5.size());

  fifo5 = std::move(fifo4);
  LOG__INFO(Logger::get().TEST_LOG, "Moved fifo4 to fifo5: {:n}", fifo5.size());
  EXPECT_EQ(1'000, fifo5.size());
  //EXPECT_EQ(0, fifo4.size());

}

TEST_F(FifoTest, pushPop) {
  Fifo<std::string> fifo1;
  for (int i = 0; i < 1'000; ++i) {
    fifo1.push(std::to_string(randomU64(rd)));
  }
  LOG__INFO(Logger::get().TEST_LOG, "Entries in fifo: {:n}", fifo1.size());
  EXPECT_EQ(1'000, fifo1.size());

  auto item = fifo1.pop();
  LOG__INFO(Logger::get().TEST_LOG, "Popped on item: {}", item);
  LOG__INFO(Logger::get().TEST_LOG, "Entries in fifo: {:n}", fifo1.size());
  EXPECT_EQ(999, fifo1.size());

  std::string str;
  fifo1.pop(str);
  LOG__INFO(Logger::get().TEST_LOG, "Popped on item: {}", str);
  LOG__INFO(Logger::get().TEST_LOG, "Entries in fifo: {:n}", fifo1.size());
  EXPECT_EQ(998, fifo1.size());

  item = fifo1.pop_wait();
  LOG__INFO(Logger::get().TEST_LOG, "Popped on item: {}", item);
  LOG__INFO(Logger::get().TEST_LOG, "Entries in fifo: {:n}", fifo1.size());
  EXPECT_EQ(997, fifo1.size());

  fifo1.pop_wait(str);
  LOG__INFO(Logger::get().TEST_LOG, "Popped on item: {}", str);
  LOG__INFO(Logger::get().TEST_LOG, "Entries in fifo: {:n}", fifo1.size());
  EXPECT_EQ(996, fifo1.size());
}

TEST_F(FifoTest, popWait) {
  Fifo<std::string> fifo1;
  auto t = std::thread([&]{
    sleep(5);
    fifo1.push("This it the first item in fifo");
  });
  LOG__INFO(Logger::get().TEST_LOG, "Fifo emtpy");
  const auto start = std::chrono::high_resolution_clock::now();
  EXPECT_EQ(0, fifo1.size());
  EXPECT_TRUE(fifo1.empty());
  LOG__INFO(Logger::get().TEST_LOG, "Waiting for item");
  auto item = fifo1.pop_wait();
  const auto stop = std::chrono::high_resolution_clock::now();
  const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
  LOG__INFO(Logger::get().TEST_LOG, "Got item '{}' after {:n} ms", item, elapsed.count());
  EXPECT_GT(elapsed.count(), 5'000);
  t.join();
}
