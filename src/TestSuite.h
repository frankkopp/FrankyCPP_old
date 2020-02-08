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
#include <vector>
#include "types.h"

// forward declaration
class Search;
class SearchLimits;

class TestSuite {

public:

  enum TestType {
    NONE,
    DM,
    BM,
    AM
  };

  enum ResultType {
    NOT_TESTED,
    SKIPPED,
    FAILED,
    SUCCESS
  };

  struct Test {
    Test(std::string _id = "",
         std::string _fen = "",
         TestType _type = NONE,
         std::string _targetString = "",
         Depth _mateDepth = DEPTH_NONE,
         Move _target = MOVE_NONE,
         Move _actual = MOVE_NONE,
         Value _value = VALUE_NONE,
         ResultType _result = NOT_TESTED)
      : id(std::move(_id)), fen(std::move(_fen)), type(_type),
        expectedString(std::move(_targetString)), mateDepth(_mateDepth),
        expected(_target), actualMove(_actual), actualValue(_value), result(_result) {}

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

  //std::shared_ptr<spdlog::logger> LOG = spdlog::get("TSuite_Logger");

  std::string filePath;
  MilliSec searchTime;
  Depth searchDepth;

  std::vector<Test> testCases;

public:

  /** Creates a TestSuite instance with a given path, search time per test and
   * max search depth per test.
   * @param _filePath
   * @param _searchTime
   * @param _depth
   * // TODO: parameter checking in the functions
   */
  TestSuite(const std::string_view &_filePath, MilliSec _searchTime, Depth _depth)
    : filePath(_filePath), searchTime(_searchTime), searchDepth(_depth) {}

  /** runs the tests specified in the given EPD file */
  void runTestSuite();

  /** runs a set of tests and stores results back to the given list */
  void runTestSet(std::vector<Test> &ts) const;

  /** reads aöö tests from the given file into the given list */
  void readTestCases(const std::string &filePath, std::vector<Test> &tests) const;

  /** reads on EPD file and creates a Test */
  bool readOneEPD(std::string &line, TestSuite::Test &test) const;

  /** removes leading and trailing whitespace and comments */
  static std::string &cleanUpLine(std::string &line) ;

  /** runs a single test and stores the result back to given test */
  void runSingleTest(Search &search, SearchLimits &searchLimits, Test &t) const;

  /** returns the list of tests */
  const std::vector<Test> &getTestCases() const { return testCases; }
  
  /** string representation of result type */
  static std::string print(TestSuite::ResultType resultType);
  MoveList getResultMoveList(const Test &t) const;
};


#endif //FRANKYCPP_TESTSUITE_H
