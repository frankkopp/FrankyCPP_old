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

TEST_F(UCITest, uciTest) {

  string command = "uci";
  string expectedStart = "id name";
  string expectedEnd = "uciok\n";

  println("COMMAND: " + command);
  istringstream is(command);
  ostringstream os;
  Engine engine;
  UCI::Handler uciHandler(&engine, &is, &os);
  uciHandler.loop();
  string result = os.str();
  println("RESPONSE: " + result);

  ASSERT_EQ(expectedStart, result.substr(0, 7));
  ASSERT_EQ(expectedEnd, result.substr(result.size()-6, result.size()));
}

TEST_F(UCITest, isreadyTest) {
  string command = "isready";
  string expected = "readyok\n";

  println("COMMAND: " + command);
  istringstream is(command);
  ostringstream os;
  Engine engine;
  UCI::Handler uciHandler(&engine, &is, &os);
  uciHandler.loop();
  println("RESPONSE: " + os.str());
  ASSERT_EQ(expected, os.str());
}

TEST_F(UCITest, setoptionTest) {
  ostringstream os;
  Engine engine;

  string command = "setoption name Hash value 2048";
  println("COMMAND: " + command);
  istringstream is(command);
  UCI::Handler uciHandler(&engine, &is, &os);
  uciHandler.loop();
  ASSERT_EQ("2048", engine.getOption("Hash"));
  ASSERT_EQ(2048, engine.config.hash);

  command = "setoption name Ponder value false";
  println("COMMAND: " + command);
  is = istringstream(command);
  uciHandler = UCI::Handler(&engine, &is, &os);
  uciHandler.loop();
  ASSERT_EQ("false", engine.getOption("Ponder"));
  ASSERT_FALSE(engine.config.ponder);
}

TEST_F(UCITest, searchModeTest) {

  ostringstream os;
  Engine engine;

  string command = "go infinite";
  println("COMMAND: " + command);
  istringstream is(command);
  UCI::Handler uciHandler(&engine, &is, &os);
  uciHandler.loop();
  UCISearchMode searchMode = uciHandler.getSearchMode();
  ASSERT_TRUE(searchMode.infinite);

  command = "go ponder";
  println("COMMAND: " + command);
  is = istringstream(command);
  uciHandler = UCI::Handler(&engine, &is, &os);
  uciHandler.loop();
  searchMode = uciHandler.getSearchMode();
  ASSERT_TRUE(searchMode.ponder);
  
  command = "go perft";
  println("COMMAND: " + command);
  is = istringstream(command);
  uciHandler = UCI::Handler(&engine, &is, &os);
  uciHandler.loop();
  searchMode = uciHandler.getSearchMode();
  ASSERT_TRUE(searchMode.perft);

  command = "go depth 5";
  println("COMMAND: " + command);
  is = istringstream(command);
  uciHandler = UCI::Handler(&engine, &is, &os);
  uciHandler.loop();
  searchMode = uciHandler.getSearchMode();
  ASSERT_EQ(5, searchMode.depth);                                                       

  command = "go movetime 600 moves e2e4 d2d4";
  println("COMMAND: " + command);
  is = istringstream(command);
  uciHandler = UCI::Handler(&engine, &is, &os);
  uciHandler.loop();
  searchMode = uciHandler.getSearchMode();
  ASSERT_EQ(600, searchMode.movetime);
  ASSERT_EQ(createMove("e2e4"), searchMode.moves.front());
  ASSERT_EQ(createMove("d2d4"), searchMode.moves.back());

}

TEST_F(UCITest, positionTest) {
  ostringstream os;
  Engine engine;

  // normal
  {
    string command = "position startpos moves e2e4 e7e5";
    println("COMMAND: " + command);
    istringstream is(command);
    UCI::Handler uciHandler(&engine, &is, &os);
    uciHandler.loop();
    ASSERT_EQ("rnbqkbnr/pppp1ppp/8/4p3/4P3/8/PPPP1PPP/RNBQKBNR w KQkq e6 0 2",
              engine.getPosition()->printFen());
  }

  // castling
  {
    string command = "position fen r1bqkb1r/pppp1ppp/2n2n2/1B2p3/4P3/5N2/PPPP1PPP/RNBQK2R w KQkq - 0 0 moves e1g1";
    println("COMMAND: " + command);
    istringstream is(command);
    UCI::Handler uciHandler(&engine, &is, &os);
    uciHandler.loop();
    ASSERT_EQ("r1bqkb1r/pppp1ppp/2n2n2/1B2p3/4P3/5N2/PPPP1PPP/RNBQ1RK1 b kq - 1 1",
              engine.getPosition()->printFen());
  }

  // promotion
  {
    string command = "position fen 8/3P4/6K1/8/8/1k6/8/8 w - - 0 0 moves d7d8q";
    println("COMMAND: " + command);
    istringstream is(command);
    UCI::Handler uciHandler(&engine, &is, &os);
    uciHandler.loop();
    ASSERT_EQ("3Q4/8/6K1/8/8/1k6/8/8 b - - 0 1",
              engine.getPosition()->printFen());
  }

  // normal
  {
    string command = "position moves e2e4 e7e5";
    println("COMMAND: " + command);
    istringstream is(command);
    UCI::Handler uciHandler(&engine, &is, &os);
    uciHandler.loop();
    ASSERT_EQ("rnbqkbnr/pppp1ppp/8/4p3/4P3/8/PPPP1PPP/RNBQKBNR w KQkq e6 0 2",
              engine.getPosition()->printFen());
  }

  // normal
  {
    string command = "position fen rnbqkbnr/8/8/8/8/8/8/RNBQKBNR w KQkq - 0 1 moves e1e2 e8e7";
    println("COMMAND: " + command);
    istringstream is(command);
    UCI::Handler uciHandler(&engine, &is, &os);
    uciHandler.loop();
    ASSERT_EQ("rnbq1bnr/4k3/8/8/8/8/4K3/RNBQ1BNR w - - 2 2",
              engine.getPosition()->printFen());
  }

  // normal
  {
    string command = "position fen 7K/8/5pPk/6pP/1p1p2P1/1p1p4/1P1P4/8 w - - 0 12 moves g6g7";
    println("COMMAND: " + command);
    istringstream is(command);
    UCI::Handler uciHandler(&engine, &is, &os);
    uciHandler.loop();
    ASSERT_EQ("7K/6P1/5p1k/6pP/1p1p2P1/1p1p4/1P1P4/8 b - - 0 12",
              engine.getPosition()->printFen());
  }

  // normal
  {
    string command = "position fen 7K/6P1/5p1k/6pP/1p1p2P1/1p1p4/1P1P4/8 b - - 0 12 moves f6f5";
    println("COMMAND: " + command);
    istringstream is(command);
    UCI::Handler uciHandler(&engine, &is, &os);
    uciHandler.loop();
    ASSERT_EQ("7K/6P1/7k/5ppP/1p1p2P1/1p1p4/1P1P4/8 w - - 0 13",
              engine.getPosition()->printFen());
  }
}

TEST_F(UCITest, moveTest) {
  ostringstream os;
  Engine engine;

  string command = "position startpos moves e2e4";
  println("COMMAND: " + command);
  istringstream is(command);
  UCI::Handler uciHandler(&engine, &is, &os);
  uciHandler.loop();

  command = "go wtime 60000 btime 60000 winc 0 binc 0 movestogo 40";
  println("COMMAND: " + command);
  is = istringstream(command);
  uciHandler.loop(&is);

  cout << "Waiting until search ends..." << endl;
  engine.waitWhileSearching();
  cout << "SEARCH ENDED" << endl;

}