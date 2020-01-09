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

#include "Logging.h"
#include "types.h"
#include "UCIHandler.h"
#include "Engine.h"

using namespace std;
using testing::Eq;

class UCITest : public ::testing::Test {
public:
  static void SetUpTestSuite() {
    NEWLINE;
    LOGGING::init();
    INIT::init();
    NEWLINE;
    // turn off info and below logging in the application
    spdlog::set_level(spdlog::level::trace);
    auto UCI_LOG = spdlog::get("UCI_Logger");
    UCI_LOG->set_level(spdlog::level::debug);
  }

  shared_ptr<spdlog::logger> LOG = spdlog::get("Test_Logger");
protected:
  void SetUp() override {
    LOG->set_level(spdlog::level::debug);
  }

  void TearDown() override {}
};

TEST_F(UCITest, uciTest) {

  string command = "uci";
  string expectedStart = "id name";
  string expectedEnd = "uciok\n";

  LOG__INFO(LOG, "COMMAND: " + command);
  istringstream is(command);
  ostringstream os;
  Engine engine;
  UCI::Handler uciHandler(&engine, &is, &os);
  uciHandler.loop();
  string result = os.str();
  LOG__DEBUG(LOG, "RESPONSE: \n" + result);

  ASSERT_EQ(expectedStart, result.substr(0, 7));
  ASSERT_EQ(expectedEnd, result.substr(result.size() - 6, result.size()));
}

TEST_F(UCITest, isreadyTest) {
  string command = "isready";
  string expected = "readyok\n";

  LOG__INFO(LOG, "COMMAND: " + command);
  istringstream is(command);
  ostringstream os;
  Engine engine;
  UCI::Handler uciHandler(&engine, &is, &os);
  uciHandler.loop();
  LOG__DEBUG(LOG, "RESPONSE: " + os.str());
  ASSERT_EQ(expected, os.str());
}

TEST_F(UCITest, setoptionTest) {
  ostringstream os;
  Engine engine;

  string command = "setoption name Hash value 2048";
  LOG__INFO(LOG, "COMMAND: " + command);
  istringstream is(command);
  UCI::Handler uciHandler(&engine, &is, &os);
  uciHandler.loop();
  //  ASSERT_EQ("2048", engine.getOption("Hash"));
  //  ASSERT_EQ(2048, engine.config.hash);

  command = "setoption name Ponder value false";
  LOG__INFO(LOG, "COMMAND: " + command);
  is = istringstream(command);
  uciHandler = UCI::Handler(&engine, &is, &os);
  uciHandler.loop();
  ASSERT_EQ("false", engine.getOption("Ponder"));
  ASSERT_FALSE(EngineConfig::ponder);
}


TEST_F(UCITest, positionTest) {
  ostringstream os;
  Engine engine;

  // normal
  {
    string command = "position startpos moves e2e4 e7e5";
    LOG__INFO(LOG, "COMMAND: " + command);
    istringstream is(command);
    UCI::Handler uciHandler(&engine, &is, &os);
    uciHandler.loop();
    ASSERT_EQ("rnbqkbnr/pppp1ppp/8/4p3/4P3/8/PPPP1PPP/RNBQKBNR w KQkq e6 0 2",
              engine.getPosition()->printFen());
  }

  // castling
  {
    string command = "position fen r1bqkb1r/pppp1ppp/2n2n2/1B2p3/4P3/5N2/PPPP1PPP/RNBQK2R w KQkq - 0 0 moves e1g1";
    LOG__INFO(LOG, "COMMAND: " + command);
    istringstream is(command);
    UCI::Handler uciHandler(&engine, &is, &os);
    uciHandler.loop();
    ASSERT_EQ("r1bqkb1r/pppp1ppp/2n2n2/1B2p3/4P3/5N2/PPPP1PPP/RNBQ1RK1 b kq - 1 1",
              engine.getPosition()->printFen());
  }

  // promotion
  {
    string command = "position fen 8/3P4/6K1/8/8/1k6/8/8 w - - 0 0 moves d7d8q";
    LOG__INFO(LOG, "COMMAND: " + command);
    istringstream is(command);
    UCI::Handler uciHandler(&engine, &is, &os);
    uciHandler.loop();
    ASSERT_EQ("3Q4/8/6K1/8/8/1k6/8/8 b - - 0 1",
              engine.getPosition()->printFen());
  }

  // normal
  {
    string command = "position moves e2e4 e7e5";
    LOG__INFO(LOG, "COMMAND: " + command);
    istringstream is(command);
    UCI::Handler uciHandler(&engine, &is, &os);
    uciHandler.loop();
    ASSERT_EQ("rnbqkbnr/pppp1ppp/8/4p3/4P3/8/PPPP1PPP/RNBQKBNR w KQkq e6 0 2",
              engine.getPosition()->printFen());
  }

  // normal
  {
    string command = "position fen rnbqkbnr/8/8/8/8/8/8/RNBQKBNR w KQkq - 0 1 moves e1e2 e8e7";
    LOG__INFO(LOG, "COMMAND: " + command);
    istringstream is(command);
    UCI::Handler uciHandler(&engine, &is, &os);
    uciHandler.loop();
    ASSERT_EQ("rnbq1bnr/4k3/8/8/8/8/4K3/RNBQ1BNR w - - 2 2",
              engine.getPosition()->printFen());
  }

  // normal
  {
    string command = "position fen 7K/8/5pPk/6pP/1p1p2P1/1p1p4/1P1P4/8 w - - 0 12 moves g6g7";
    LOG__INFO(LOG, "COMMAND: " + command);
    istringstream is(command);
    UCI::Handler uciHandler(&engine, &is, &os);
    uciHandler.loop();
    ASSERT_EQ("7K/6P1/5p1k/6pP/1p1p2P1/1p1p4/1P1P4/8 b - - 0 12",
              engine.getPosition()->printFen());
  }

  // normal
  {
    string command = "position fen 7K/6P1/5p1k/6pP/1p1p2P1/1p1p4/1P1P4/8 b - - 0 12 moves f6f5";
    LOG__INFO(LOG, "COMMAND: " + command);
    istringstream is(command);
    UCI::Handler uciHandler(&engine, &is, &os);
    uciHandler.loop();
    ASSERT_EQ("7K/6P1/7k/5ppP/1p1p2P1/1p1p4/1P1P4/8 w - - 0 13",
              engine.getPosition()->printFen());
  }
}

TEST_F(UCITest, goPerft) {

  ostringstream os;
  Engine engine;

  string command = "go perft 6";
  LOG__INFO(LOG, "COMMAND: " + command);
  istringstream is(command);
  UCI::Handler uciHandler(&engine, &is, &os);
  uciHandler.loop();

  engine.stopSearch();
  engine.waitWhileSearching();

  ASSERT_TRUE(engine.getSearchLimits().isPerft());
  ASSERT_FALSE(engine.getSearchLimits().isInfinite());
  ASSERT_FALSE(engine.getSearchLimits().isPonder());
  ASSERT_FALSE(engine.getSearchLimits().isTimeControl());
  ASSERT_EQ(6, engine.getSearchLimits().getMaxDepth());
}

TEST_F(UCITest, goInfinite) {

  ostringstream os;
  Engine engine;

  string command = "go infinite";
  LOG__INFO(LOG, "COMMAND: " + command);
  istringstream is(command);
  UCI::Handler uciHandler(&engine, &is, &os);
  uciHandler.loop();

  engine.stopSearch();
  engine.waitWhileSearching();

  ASSERT_FALSE(engine.getSearchLimits().isPerft());
  ASSERT_TRUE(engine.getSearchLimits().isInfinite());
  ASSERT_FALSE(engine.getSearchLimits().isPonder());
  ASSERT_FALSE(engine.getSearchLimits().isTimeControl());
  ASSERT_EQ(PLY_MAX, engine.getSearchLimits().getMaxDepth());
}

TEST_F(UCITest, goPonder) {
  ostringstream os;
  Engine engine;

  string command = "go ponder movetime 10000";
  LOG__INFO(LOG, "COMMAND: " + command);
  istringstream is(command);
  UCI::Handler uciHandler(&engine, &is, &os);
  uciHandler.loop();

  engine.stopSearch();
  engine.waitWhileSearching();

  ASSERT_FALSE(engine.getSearchLimits().isPerft());
  ASSERT_FALSE(engine.getSearchLimits().isInfinite());
  ASSERT_TRUE(engine.getSearchLimits().isPonder());
  ASSERT_FALSE(engine.getSearchLimits().isTimeControl());
  ASSERT_EQ(PLY_MAX, engine.getSearchLimits().getMaxDepth());
}

TEST_F(UCITest, goMate) {
  ostringstream os;
  Engine engine;

  string command = "go mate 4";
  LOG__INFO(LOG, "COMMAND: " + command);
  istringstream is(command);
  UCI::Handler uciHandler(&engine, &is, &os);
  uciHandler.loop();

  engine.stopSearch();
  engine.waitWhileSearching();

  ASSERT_FALSE(engine.getSearchLimits().isPerft());
  ASSERT_FALSE(engine.getSearchLimits().isInfinite());
  ASSERT_FALSE(engine.getSearchLimits().isPonder());
  ASSERT_FALSE(engine.getSearchLimits().isTimeControl());
  ASSERT_EQ(4, engine.getSearchLimits().getMate());
  ASSERT_EQ(PLY_MAX, engine.getSearchLimits().getMaxDepth());
}

TEST_F(UCITest, goMateDepth) {
  ostringstream os;
  Engine engine;

  string command = "go mate 4 depth 4";
  LOG__INFO(LOG, "COMMAND: " + command);
  istringstream is(command);
  UCI::Handler uciHandler(&engine, &is, &os);
  uciHandler.loop();

  engine.stopSearch();
  engine.waitWhileSearching();

  ASSERT_FALSE(engine.getSearchLimits().isPerft());
  ASSERT_FALSE(engine.getSearchLimits().isInfinite());
  ASSERT_FALSE(engine.getSearchLimits().isPonder());
  ASSERT_FALSE(engine.getSearchLimits().isTimeControl());
  ASSERT_EQ(4, engine.getSearchLimits().getMate());
  ASSERT_EQ(4, engine.getSearchLimits().getMaxDepth());
}

TEST_F(UCITest, goMateTime) {
  ostringstream os;
  Engine engine;

  string command = "go mate 4 movetime 15";
  LOG__INFO(LOG, "COMMAND: " + command);
  istringstream is(command);
  UCI::Handler uciHandler(&engine, &is, &os);
  uciHandler.loop();

  engine.stopSearch();
  engine.waitWhileSearching();

  ASSERT_FALSE(engine.getSearchLimits().isPerft());
  ASSERT_FALSE(engine.getSearchLimits().isInfinite());
  ASSERT_FALSE(engine.getSearchLimits().isPonder());
  ASSERT_TRUE(engine.getSearchLimits().isTimeControl());
  ASSERT_EQ(4, engine.getSearchLimits().getMate());
  ASSERT_EQ(PLY_MAX, engine.getSearchLimits().getMaxDepth());
  ASSERT_EQ(15, engine.getSearchLimits().getMoveTime());
}

TEST_F(UCITest, goMateDepthTime) {
  ostringstream os;
  Engine engine;

  string command = "go mate 4 depth 4 movetime 15";
  LOG__INFO(LOG, "COMMAND: " + command);
  istringstream is(command);
  UCI::Handler uciHandler(&engine, &is, &os);
  uciHandler.loop();

  engine.stopSearch();
  engine.waitWhileSearching();

  ASSERT_FALSE(engine.getSearchLimits().isInfinite());
  ASSERT_FALSE(engine.getSearchLimits().isPonder());
  ASSERT_TRUE(engine.getSearchLimits().isTimeControl());
  ASSERT_EQ(4, engine.getSearchLimits().getMate());
  ASSERT_EQ(4, engine.getSearchLimits().getMaxDepth());
  ASSERT_EQ(15, engine.getSearchLimits().getMoveTime());
  ASSERT_FALSE(engine.getSearchLimits().isPerft());

}

TEST_F(UCITest, goTimed) {
  ostringstream os;
  Engine engine;

  string command = "go wtime 500001 btime 500002";
  LOG__INFO(LOG, "COMMAND: " + command);
  istringstream is(command);
  UCI::Handler uciHandler(&engine, &is, &os);
  uciHandler.loop();

  engine.stopSearch();
  engine.waitWhileSearching();

  ASSERT_FALSE(engine.getSearchLimits().isPerft());
  ASSERT_FALSE(engine.getSearchLimits().isInfinite());
  ASSERT_FALSE(engine.getSearchLimits().isPonder());
  ASSERT_TRUE(engine.getSearchLimits().isTimeControl());
  ASSERT_EQ(PLY_MAX, engine.getSearchLimits().getMaxDepth());
  ASSERT_EQ(500'001, engine.getSearchLimits().getWhiteTime());
  ASSERT_EQ(500'002, engine.getSearchLimits().getBlackTime());
}

TEST_F(UCITest, goMovestogo) {
  ostringstream os;
  Engine engine;

  // normal game with time for each player and remaining moves until time control
  string command = "go wtime 300001 btime 300002 movestogo 20";
  LOG__INFO(LOG, "COMMAND: " + command);
  istringstream is(command);
  UCI::Handler uciHandler(&engine, &is, &os);
  uciHandler.loop();

  engine.stopSearch();
  engine.waitWhileSearching();

  ASSERT_FALSE(engine.getSearchLimits().isPerft());
  ASSERT_FALSE(engine.getSearchLimits().isInfinite());
  ASSERT_FALSE(engine.getSearchLimits().isPonder());
  ASSERT_TRUE(engine.getSearchLimits().isTimeControl());
  ASSERT_EQ(PLY_MAX, engine.getSearchLimits().getMaxDepth());
  ASSERT_EQ(300'001, engine.getSearchLimits().getWhiteTime());
  ASSERT_EQ(300'002, engine.getSearchLimits().getBlackTime());
  ASSERT_EQ(20, engine.getSearchLimits().getMovesToGo());
}

TEST_F(UCITest, goInc) {
  ostringstream os;
  Engine engine;

  // normal game with time for each player and increases per move
  string command = "go wtime 300001 btime 300002 winc 2001 binc 2002";
  LOG__INFO(LOG, "COMMAND: " + command);
  istringstream is(command);
  UCI::Handler uciHandler(&engine, &is, &os);
  uciHandler.loop();

  engine.stopSearch();
  engine.waitWhileSearching();

  ASSERT_FALSE(engine.getSearchLimits().isPerft());
  ASSERT_FALSE(engine.getSearchLimits().isInfinite());
  ASSERT_FALSE(engine.getSearchLimits().isPonder());
  ASSERT_TRUE(engine.getSearchLimits().isTimeControl());
  ASSERT_EQ(PLY_MAX, engine.getSearchLimits().getMaxDepth());
  ASSERT_EQ(300'001, engine.getSearchLimits().getWhiteTime());
  ASSERT_EQ(300'002, engine.getSearchLimits().getBlackTime());
  ASSERT_EQ(2001, engine.getSearchLimits().getWhiteInc());
  ASSERT_EQ(2002, engine.getSearchLimits().getBlackInc());
}

TEST_F(UCITest, goMovetime) {
  ostringstream os;
  Engine engine;

  // move time limited
  string command = "go movetime 5000";
  LOG__INFO(LOG, "COMMAND: " + command);
  istringstream is(command);
  UCI::Handler uciHandler(&engine, &is, &os);
  uciHandler.loop();

  engine.stopSearch();
  engine.waitWhileSearching();

  ASSERT_FALSE(engine.getSearchLimits().isPerft());
  ASSERT_FALSE(engine.getSearchLimits().isInfinite());
  ASSERT_FALSE(engine.getSearchLimits().isPonder());
  ASSERT_TRUE(engine.getSearchLimits().isTimeControl());
  ASSERT_EQ(0, engine.getSearchLimits().getMate());
  ASSERT_EQ(PLY_MAX, engine.getSearchLimits().getMaxDepth());
  ASSERT_EQ(5000, engine.getSearchLimits().getMoveTime());
}

TEST_F(UCITest, goDepth) {
  ostringstream os;
  Engine engine;
  // depth only limited
  string command = "go depth 5";
  LOG__INFO(LOG, "COMMAND: " + command);
  istringstream is(command);
  UCI::Handler uciHandler(&engine, &is, &os);
  uciHandler.loop();

  engine.stopSearch();
  engine.waitWhileSearching();

  ASSERT_FALSE(engine.getSearchLimits().isPerft());
  ASSERT_FALSE(engine.getSearchLimits().isInfinite());
  ASSERT_FALSE(engine.getSearchLimits().isPonder());
  ASSERT_FALSE(engine.getSearchLimits().isTimeControl());
  ASSERT_EQ(1, engine.getSearchLimits().getStartDepth());
  ASSERT_EQ(5, engine.getSearchLimits().getMaxDepth());
  ASSERT_EQ(0, engine.getSearchLimits().getNodes());
  engine.stopSearch();
  engine.waitWhileSearching();
}

TEST_F(UCITest, goNodes) {
  ostringstream os;
  Engine engine;
  // nodes only limited
  string command = "go nodes 1000000";
  LOG__INFO(LOG, "COMMAND: " + command);
  istringstream is(command);
  UCI::Handler uciHandler(&engine, &is, &os);
  uciHandler.loop();

  engine.stopSearch();
  engine.waitWhileSearching();

  ASSERT_FALSE(engine.getSearchLimits().isPerft());
  ASSERT_FALSE(engine.getSearchLimits().isInfinite());
  ASSERT_FALSE(engine.getSearchLimits().isPonder());
  ASSERT_FALSE(engine.getSearchLimits().isTimeControl());
  ASSERT_EQ(1, engine.getSearchLimits().getStartDepth());
  ASSERT_EQ(PLY_MAX, engine.getSearchLimits().getMaxDepth());
  ASSERT_EQ(1'000'000, engine.getSearchLimits().getNodes());
}

TEST_F(UCITest, goNodesDepth) {
  ostringstream os;
  Engine engine;
  // nodes and depth limited
  string command = "go nodes 1000000 depth 5";
  LOG__INFO(LOG, "COMMAND: " + command);
  istringstream is(command);
  UCI::Handler uciHandler(&engine, &is, &os);
  uciHandler.loop();

  engine.stopSearch();
  engine.waitWhileSearching();

  ASSERT_FALSE(engine.getSearchLimits().isPerft());
  ASSERT_FALSE(engine.getSearchLimits().isInfinite());
  ASSERT_FALSE(engine.getSearchLimits().isPonder());
  ASSERT_FALSE(engine.getSearchLimits().isTimeControl());
  ASSERT_EQ(1, engine.getSearchLimits().getStartDepth());
  ASSERT_EQ(5, engine.getSearchLimits().getMaxDepth());
  ASSERT_EQ(1'000'000, engine.getSearchLimits().getNodes());
}

TEST_F(UCITest, goMoves) {
  ostringstream os;
  Engine engine;
  // move time limited with a list of moves to search
  string command = "go movetime 15 searchmoves d2d4 e2e4";
  LOG__INFO(LOG, "COMMAND: " + command);
  istringstream is(command);
  UCI::Handler uciHandler(&engine, &is, &os);
  uciHandler.loop();

  engine.stopSearch();
  engine.waitWhileSearching();

  ASSERT_FALSE(engine.getSearchLimits().isPerft());
  ASSERT_FALSE(engine.getSearchLimits().isInfinite());
  ASSERT_FALSE(engine.getSearchLimits().isPonder());
  ASSERT_TRUE(engine.getSearchLimits().isTimeControl());
  ASSERT_EQ(0, engine.getSearchLimits().getMate());
  ASSERT_EQ(PLY_MAX, engine.getSearchLimits().getMaxDepth());
  ASSERT_EQ(15, engine.getSearchLimits().getMoveTime());
  ASSERT_EQ(createMove("d2d4"), engine.getSearchLimits().getMoves().front());
  ASSERT_EQ(createMove("e2e4"), engine.getSearchLimits().getMoves().back());
}

TEST_F(UCITest, moveTest) {
  ostringstream os;
  Engine engine;

  string command = "position startpos moves e2e4";
  LOG__INFO(LOG, "COMMAND: " + command);
  istringstream is(command);
  UCI::Handler uciHandler(&engine, &is, &os);
  uciHandler.loop();

  command = "go wtime 60000 btime 60000 winc 2000 binc 2000 movestogo 40";
  LOG__INFO(LOG, "COMMAND: " + command);
  is = istringstream(command);
  uciHandler.loop(&is);

  sleep(5);
  engine.stopSearch();
  LOG__DEBUG(LOG, "Waiting until search ends...");
  engine.waitWhileSearching();
  LOG__DEBUG(LOG, "SEARCH ENDED");

}

TEST_F(UCITest, moveTestDepth) {
  ostringstream os;
  Engine engine;

  string command = "position startpos moves e2e4";
  LOG__INFO(LOG, "COMMAND: " + command);
  istringstream is(command);
  UCI::Handler uciHandler(&engine, &is, &os);
  uciHandler.loop();

  command = "go depth 5";
  LOG__INFO(LOG, "COMMAND: " + command);
  is = istringstream(command);
  uciHandler.loop(&is);

  LOG__DEBUG(LOG, "Waiting until search ends...");
  engine.waitWhileSearching();
  LOG__DEBUG(LOG, "SEARCH ENDED");

}

TEST_F(UCITest, ponderRunningStop) {
  ostringstream os;
  Engine engine;

  // stop while pondering
  string command = "position startpos moves e2e4 e7e5";
  LOG__INFO(LOG, "COMMAND: " + command);
  istringstream is(command);
  UCI::Handler uciHandler(&engine, &is, &os);
  uciHandler.loop();

  command = "go ponder wtime 600000 btime 600000";
  LOG__INFO(LOG, "COMMAND: " + command);
  is = istringstream(command);
  uciHandler.loop(&is);

  sleep(1);
  ASSERT_TRUE(engine.isSearching());
  ASSERT_TRUE(engine.getSearchLimits().isPonder());

  command = "stop";
  LOG__INFO(LOG, "COMMAND: " + command);
  is = istringstream(command);
  uciHandler.loop(&is);

  sleep(1);
  ASSERT_FALSE(engine.isSearching());
  ASSERT_TRUE(engine.getLastResult().valid);

  // mate position should finish search quickly
  // position startpos moves d2d4 d7d5 e2e4 d5e4 d4d5 g8f6 b1c3 c7c6 g1f3 e4f3 g2f3 c6d5 f1b5 c8d7 b5d7 d8d7 e1e2 d5d4 d1d4 d7d4 c3d5 d4c4 e2d1 c4d5 d1e1 d5f3 e1d2 b8c6 h1d1 e8c8
  // go wtime 37776 btime 45570 movestogo 25 ponder
}

TEST_F(UCITest, ponderFinishedStop) {
  ostringstream os;
  Engine engine;

  // stop when pondering has already finished
  string command = "position fen 8/8/8/8/8/6K1/R7/6k1 w - - 0 8";
  LOG__INFO(LOG, "COMMAND: " + command);
  istringstream is(command);
  UCI::Handler uciHandler(&engine, &is, &os);
  uciHandler.loop();

  command = "go ponder wtime 600000 btime 600000";
  LOG__INFO(LOG, "COMMAND: " + command);
  is = istringstream(command);
  uciHandler.loop(&is);

  sleep(1);

  ASSERT_TRUE(engine.isSearching());
  ASSERT_TRUE(engine.getSearchLimits().isPonder());

  command = "stop";
  LOG__INFO(LOG, "COMMAND: " + command);
  is = istringstream(command);
  uciHandler.loop(&is);

  sleep(1);
  ASSERT_FALSE(engine.isSearching());
  ASSERT_TRUE(engine.getLastResult().valid);

}

TEST_F(UCITest, ponderMiss) {
  ostringstream os;
  Engine engine;

  EngineConfig::ponder = true;

  // black just played e7e6 and sent ponder on d2d4
  string command = "position startpos moves e2e4 e7e6 d2d4";
  LOG__INFO(LOG, "COMMAND: " + command);
  istringstream is(command);
  UCI::Handler uciHandler(&engine, &is, &os);
  uciHandler.loop();

  // black to ponder on d2d4
  command = "go wtime 600000 btime 600000 ponder";
  LOG__INFO(LOG, "COMMAND: " + command);
  is = istringstream(command);
  uciHandler.loop(&is);

  sleep(1);
  ASSERT_TRUE(engine.isSearching());
  ASSERT_TRUE(engine.getSearchLimits().isPonder());
  sleep(1);

  // user played different move (g1h3) - ponder miss
  command = "stop";
  LOG__INFO(LOG, "COMMAND: " + command);
  is = istringstream(command);
  uciHandler.loop(&is);

  sleep(1);
  ASSERT_FALSE(engine.isSearching());
  ASSERT_FALSE(engine.getSearchLimits().isPonder());
  sleep(1);

  // black getting new position after ponder miss 
  command = "position startpos moves e2e4 e7e6 g1h3";
  LOG__INFO(LOG, "COMMAND: " + command);
  is = istringstream(command);
  uciHandler.loop(&is);

  // black to search on new position
  command = "go wtime 600000 btime 600000";
  LOG__INFO(LOG, "COMMAND: " + command);
  is = istringstream(command);
  uciHandler.loop(&is);

  sleep(1);
  ASSERT_TRUE(engine.isSearching());
  ASSERT_FALSE(engine.getSearchLimits().isPonder());

  // stop search
  command = "stop";
  LOG__INFO(LOG, "COMMAND: " + command);
  is = istringstream(command);
  uciHandler.loop(&is);

  sleep(1);
  ASSERT_FALSE(engine.isSearching());
  ASSERT_FALSE(engine.getSearchLimits().isPonder());

  LOG__DEBUG(LOG, "Waiting until search ends...");
  engine.waitWhileSearching();
  LOG__DEBUG(LOG, "SEARCH ENDED");
}


TEST_F(UCITest, ponderFinishedMiss) {
  ostringstream os;
  Engine engine;

  EngineConfig::ponder = true;

  // black just played e7e6 and sent ponder on d2d4
  string command = "position fen 8/8/8/8/8/6K1/R7/6k1 w - - 0 8";
  LOG__INFO(LOG, "COMMAND: " + command);
  istringstream is(command);
  UCI::Handler uciHandler(&engine, &is, &os);
  uciHandler.loop();

  // black to ponder on d2d4
  command = "go wtime 600000 btime 600000 ponder";
  LOG__INFO(LOG, "COMMAND: " + command);
  is = istringstream(command);
  uciHandler.loop(&is);

  sleep(1);
  ASSERT_TRUE(engine.isSearching());
  ASSERT_TRUE(engine.getSearchLimits().isPonder());
  sleep(1);

  // user played different move (g1h3) - ponder miss
  command = "stop";
  LOG__INFO(LOG, "COMMAND: " + command);
  is = istringstream(command);
  uciHandler.loop(&is);

  sleep(1);
  ASSERT_FALSE(engine.isSearching());
  ASSERT_FALSE(engine.getSearchLimits().isPonder());
  sleep(1);

  // black getting new position after ponder miss
  command = "position startpos moves e2e4 e7e6 g1h3";
  LOG__INFO(LOG, "COMMAND: " + command);
  is = istringstream(command);
  uciHandler.loop(&is);

  // black to search on new position
  command = "go wtime 600000 btime 600000";
  LOG__INFO(LOG, "COMMAND: " + command);
  is = istringstream(command);
  uciHandler.loop(&is);

  sleep(1);
  ASSERT_TRUE(engine.isSearching());
  ASSERT_FALSE(engine.getSearchLimits().isPonder());

  // stop search
  command = "stop";
  LOG__INFO(LOG, "COMMAND: " + command);
  is = istringstream(command);
  uciHandler.loop(&is);

  sleep(1);
  ASSERT_FALSE(engine.isSearching());
  ASSERT_FALSE(engine.getSearchLimits().isPonder());

  LOG__DEBUG(LOG, "Waiting until search ends...");
  engine.waitWhileSearching();
  LOG__DEBUG(LOG, "SEARCH ENDED");
}

TEST_F(UCITest, ponderFinishedHit) {
  ostringstream os;
  Engine engine;

  EngineConfig::ponder = true;

  string command = "setoption name Ponder value true";
  LOG__INFO(LOG, "COMMAND: " + command);
  istringstream is = istringstream(command);
  UCI::Handler uciHandler(&engine, &is, &os);
  uciHandler.loop();

  command = "position fen 8/8/8/8/8/6K1/R7/6k1 w - - 0 8";
  LOG__INFO(LOG, "COMMAND: " + command);
  is = istringstream(command);
  uciHandler.loop();

  command = "go wtime 300000 btime 300000 ponder";
  LOG__INFO(LOG, "COMMAND: " + command);
  is = istringstream(command);
  uciHandler.loop(&is);

  sleep(1);
  ASSERT_TRUE(engine.isSearching());
  ASSERT_TRUE(engine.getSearchLimits().isPonder());
  sleep(1);

  command = "ponderhit";
  LOG__INFO(LOG, "COMMAND: " + command);
  is = istringstream(command);
  uciHandler.loop(&is);

  sleep(1);
  ASSERT_FALSE(engine.isSearching());
  ASSERT_FALSE(engine.getSearchLimits().isPonder());

  // stop search
  command = "stop";
  LOG__INFO(LOG, "COMMAND: " + command);
  is = istringstream(command);
  uciHandler.loop(&is);

  sleep(1);
  ASSERT_FALSE(engine.isSearching());
  ASSERT_FALSE(engine.getSearchLimits().isPonder());

  LOG__DEBUG(LOG, "Waiting until search ends...");
  engine.waitWhileSearching();
  LOG__DEBUG(LOG, "SEARCH ENDED");
}


TEST_F(UCITest, ponderHit) {
  ostringstream os;
  Engine engine;

  EngineConfig::ponder = true;

  string command = "setoption name Ponder value true";
  LOG__INFO(LOG, "COMMAND: " + command);
  istringstream is = istringstream(command);
  UCI::Handler uciHandler(&engine, &is, &os);
  uciHandler.loop();

  command = "position startpos moves e2e4 e7e6 d2d4";
  LOG__INFO(LOG, "COMMAND: " + command);
  is = istringstream(command);
  uciHandler.loop();

  command = "go wtime 300000 btime 300000 ponder";
  LOG__INFO(LOG, "COMMAND: " + command);
  is = istringstream(command);
  uciHandler.loop(&is);

  sleep(1);
  ASSERT_TRUE(engine.isSearching());
  ASSERT_TRUE(engine.getSearchLimits().isPonder());
  sleep(1);

  command = "ponderhit";
  LOG__INFO(LOG, "COMMAND: " + command);
  is = istringstream(command);
  uciHandler.loop(&is);

  sleep(2);
  ASSERT_TRUE(engine.isSearching());
  ASSERT_FALSE(engine.getSearchLimits().isPonder());

  // stop search
  command = "stop";
  LOG__INFO(LOG, "COMMAND: " + command);
  is = istringstream(command);
  uciHandler.loop(&is);

  sleep(1);
  ASSERT_FALSE(engine.isSearching());
  ASSERT_FALSE(engine.getSearchLimits().isPonder());

  LOG__DEBUG(LOG, "Waiting until search ends...");
  engine.waitWhileSearching();
  LOG__DEBUG(LOG, "SEARCH ENDED");

}

TEST_F(UCITest, testingBugs) {
  ostringstream os;
  Engine engine;

  string command = "position startpos moves d2d4 d7d6 d4d5 c7c6 d5c6 b7c6 d1d6 d8d6 e2e4 b8a6 f1a6 c8a6 e4e5 d6e5";
  LOG__INFO(LOG, "COMMAND: " + command);
  istringstream is(command);
  UCI::Handler uciHandler(&engine, &is, &os);
  uciHandler.loop();

  command = "go wtime 48330 btime 49040 movestogo 33";
  command = "go depth 4";
  LOG__INFO(LOG, "COMMAND: " + command);
  is = istringstream(command);
  uciHandler.loop(&is);

  LOG__DEBUG(LOG, "Waiting until search ends...");
  engine.waitWhileSearching();
  LOG__DEBUG(LOG, "SEARCH ENDED");

}