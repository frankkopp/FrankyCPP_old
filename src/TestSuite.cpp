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
#include <fstream>
#include <regex>
#include "Logging.h"
#include "TestSuite.h"
#include "Position.h"
#include "Search.h"
#include "misc.h"

#include <boost/algorithm/string.hpp>
#include <boost/timer/timer.hpp>
using namespace boost::timer;

void TestSuite::runTestSuite() {

  cpu_timer timer;

  testCases.clear();

  fprintln("Running Test Suite");
  fprintln("==================================================================");
  fprintln("EPD File:   {}", filePath);
  fprintln("SearchTime: {}", searchTime);
  fprintln("MaxDepth:   {}", searchDepth);
  fprintln("");

  // read EPD file
  fprintln("Reading EPD File: ...");
  readTestCases(filePath, testCases);
  fprintln("                  ... DONE");
  fprintln("");

  fprintln("Running {} tests ...", testCases.size());
  // runtTestSet
  runTestSet(testCases);
  fprintln("All {} tests DONE", testCases.size());
  fprintln("");

  fprintln("Results for Test Suite {}", filePath);
  fprintln("=================================================================="
           "==================================================================");
  fprintln(" {:<4s} | {:<10s} | {:<8s} | {:<8s} | {:<15s} | {:s} | {:s}",
           " Nr.", "Result", "Move", "Value", "Expected Result", "Fen", "ID");
  fprintln("=================================================================="
           "==================================================================");

  int counter = 0;
  int successCounter = 0;
  int failedCounter = 0;
  int skippedCounter = 0;
  int notTestedCounter = 0;

  for (const Test &t : testCases) {
    switch (t.result) {
      case NOT_TESTED:
        notTestedCounter++;
        break;
      case SKIPPED:
        skippedCounter++;
        break;
      case FAILED:
        failedCounter++;
        break;
      case SUCCESS:
        successCounter++;
        break;
    }
    fprintln(" {:<4d} | {:<10s} | {:<8s} | {:<8s} | {:<15s} | {:s} | {:s}",
             ++counter, print(t.result), printMove(t.actualMove),
             printValue(t.actualValue),
             (t.type == DM ? "dm " : t.type == BM ? "bm " : "-") + t.expectedString,
             t.fen, t.id);
  }

  fprintln("=================================================================="
           "==================================================================");
  timer.stop();
  fprintln("{}", timer.format());
  fprintln("Successful: {:3n} ({:d} %)", successCounter, 100 * successCounter / testCases.size());
  fprintln("Failed:     {:3n} ({:d} %)", failedCounter, 100 * failedCounter / testCases.size());
  fprintln("Skipped:    {:3n} ({:d} %)", skippedCounter, 100 * skippedCounter / testCases.size());
  fprintln("Not tested: {:3n} ({:d} %)", notTestedCounter, 100 * notTestedCounter / testCases.size());
  fprintln("");

}

void TestSuite::runTestSet(std::vector<Test> &ts) const {
  Search search;
  SearchLimits searchLimits;
  searchLimits.setMoveTime(searchTime);
  searchLimits.setDepth(searchDepth);

  for (Test &t : ts) {
    LOG__INFO(Logger::get().TSUITE_LOG, "Running Test {} {}", t.id, t.fen);
    runSingleTest(search, searchLimits, t);
  }

}

void
TestSuite::runSingleTest(Search &search, SearchLimits &searchLimits, TestSuite::Test &t) const {

  LOG__INFO(Logger::get().TSUITE_LOG, "Testing TestSet: ID \"{}\"", t.id);

  // clear search
  search.clearHash();

  // reset mate depth
  searchLimits.setMate(0);

  const Position position(t.fen);
  switch (t.type) {

    // direct mate test
    case DM: {
      // get target mate depth
      const auto mateIn = static_cast<Depth>(std::stoi(t.expectedString));
      t.mateDepth = mateIn;
      searchLimits.setMate(t.mateDepth);

      // start search
      search.startSearch(position, searchLimits);
      search.waitWhileSearching();

      // check and store result
      if ("mate " + t.expectedString == printValue(search.getLastSearchResult().bestMoveValue)) {
        LOG__INFO(Logger::get().TSUITE_LOG, "TestSet: ID \"{}\" SUCCESS", t.id);
        t.actualMove = search.getLastSearchResult().bestMove;
        t.actualValue = search.getLastSearchResult().bestMoveValue;
        t.result = SUCCESS;
        return;
      }
      else {
        LOG__INFO(Logger::get().TSUITE_LOG, "TestSet: ID \"{}\" FAILED", t.id);
        t.actualMove = search.getLastSearchResult().bestMove;
        t.actualValue = search.getLastSearchResult().bestMoveValue;
        t.result = SUCCESS;
        return;
      }
    }

      // best move test
    case BM: {
      // get best move
      // EPD allows for multiple best moves
      MoveList moves = getResultMoveList(t);

      if (moves.empty()) {
        LOG__WARN(Logger::get().TSUITE_LOG, "Skipping test {} as expected result {} could not be read", t.id, t.expectedString);
        t.result = SKIPPED;
        return;
      }

      // do the search
      search.startSearch(position, searchLimits);
      search.waitWhileSearching();

      // get the result
      const Move actual = moveOf(search.getLastSearchResult().bestMove);

      // check against expected moves
      for (Move m : moves) {
        if (m == actual) {
          LOG__INFO(Logger::get().TSUITE_LOG, "TestSet: ID \"{}\" SUCCESS", t.id);
          t.actualMove = search.getLastSearchResult().bestMove;
          t.actualValue = search.getLastSearchResult().bestMoveValue;
          t.result = SUCCESS;
          return;
        }
        else {
          continue;
        }
      }
      LOG__INFO(Logger::get().TSUITE_LOG, "TestSet: ID \"{}\" FAILED", t.id);
      t.actualMove = search.getLastSearchResult().bestMove;
      t.actualValue = search.getLastSearchResult().bestMoveValue;
      t.result = FAILED;
      return;
    }

    // Avoid move test
    case AM: {
      // get moves to avoid
      // EPD allows for multiple moves
      MoveList moves = getResultMoveList(t);

      if (moves.empty()) {
        LOG__WARN(Logger::get().TSUITE_LOG, "Skipping test {} as expected result {} could not be read", t.id, t.expectedString);
        t.result = SKIPPED;
        return;
      }

      // do the search
      search.startSearch(position, searchLimits);
      search.waitWhileSearching();

      // get the result
      const Move actual = moveOf(search.getLastSearchResult().bestMove);

      // check against expected moves to avoid
      for (Move m : moves) {
        if (m == actual) {
          LOG__INFO(Logger::get().TSUITE_LOG, "TestSet: ID \"{}\" FAILED", t.id);
          t.actualMove = search.getLastSearchResult().bestMove;
          t.actualValue = search.getLastSearchResult().bestMoveValue;
          t.result = FAILED;
          return;
        }
        else {
          continue;
        }
      }
      LOG__INFO(Logger::get().TSUITE_LOG, "TestSet: ID \"{}\" SUCCESS", t.id);
      t.actualMove = search.getLastSearchResult().bestMove;
      t.actualValue = search.getLastSearchResult().bestMoveValue;
      t.result = SUCCESS;
      return;
    }

    case NONE:
    default:
      LOG__WARN(Logger::get().TSUITE_LOG, "Test has invalid type.");
      t.result = SKIPPED;
      return;
  }
}

MoveList TestSuite::getResultMoveList(const TestSuite::Test &t) const {
  MoveList moves;
  std::regex splitPattern("\\s+");
  std::sregex_token_iterator iter(t.expectedString.begin(), t.expectedString.end(), splitPattern, -1);
  std::sregex_token_iterator end;
  while (iter != end) {
    Move move = Misc::getMoveFromSAN(Position(t.fen), iter->str());
    if (move) moves.push_back(move);
    ++iter;
  }
  return moves;
}

void TestSuite::readTestCases(const std::string &filePathStr, std::vector<Test> &tests) const {
  std::ifstream file(filePathStr);
  if (file.is_open()) {
    std::string line;
    while (getline(file, line)) {
      Test test;
      if (readOneEPD(line, test)) {
        tests.push_back(test);
      }
    }
    file.close();
  }
  else {
    LOG__ERROR(Logger::get().TSUITE_LOG, "Could not open file: {}", filePath);
    return;
  }
}

bool TestSuite::readOneEPD(std::string &line, TestSuite::Test &test) const {

  LOG__DEBUG(Logger::get().TSUITE_LOG, "EPD: {}", line);

  // skip empty lines and comments
  cleanUpLine(line);
  if (line.empty()) {
    return false;
  }

  // Find a EPD line
  std::regex regexPattern(R"(^\s*(.*) (bm|dm|am) (.*?);(.* id \"(.*?)\";)?.*$)");
  std::smatch matcher;
  if (!std::regex_match(line, matcher, regexPattern)) {
    LOG__WARN(Logger::get().TSUITE_LOG, "No EPD match found in {}", line);
    return false;
  }

  // get the parts
  std::string fen = matcher.str(1);
  std::string type = matcher.str(2);
  std::string result = matcher.str(3);
  std::string id = matcher.str(5).empty() ? "no ID" : matcher.str(5);
  LOG__DEBUG(Logger::get().TSUITE_LOG, "Fen: {}    Type: {}    Result: {}    ID: {}", fen, type, result, id);

  // get test type
  TestType testType;
  if (type == "dm") {
    testType = DM;
  }
  else if (type == "bm") {
    testType = BM;
  }
  else if (type == "am") {
    testType = AM;
  }
  else {
    LOG__WARN(Logger::get().TSUITE_LOG, "Invalid TestType {}", type);
    return false;
  }

  // Cleanup Result
  if (testType == BM || testType == AM) {
    boost::replace_all(result,"!", "");
    boost::replace_all(result,"?", "");
  }

  // store
  test = {id, fen, testType, result};

  return true;
}

std::string &TestSuite::cleanUpLine(std::string &line) {
  //  fprintln("{}", line);
  std::regex whiteSpaceTrim(R"(^\s*(.*)\s*$)");
  line = std::regex_replace(line, whiteSpaceTrim, "$1");
  //  fprintln("{}", line);
  std::regex leadCommentTrim(R"(^\s*#.*$)");
  line = std::regex_replace(line, leadCommentTrim, "");
  //  fprintln("{}", line);
  std::regex trailCommentTrim(R"(^(.*)#([^;]*)$)");
  line = std::regex_replace(line, trailCommentTrim, "$1;");
  //  fprintln("{}", line);
  return line;
}

std::string TestSuite::print(TestSuite::ResultType resultType) {
  switch (resultType) {
    case TestSuite::NOT_TESTED:
      return "NOT TESTED";
    case TestSuite::SKIPPED:
      return "SKIPPED";
    case TestSuite::FAILED:
      return "FAILED";
    case TestSuite::SUCCESS:
      return "SUCCESS";
  }
  return "";
}


