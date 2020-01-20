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
    LOGGING::init();
    INIT::init();
    NEWLINE;
  }

  std::shared_ptr<spdlog::logger> LOG = spdlog::get("Test_Logger");


protected:
  void SetUp() override {

    // @formatter:off
    SearchConfig::USE_QUIESCENCE      = true;
    SearchConfig::USE_ALPHABETA       = true;
    SearchConfig::USE_PVS             = true;


    SearchConfig::USE_TT              = true;
    SearchConfig::USE_TT_QSEARCH      = true;

    SearchConfig::USE_KILLER_MOVES    = true;
    SearchConfig::USE_PV_MOVE_SORTING = true;
    SearchConfig::USE_IID             = true;

    SearchConfig::USE_MDP             = true;
    SearchConfig::USE_MPP             = true;
    SearchConfig::USE_QS_STANDPAT_CUT = true;
    SearchConfig::USE_RFP             = true;
    SearchConfig::USE_RAZOR_PRUNING   = true;
    SearchConfig::USE_NMP             = true;

    SearchConfig::USE_EXTENSIONS      = true;

    SearchConfig::TT_SIZE_MB          = 64;
    SearchConfig::MAX_EXTRA_QDEPTH    = static_cast<Depth>(20);
    SearchConfig::NO_KILLER_MOVES     = 2;

    // @formatter:on

  }

  void TearDown() override {}
};


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
}

TEST_F(TestSuiteTests, readFile) {

  fprintln("Project Root is '{}'", FrankyCPP_PROJECT_ROOT );

  std::string filePath = "../../testsets/franky_tests.epd";
  MilliSec moveTime = 5'000;
  Depth depth = static_cast<Depth>(10);

  TestSuite testSuite(filePath, moveTime, depth);
  std::vector<TestSuite::Test> ts;

  testSuite.readTestCases(filePath, ts);

  ASSERT_EQ(12, ts.size());
}


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
    LOG__INFO(LOG, "Test '{}' {}", t.id, TestSuite::print(t.result));
  }
}

TEST_F(TestSuiteTests, singleTest) {
  std::string filePath = "";
  MilliSec moveTime = 2'000;
  Depth depth = static_cast<Depth>(0);

  Search search;
  SearchLimits searchLimits;
  searchLimits.setMoveTime(moveTime);
  searchLimits.setDepth(depth);
  TestSuite testSuite(filePath, moveTime, depth);

  TestSuite::Test test = {"N5xf3", "7k/8/3p4/4N3/8/5p2/P7/1K2N3 w - -", TestSuite::BM, "N5xf3"};

  testSuite.runSingleTest(search, searchLimits, test);
}

// 100 %
TEST_F(TestSuiteTests, FrankyTestSuite) {
  std::string filePath = "../../../testsets/franky_tests.epd";
  MilliSec moveTime = 1'000;
  Depth depth = static_cast<Depth>(0);
  TestSuite testSuite(filePath, moveTime, depth);
  testSuite.runTestSuite();
}

TEST_F(TestSuiteTests, ecm98) {
  std::string filePath = "../../../testsets/ecm98.epd";
  MilliSec moveTime = 3'000;
  Depth depth = static_cast<Depth>(0);
  TestSuite testSuite(filePath, moveTime, depth);
  testSuite.runTestSuite();
}