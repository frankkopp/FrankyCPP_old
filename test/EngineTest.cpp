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

#include <gtest/gtest.h>
#include "types.h"
#include "Logging.h"
#include "Engine.h"
#include "Position.h"


using testing::Eq;
using namespace std;

class EngineTest : public ::testing::Test {
public:
  static void SetUpTestSuite() {
    NEWLINE;
    INIT::init();
    NEWLINE;
  }
protected:
  void SetUp() override {}
  void TearDown() override {}
};

TEST_F(EngineTest, startSearch) {
  Engine engine;
  UCISearchMode uciSearchMode;
  uciSearchMode.depth = 8;
  engine.startSearch(uciSearchMode);

  LOG__INFO(Logger::get().TEST_LOG, "{}: Start and Stop test...", __FUNCTION__);
  for (int i = 0; i < 3; ++i) {
    sleep(3);
    engine.stopSearch();
    engine.waitWhileSearching();

    engine.startSearch(uciSearchMode);

    sleep(3);
    engine.stopSearch();
    engine.waitWhileSearching();
  }
  SUCCEED();
}

TEST_F(EngineTest, doMove) {
  Logger::get().ENGINE_LOG->set_level(spdlog::level::debug);
  Engine engine;
  // position fen 8/P7/8/7k/8/8/1p6/5K2 w - - 1 1 moves a7a8q
  engine.setPosition("8/P7/8/7k/8/8/1p6/5K2 w - - 1 1");
  engine.doMove("a7a8q");
  EXPECT_EQ("Q7/8/8/7k/8/8/1p6/5K2 b - - 0 1", engine.getPosition()->printFen());

  engine.setPosition("8/P7/8/7k/8/8/1p6/5K2 w - - 1 1");
  engine.doMove("a7a8Q");
  EXPECT_EQ("Q7/8/8/7k/8/8/1p6/5K2 b - - 0 1", engine.getPosition()->printFen());

  engine.setPosition("8/P7/8/7k/8/8/1p6/5K2 b - - 1 1");
  engine.doMove("b2b1q");
  EXPECT_EQ("8/P7/8/7k/8/8/8/1q3K2 w - - 0 2", engine.getPosition()->printFen());

  engine.setPosition("8/P7/8/7k/8/8/1p6/5K2 b - - 1 1");
  engine.doMove("b2b1Q");
  EXPECT_EQ("8/P7/8/7k/8/8/8/1q3K2 w - - 0 2", engine.getPosition()->printFen());
}




