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

#include "Logging.h"
#include "EngineConfig.h"
#include "UCIOption.h"
#include "UCIHandler.h"
#include "Position.h"
#include "Search.h"
#include "SearchLimits.h"

#define MAP(name, option) optionMap.insert(std::make_pair(name, option))

namespace UCI { class Handler; }

class Engine {

  std::shared_ptr<spdlog::logger> LOG = spdlog::get("Engine_Logger");

  // a map for the engine's available options
  std::map<const std::string, UCI::Option> optionMap;

  // callback reference for sending responses to the uci ui
  UCI::Handler *pUciHandler{nullptr};

  // engine's search instance
  Search search = Search(this);

  // engine's current position
  Position position;

  // engine's search limits
  SearchLimits searchLimits;

public:

  ////////////////////////////////////////////////
  ///// CONSTRUCTORS

  Engine();

  ////////////////////////////////////////////////
  ///// PUBLIC

  // callback reference for sending responses to the uci ui
  void registerUCIHandler(UCI::Handler* const handler) { pUciHandler = handler; };

  // output
  std::string str() const;
  friend std::ostream &operator<<(std::ostream &os, const Engine &engine);

  // commands
  void clearHash();
  void setOption(const std::string &name, const std::string &value);
  std::string getOption(const std::string &name);
  void newGame();
  void setPosition(const std::string &fen);
  Position *getPosition() { return &position; };
  void doMove(const std::string &moveStr);
  void startSearch(const UCISearchMode &uciSearchMode);
  void stopSearch();
  bool isSearching() const { return search.isRunning(); }
  void ponderHit();

  // send to UCI
  void
  sendIterationEndInfo(int depth, int seldepth, Value value, long nodes, int nps, MilliSec time,
                       const MoveList& pv) const;
  void sendCurrentRootMove(Move currmove, MoveList::size_type movenumber) const;
  void
  sendSearchUpdate(int depth, int seldepth, long nodes, int nps, MilliSec time, int hashfull) const;
  void sendCurrentLine(const MoveList& moveList) const;
  void sendResult(Move bestMove, Move ponderMove) const;

  // other
  void waitWhileSearching();

  // getter
  inline const SearchLimits &getSearchLimits() const { return searchLimits; };

private:

  ////////////////////////////////////////////////
  ///// PRIVATE

  void initOptions();
  void updateConfig();
  int getInt(const std::string &value) const;
};

#endif //FRANKYCPP_ENGINE_H
