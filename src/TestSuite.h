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

#ifndef FRANKYCPP_TESTSUITE_H
#define FRANKYCPP_TESTSUITE_H

#include <string>
#include <utility>
#include "Logging.h"
#include "types.h"
#include "Position.h"
#include "Search.h"
#include "SearchLimits.h"
#include "misc.h"

class TestSuite {

public:

  enum TestType {
    NONE,
    DM,
    BM
  };

  enum ResultType {
    NOT_TESTED,
    SKIPPED,
    FAILED,
    SUCCESS
  };

  struct Test {
    Test(std::string id = "", std::string fen = "", TestType type = NONE,
         std::string targetString = "",
         Depth mateDepth = DEPTH_NONE, Move target = MOVE_NONE,
         Move actual = MOVE_NONE, Value value = VALUE_NONE, ResultType result = NOT_TESTED)
      : id(std::move(id)), fen(std::move(fen)), type(type),
        expectedString(std::move(targetString)), mateDepth(mateDepth),
        expected(target), actualMove(actual), actualValue(value), result(result) {}

    std::string id;
    std::string fen;
    TestType type;
    std::string expectedString;
    Depth mateDepth;
    Move expected;
    Move actualMove;
    Value actualValue;
    ResultType result;
  };

private:

  std::shared_ptr<spdlog::logger> LOG = spdlog::get("TSuite_Logger");

  std::string filePath;
  MilliSec searchTime;
  Depth searchDepth;

  std::vector<Test> testCases;

public:

  /** Creates a TestSuite instance with a given path, search time per test and
   * max search depth per test.
   * @param filePath
   * @param searchTime
   * @param depth
   * // TODO: parameter checking in the functions
   */
  TestSuite(const std::string_view &filePath, MilliSec searchTime, Depth depth)
    : filePath(filePath), searchTime(searchTime), searchDepth(depth) {}

  /** runs the tests specified in the given EPD file */
  void runTestSuite();

  /** runs a set of tests and stores results back to the given list */
  void runTestSet(std::vector<Test> &ts) const;

  /** reads aöö tests from the given file into the given list */
  void readTestCases(std::string &filePath, std::vector<Test> &tests) const;

  /** reads on EPD file and creates a Test */
  bool readOneEPD(const std::string &line, TestSuite::Test &test) const;

  /** runs a single test and stores the result back to given test */
  void runSingleTest(Search &search, SearchLimits &searchLimits, Test &t) const;

  /** returns the list of tests */
  const std::vector<Test> &getTestCases() const { return testCases; }

  /** string representation of result type */
  static std::string print(TestSuite::ResultType resultType);
};


#endif //FRANKYCPP_TESTSUITE_H