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
  // TODO
  LOG->error("Engine: Clear Hash not implemented yet!");
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
  // TODO
  LOG->error("Engine: New Game not implemented yet!");
}

void Engine::setPosition(const std::string &fen) {
  LOG->info("Engine: Set position to {}", fen);
  position = Position(fen);
}

void Engine::doMove(const std::string &moveStr) {
  LOG->info("Engine: Do move {}", moveStr);
  MoveGenerator moveGenerator;
  MoveList moves = moveGenerator.generateLegalMoves(GENALL, &position);
  for (Move m : moves) {
    if (printMove(m) == moveStr) {
      position.doMove(m);
      return;
    }
  }
  LOG->warn("Invalid move {}", moveStr);
}

void Engine::startSearch(UCISearchMode *pSearchMode) {
  LOG->info("Engine: Start Search");

  if (search.isRunning()) {
    // Previous search was still running. Stopping to start new search!
    LOG->warn("Egine was already searching. Stopping search to start new search.");
    search.stopSearch();
  }

  assert(pSearchMode->whiteTime >= 0 && pSearchMode->blackTime >= 0 &&
         pSearchMode->whiteInc >= 0 && pSearchMode->blackInc >= 0 &&
         pSearchMode->movetime >= 0);

  searchLimits = SearchLimits(static_cast<MilliSec>(pSearchMode->whiteTime),
                              static_cast<MilliSec>(pSearchMode->blackTime),
                              static_cast<MilliSec>(pSearchMode->whiteInc),
                              static_cast<MilliSec>(pSearchMode->blackInc),
                              static_cast<MilliSec>(pSearchMode->movetime),
                              pSearchMode->movesToGo, pSearchMode->depth,
                              pSearchMode->nodes, pSearchMode->moves,
                              pSearchMode->mate, pSearchMode->ponder,
                              pSearchMode->infinite, pSearchMode->perft);

  search.startSearch(position, &searchLimits);
}

void Engine::stopSearch() {
  LOG->info("Engine: Stop Search");
  search.stopSearch();
}

bool Engine::isSearching() { return search.isRunning(); }

void Engine::ponderHit() {
  LOG->info("Engine: Ponder Hit");
  // TODO
  LOG->error("Engine: Ponder Hit not implemented yet!");
}

void Engine::sendResult(Move bestMove, Move ponderMove) const {
  if (pUciHandler) pUciHandler->sendResult(bestMove, ponderMove);
  else
    LOG->warn("No UCIHandler defined: Engine Result: Best Move = {} Ponder Move = {}",
              printMoveVerbose(bestMove), printMoveVerbose(ponderMove));
}

void Engine::sendInfo(const std::string &info) const {
  if (pUciHandler) pUciHandler->send("info string " + info);
  else LOG->warn("No UCIHandler defined: info string {}", info);
}

void Engine::waitWhileSearching() {
  search.waitWhileSearching();
}

////////////////////////////////////////////////
///// PRIVATE

void Engine::initOptions() {
  // @formatter:off
  MAP("Hash", UCI::Option("Hash", config.hash, 1, 8192)); // spin
  MAP("Clear Hash", UCI::Option("Clear Hash"));           // button
  MAP("Ponder", UCI::Option("Ponder", config.ponder));    // check
  // @formatter:on
  updateConfig();
}

void Engine::updateConfig() {
  // iterate through all UCI options and update config accordingly
  for (const auto &it : optionMap) {
    const UCI::Option &option = it.second;
    const std::string &name = option.getNameID();

    if (name == "Hash") config.hash = getInt(option.getCurrentValue());
    else if (name == "Ponder") config.ponder = option.getCurrentValue() == "true";
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


