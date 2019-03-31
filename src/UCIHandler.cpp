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
#include "unistd.h"

#include "datatypes.h"

#include "Engine.h"
#include "UCIHandler.h"
#include "Search.h"
#include "UCISearchMode.h"

using namespace std;

namespace UCI {

  Handler::Handler(Engine *pEngine) {
    this->pEngine = pEngine;
    pEngine->registerUCIHandler(this);
  }

  Handler::Handler(Engine *pEng, istream *pIstream, ostream *pOstream)
    : Handler::Handler(pEng) {
    pInputStream = pIstream;
    pOutputStream = pOstream;
  }

  Handler::~Handler() = default;

  void Handler::loop() {
    loop(pInputStream);
  }

  void Handler::loop(istream *pIstream) {
    string cmd, token;
    do {
      cout << "HANDLER WAIT FOR COMMAND:" << endl;
      // Block here waiting for input or EOF
      // only blocks on cin!!
      if (!getline(*pIstream, cmd)) cmd = "quit";
      //  create the stream object
      istringstream inStream(cmd);

      // clear possible previous entries
      token.clear();

      // read word from stream delimiter is whitespace
      // to get line use inStream.str()
      inStream >> skipws >> token;
      cout << "HANDLER COMMAND RECEIVED: " << token << endl;

      if (token == "quit") break;
      else if (token == "uci") uciCommand();
      else if (token == "isready") isReadyCommand();
      else if (token == "setoption") setOptionCommand(inStream);
      else if (token == "ucinewgame") uciNewGameCommand();
      else if (token == "position") positionCommand(inStream);
      else if (token == "go") goCommand(inStream);
      else if (token == "stop") stopCommand();
      else if (token == "ponderhit") ponderHitCommand();
      else if (token == "register") registerCommand();
      else if (token == "debug") debugCommand();
      else if (token == "noop") /* noop */;
      else cerr << "Unknown UCI command: " << token << endl;
      cout << "HANDLER COMMAND PROCESSED: " << token << endl;

    } while (token != "quit");

  }

  void Handler::uciCommand() {
    send("id name FrankyCPP");
    send("id author Frank Kopp, Germany");
    send(pEngine->str());
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
    pEngine->setOption(name, value);
  }

  void Handler::uciNewGameCommand() {
    pEngine->newGame();
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
    pEngine->setPosition(startFen);
    if (token == "moves") {
      vector<string> moves;
      while (inStream >> token) {
        moves.push_back(token);
      }
      for (const string &move : moves) {
        pEngine->doMove(move);
      }
    }
  }

  void Handler::goCommand(istringstream &inStream) {
    searchMode = UCISearchMode();

    string token, startFen;

    while (inStream >> token) {
      if (token == "moves") {
        MoveList searchMoves;
        while (inStream >> token) {
          Move move = createMove(token.c_str());
          if (isMove(move)) searchMoves.push_back(move);
          else break;
        }
        if (!searchMoves.empty()) searchMode.moves = searchMoves;
      }
      if (token == "ponder") {
        searchMode.ponder = true;
      }
      if (token == "wtime") {
        inStream >> token;
        try {
          searchMode.whiteTime = stoi(token);
        }
        catch (invalid_argument &e) {
          cerr << "wtime invalid - expected numeric value. Was " << token << endl;
        }
        catch (out_of_range &e ) {
          cerr << "wtime invalid - numeric value out of range of int. Was " << token << endl;
        }
      }
      if (token == "btime") {
        inStream >> token;
        try {
          searchMode.blackTime = stoi(token);
        }
        catch (invalid_argument &e) {
          cerr << "btime invalid - expected numeric value. Was " << token << endl;
        }
        catch (out_of_range &e ) {
          cerr << "btime invalid - numeric value out of range of int. Was " << token << endl;
        }
      }
      if (token == "winc") {
        inStream >> token;
        try {
          searchMode.whiteInc = stoi(token);
        }
        catch (invalid_argument &e) {
          cerr << "winc invalid - expected numeric value. Was " << token << endl;
        }
        catch (out_of_range &e ) {
          cerr << "winc invalid - numeric value out of range of int. Was " << token << endl;
        }
      }
      if (token == "binc") {
        inStream >> token;
        try {
          searchMode.blackInc = stoi(token);
        }
        catch (invalid_argument &e) {
          cerr << "binc invalid - expected numeric value. Was " << token << endl;
        }
        catch (out_of_range &e ) {
          cerr << "binc invalid - numeric value out of range of int. Was " << token << endl;
        }
      }
      if (token == "movestogo") {
        inStream >> token;
        try {
          searchMode.movesToGo = stoi(token);
        }
        catch (invalid_argument &e) {
          cerr << "movestogo invalid - expected numeric value. Was " << token << endl;
        }
        catch (out_of_range &e ) {
          cerr << "movestogo invalid - numeric value out of range of int. Was " << token << endl;
        }
      }
      if (token == "depth") {
        inStream >> token;
        try {
          searchMode.depth = stoi(token);
        }
        catch (invalid_argument &e) {
          cerr << "depth invalid - expected numeric value. Was " << token << endl;
        }
        catch (out_of_range &e ) {
          cerr << "depth invalid - numeric value out of range of int. Was " << token << endl;
        }
      }
      if (token == "mate") {
        inStream >> token;
        try {
          searchMode.mate = stoi(token);
        }
        catch (invalid_argument &e) {
          cerr << "mate invalid - expected numeric value. Was " << token << endl;
        }
        catch (out_of_range &e ) {
          cerr << "mate invalid - numeric value out of range of int. Was " << token << endl;
        }
      }
      if (token == "movetime") {
        inStream >> token;
        try {
          searchMode.movetime = stoi(token);
        }
        catch (invalid_argument &e) {
          cerr << "movetime invalid - expected numeric value. Was " << token << endl;
        }
        catch (out_of_range &e ) {
          cerr << "movetime invalid - numeric value out of range of int. Was " << token << endl;
        }
      }
      if (token == "infinite") {
        searchMode.infinite = true;
      }
      if (token == "perft") {
        searchMode.perft = true;
      }
    }

    // start search in engine
    pEngine->startSearch(&searchMode);
  }

  void Handler::stopCommand() {
    pEngine->stopSearch();

  }

  void Handler::ponderHitCommand() {
    pEngine->ponderHit();
  }

  void Handler::registerCommand() {
    cerr << "UCI Protocol Command: register not implemented!" << endl;
  }

  void Handler::debugCommand() {
    cerr << "UCI Protocol Command: debug not implemented!" << endl;
  }

  void Handler::send(const string& toSend) const {
    *pOutputStream << toSend << endl;
  }

  void Handler::sendResult(Move bestMove, Move ponderMove) {
    *pOutputStream << "bestmove " << bestMove;
    if (isMove(ponderMove)) *pOutputStream << " ponderOption " << ponderMove;
    *pOutputStream << endl;
  }

}