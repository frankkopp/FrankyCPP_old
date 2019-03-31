/*
 * MIT License
 *
 * Copyright (c) 2019 Frank Kopp
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

#ifndef FRANKYCPP_ENGINE_H
#define FRANKYCPP_ENGINE_H

#include <map>
#include <ostream>

#include "EngineConfig.h"
#include "UCIOption.h"
#include "UCIHandler.h"
#include "Position.h"
#include "Search.h"
#include "SearchLimits.h"

#define MAP(name, option) optionMap.insert(make_pair(name, option))

using namespace std;

namespace UCI { class Handler; }

class Engine {

  // a map for the engine's available options
  map<const string, UCI::Option> optionMap;

  // callback reference for sending responses to the uci ui
  UCI::Handler *pUciHandler;

  // engine's search instance
  Search search = Search(this);

  // engine's current position
  Position position;

  // engine's search limits
  SearchLimits searchLimits;

public:

  // configuration (public for convenience)
  Config config;

  ////////////////////////////////////////////////
  ///// CONSTRUCTORS

  Engine();

  ////////////////////////////////////////////////
  ///// PUBLIC

  // callback reference for sending responses to the uci ui
  void registerUCIHandler(UCI::Handler *handler) { pUciHandler = handler; };

  // output
  string str() const;
  friend ostream &operator<<(ostream &os, const Engine &engine);

  // commands
  void clearHash();
  void setOption(const string& name, const string& value);
  string getOption(const string &name);
  void newGame();
  void setPosition(const string& fen);
  Position *getPosition() { return &position; };
  void doMove(const string& moveStr);
  void startSearch(UCISearchMode *pSearchMode);
  void stopSearch();
  bool isSearching();
  void ponderHit();

  // send to UCI
  void sendResult(Move bestMove, Move ponderMove) const;
  void sendInfo(const string &info) const;

  // other
  void waitWhileSearching();

private:

  ////////////////////////////////////////////////
  ///// PRIVATE

  void initOptions();
  void updateConfig();
  int getInt(const string &value) const;
};

#endif //FRANKYCPP_ENGINE_H
