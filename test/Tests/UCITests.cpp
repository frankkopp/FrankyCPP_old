/*
 * MIT License
 *
 * Copyright (c) 2018 Frank Kopp
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
#include "../../src/globals.h"
#include "../../src/UCIHandler.h"
#include "../../src/Engine.h"

using namespace std;
using testing::Eq;

class UCITest : public ::testing::Test {
public:
  shared_ptr<spdlog::logger> LOG = spdlog::stdout_color_mt("Test_Logger");

  static void SetUpTestSuite() {
    NEWLINE;
    LOGGING::init();
    INIT::init();
    NEWLINE;

    // turn off info and below logging in the application
    spdlog::set_level(spdlog::level::warn);
  }
protected:
  void SetUp() override {
    LOG->set_level(spdlog::level::info);
  }
  void TearDown() override {}
};

TEST_F(UCITest, uciTest) {

  string command = "uci";
  string expectedStart = "id name";
  string expectedEnd = "uciok\n";

  LOG->info("COMMAND: " + command);
  istringstream is(command);
  ostringstream os;
  Engine engine;
  UCI::Handler uciHandler(&engine, &is, &os);
  uciHandler.loop();
  string result = os.str();
  LOG->debug("RESPONSE: \n" + result);

  ASSERT_EQ(expectedStart, result.substr(0, 7));
  ASSERT_EQ(expectedEnd, result.substr(result.size() - 6, result.size()));
}

TEST_F(UCITest, isreadyTest) {
  string command = "isready";
  string expected = "readyok\n";

  LOG->info("COMMAND: " + command);
  istringstream is(command);
  ostringstream os;
  Engine engine;
  UCI::Handler uciHandler(&engine, &is, &os);
  uciHandler.loop();
  LOG->debug("RESPONSE: " + os.str());
  ASSERT_EQ(expected, os.str());
}

TEST_F(UCITest, setoptionTest) {
  ostringstream os;
  Engine engine;

  string command = "setoption name Hash value 2048";
  LOG->info("COMMAND: " + command);
  istringstream is(command);
  UCI::Handler uciHandler(&engine, &is, &os);
  uciHandler.loop();
  ASSERT_EQ("2048", engine.getOption("Hash"));
  ASSERT_EQ(2048, engine.config.hash);

  command = "setoption name Ponder value false";
  LOG->info("COMMAND: " + command);
  is = istringstream(command);
  uciHandler = UCI::Handler(&engine, &is, &os);
  uciHandler.loop();
  ASSERT_EQ("false", engine.getOption("Ponder"));
  ASSERT_FALSE(engine.config.ponder);
}


TEST_F(UCITest, positionTest) {
  ostringstream os;
  Engine engine;

  // normal
  {
    string command = "position startpos moves e2e4 e7e5";
    LOG->info("COMMAND: " + command);
    istringstream is(command);
    UCI::Handler uciHandler(&engine, &is, &os);
    uciHandler.loop();
    ASSERT_EQ("rnbqkbnr/pppp1ppp/8/4p3/4P3/8/PPPP1PPP/RNBQKBNR w KQkq e6 0 2",
              engine.getPosition()->printFen());
  }

  // castling
  {
    string command = "position fen r1bqkb1r/pppp1ppp/2n2n2/1B2p3/4P3/5N2/PPPP1PPP/RNBQK2R w KQkq - 0 0 moves e1g1";
    LOG->info("COMMAND: " + command);
    istringstream is(command);
    UCI::Handler uciHandler(&engine, &is, &os);
    uciHandler.loop();
    ASSERT_EQ("r1bqkb1r/pppp1ppp/2n2n2/1B2p3/4P3/5N2/PPPP1PPP/RNBQ1RK1 b kq - 1 1",
              engine.getPosition()->printFen());
  }

  // promotion
  {
    string command = "position fen 8/3P4/6K1/8/8/1k6/8/8 w - - 0 0 moves d7d8q";
    LOG->info("COMMAND: " + command);
    istringstream is(command);
    UCI::Handler uciHandler(&engine, &is, &os);
    uciHandler.loop();
    ASSERT_EQ("3Q4/8/6K1/8/8/1k6/8/8 b - - 0 1",
              engine.getPosition()->printFen());
  }

  // normal
  {
    string command = "position moves e2e4 e7e5";
    LOG->info("COMMAND: " + command);
    istringstream is(command);
    UCI::Handler uciHandler(&engine, &is, &os);
    uciHandler.loop();
    ASSERT_EQ("rnbqkbnr/pppp1ppp/8/4p3/4P3/8/PPPP1PPP/RNBQKBNR w KQkq e6 0 2",
              engine.getPosition()->printFen());
  }

  // normal
  {
    string command = "position fen rnbqkbnr/8/8/8/8/8/8/RNBQKBNR w KQkq - 0 1 moves e1e2 e8e7";
    LOG->info("COMMAND: " + command);
    istringstream is(command);
    UCI::Handler uciHandler(&engine, &is, &os);
    uciHandler.loop();
    ASSERT_EQ("rnbq1bnr/4k3/8/8/8/8/4K3/RNBQ1BNR w - - 2 2",
              engine.getPosition()->printFen());
  }

  // normal
  {
    string command = "position fen 7K/8/5pPk/6pP/1p1p2P1/1p1p4/1P1P4/8 w - - 0 12 moves g6g7";
    LOG->info("COMMAND: " + command);
    istringstream is(command);
    UCI::Handler uciHandler(&engine, &is, &os);
    uciHandler.loop();
    ASSERT_EQ("7K/6P1/5p1k/6pP/1p1p2P1/1p1p4/1P1P4/8 b - - 0 12",
              engine.getPosition()->printFen());
  }

  // normal
  {
    string command = "position fen 7K/6P1/5p1k/6pP/1p1p2P1/1p1p4/1P1P4/8 b - - 0 12 moves f6f5";
    LOG->info("COMMAND: " + command);
    istringstream is(command);
    UCI::Handler uciHandler(&engine, &is, &os);
    uciHandler.loop();
    ASSERT_EQ("7K/6P1/7k/5ppP/1p1p2P1/1p1p4/1P1P4/8 w - - 0 13",
              engine.getPosition()->printFen());
  }
}

TEST_F(UCITest, searchLimitsTest) {

  ostringstream os;
  Engine engine;

  { // perft
    string command = "go perft depth 4";
    LOG->info("COMMAND: " + command);
    istringstream is(command);
    UCI::Handler uciHandler(&engine, &is, &os);
    uciHandler.loop();
    SearchLimits searchLimits = engine.getSearchLimits();
    engine.stopSearch();
    engine.waitWhileSearching();
    ASSERT_TRUE(searchLimits.perft);
    ASSERT_FALSE(searchLimits.infinite);
    ASSERT_FALSE(searchLimits.ponder);
    ASSERT_FALSE(searchLimits.timeControl);
    ASSERT_EQ(4, searchLimits.maxDepth);
  }

  { // infinite
    string command = "go infinite";
    LOG->info("COMMAND: " + command);
    istringstream is(command);
    UCI::Handler uciHandler(&engine, &is, &os);
    uciHandler.loop();
    SearchLimits searchLimits = engine.getSearchLimits();
    engine.stopSearch();
    engine.waitWhileSearching();
    ASSERT_FALSE(searchLimits.perft);
    ASSERT_TRUE(searchLimits.infinite);
    ASSERT_FALSE(searchLimits.ponder);
    ASSERT_FALSE(searchLimits.timeControl);
    ASSERT_EQ(MAX_PLY, searchLimits.maxDepth);
  }

  { // ponder
    string command = "go ponder";
    LOG->info("COMMAND: " + command);
    istringstream is(command);
    UCI::Handler uciHandler(&engine, &is, &os);
    uciHandler.loop();
    SearchLimits searchLimits = engine.getSearchLimits();
    engine.stopSearch();
    engine.waitWhileSearching();
    ASSERT_FALSE(searchLimits.perft);
    ASSERT_FALSE(searchLimits.infinite);
    ASSERT_TRUE(searchLimits.ponder);
    ASSERT_FALSE(searchLimits.timeControl);
    ASSERT_EQ(MAX_PLY, searchLimits.maxDepth);
  }

  { // mate infinite
    string command = "go mate 4";
    LOG->info("COMMAND: " + command);
    istringstream is(command);
    UCI::Handler uciHandler(&engine, &is, &os);
    uciHandler.loop();
    SearchLimits searchLimits = engine.getSearchLimits();
    engine.stopSearch();
    engine.waitWhileSearching();
    ASSERT_FALSE(searchLimits.perft);
    ASSERT_FALSE(searchLimits.infinite);
    ASSERT_FALSE(searchLimits.ponder);
    ASSERT_FALSE(searchLimits.timeControl);
    ASSERT_EQ(4, searchLimits.mate);
    ASSERT_EQ(MAX_PLY, searchLimits.maxDepth);
  }

  { // mate depth limited
    string command = "go mate 4 depth 4";
    LOG->info("COMMAND: " + command);
    istringstream is(command);
    UCI::Handler uciHandler(&engine, &is, &os);
    uciHandler.loop();
    SearchLimits searchLimits = engine.getSearchLimits();
    engine.stopSearch();
    engine.waitWhileSearching();
    ASSERT_FALSE(searchLimits.perft);
    ASSERT_FALSE(searchLimits.infinite);
    ASSERT_FALSE(searchLimits.ponder);
    ASSERT_FALSE(searchLimits.timeControl);
    ASSERT_EQ(4, searchLimits.mate);
    ASSERT_EQ(4, searchLimits.maxDepth);
  }

  { // mate time limited
    string command = "go mate 4 movetime 15";
    LOG->info("COMMAND: " + command);
    istringstream is(command);
    UCI::Handler uciHandler(&engine, &is, &os);
    uciHandler.loop();
    SearchLimits searchLimits = engine.getSearchLimits();
    engine.stopSearch();
    engine.waitWhileSearching();
    ASSERT_FALSE(searchLimits.perft);
    ASSERT_FALSE(searchLimits.infinite);
    ASSERT_FALSE(searchLimits.ponder);
    ASSERT_TRUE(searchLimits.timeControl);
    ASSERT_EQ(4, searchLimits.mate);
    ASSERT_EQ(MAX_PLY, searchLimits.maxDepth);
    ASSERT_EQ(15, searchLimits.moveTime);
  }

  { // mate depth and time limited
    string command = "go mate 4 depth 4 movetime 15";
    LOG->info("COMMAND: " + command);
    istringstream is(command);
    UCI::Handler uciHandler(&engine, &is, &os);
    uciHandler.loop();
    SearchLimits searchLimits = engine.getSearchLimits();
    engine.stopSearch();
    engine.waitWhileSearching();
    ASSERT_FALSE(searchLimits.perft);
    ASSERT_FALSE(searchLimits.infinite);
    ASSERT_FALSE(searchLimits.ponder);
    ASSERT_TRUE(searchLimits.timeControl);
    ASSERT_EQ(4, searchLimits.mate);
    ASSERT_EQ(4, searchLimits.maxDepth);
    ASSERT_EQ(15, searchLimits.moveTime);
  }

  { // normal game with time for each player
    string command = "go wtime 500001 btime 500002";
    LOG->info("COMMAND: " + command);
    istringstream is(command);
    UCI::Handler uciHandler(&engine, &is, &os);
    uciHandler.loop();
    SearchLimits searchLimits = engine.getSearchLimits();
    engine.stopSearch();
    engine.waitWhileSearching();
    ASSERT_FALSE(searchLimits.perft);
    ASSERT_FALSE(searchLimits.infinite);
    ASSERT_FALSE(searchLimits.ponder);
    ASSERT_TRUE(searchLimits.timeControl);
    ASSERT_EQ(MAX_PLY, searchLimits.maxDepth);
    ASSERT_EQ(500'001, searchLimits.whiteTime);
    ASSERT_EQ(500'002, searchLimits.blackTime);
  }

  { // normal game with time for each player and remaining moves until time control
    string command = "go wtime 300001 btime 300002 movestogo 20";
    LOG->info("COMMAND: " + command);
    istringstream is(command);
    UCI::Handler uciHandler(&engine, &is, &os);
    uciHandler.loop();
    SearchLimits searchLimits = engine.getSearchLimits();
    engine.stopSearch();
    engine.waitWhileSearching();
    ASSERT_FALSE(searchLimits.perft);
    ASSERT_FALSE(searchLimits.infinite);
    ASSERT_FALSE(searchLimits.ponder);
    ASSERT_TRUE(searchLimits.timeControl);
    ASSERT_EQ(MAX_PLY, searchLimits.maxDepth);
    ASSERT_EQ(300'001, searchLimits.whiteTime);
    ASSERT_EQ(300'002, searchLimits.blackTime);
    ASSERT_EQ(20, searchLimits.movesToGo);
  }

  { // normal game with time for each player and increases per move
    string command = "go wtime 300001 btime 300002 winc 2001 binc 2002";
    LOG->info("COMMAND: " + command);
    istringstream is(command);
    UCI::Handler uciHandler(&engine, &is, &os);
    uciHandler.loop();
    SearchLimits searchLimits = engine.getSearchLimits();
    engine.stopSearch();
    engine.waitWhileSearching();
    ASSERT_FALSE(searchLimits.perft);
    ASSERT_FALSE(searchLimits.infinite);
    ASSERT_FALSE(searchLimits.ponder);
    ASSERT_TRUE(searchLimits.timeControl);
    ASSERT_EQ(MAX_PLY, searchLimits.maxDepth);
    ASSERT_EQ(300'001, searchLimits.whiteTime);
    ASSERT_EQ(300'002, searchLimits.blackTime);
    ASSERT_EQ(2001, searchLimits.whiteInc);
    ASSERT_EQ(2002, searchLimits.blackInc);
  }

  { // move time limited
    string command = "go movetime 15";
    LOG->info("COMMAND: " + command);
    istringstream is(command);
    UCI::Handler uciHandler(&engine, &is, &os);
    uciHandler.loop();
    SearchLimits searchLimits = engine.getSearchLimits();
    engine.stopSearch();
    engine.waitWhileSearching();
    ASSERT_FALSE(searchLimits.perft);
    ASSERT_FALSE(searchLimits.infinite);
    ASSERT_FALSE(searchLimits.ponder);
    ASSERT_TRUE(searchLimits.timeControl);
    ASSERT_EQ(0, searchLimits.mate);
    ASSERT_EQ(MAX_PLY, searchLimits.maxDepth);
    ASSERT_EQ(15, searchLimits.moveTime);
  }

  { // depth only limited
    string command = "go depth 5";
    LOG->info("COMMAND: " + command);
    istringstream is(command);
    UCI::Handler uciHandler(&engine, &is, &os);
    uciHandler.loop();
    SearchLimits searchLimits = engine.getSearchLimits();
    engine.stopSearch();
    engine.waitWhileSearching();
    ASSERT_FALSE(searchLimits.perft);
    ASSERT_FALSE(searchLimits.infinite);
    ASSERT_FALSE(searchLimits.ponder);
    ASSERT_FALSE(searchLimits.timeControl);
    ASSERT_EQ(1, searchLimits.startDepth);
    ASSERT_EQ(5, searchLimits.maxDepth);
    ASSERT_EQ(0, searchLimits.nodes);
  }

  { // nodes only limited
    string command = "go nodes 1000000";
    LOG->info("COMMAND: " + command);
    istringstream is(command);
    UCI::Handler uciHandler(&engine, &is, &os);
    uciHandler.loop();
    SearchLimits searchLimits = engine.getSearchLimits();
    engine.stopSearch();
    engine.waitWhileSearching();
    ASSERT_FALSE(searchLimits.perft);
    ASSERT_FALSE(searchLimits.infinite);
    ASSERT_FALSE(searchLimits.ponder);
    ASSERT_FALSE(searchLimits.timeControl);
    ASSERT_EQ(1, searchLimits.startDepth);
    ASSERT_EQ(MAX_PLY, searchLimits.maxDepth);
    ASSERT_EQ(1'000'000, searchLimits.nodes);
  }

  { // nodes and depth limited
    string command = "go nodes 1000000 depth 5";
    LOG->info("COMMAND: " + command);
    istringstream is(command);
    UCI::Handler uciHandler(&engine, &is, &os);
    uciHandler.loop();
    SearchLimits searchLimits = engine.getSearchLimits();
    engine.stopSearch();
    engine.waitWhileSearching();
    ASSERT_FALSE(searchLimits.perft);
    ASSERT_FALSE(searchLimits.infinite);
    ASSERT_FALSE(searchLimits.ponder);
    ASSERT_FALSE(searchLimits.timeControl);
    ASSERT_EQ(1, searchLimits.startDepth);
    ASSERT_EQ(5, searchLimits.maxDepth);
    ASSERT_EQ(1'000'000, searchLimits.nodes);
  }

  { // move time limited with a list of moves to search
    string command = "go movetime 15 searchmoves d2d4 e2e4";
    LOG->info("COMMAND: " + command);
    istringstream is(command);
    UCI::Handler uciHandler(&engine, &is, &os);
    uciHandler.loop();
    SearchLimits searchLimits = engine.getSearchLimits();
    engine.stopSearch();
    engine.waitWhileSearching();
    ASSERT_FALSE(searchLimits.perft);
    ASSERT_FALSE(searchLimits.infinite);
    ASSERT_FALSE(searchLimits.ponder);
    ASSERT_TRUE(searchLimits.timeControl);
    ASSERT_EQ(0, searchLimits.mate);
    ASSERT_EQ(MAX_PLY, searchLimits.maxDepth);
    ASSERT_EQ(15, searchLimits.moveTime);
    ASSERT_EQ(createMove("d2d4"), searchLimits.moves.front());
    ASSERT_EQ(createMove("e2e4"), searchLimits.moves.back());
  }
}

TEST_F(UCITest, moveTest) {
  ostringstream os;
  Engine engine;

  string command = "position startpos moves e2e4";
  LOG->info("COMMAND: " + command);
  istringstream is(command);
  UCI::Handler uciHandler(&engine, &is, &os);
  uciHandler.loop();

  command = "go wtime 60000 btime 60000 winc 0 binc 0 movestogo 40";
  LOG->info("COMMAND: " + command);
  is = istringstream(command);
  uciHandler.loop(&is);

  sleep(5);
  engine.stopSearch();
  LOG->debug("Waiting until search ends...");
  engine.waitWhileSearching();
  LOG->debug("SEARCH ENDED");

}

TEST_F(UCITest, moveTestDepth) {
  ostringstream os;
  Engine engine;

  string command = "position startpos moves e2e4";
  LOG->info("COMMAND: " + command);
  istringstream is(command);
  UCI::Handler uciHandler(&engine, &is, &os);
  uciHandler.loop();

  command = "go depth 5";
  LOG->info("COMMAND: " + command);
  is = istringstream(command);
  uciHandler.loop(&is);

  LOG->debug("Waiting until search ends...");
  engine.waitWhileSearching();
  LOG->debug("SEARCH ENDED");

}