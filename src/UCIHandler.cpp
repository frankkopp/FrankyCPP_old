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

#include <iostream>
#include <sstream>

#include "datatypes.h"
#include "UCIHandler.h"
#include "Search.h"

using namespace std;

namespace UCI {

  Handler::Handler() = default;
  Handler::~Handler() = default;

  void Handler::loop() {
    string cmd, token;
    do {

      cerr << "WAIT FOR COMMAND:" << endl;

      // Block here waiting for input or EOF
      if (!getline(cin, cmd)) cmd = "quit";

      // create the stream object
      istringstream inStream(cmd);

      // clear possible previous entries
      token.clear();

      // read word from stream delimiter is whitespace
      // to get line use inStream.str()
      inStream >> skipws >> token;
      cerr << "RECEIVED: " << token << endl;

      if (token == "quit") break;
      else if (token == "uci") uciCommand();
      else if (token == "isready") isReadyCommand();
      else if (token == "setoption") setOptionCommand(inStream);
      else if (token == "ucinewgame") uciNewGameCommand();
      else if (token == "position") positionCommand(inStream);
      else if (token == "go") goCommand(inStream);
      else if (token == "stop") stopCommand();
      else if (token == "ponderhit") ponderHitCommand();
      else if (token == "register") registerCommand(inStream);
      else if (token == "debug") debugCommand(inStream);
      else cerr << "Unknown UCI command: " << token << endl;
      cerr << "COMMAND PROCESSED: " << token << endl;

    } while (token != "quit");
  }

  void Handler::uciCommand() {
    send("id name FrankyCPP");
    send("id author Frank Kopp, Germany");
    send(engine.str());
    send("uciok");
  }

  void Handler::isReadyCommand() {
    send("readyok");
  }

  void Handler::setOptionCommand(istringstream &inStream) {
    string token, name, value;

    if (inStream >> token && token != "name") {
      cerr << ("Command setoption is malformed - expected 'name': " + token) << endl;
      return;
    }

    // read name which could contain spaces
    while (inStream >> token && token != "value") {
      if (!name.empty()) name += " ";
      name += token;
    }

    // read value which could contain spaces
    while (inStream >> token) {
      if (!value.empty()) name += " ";
      value += token;
    }
    engine.setOption(name, value);
  }

  void Handler::uciNewGameCommand() {
    engine.newGame();
  }

  void Handler::positionCommand(istringstream &inStream) {
    string token, startFen;
    inStream >> token;
    if (token == "startpos") {
      startFen = START_POSITION_FEN;
      inStream >> token;
    }
    else if (token == "fen") {
      while (inStream >> token && token != "moves") {
        startFen += token + " ";
      }
    }
    engine.setPosition(startFen);
    if (token == "moves") {
      vector<string> moves;
      while (inStream >> token) {
        moves.push_back(token);
      }
      for (const string &move : moves) {
        engine.doMove(move);
      }
    }
  }

  void Handler::goCommand(istringstream &inStream) {
    // DEBUG
    // This need to be done in Engine then
    Search search;
    search.start();
  }

  void Handler::stopCommand() {

  }

  void Handler::ponderHitCommand() {

  }

  void Handler::registerCommand(istringstream &inStream) {
    cerr << "UCI Protocol Command: register not implemented!" << endl;
  }

  void Handler::debugCommand(istringstream &inStream) {

  }

  void Handler::send(string toSend) const {
    cout << toSend << endl;
  }

}