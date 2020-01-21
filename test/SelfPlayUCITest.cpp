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

#include <SearchConfig.h>
#include <gtest/gtest.h>
#include <random>

#include "Engine.h"
#include "Logging.h"
#include "UCIHandler.h"
#include "types.h"

using testing::Eq;

class UCISelfPlayUCITest : public ::testing::Test {
public:
  static void SetUpTestSuite() {
    NEWLINE;
    LOGGING::init();
    INIT::init();
    NEWLINE;
    // turn off info and below logging in the application
    spdlog::set_level(spdlog::level::trace);
  }

  std::shared_ptr<spdlog::logger> LOG = spdlog::get("Test_Logger");

protected:
  void SetUp() override { LOG->set_level(spdlog::level::debug); }

  void TearDown() override {}

  std::string sendCommand(Engine &engine, const std::string &command) {
    LOG__INFO(LOG, "COMMAND: " + command);
    std::istringstream is(command);
    std::ostringstream os = std::ostringstream();
    UCI::UCI_Handler uciHandler(&engine, &is, &os);
    uciHandler.loop();
    std::string response = os.str();
    return response;
  }

  void stopSearch(Engine &engine) {
    engine.stopSearch();
    engine.waitWhileSearching();
  }

  void expect(std::string test, std::string str) {
    LOG__DEBUG(LOG, "{}", str);
    ASSERT_EQ(test, str.substr(0, test.length()));
  }
};

TEST_F(UCISelfPlayUCITest, uciTest) {
  auto UCI_LOG = spdlog::get("UCI_Logger");
  UCI_LOG->set_level(spdlog::level::warn);
  auto UCIHANDLER_LOG = spdlog::get("UCIHandler_Logger");
  UCIHANDLER_LOG->set_level(spdlog::level::warn);

  std::mt19937_64 rg(12345);
  std::uniform_int_distribution<MilliSec> moveTime(200, 1000);

  Engine engine;
  Position position;

  SearchConfig::USE_TT = true;
  SearchConfig::USE_TT_QSEARCH = true;
  SearchConfig::USE_PV_MOVE_SORTING = true;

  expect("id name FrankyCPP", sendCommand(engine, "uci"));
  expect("readyok", sendCommand(engine, "isready"));

  do {
    sendCommand(engine, "position fen " + position.printFen());
    // sendCommand(engine, "go depth 4");
    sendCommand(engine, "go movetime " + std::to_string(moveTime(rg)));
    engine.waitWhileSearching();
    Move move = engine.getLastResult().bestMove;
    if (!move)
      break;
    LOG__INFO(LOG, "UCI NEXT MOVE: {} on position {} (key={})",
              printMoveVerbose(move), position.printFen(),
              position.getZobristKey());
    position.doMove(move);
  } while (1);

  engine.waitWhileSearching();
}
