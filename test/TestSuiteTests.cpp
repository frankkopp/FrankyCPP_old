/*
 * MIT License
 *
 * Copyright (c) 2020 Frank Kopp
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
#include <regex>
#include "types.h"
#include "misc.h"
#include "Search.h"
#include "Engine.h"
#include "Logging.h"
#include <gtest/gtest.h>
#include <TestSuite.h>

using testing::Eq;

class TestSuiteTest : public ::testing::Test {
public:
  static void SetUpTestSuite() {
    NEWLINE;
    INIT::init();
    NEWLINE;
  }
protected:
  void SetUp() override {
    Logger::get().TSUITE_LOG->set_level(spdlog::level::debug);
    Logger::get().SEARCH_LOG->set_level(spdlog::level::info);
  }

  void TearDown() override {}
};

TEST_F(TestSuiteTest, BlunderTestSuite) {
  std::string filePath = FrankyCPP_PROJECT_ROOT;
  filePath+= + "/testsets/franky_blunders.epd";
  MilliSec moveTime = 2'000;
  Depth depth{0};
  TestSuite testSuite(filePath, moveTime, depth);
  testSuite.runTestSuite();
}

TEST_F(TestSuiteTest, MateTestSuite) {
  std::string filePath = FrankyCPP_PROJECT_ROOT;
  filePath+= + "/testsets/mate_test_suite.epd";
  MilliSec moveTime = 1'000;
  Depth depth{0};
  TestSuite testSuite(filePath, moveTime, depth);
  testSuite.runTestSuite();
}

TEST_F(TestSuiteTest, CCC1Suite) {
  std::string filePath = FrankyCPP_PROJECT_ROOT;
  filePath+= + "/testsets/ccc-1.epd";
  MilliSec moveTime = 5'000;
  Depth depth{0};
  TestSuite testSuite(filePath, moveTime, depth);
  testSuite.runTestSuite();
}

TEST_F(TestSuiteTest, NoLotSuite) {
  std::string filePath = FrankyCPP_PROJECT_ROOT;
  filePath+= + "/testsets/nolot.epd";
  MilliSec moveTime = 5'000;
  Depth depth{0};
  TestSuite testSuite(filePath, moveTime, depth);
  testSuite.runTestSuite();
}

TEST_F(TestSuiteTest, ZugzwangSuite) {
  std::string filePath = FrankyCPP_PROJECT_ROOT;
  filePath+= + "/testsets/nullMoveZugZwangTest.epd";
  MilliSec moveTime = 5'000;
  Depth depth{0};
  TestSuite testSuite(filePath, moveTime, depth);
  testSuite.runTestSuite();
}

TEST_F(TestSuiteTest, WAC_Suite) {
  std::string filePath = FrankyCPP_PROJECT_ROOT;
  filePath+= + "/testsets/wac.epd";
  MilliSec moveTime = 5'000;
  Depth depth{0};
  TestSuite testSuite(filePath, moveTime, depth);
  testSuite.runTestSuite();
}

TEST_F(TestSuiteTest, ecm98) {
  std::string filePath = FrankyCPP_PROJECT_ROOT;
  filePath+= + "/testsets/ecm98.epd";
  MilliSec moveTime = 1'000;
  Depth depth{0};
  TestSuite testSuite(filePath, moveTime, depth);
  testSuite.runTestSuite();
}

/**
 * https://sites.google.com/site/strategictestsuite/
 * 10 sec
 */
TEST_F(TestSuiteTest, STS_Suite) {
  std::string filePath = FrankyCPP_PROJECT_ROOT;
  filePath+= + "/testsets/STS1-STS15_LAN.EPD";
  MilliSec moveTime = 10'000;
  Depth depth{0};
  TestSuite testSuite(filePath, moveTime, depth);
  testSuite.runTestSuite();
}
