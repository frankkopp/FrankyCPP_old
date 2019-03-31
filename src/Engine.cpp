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

#include "Engine.h"
#include "SearchLimits.h"
#include <map>

using namespace std;

////////////////////////////////////////////////
///// CONSTRUCTORS

Engine::Engine() { initOptions(); }

////////////////////////////////////////////////
///// PUBLIC

std::ostream &operator<<(std::ostream &os, const Engine &engine) {
  os << engine.str();
  return os;
}

std::string Engine::str() const {
  stringstream os;
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
  println("Engine: Clear Hash");
  // TODO
}

void Engine::setOption(const string &name, const string &value) {
  println("Engine: Set option " + name + "=" + value);
  const auto pos = optionMap.find(name);
  if (pos != optionMap.end()) {
    pos->second.setCurrentValue(value);
  } else {
    cerr << "No such option: " << name << endl;
  }
  updateConfig();
}

string Engine::getOption(const string &name) {
  println("Engine: Get option " + name);
  const auto pos = optionMap.find(name);
  if (pos != optionMap.end())
    return pos->second.getCurrentValue();
  else {
    cerr << "No such option: " << name << endl;
    return "";
  }
}

void Engine::newGame() {
  println("Engine: New Game");
  // TODO
}

void Engine::setPosition(const string& fen) {
  println("Engine: Set position to " + fen);
  position = Position(fen);
}

void Engine::doMove(const string& moveStr) {
  println("Engine: Do move " + moveStr);

  const Move move = createMove(moveStr.c_str());
  if (!isMove(move)) {
    cerr << "Invalid move " << moveStr << endl;
    return;
  }
  position.doMove(move);
}

void Engine::startSearch(UCISearchMode *pSearchMode) {
  println("Engine: START Search Command received");

  if (search.isRunning()) {
    // Previous search was still running. Stopping to start new search!
    search.stopSearch();
  }

  assert(pSearchMode->whiteTime >= 0 && pSearchMode->blackTime >= 0 &&
         pSearchMode->whiteInc >= 0 && pSearchMode->blackInc >= 0 &&
         pSearchMode->movetime >= 0);

  searchLimits = SearchLimits(static_cast<Millisec>(pSearchMode->whiteTime),
                              static_cast<Millisec>(pSearchMode->blackTime),
                              static_cast<Millisec>(pSearchMode->whiteInc),
                              static_cast<Millisec>(pSearchMode->blackInc),
                              static_cast<Millisec>(pSearchMode->movetime),
                              pSearchMode->movesToGo, pSearchMode->depth,
                              pSearchMode->nodes, pSearchMode->moves,
                              pSearchMode->mate, pSearchMode->ponder,
                              pSearchMode->infinite, pSearchMode->perft);

  search.startSearch(&searchLimits);
}

void Engine::stopSearch() {
  println("Engine: STOP Search Command received");
  search.stopSearch();
}

bool Engine::isSearching() { return search.isRunning(); }

void Engine::ponderHit() {
  println("Engine: Ponder Hit Command received");
  // TODO
}

void Engine::sendResult(Move bestMove, Move ponderMove) const {
  println("Engine Result: Best Move = " + printMove(bestMove) +
          " Ponder Move = " + printMove(ponderMove));
  pUciHandler->sendResult(bestMove, ponderMove);
}

void Engine::sendInfo(const string &info) const {
  pUciHandler->send("info string " + info);
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
    const string &name = option.getNameID();

    if (name == "Hash") config.hash = getInt(option.getCurrentValue());
    else if (name == "Ponder") config.ponder = option.getCurrentValue() == "true";
  }
}

int Engine::getInt(const string &value) const {
  int intValue = 0;
  try {
    intValue = stoi(value);
  }
  catch (invalid_argument &e) {
    cerr << "depth invalid - expected numeric value. Was " << value << endl;
  }
  catch (out_of_range &e) {
    cerr << "depth invalid - numeric value out of range of int. Was " << value << endl;
  }
  return intValue;
}


