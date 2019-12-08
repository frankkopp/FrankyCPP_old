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

#include <gtest/gtest.h>
#include "../../src/logging.h"
#include "../../src/Engine.h"

using testing::Eq;
using namespace std;

class EngineTest : public ::testing::Test {
public:
  static void SetUpTestSuite() {
    NEWLINE;
    LOGGING::init();
    INIT::init();
    NEWLINE;
  }
protected:
  void SetUp() override {}
  void TearDown() override {}
};

TEST_F(EngineTest, basic) {
  Engine engine;
  cout << "\nEngine" << engine << endl;
}

// TODO Test doMove command


TEST_F(EngineTest, startSearch) {
  Engine engine;
  UCISearchMode uciSearchMode;
  uciSearchMode.depth = 3;
  engine.startSearch(&uciSearchMode);

  cout << "Start and Stop test..." << endl;
  for (int i = 0; i < 3; ++i) {
    sleep(3);
    engine.stopSearch();
    engine.waitWhileSearching();

    engine.startSearch(&uciSearchMode);

    sleep(3);
    engine.stopSearch();
    engine.waitWhileSearching();
  }
  cout << "...FINISHED" << endl;

}


