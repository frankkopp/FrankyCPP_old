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
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FRoptionMap,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */

#include "datatypes.h"
#include "Engine.h"
#include "Position.h"

using namespace std;

Engine::Engine() {
  initOptions();
}

void Engine::initOptions() {
  // @formatter:off
  MAP("Hash",       UCI::Option("Hash", 1024, 1, 8192)); // spin
  MAP("Clear Hash", UCI::Option("Clear Hash"));          // button
  // @formatter:on
}

std::ostream &operator<<(std::ostream &os, const Engine &engine) {
  os << engine.str();
  return os;
}

std::string Engine::str() const {
  stringstream os;
  for (const auto &it : optionMap) {
    UCI::Option o = it.second;
    os << "\noption name " << it.first << " type " << o.getTypeString();
    if (o.getType() == UCI::STRING
        || o.getType() == UCI::CHECK
        || o.getType() == UCI::COMBO)
      os << " default " << o.getDefaultValue();

    if (o.getType() == UCI::SPIN)
      os << " default " << stof(o.getDefaultValue())
         << " min " << o.getMinValue()
         << " max " << o.getMaxValue();

  }
  return os.str();
}

void Engine::clearHash() {
  // TODO
  println("Engine: Clear Hash");
}

void Engine::setOption(string name, string value) {
  // TODO
  println("Engine: Set option " + name + "=" + value);
}

void Engine::newGame() {
  // TODO
  println("Engine: New Game");
}

void Engine::setPosition(string fen) {
  println("Engine: Set position to " + fen);
  position = Position(fen);
}

void Engine::doMove(string moveStr) {
  println("Engine: Do move " + moveStr);
  const Move move = createMove(moveStr.c_str());
  if (!isMove(move)) {
    cerr << "Invalid move " << moveStr << endl;
    return;
  }
  position.doMove(move);
}

void Engine::startSearch(SearchMode *pSearchMode) {
  // TODO - this is just a placeholder
  search.start();
}

void Engine::stopSearch() {
  // TODO
  println("Engine: STOP Search Command received");
}
void Engine::ponderHit() {
  // TODO
  println("Engine: Ponder Hit Command received");
}
