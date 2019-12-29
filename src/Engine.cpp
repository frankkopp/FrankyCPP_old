/*
 * MIT License
 *
 * Copyright (c) 2019 Frank Kopp
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

#include "Engine.h"
#include "SearchLimits.h"

////////////////////////////////////////////////
///// CONSTRUCTORS

Engine::Engine() {
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
    UCI::Option o = it.second;
    os << "\noption name " << it.first << " type " << o.getTypeString();
    if (o.getType() == UCI::STRING || o.getType() == UCI::CHECK ||
        o.getType() == UCI::COMBO)
      os << " default " << o.getDefaultValue();

    if (o.getType() == UCI::SPIN)
      os << " default " << stof(o.getDefaultValue()) << " min "
         << o.getMinValue() << " max " << o.getMaxValue();
  }
  return os.str();
}

void Engine::clearHash() {
  LOG->info("Engine: Clear Hash");
  search.clearHash();
}

void Engine::setOption(const std::string &name, const std::string &value) {
  LOG->info("Engine: Set option {} = {}", name, value);
  const auto pos = optionMap.find(name);
  if (pos != optionMap.end()) {
    pos->second.setCurrentValue(value);
  }
  else {
    LOG->warn("No such option: {}", name);
  }
  updateConfig();
}

std::string Engine::getOption(const std::string &name) {
  LOG->info("Engine: Get option {}", name);
  const auto pos = optionMap.find(name);
  if (pos != optionMap.end())
    return pos->second.getCurrentValue();
  else {
    LOG->warn("No such option: {}", name);
    return "";
  }
}

void Engine::newGame() {
  LOG->info("Engine: New Game");
  // stop any search - this is necessary in case of ponder miss
  stopSearch();
}

void Engine::setPosition(const std::string &fen) {
  LOG->info("Engine: Set position to {}", fen);
  // stop any search - this is necessary in case of ponder miss
  stopSearch();
  position = Position(fen);
}

void Engine::doMove(const std::string &moveStr) {
  LOG->info("Engine: Do move {}", moveStr);
  MoveGenerator moveGenerator;
  MoveList moves = *moveGenerator.generateLegalMoves<MoveGenerator::GENALL>(position);
  for (Move m : moves) {
    if (printMove(m) == moveStr) {
      position.doMove(m);
      return;
    }
  }
  LOG->warn("Invalid move {}", moveStr);
}

void Engine::startSearch(const UCISearchMode &uciSearchMode) {
  LOG->info("Engine: Start Search");

  if (search.isRunning()) {
    // Previous search was still running. Stopping to start new search!
    LOG->warn("Engine was already searching. Stopping search to start new search.");
    search.stopSearch();
  }

  assert(uciSearchMode.whiteTime >= 0 && uciSearchMode.blackTime >= 0 &&
         uciSearchMode.whiteInc >= 0 && uciSearchMode.blackInc >= 0 &&
         uciSearchMode.movetime >= 0);

  searchLimits = SearchLimits(static_cast<MilliSec>(uciSearchMode.whiteTime),
                              static_cast<MilliSec>(uciSearchMode.blackTime),
                              static_cast<MilliSec>(uciSearchMode.whiteInc),
                              static_cast<MilliSec>(uciSearchMode.blackInc),
                              static_cast<MilliSec>(uciSearchMode.movetime),
                              uciSearchMode.movesToGo, uciSearchMode.depth,
                              uciSearchMode.nodes, uciSearchMode.moves,
                              uciSearchMode.mate, uciSearchMode.ponder,
                              uciSearchMode.infinite, uciSearchMode.perft);

  // do not start pondering if not ponder option is set
  if (searchLimits.isPonder() && !EngineConfig::ponder) {
    LOG->warn("Engine: go ponder command but ponder option is set to false.");
    return;
  }

  search.startSearch(position, searchLimits);
}

void Engine::stopSearch() {
  LOG->info("Engine: Stop Search");
  search.stopSearch();
}

void Engine::ponderHit() {
  LOG->info("Engine: Ponder Hit");
  search.ponderhit();
}

void Engine::sendIterationEndInfo(int depth, int seldepth, Value value, long nodes, int nps,
                                  MilliSec time, const MoveList &pv) const {
  if (pUciHandler)
    pUciHandler->sendIterationEndInfo(depth, seldepth, value, nodes, nps, time, pv);
  else
    LOG->warn(
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
  if (pUciHandler) pUciHandler->sendCurrentRootMove(currmove, movenumber);
  else
    LOG->warn("<no uci handler>: Engine current move: currmove {} currmovenumber {}",
              printMove(currmove),
              movenumber);
}

void Engine::sendSearchUpdate(int depth, int seldepth, long nodes, int nps, MilliSec time,
                              int hashfull) const {
  if (pUciHandler) pUciHandler->sendSearchUpdate(depth, seldepth, nodes, nps, time, hashfull);
  else
    LOG->warn(
      "<no uci handler>: Engine search update: depth {} seldepth {} nodes {} nps {} time {} hashfull {}",
      depth,
      seldepth,
      nodes,
      nps,
      time,
      hashfull);
}

void Engine::sendCurrentLine(const MoveList &moveList) const {
  if (pUciHandler) pUciHandler->sendCurrentLine(moveList);
  else
    LOG->warn("<no uci handler>: Engine current line: {}", printMoveList(moveList));
}

void Engine::sendResult(Move bestMove, Move ponderMove) const {
  if (pUciHandler) pUciHandler->sendResult(bestMove, ponderMove);
  else
    LOG->warn("<no uci handler>: Engine Result: Best Move = {} ({}) Ponder Move = {}",
              printMoveVerbose(bestMove), valueOf(bestMove), printMoveVerbose(ponderMove));
}

void Engine::waitWhileSearching() {
  search.waitWhileSearching();
}

////////////////////////////////////////////////
///// PRIVATE

void Engine::initOptions() {
  // TODO: add option for TT 
  // @formatter:off
  // MAP("Hash", UCI::Option("Hash", config.hash, 1, 8192)); // spin
  // MAP("Clear Hash", UCI::Option("Clear Hash"));           // button
  MAP("Ponder", UCI::Option("Ponder", EngineConfig::ponder));    // check
  // @formatter:on
  updateConfig();
}

void Engine::updateConfig() {
  // iterate through all UCI options and update config accordingly
  for (const auto &it : optionMap) {
    const UCI::Option &option = it.second;
    const std::string &name = option.getNameID();

    if (name == "Hash") EngineConfig::hash = getInt(option.getCurrentValue());
    else
      if (name == "Ponder") {
        EngineConfig::ponder = to_bool(option.getCurrentValue());
      }
  }
}

int Engine::getInt(const std::string &value) const {
  int intValue = 0;
  try {
    intValue = stoi(value);
  }
  catch (std::invalid_argument &e) {
    LOG->warn("depth invalid - expected numeric value. Was {}", value);
  }
  catch (std::out_of_range &e) {
    LOG->warn("depth invalid - numeric value out of range of int. Was {}", value);
  }
  return intValue;
}







