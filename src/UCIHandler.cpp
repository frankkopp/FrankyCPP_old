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
#include "version.h"
#include "types.h"
#include "Engine.h"
#include "UCIHandler.h"
#include "UCISearchMode.h"

namespace UCI {

  Handler::Handler(Engine *pEngine) {
    this->pEngine = pEngine;
    pEngine->registerUCIHandler(this);
  }

  Handler::Handler(Engine *pEng, std::istream *pIstream, std::ostream *pOstream)
    : Handler::Handler(pEng) {
    pInputStream = pIstream;
    pOutputStream = pOstream;
  }

  void Handler::loop() {
    loop(pInputStream);
  }

  void Handler::loop(std::istream *pIstream) {
    std::string cmd, token;
    do {
      LOG->info("UCI Handler waiting for command:");

      // Block here waiting for input or EOF
      // only blocks on cin!!
      if (!getline(*pIstream, cmd)) cmd = "quit";
      //  create the stream object
      std::istringstream inStream(cmd);

      UCI_LOG->info("<< {}", inStream.str());
      LOG->info("UCI Handler received command: {}", inStream.str());

      // clear possible previous entries
      token.clear();

      // read word from stream delimiter is whitespace
      // to get line use inStream.str()
      inStream >> skipws >> token;

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
      else LOG__WARN(LOG, "Unknown UCI command: {}", token);

      LOG->info("UCI Handler processed command: {}", token);

    } while (token != "quit");

  }

  void Handler::uciCommand() const {
    send("id name FrankyCPP v" + to_string(FrankyCPP_VERSION_MAJOR) + "." +
         to_string(FrankyCPP_VERSION_MINOR));
    send("id author Frank Kopp, Germany");
    send(pEngine->str());
    send("uciok");
  }

  void Handler::isReadyCommand() const { send("readyok"); }

  void Handler::setOptionCommand(std::istringstream &inStream) const {
    string token, name, value;

    if (inStream >> token && token != "name") {
      LOG__WARN(LOG, "Command setoption is malformed - expected 'name': {}",
                token);
      return;
    }

    // read name which could contain spaces
    while (inStream >> token && token != "value") {
      if (!name.empty())
        name += " ";
      name += token;
    }

    // read value which could contain spaces
    while (inStream >> token) {
      if (!value.empty()) name += " ";
      value += token;
    }
    pEngine->setOption(name, value);
  }

  void Handler::uciNewGameCommand() const { pEngine->newGame(); }

  void Handler::positionCommand(std::istringstream &inStream) const {

    // retrieve additional command parameter
    std::string token, startFen;
    inStream >> token;

    // default
    startFen = START_POSITION_FEN;
    if (token == "startpos") { // just keep default
      inStream >> token;
    } else if (token == "fen") {
      startFen.clear(); // reset to empty
      while (inStream >> token && token != "moves") {
        startFen += token + " ";
      }
    }

    pEngine->setPosition(startFen);

    if (token == "moves") {
      std::vector<string> moves;
      while (inStream >> token) {
        moves.push_back(token);
      }
      for (const std::string &move : moves) {
        pEngine->doMove(move);
      }
    }
  }

  void Handler::goCommand(std::istringstream &inStream) {
    std::string token, startFen;

    // resetting search mode
    searchMode = UCISearchMode();

    while (inStream >> token) {
      if (token == "searchmoves") {
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
        } catch (...) {
          cerr << "PANIK";
        }
        if (searchMode.whiteTime <= 0) {
          LOG__WARN(LOG, "Invalid wtime. Was '{}'", token);
          return;
        }
      }
      if (token == "btime") {
        inStream >> token;
        try {
          searchMode.blackTime = stoi(token);
        } catch (...) {
          cerr << "PANIK";
        }
        if (searchMode.blackTime <= 0) {
          LOG__WARN(LOG, "Invalid btime. Was '{}'", token);
          return;
        }
      }
      if (token == "winc") {
        inStream >> token;
        try {
          searchMode.whiteInc = std::stoi(token);
        } catch (...) {
          cerr << "PANIK";
        }
        if (searchMode.whiteInc < 0) {
          LOG__WARN(LOG, "Invalid winc. Was '{}'", token);
          return;
        }
      }
      if (token == "binc") {
        inStream >> token;
        try {
          searchMode.blackInc = std::stoi(token);
        } catch (...) {
          cerr << "PANIK";
        }
        if (searchMode.blackInc < 0) {
          LOG__WARN(LOG, "Invalid binc. Was '{}'", token);
          return;
        }
      }
      if (token == "movestogo") {
        inStream >> token;
        try {
          searchMode.movesToGo = std::stoi(token);
        } catch (...) {
          cerr << "PANIK";
        }
        if (searchMode.movesToGo <= 0) {
          LOG__WARN(LOG, "Invalid movestogo. Was '{}'", token);
          return;
        }
      }
      if (token == "depth") {
        inStream >> token;
        try {
          searchMode.depth = std::stoi(token);
        } catch (...) {
          cerr << "PANIK";
        }
        if (searchMode.depth <= 0 || searchMode.depth > PLY_MAX) {
          LOG__WARN(LOG, "depth not between 1 and {}. Was '{}'", PLY_MAX, token);
          return;
        }
      }
      if (token == "nodes") {
        inStream >> token;
        try {
          searchMode.nodes = std::stoi(token);
        } catch (...) {
          cerr << "PANIK";
        }
        if (searchMode.nodes <= 0) {
          LOG__WARN(LOG, "Invalid nodes. Was '{}'", token);
          return;
        }
      }
      if (token == "mate") {
        inStream >> token;
        try {
          searchMode.mate = stoi(token);
        } catch (...) {
          cerr << "PANIK";
        }
        if (searchMode.mate <= 0 || searchMode.mate > PLY_MAX) {
          LOG__WARN(LOG, "mate not between 1 and {}. Was '{}'", PLY_MAX, token);
          return;
        }
      }
      if (token == "movetime") {
        inStream >> token;
        try {
          searchMode.movetime = stoi(token);
        } catch (...) {
          cerr << "PANIK";
        }
        if (searchMode.movetime <= 0) {
          LOG__WARN(LOG, "Invalid movetime. Was '{}'", token);
          return;
        }
      }
      if (token == "infinite") {
        searchMode.infinite = true;
      }
      if (token == "perft") {
        searchMode.perft = true;
        inStream >> token;
        try {
          searchMode.depth = stoi(token);
        } catch (...) {
          cerr << "PANIK";
        }
        if (searchMode.depth <= 0 || searchMode.depth > PLY_MAX) {
          LOG__WARN(LOG, "perft depth not between 1 and {}. Was '{}'", PLY_MAX, token);
          return;
        }
      }
    }

    // start search in engine
    pEngine->startSearch(searchMode);
  }

  void Handler::stopCommand() const { pEngine->stopSearch(); }

  void Handler::ponderHitCommand() const { pEngine->ponderHit(); }

  void Handler::registerCommand() const {
    LOG__WARN(LOG, "UCI Protocol Command: register not implemented!");
  }

  void Handler::debugCommand() const {
    LOG__WARN(LOG, "UCI Protocol Command: debug not implemented!");
  }

  void Handler::send(const std::string &toSend) const {
    UCI_LOG->info(">> {}", toSend);
    *pOutputStream << toSend << endl;
  }

  void Handler::sendResult(Move bestMove, Move ponderMove) const {
    send(fmt::format("bestmove {}{}", printMove(bestMove),
                     (ponderMove ? " ponder " + printMove(ponderMove) : "")));
  }

  void Handler::sendCurrentLine(const MoveList &moveList) const {
    send(fmt::format("currline {}", printMoveListUCI(moveList)));
  }

  void Handler::sendIterationEndInfo(int depth, int seldepth, Value value,
                                     long nodes, int nps, MilliSec time,
                                     const MoveList &pv) const {
    send(fmt::format("info depth {} seldepth {} multipv 1 score {} nodes {} "
                     "nps {} time {} pv {}",
                     depth, seldepth, printValue(Value(value)), nodes, nps,
                     time, printMoveListUCI(pv)));
  }

  void Handler::sendCurrentRootMove(Move currmove, int movenumber) const {
    send(fmt::format("currmove {} currmovenumber {}", printMove(currmove),
                     movenumber));
  }

  void Handler::sendSearchUpdate(int depth, int seldepth, long nodes, int nps,
                                 MilliSec time, int hashfull) const {
    send(fmt::format("depth {} seldepth {} nodes {} nps {} time {} hashfull {}",
                     depth, seldepth, nodes, nps, time, hashfull));
  }

  }