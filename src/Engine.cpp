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

#include <vector>
#include "misc.h"
#include "Engine.h"
#include "Position.h"
#include "SearchLimits.h"
#include "SearchConfig.h"
#include "Search.h"
#include "UCIHandler.h"
#include "UCIOption.h"
#include "MoveGenerator.h"
#include "TT.h"

#define MAP(name, option) optionVector.push_back(std::make_pair(name, option))

////////////////////////////////////////////////
///// CONSTRUCTORS

Engine::Engine() {
  pPosition = std::make_shared<Position>();
  pSearch = std::make_shared<Search>(this, EngineConfig::hash);
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
  for (const auto &it : optionVector) {
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

  // find option entry
  const auto optionIterator =
    std::find_if(optionVector.begin(), optionVector.end(),
                 [&](std::pair<std::string, UCI_Option> p) {
                   return name == p.first;
                 });

  if (optionIterator != optionVector.end()) {

    // handle commands like "Clear Hash"
    if (optionIterator->first == "Clear Hash") {
      clearHash();
      return;
    }

    // handle real options with name and value
    optionIterator->second.setCurrentValue(value);

    if (name == "Ponder") {
      EngineConfig::ponder = to_bool(optionIterator->second.getCurrentValue());
    }
    else if (name == "Use_Hash") {
      SearchConfig::USE_TT = to_bool(optionIterator->second.getCurrentValue());
    }
    else if (name == "Hash") {
      EngineConfig::hash = getInt(optionIterator->second.getCurrentValue());
      pSearch->setHashSize(EngineConfig::hash);
    }
    else if (name == "Use_AlphaBeta") {
      SearchConfig::USE_ALPHABETA = to_bool(optionIterator->second.getCurrentValue());
    }
    else if (name == "Use_PVS") {
      SearchConfig::USE_PVS = to_bool(optionIterator->second.getCurrentValue());
    }
    else if (name == "Use_Aspiration") {
      SearchConfig::USE_ASPIRATION_WINDOW = to_bool(optionIterator->second.getCurrentValue());
    }
    else if (name == "Aspiration_Depth") {
      SearchConfig::ASPIRATION_START_DEPTH = static_cast<Depth>(getInt(optionIterator->second.getCurrentValue()));
    }
    else if (name == "Use_Quiescence") {
      SearchConfig::USE_QUIESCENCE = to_bool(optionIterator->second.getCurrentValue());
    }
    else if (name == "Max_Extra_Depth") {
      SearchConfig::MAX_EXTRA_QDEPTH = static_cast<Depth>(getInt(optionIterator->second.getCurrentValue()));
    }
    else if (name == "Use_KillerMoves") {
      SearchConfig::USE_KILLER_MOVES = to_bool(optionIterator->second.getCurrentValue());
    }
    else if (name == "No_Of_Killer") {
      SearchConfig::NO_KILLER_MOVES = getInt(optionIterator->second.getCurrentValue());
    }
    else if (name == "Use_PV_Sort") {
      SearchConfig::USE_PV_MOVE_SORT = to_bool(optionIterator->second.getCurrentValue());
    }
    else if (name == "Use_MDP") {
      SearchConfig::USE_MDP = to_bool(optionIterator->second.getCurrentValue());
    }
    else if (name == "Use_MPP") {
      SearchConfig::USE_MPP = to_bool(optionIterator->second.getCurrentValue());
    }
    else if (name == "Use_Standpat") {
      SearchConfig::USE_QS_STANDPAT_CUT = to_bool(optionIterator->second.getCurrentValue());
    }
    else if (name == "Use_RFP") {
      SearchConfig::USE_RFP = to_bool(optionIterator->second.getCurrentValue());
    }
    else if (name == "RFP_Margin") {
      SearchConfig::RFP_MARGIN = static_cast<Value>(getInt(optionIterator->second.getCurrentValue()));
    }
    else if (name == "Use_NMP") {
      SearchConfig::USE_NMP = to_bool(optionIterator->second.getCurrentValue());
    }
    else if (name == "NMP_Depth") {
      SearchConfig::NMP_DEPTH = static_cast<Depth>(getInt(optionIterator->second.getCurrentValue()));
    }
    else if (name == "NMP_Reduction") {
      SearchConfig::NMP_REDUCTION = static_cast<Depth>(getInt(optionIterator->second.getCurrentValue()));
    }
    else if (name == "Use_NMPVer") {
      SearchConfig::NMP_VERIFICATION = to_bool(optionIterator->second.getCurrentValue());
    }
    else if (name == "NMPV_Reduction") {
      SearchConfig::NMP_V_REDUCTION = static_cast<Depth>(getInt(optionIterator->second.getCurrentValue()));
    }
    else if (name == "Use_EXT") {
      SearchConfig::USE_EXTENSIONS = to_bool(optionIterator->second.getCurrentValue());
    }
    else if (name == "Use_FP") {
      SearchConfig::USE_FP = to_bool(optionIterator->second.getCurrentValue());
    }
    else if (name == "FP_Margin") {
      SearchConfig::FP_MARGIN = static_cast<Value>(getInt(optionIterator->second.getCurrentValue()));
    }
    else if (name == "Use_EFP") {
      SearchConfig::USE_EFP = to_bool(optionIterator->second.getCurrentValue());
    }
    else if (name == "EFP_Margin") {
      SearchConfig::EFP_MARGIN = static_cast<Value>(getInt(optionIterator->second.getCurrentValue()));
    }
    else if (name == "Use_LMR") {
      SearchConfig::USE_LMR = to_bool(optionIterator->second.getCurrentValue());
    }
    else if (name == "LMR_Min_Depth") {
      SearchConfig::LMR_MIN_DEPTH = static_cast<Depth>(getInt(optionIterator->second.getCurrentValue()));
    }
    else if (name == "LMR_Min_Moves") {
      SearchConfig::LMR_MIN_MOVES = static_cast<Value>(getInt(optionIterator->second.getCurrentValue()));
    }
    else if (name == "LMR_Reduction") {
      SearchConfig::LMR_REDUCTION = static_cast<Depth>(getInt(optionIterator->second.getCurrentValue()));
    }

  }
  else {
    LOG__WARN(Logger::get().ENGINE_LOG, "No such option: {}", name);
  }

}

std::string Engine::getOption(const std::string &name) {
  LOG__INFO(Logger::get().ENGINE_LOG, "Engine: Get option {}", name);
  // find option entry
  const auto optionIterator =
    std::find_if(optionVector.begin(), optionVector.end(),
                 [&](std::pair<std::string, UCI_Option> p) {
                   return name == p.first;
                 });

  if (optionIterator != optionVector.end()) {
    return optionIterator->second.getCurrentValue();
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

void
Engine::sendAspirationResearchInfo(int depth, int seldepth, Value value, const std::string &bound,
                                   uint64_t nodes, uint64_t nps, MilliSec time,
                                   const MoveList &pv) const {
  if (pUciHandler) {
    pUciHandler->sendAspirationResearchInfo(depth, seldepth, value, bound, nodes, nps, time, pv);
  }
  else
    LOG__WARN(Logger::get().ENGINE_LOG,
              "<no uci handler>: Engine iteration end: depth {} seldepth {} multipv 1 {} {} nodes {} nps {} time {} pv {}",
              depth,
              seldepth,
              value,
              bound,
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

  MAP("Clear Hash",       UCI_Option("Clear Hash"));
  MAP("Use_Hash",         UCI_Option("Use_Hash",         SearchConfig::USE_TT));
  MAP("Hash",             UCI_Option("Hash",             EngineConfig::hash, 0, TT::MAX_SIZE_MB));
  MAP("Ponder",           UCI_Option("Ponder",           EngineConfig::ponder));
  MAP("Use_AlphaBeta",    UCI_Option("Use_AlphaBeta",    SearchConfig::USE_ALPHABETA));
  MAP("Use_PVS",          UCI_Option("Use_PVS",          SearchConfig::USE_PVS));
  MAP("Use_Aspiration",   UCI_Option("Use_Aspiration",   SearchConfig::USE_ASPIRATION_WINDOW));
  MAP("Aspiration_Depth", UCI_Option("Aspiration_Depth", SearchConfig::ASPIRATION_START_DEPTH, 1, DEPTH_MAX));
  MAP("Use_Quiescence",   UCI_Option("Use_Quiescence",   SearchConfig::USE_QUIESCENCE));
  MAP("Max_Extra_Depth",  UCI_Option("Max_Extra_Depth",  SearchConfig::MAX_EXTRA_QDEPTH, 1, DEPTH_MAX));
  MAP("Use_KillerMoves",  UCI_Option("Use_KillerMoves",  SearchConfig::USE_KILLER_MOVES));
  MAP("No_Of_Killer",     UCI_Option("No_Of_Killer",     SearchConfig::NO_KILLER_MOVES, 1, 9));
  MAP("Use_PV_Sort",      UCI_Option("Use_PV_Sort",      SearchConfig::USE_PV_MOVE_SORT));
  MAP("Use_MDP",          UCI_Option("Use_MDP",          SearchConfig::USE_MDP));
  MAP("Use_MPP",          UCI_Option("Use_MPP",          SearchConfig::USE_MPP));
  MAP("Use_Standpat",     UCI_Option("Use_Standpat",     SearchConfig::USE_QS_STANDPAT_CUT));
  MAP("Use_RFP",          UCI_Option("Use_RFP",          SearchConfig::USE_RFP));
  MAP("RFP_Margin",       UCI_Option("RFP_Margin",       SearchConfig::RFP_MARGIN, 0, VALUE_MAX));
  MAP("Use_NMP",          UCI_Option("Use_NMP",          SearchConfig::USE_NMP));
  MAP("NMP_Depth",        UCI_Option("NMP_Depth",        SearchConfig::NMP_DEPTH, 0, DEPTH_MAX));
  MAP("NMP_Reduction",    UCI_Option("NMP_Reduction",    SearchConfig::NMP_REDUCTION, 0, DEPTH_MAX));
  MAP("Use_NMPVer",       UCI_Option("Use_NMPVer",       SearchConfig::NMP_VERIFICATION));
  MAP("NMPV_Reduction",   UCI_Option("NMPV_Reduction",   SearchConfig::NMP_V_REDUCTION, 0, DEPTH_MAX));
  MAP("Use_EXT",          UCI_Option("Use_EXT",          SearchConfig::USE_EXTENSIONS));
  MAP("Use_FP",           UCI_Option("Use_FP",           SearchConfig::USE_FP));
  MAP("FP_Margin",        UCI_Option("FP_Margin",        SearchConfig::FP_MARGIN, 0, VALUE_MAX));
  MAP("Use_EFP",          UCI_Option("Use_EFP",          SearchConfig::USE_EFP));
  MAP("EFP_Margin",       UCI_Option("EFP_Margin",       SearchConfig::EFP_MARGIN, 0, VALUE_MAX));
  MAP("Use_LMR",          UCI_Option("Use_LMR",          SearchConfig::USE_LMR));
  MAP("LMR_Min_Depth",    UCI_Option("LMR_Min_Depth",    SearchConfig::LMR_MIN_DEPTH, 0, DEPTH_MAX));
  MAP("LMR_Min_Moves",    UCI_Option("LMR_Min_Moves",    SearchConfig::LMR_MIN_MOVES, 0, DEPTH_MAX));
  MAP("LMR_Reduction",    UCI_Option("LMR_Reduction",    SearchConfig::LMR_REDUCTION, 0, DEPTH_MAX));


  // @formatter:on
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







