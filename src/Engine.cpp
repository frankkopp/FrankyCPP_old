/*
 * MIT License
 *
 * Copyright (c) 2018-2020 Frank Kopp
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whoptionMap the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FRoptionMap, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 */

#include <map>
#include "misc.h"
#include "Engine.h"
#include "Position.h"
#include "SearchLimits.h"
#include "Search.h"
#include "UCIHandler.h"
#include "UCIOption.h"
#include "MoveGenerator.h"

#define MAP(name, option) optionMap.insert(std::make_pair(name, option))

////////////////////////////////////////////////
///// CONSTRUCTORS

Engine::Engine() {
  pPosition = std::make_shared<Position>();
  pSearch = std::make_shared<Search>(this);
  pSearchLimits = std::make_shared<SearchLimits>();
  initOptions();
}

////////////////////////////////////////////////
///// PUBLIC

std::ostream &operator<<(std::ostream &os, const Engine &engine) {
  os << engine.str();
  return os;
}

std::string Engine::str() const {
  std::stringstream os;
  for (const auto &it : optionMap) {
    UCI_Option o = it.second;
    os << "\noption name " << it.first << " type " << o.getTypeString();
    if (o.getType() == UCI_Option::STRING || o.getType() == UCI_Option::CHECK ||
        o.getType() == UCI_Option::COMBO) {
      os << " default " << o.getDefaultValue();
    }

    if (o.getType() == UCI_Option::SPIN) {
      os << " default " << stof(o.getDefaultValue()) << " min "
         << o.getMinValue() << " max " << o.getMaxValue();
    }
  }
  return os.str();
}


void Engine::setOption(const std::string &name, const std::string &value) {
  LOG__INFO(Logger::get().ENGINE_LOG, "Engine: Set option {} = {}", name, value);

  const auto mapIterator = optionMap.find(name);
  if (mapIterator != optionMap.end()) {
    // handle commands like "Clear Hash"
    if (mapIterator->first == "Clear Hash") {
      clearHash();
      return;
    }
    // handle real options with name and value
    mapIterator->second.setCurrentValue(value);
    updateConfig();
  }
  else {
    LOG__WARN(Logger::get().ENGINE_LOG, "No such option: {}", name);
  }

}

std::string Engine::getOption(const std::string &name) {
  LOG__INFO(Logger::get().ENGINE_LOG, "Engine: Get option {}", name);
  const auto pos = optionMap.find(name);
  if (pos != optionMap.end()) {
    return pos->second.getCurrentValue();
  }
  else {
    LOG__WARN(Logger::get().ENGINE_LOG, "No such option: {}", name);
    return "";
  }
}

void Engine::newGame() {
  LOG__INFO(Logger::get().ENGINE_LOG, "Engine: New Game");
  if (pSearch->isRunning()) stopSearch();
  pSearch->clearHash();
}

void Engine::setPosition(const std::string &fen) {
  LOG__INFO(Logger::get().ENGINE_LOG, "Engine: Set position to {}", fen);
  pPosition = std::make_unique<Position>(fen);
}

void Engine::doMove(const std::string &moveStr) {
  LOG__INFO(Logger::get().ENGINE_LOG, "Engine: Do move {}", moveStr);
  // this is used to check if the given move is valid
  // on this position and then uses the generated move to
  // commit the move the internal Engine position
  MoveGenerator moveGenerator;
  const MoveList* movesPtr = moveGenerator.generateLegalMoves<MoveGenerator::GENALL>(*pPosition);
  for (Move m : *movesPtr) {
    if (Misc::toLowerCase(printMove(m)) == Misc::toLowerCase(moveStr)) {
      pPosition->doMove(m);
      return;
    }
  }
  LOG__ERROR(Logger::get().ENGINE_LOG, "Invalid move {}", moveStr);
}

void Engine::startSearch(const UCISearchMode &uciSearchMode) {
  LOG__INFO(Logger::get().ENGINE_LOG, "Engine: Start Search");

  if (pSearch->isRunning()) {
    // Previous search was still running. Stopping to start new search!
    LOG__WARN(Logger::get().ENGINE_LOG, "Engine was already searching. Stopping search to start new search.");
    pSearch->stopSearch();
  }

  // clear last result
  lastResult = {false, MOVE_NONE, MOVE_NONE};

  assert(uciSearchMode.whiteTime >= 0 && uciSearchMode.blackTime >= 0 &&
         uciSearchMode.whiteInc >= 0 && uciSearchMode.blackInc >= 0 &&
         uciSearchMode.movetime >= 0);

  pSearchLimits = std::make_shared<SearchLimits>(static_cast<MilliSec>(uciSearchMode.whiteTime),
                                                 static_cast<MilliSec>(uciSearchMode.blackTime),
                                                 static_cast<MilliSec>(uciSearchMode.whiteInc),
                                                 static_cast<MilliSec>(uciSearchMode.blackInc),
                                                 static_cast<MilliSec>(uciSearchMode.movetime),
                                                 uciSearchMode.movesToGo, uciSearchMode.depth,
                                                 uciSearchMode.nodes, uciSearchMode.moves,
                                                 uciSearchMode.mate, uciSearchMode.ponder,
                                                 uciSearchMode.infinite, uciSearchMode.perft);

  // do not start pondering if not ponder option is set
  if (pSearchLimits->isPonder() && !EngineConfig::ponder) {
    LOG__WARN(Logger::get().ENGINE_LOG, "Engine: go ponder command but ponder option is set to false.");
    return;
  }

  pSearch->startSearch(*pPosition, *pSearchLimits);
}

void Engine::stopSearch() {
  LOG__INFO(Logger::get().ENGINE_LOG, "Engine: Stop Search");
  pSearch->stopSearch();
}

void Engine::ponderHit() {
  LOG__INFO(Logger::get().ENGINE_LOG, "Engine: Ponder Hit");
  pSearch->ponderhit();
}

void Engine::clearHash() {
  LOG__INFO(Logger::get().ENGINE_LOG, "Engine: Clear Hash");
  pSearch->clearHash();
}


void
Engine::sendIterationEndInfo(int depth, int seldepth, Value value, uint64_t nodes, uint64_t nps,
                             MilliSec time, const MoveList &pv) const {
  if (pUciHandler) {
    pUciHandler->sendIterationEndInfo(depth, seldepth, value, nodes, nps, time, pv);
  }
  else
    LOG__WARN(Logger::get().ENGINE_LOG,
              "<no uci handler>: Engine iteration end: depth {} seldepth {} multipv 1 {} nodes {} nps {} time {} pv {}",
              depth,
              seldepth,
              value,
              nodes,
              nps,
              time,
              printMoveListUCI(pv));
}

void Engine::sendCurrentRootMove(Move currmove, MoveList::size_type movenumber) const {
  if (pUciHandler) { pUciHandler->sendCurrentRootMove(currmove, movenumber); }
  else
    LOG__WARN(Logger::get().ENGINE_LOG, "<no uci handler>: Engine current move: currmove {} currmovenumber {}",
              printMove(currmove), movenumber);
}

void Engine::sendSearchUpdate(int depth, int seldepth, uint64_t nodes, uint64_t nps, MilliSec time,
                              int hashfull) const {
  if (pUciHandler) { pUciHandler->sendSearchUpdate(depth, seldepth, nodes, nps, time, hashfull); }
  else
    LOG__WARN(Logger::get().ENGINE_LOG,
              "<no uci handler>: Engine search update: depth {} seldepth {} nodes {} nps {} time {} hashfull {}",
              depth, seldepth, nodes, nps, time, hashfull);
}

void Engine::sendCurrentLine(const MoveList &moveList) const {
  if (pUciHandler) { pUciHandler->sendCurrentLine(moveList); }
  else
    LOG__WARN(Logger::get().ENGINE_LOG, "<no uci handler>: Engine current line: {}", printMoveList(moveList));
}

void Engine::sendResult(const Move bestMove, const Value value, const Move ponderMove) {
  lastResult = {true, bestMove, ponderMove};

  if (pUciHandler) { pUciHandler->sendResult(bestMove, ponderMove); }
  else
    LOG__WARN(Logger::get().ENGINE_LOG, "<no uci handler>: Engine Result: Best Move = {} ({}) Ponder Move = {}",
              printMoveVerbose(bestMove), printValue(value), printMoveVerbose(ponderMove));
}

void Engine::sendString(const std::string &anyString) const {
  if (pUciHandler) { pUciHandler->sendString(anyString); }
  else
    LOG__WARN(Logger::get().ENGINE_LOG, "<no uci handler>: Engine String: {}", anyString);
}

void Engine::waitWhileSearching() {
  pSearch->waitWhileSearching();
}

bool Engine::isSearching() {
  return pSearch->isRunning();
}

////////////////////////////////////////////////
///// PRIVATE

void Engine::initOptions() {
  // @formatter:off
  MAP("Hash", UCI_Option("Hash", EngineConfig::hash, 1, 4096)); // spin
  MAP("Clear Hash", UCI_Option("Clear Hash"));                  // button
  MAP("Ponder", UCI_Option("Ponder", EngineConfig::ponder));    // check
  // @formatter:on
  updateConfig();
}

void Engine::updateConfig() {
  // iterate through all UCI options and update config accordingly
  for (const auto &it : optionMap) {
    const UCI_Option &option = it.second;
    const std::string &name = option.getNameID();

    if (name == "Hash") {
      EngineConfig::hash = getInt(option.getCurrentValue());
      LOG__DEBUG(Logger::get().ENGINE_LOG, "Setting hash table size to {} MB", EngineConfig::hash);
      pSearch->setHashSize(EngineConfig::hash);
    }
    else if (name == "Ponder") {
      EngineConfig::ponder = to_bool(option.getCurrentValue());
    }
  }
}

int Engine::getInt(const std::string &value) {
  int intValue = 0;
  try {
    intValue = stoi(value);
  }
  catch (std::invalid_argument &e) {
    LOG__WARN(Logger::get().ENGINE_LOG, "depth invalid - expected numeric value. Was {}", value);
  }
  catch (std::out_of_range &e) {
    LOG__WARN(Logger::get().ENGINE_LOG, "depth invalid - numeric value out of range of int. Was {}", value);
  }
  return intValue;
}







