/*
 * MIT License
 *
 * Copyright (c) 2018-2020 Frank Kopp
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
#include <memory>
#include <utility>
#include "UCIOption.h"
#include "EngineConfig.h"
#include "UCISearchMode.h"

// forward declarations
class UCI_Handler;
class Position;
class Search;
class SearchLimits;

class Engine {

  //  std::shared_ptr<spdlog::logger> LOG = spdlog::get("Engine_Logger");

  // a map for the engine's available options
  std::map<const std::string, UCI_Option> optionMap;

  // callback reference for sending responses to the uci ui
  std::shared_ptr<UCI_Handler> pUciHandler{nullptr};

  // engine's search instance
  std::shared_ptr<Search> pSearch{nullptr};

  // engine's search limits
  std::shared_ptr<SearchLimits> pSearchLimits{nullptr};

  // engine's current position
  std::shared_ptr<Position> pPosition{nullptr};

  // last result
  struct Result {
    bool valid = false;
    Move bestMove = MOVE_NONE;
    Move ponderMove = MOVE_NONE;
  };
  Result lastResult = Result();

public:

  ////////////////////////////////////////////////
  ///// CONSTRUCTORS

  Engine();

  ////////////////////////////////////////////////
  ///// PUBLIC

  // callback reference for sending responses to the uci ui
  void registerUCIHandler(std::shared_ptr<UCI_Handler> handler) {
    pUciHandler = std::move(handler);
  };

  // output
  std::string str() const;
  friend std::ostream &operator<<(std::ostream &os, const Engine &engine);

  // commands
  void clearHash();
  void setOption(const std::string &name, const std::string &value);
  std::string getOption(const std::string &name);
  void newGame();
  void setPosition(const std::string &fen);
  const std::shared_ptr<Position> &getPosition() const { return pPosition; }
  const std::shared_ptr<Search> &getPSearch() const { return pSearch; }
  const std::shared_ptr<SearchLimits> &getPSearchLimits() const { return pSearchLimits; }
  void doMove(const std::string &moveStr);
  void startSearch(const UCISearchMode &uciSearchMode);
  void stopSearch();
  bool isSearching();
  void ponderHit();

  // send to UCI
  void
  sendIterationEndInfo(int depth, int seldepth, Value value, uint64_t nodes, uint64_t nps,
                       MilliSec time, const MoveList &pv) const;
  void sendAspirationResearchInfo(int depth, int seldepth, Value value, const std::string& bound,
                                  uint64_t nodes, uint64_t nps, MilliSec time,
                                  const MoveList &pv) const;
  void sendCurrentRootMove(Move currmove, MoveList::size_type movenumber) const;
  void
  sendSearchUpdate(int depth, int seldepth, uint64_t nodes, uint64_t nps, MilliSec time,
                   int hashfull) const;
  void sendCurrentLine(const MoveList &moveList) const;
  void sendResult(Move bestMove, Value value, Move ponderMove);
  void sendString(const std::string &anyString) const;

  // other
  void waitWhileSearching();

  // getter
  std::shared_ptr<SearchLimits> getSearchLimits() { return pSearchLimits; };
  static int getHashSize() { return EngineConfig::hash; };
  const Result &getLastResult() const { return lastResult; }
  std::shared_ptr<Search> getSearch() const { return pSearch; }

private:

  ////////////////////////////////////////////////
  ///// PRIVATE

  void initOptions();
  void updateConfig();
  static int getInt(const std::string &value);

};

#endif //FRANKYCPP_ENGINE_H
