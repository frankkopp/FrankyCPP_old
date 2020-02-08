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
#include "SearchConfig.h"
#include <gtest/gtest.h>
#include <TestSuite.h>

using testing::Eq;

class TestSuiteTests : public ::testing::Test {
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


TEST_F(TestSuiteTests, runTestSet) {

  std::string filePath = "abc";
  MilliSec moveTime = 5'000;
  Depth depth = static_cast<Depth>(10);

  TestSuite testSuite(filePath, moveTime, depth);

  std::vector<TestSuite::Test> ts = {
    {"Mate in 4",          "8/8/8/8/8/3K4/R7/5k2 w - -",                          TestSuite::DM, "4"},
    {"Best move Ke3",      "8/8/8/8/8/3K4/R7/5k2 w - -",                          TestSuite::BM, "Ke3"},
    {"Several best moves", "3r3k/1r3p1p/p1pB1p2/8/p1qNP1Q1/P6P/1P4P1/3R3K w - -", TestSuite::BM, "Bf8 Nf5 Qf4"}
  };

  testSuite.runTestSet(ts);
  for (TestSuite::Test t : ts) {
    LOG__INFO(Logger::get().TEST_LOG, "Test '{}' {}", t.id, TestSuite::print(t.result));
  }
}

TEST_F(TestSuiteTests, cleanUpLine) {
  std::string filePath = "";
  MilliSec moveTime = 5'000;
  Depth depth = static_cast<Depth>(10);
  TestSuite testSuite(filePath, moveTime, depth);
  TestSuite::Test test;

  std::string line = "# 2rqk2r/pb1nbp1p/4p1p1/1B1n4/Np1N4/7Q/PP3PPP/R1B1R1K1 w - - bm Rxe6; id \"CCC-I No.1\";";
  TestSuite::cleanUpLine(line);
  ASSERT_TRUE(line.empty());
}

TEST_F(TestSuiteTests, readLine) {

  std::string filePath = "";
  MilliSec moveTime = 5'000;
  Depth depth = static_cast<Depth>(10);
  TestSuite testSuite(filePath, moveTime, depth);
  TestSuite::Test test;

  std::string line = "8/8/8/8/8/3K4/R7/5k2 w - - dm 4; id \"FRANKY-1 #1\";";
  ASSERT_TRUE(testSuite.readOneEPD(line, test));

  line = "3r3k/1r3p1p/p1pB1p2/8/p1qNP1Q1/P6P/1P4P1/3R3K w - - bm Bf8 Nf5 Qf4; id \"WAC.294\";";
  ASSERT_TRUE(testSuite.readOneEPD(line, test));

  line = "r1bqk2r/pp1n1ppp/2pbpn2/3p4/2PP4/3BPN2/PP1N1PPP/R1BQK2R w KQkq - bm e4; id \"Crafty Test Pos.21\"; c0 \"GK/DB Philadelphia 1996, Game 4, move 7W (e4)\";";
  ASSERT_TRUE(testSuite.readOneEPD(line, test));

  line = "7k/8/3p4/4N3/8/5p2/P7/1K2N3 w - - bm N5xf3; id \"FRANKY-1 #6\";";
  ASSERT_TRUE(testSuite.readOneEPD(line, test));

  line = "# 2rqk2r/pb1nbp1p/4p1p1/1B1n4/Np1N4/7Q/PP3PPP/R1B1R1K1 w - - bm Rxe6; id \"CCC-I No.1\";";
  ASSERT_FALSE(testSuite.readOneEPD(line, test));

  // no id
  line = "4r1b1/1p4B1/pN2pR2/RB2k3/1P2N2p/2p3b1/n2P1p1r/5K1n w - - dm 3;";
  ASSERT_TRUE(testSuite.readOneEPD(line, test));

  // Result has additional chars (! or ?)
  line = "2kr4/ppq2pp1/2b1pn2/2P4r/2P5/3BQN1P/P4PP1/R4RK1 b - - bm Ng4!; id \"CCC-I No.3\";";
  ASSERT_TRUE(testSuite.readOneEPD(line, test));

  line = "6k1/p3b1np/6pr/6P1/1B2p2Q/K7/7P/8 w - - am Qxh6??; id \"CCC-I No.6\";";
  ASSERT_TRUE(testSuite.readOneEPD(line, test));

  line = "7r/8/pB1p1R2/4k2q/1p6/1Pr5/P5Q1/6K1 w - - bm Bd4+; c0 \"M15\"; id \"CCC-I No.8\";";
  ASSERT_TRUE(testSuite.readOneEPD(line, test));

}

TEST_F(TestSuiteTests, readFile) {

  std::string filePath = FrankyCPP_PROJECT_ROOT;
  filePath += +"/testsets/franky_tests.epd";
  MilliSec moveTime = 5'000;
  Depth depth = static_cast<Depth>(10);

  TestSuite testSuite(filePath, moveTime, depth);
  std::vector<TestSuite::Test> ts;

  testSuite.readTestCases(filePath, ts);

  ASSERT_EQ(13, ts.size());
}

TEST_F(TestSuiteTests, singleTest) {
  std::string filePath{};
  MilliSec moveTime = 2'000;
  Depth depth{0};

  Search search;
  SearchLimits searchLimits;
  searchLimits.setMoveTime(moveTime);
  searchLimits.setDepth(depth);
  TestSuite testSuite(filePath, moveTime, depth);

  TestSuite::Test test = {"CaptureTest", "7k/8/3p4/4N3/8/5p2/P7/1K2N3 w - -", TestSuite::BM,
                          "N5xf3"};
  testSuite.runSingleTest(search, searchLimits, test);
  EXPECT_EQ(TestSuite::SUCCESS, test.result);

  // 6k1/p3b1np/6pr/6P1/1B2p2Q/K7/7P/8 w - - am Qxh6??; id "CCC-I No.6";
  // Currently this test will not avoid the move
  test = {"AvoidMoveTest", "6k1/p3b1np/6pr/6P1/1B2p2Q/K7/7P/8 w - -", TestSuite::AM, "Qxh6"};
  testSuite.runSingleTest(search, searchLimits, test);
  EXPECT_EQ(TestSuite::SUCCESS, test.result);

  // Direct mate
  test = {"Direct Mate #1", "4r1b1/1p4B1/pN2pR2/RB2k3/1P2N2p/2p3b1/n2P1p1r/5K1n w - -", TestSuite::DM, "3"};
  testSuite.runSingleTest(search, searchLimits, test);
  EXPECT_EQ(TestSuite::SUCCESS, test.result);

  test = {"Direct Mate #2", "r3r3/p1p2p1k/3p2pp/2p5/2P2n2/2N2B2/PPR1PP1q/3RQK2 b - -", TestSuite::DM, "4"};
  testSuite.runSingleTest(search, searchLimits, test);
  EXPECT_EQ(TestSuite::SUCCESS, test.result);
}

// 100%
TEST_F(TestSuiteTests, FrankyTestSuite) {
  std::string filePath = FrankyCPP_PROJECT_ROOT;
  filePath += +"/testsets/franky_tests.epd";
  MilliSec moveTime = 1'000;
  Depth depth{0};
  TestSuite testSuite(filePath, moveTime, depth);
  testSuite.runTestSuite();
  EXPECT_EQ(testSuite.getTestResults().counter, testSuite.getTestResults().successCounter);
  EXPECT_EQ(0, testSuite.getTestResults().skippedCounter);
  EXPECT_EQ(0, testSuite.getTestResults().notTestedCounter);
  EXPECT_EQ(0, testSuite.getTestResults().failedCounter);
}
