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
#include <gmock/gmock.h>

#include "../../src/datatypes.h"
#include "../../src/UCIHandler.h"
#include "../../src/Engine.h"

using namespace std;
using testing::Eq;

TEST(UCITest, uciTest) {
  INIT::init();
  NEWLINE

  string command = "uci";
  string expectedStart = "id name";
  string expectedEnd = "uciok\n";

  println("COMMAND: " + command)
  istringstream is(command);
  ostringstream os;
  Engine engine;
  UCI::Handler uciHandler(&engine, &is, &os);
  uciHandler.loop();
  string result = os.str();
  println("RESPONSE: " + result)

  ASSERT_EQ(expectedStart, result.substr(0, 7));
  ASSERT_EQ(expectedEnd, result.substr(result.size()-6, result.size()));
}

TEST(UCITest, isreadyTest) {
  INIT::init();
  NEWLINE

  string command = "isready";
  string expected = "readyok\n";

  println("COMMAND: " + command)
  istringstream is(command);
  ostringstream os;
  Engine engine;
  UCI::Handler uciHandler(&engine, &is, &os);
  uciHandler.loop();
  println("RESPONSE: " + os.str())
  ASSERT_EQ(expected, os.str());
}

// TODO Test Position command


// TODO Test setoption command

TEST(UCITest, setoptionTest) {
  INIT::init();
  NEWLINE

  ostringstream os;
  Engine engine;

  string command = "setoption name Hash value 2048";
  println("COMMAND: " + command)
  istringstream is(command);
  UCI::Handler uciHandler(&engine, &is, &os);
  uciHandler.loop();
  ASSERT_EQ("2048", engine.getOption("Hash"));
  ASSERT_EQ(2048, engine.config.hash);

  command = "setoption name Ponder value false";
  println("COMMAND: " + command)
  is = istringstream(command);
  uciHandler = UCI::Handler(&engine, &is, &os);
  uciHandler.loop();
  ASSERT_EQ("false", engine.getOption("Ponder"));
  ASSERT_FALSE(engine.config.ponder);
}

TEST(UCITest, goTest) {
  INIT::init();
  NEWLINE

  ostringstream os;
  Engine engine;

  string command = "go infinite";
  println("COMMAND: " + command)
  istringstream is(command);
  UCI::Handler uciHandler(&engine, &is, &os);
  uciHandler.loop();
  UCISearchMode searchMode = uciHandler.getSearchMode();
  ASSERT_TRUE(searchMode.infinite);

  command = "go ponder";
  println("COMMAND: " + command)
  is = istringstream(command);
  uciHandler = UCI::Handler(&engine, &is, &os);
  uciHandler.loop();
  searchMode = uciHandler.getSearchMode();
  ASSERT_TRUE(searchMode.ponder);
  
  command = "go perft";
  println("COMMAND: " + command)
  is = istringstream(command);
  uciHandler = UCI::Handler(&engine, &is, &os);
  uciHandler.loop();
  searchMode = uciHandler.getSearchMode();
  ASSERT_TRUE(searchMode.perft);

  command = "go depth 5";
  println("COMMAND: " + command)
  is = istringstream(command);
  uciHandler = UCI::Handler(&engine, &is, &os);
  uciHandler.loop();
  searchMode = uciHandler.getSearchMode();
  ASSERT_EQ(5, searchMode.depth);                                                       

  command = "go movetime 600 moves e2e4 d2d4";
  println("COMMAND: " + command)
  is = istringstream(command);
  uciHandler = UCI::Handler(&engine, &is, &os);
  uciHandler.loop();
  searchMode = uciHandler.getSearchMode();
  ASSERT_EQ(600, searchMode.movetime);
  ASSERT_EQ(createMove("e2e4"), searchMode.moves.front());
  ASSERT_EQ(createMove("d2d4"), searchMode.moves.back());

}

TEST(UCITest, moveTest) {
  INIT::init();
  NEWLINE

  ostringstream os;
  Engine engine;

  string command = "position startpos moves e2e4";
  println("COMMAND: " + command)
  istringstream is(command);
  UCI::Handler uciHandler(&engine, &is, &os);
  uciHandler.loop();

  command = "go wtime 60000 btime 60000 winc 0 binc 0 movestogo 40";
  println("COMMAND: " + command)
  is = istringstream(command);
  uciHandler.loop(&is);

  cout << "Waiting until search ends..." << endl;
  engine.waitWhileSearching();
  cout << "SEARCH ENDED" << endl;

}