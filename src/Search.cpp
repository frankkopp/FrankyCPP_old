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
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
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
#include <chrono>
#include <thread>
#include "Logging.h"
#include "Search.h"
#include "Bitboards.h"
#include "Evaluator.h"
#include "Engine.h"
#include "SearchConfig.h"
#include "Position.h"
#include "TT.h"

////////////////////////////////////////////////
///// CONSTRUCTORS

Search::Search() : Search(nullptr) {}

Search::Search(Engine* pEng) {
  pEngine = pEng;
  pEvaluator = std::make_unique<Evaluator>();

  tt = new TT;
  if (SearchConfig::USE_TT) {
    int hashSize = SearchConfig::TT_SIZE_MB;
    if (pEngine) {
      int tmp = Engine::getHashSize();
      if (tmp) { hashSize = tmp; }
    }
    tt->resize(hashSize * TT::MB);
  }
  else {
    tt->resize(0);
  }
}

Search::~Search() {
  // necessary to avoid err message:
  // terminate called without an active exception
  if (searchThread.joinable()) {
    searchThread.join();
  }
  delete tt;
}

////////////////////////////////////////////////
///// PUBLIC

void Search::startSearch(const Position &position, SearchLimits &limits) {
  if (_isRunning) {
    LOG__ERROR(Logger::get().SEARCH_LOG, "Start Search: Search already running");
    return;
  }

  // make sure we have a semaphore available
  searchSemaphore.reset();

  searchLimitsPtr = &limits;

  // join() previous thread
  if (searchThread.joinable()) {
    searchThread.join();
  }
  _stopSearchFlag = false;

  // start search in a separate thread
  LOG__DEBUG(Logger::get().SEARCH_LOG, "Starting search in separate thread.");
  searchThread = std::thread(&Search::run, this, position);

  // wait until thread is initialized before returning to caller
  initSemaphore.getOrWait();
  assert(_isRunning);
  LOG__INFO(Logger::get().SEARCH_LOG, "Search started.");
}

void Search::stopSearch() {
  if (!_isRunning) {
    LOG__WARN(Logger::get().SEARCH_LOG, "Stop search called when search was not running");
    return;
  }

  if (searchLimitsPtr->isPonder()) {
    LOG__INFO(Logger::get().SEARCH_LOG, "Stopping pondering...");
    searchLimitsPtr->ponderStop();
  }
  else if (searchLimitsPtr->isInfinite()) {
    LOG__INFO(Logger::get().SEARCH_LOG, "Stopping infinite search...");
  }
  else {
    LOG__INFO(Logger::get().SEARCH_LOG, "Stopping search...");
  }

  if (hasResult()) {
    LOG__INFO(Logger::get().SEARCH_LOG, "Search has been stopped after search has finished. Sending result");
    LOG__INFO(Logger::get().SEARCH_LOG, "Search result was: {} PV {}", lastSearchResult.str(), printMoveListUCI(pv[PLY_ROOT]));
  }

  // set stop flag - search needs to check regularly and stop accordingly
  _stopSearchFlag = true;

  // Wait for the thread to die
  if (searchThread.joinable()) {
    searchThread.join();
  }
  waitWhileSearching();

  assert(!_isRunning);
  LOG__INFO(Logger::get().SEARCH_LOG, "Search stopped.");
}

void Search::waitWhileSearching() {
  LOG__TRACE(Logger::get().SEARCH_LOG, "Wait while searching");
  if (!_isRunning) { return; }
  searchSemaphore.getOrWait();
  searchSemaphore.reset();
}

void Search::ponderhit() {
  if (searchLimitsPtr->isPonder()) {
    LOG__DEBUG(Logger::get().SEARCH_LOG, "****** PONDERHIT *******");
    if (isRunning() && !_hasResult) {
      LOG__INFO(Logger::get().SEARCH_LOG, "Ponderhit when ponder search still running. Continue searching.");
      // if time based game setup the time limits
      if (searchLimitsPtr->isTimeControl()) {
        LOG__INFO(Logger::get().SEARCH_LOG, "Time Management: {} Time limit: {:n}", (searchLimitsPtr->isTimeControl() ? "ON" : "OFF"), timeLimit);
      }
    }
    else if (isRunning() && _hasResult) {
      LOG__INFO(Logger::get().SEARCH_LOG, "Ponderhit when ponder search already ended. Sending result.");
      LOG__INFO(Logger::get().SEARCH_LOG, "Search Result: {}", lastSearchResult.str());
    }
    // continue searching or send result (done in run())
    startTime = now();
    configureTimeLimits();
    startTimer();
    searchLimitsPtr->ponderHit();
  }
  else {
    LOG__WARN(Logger::get().SEARCH_LOG, "Ponderhit when not pondering!");
  }
}

////////////////////////////////////////////////
///// PRIVATE

/**
 * Called when the new search thread is started.
 * Initializes the search.
 * Calls <code>iterativeDeepening()</code> when search is initialized.
 * <p>
 * After the search has stopped calls
 * <code>Engine.sendResult(searchResult)</code> to store/hand over the result.
 * After storing the result the search is ended and the thread terminated.<br>
 * @param position the position to be searched given as value (copied)
 */
void Search::run(Position position) {
  LOG__TRACE(Logger::get().SEARCH_LOG, "Search thread started.");

  // get the search lock
  searchSemaphore.getOrWait();
  _isRunning = true;
  _hasResult = false;

  // Initialize for new search
  myColor = position.getNextPlayer();
  lastSearchResult = SearchResult();
  timeLimit = extraTime = 0;
  searchStats = SearchStats();

  // store the start time of the search
  startTime = lastUciUpdateTime = now();
  // if time based game setup the soft and hard time limits
  if (searchLimitsPtr->isTimeControl()) {
    configureTimeLimits();
    startTimer();
  }

  // Initialize ply based data
  // Each depth in search gets it own global field to avoid object creation
  // during search.
  for (int i = DEPTH_NONE; i < DEPTH_MAX; i++) {
    moveGenerators[i] = MoveGenerator();
    pv[i].clear();
    mateThreat[i] = false;
  }

  // age TT entries
  tt->ageEntries();

  // search mode
  if (searchLimitsPtr->isPerft()) {
    LOG__INFO(Logger::get().SEARCH_LOG, "Search Mode: PERFT SEARCH ({})", searchLimitsPtr->getMaxDepth());
  }
  if (searchLimitsPtr->isInfinite()) {
    LOG__INFO(Logger::get().SEARCH_LOG, "Search Mode: INFINITE SEARCH");
  }
  if (searchLimitsPtr->isPonder()) {
    LOG__INFO(Logger::get().SEARCH_LOG, "Search Mode: PONDER SEARCH");
  }
  if (searchLimitsPtr->getMate()) {
    LOG__INFO(Logger::get().SEARCH_LOG, "Search Mode: MATE SEARCH ({})", searchLimitsPtr->getMate());
  }

  // initialization done
  initSemaphore.release();

  // ###########################################################################
  // start iterative deepening
  lastSearchResult = iterativeDeepening(position);
  // ###########################################################################

  _hasResult = true;

  // if we arrive here and the search is not stopped it means that the search
  // was finished before it has been stopped (by stopSearchFlag or ponderhit)
  if (!_stopSearchFlag &&
      (searchLimitsPtr->isPonder() || searchLimitsPtr->isInfinite())) {
    LOG__INFO(Logger::get().SEARCH_LOG, "Search finished before stopped or ponderhit! Waiting for stop/ponderhit to send result");
    while (!_stopSearchFlag &&
           (searchLimitsPtr->isPonder() || searchLimitsPtr->isInfinite())) {
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
  }

  sendResultToEngine();

  // print result of the search
  LOG__INFO(Logger::get().SEARCH_LOG, "Search statistics: {}", searchStats.str());
  if (SearchConfig::USE_TT) { LOG__INFO(Logger::get().SEARCH_LOG, tt->str()); }
  LOG__INFO(Logger::get().SEARCH_LOG, "Search Depth was {} ({})", searchStats.currentSearchDepth, searchStats.currentExtraSearchDepth);
  LOG__INFO(Logger::get().SEARCH_LOG, "Search took {},{:03} sec ({:n} nps)",
            (searchStats.lastSearchTime % 1'000'000) / 1'000,
            (searchStats.lastSearchTime % 1'000),
            (searchStats.nodesVisited * 1'000) /
            (searchStats.lastSearchTime + 1));

  // check perft and print result
  if (searchLimitsPtr->isPerft()) {
    uint64_t perftResults[] = {0,
                               20,              // 1
                               400,             // 2
                               8'902,           // 3
                               197'281,         // 4
                               4'865'609,       // 5
                               119'060'324,     // 6
                               3'195'901'860,   // 7
                               84'998'978'956}; // 8

    if (searchStats.leafPositionsEvaluated == perftResults[searchLimitsPtr->getDepth()]) {
      LOG__INFO(Logger::get().SEARCH_LOG, "Perft test successful: {:n} leaf nodes at depth {}",
                searchStats.leafPositionsEvaluated, searchLimitsPtr->getDepth());
      sendStringToEngine(
        fmt::format("Perft test successful: {:n} leaf nodes at depth {}",
                    searchStats.leafPositionsEvaluated, searchLimitsPtr->getDepth()));
    }
    else {
      LOG__ERROR(Logger::get().SEARCH_LOG,
                 "Perft test failed: {:n} leaf nodes at depth {} - should have been {:n}",
                 searchStats.leafPositionsEvaluated, searchLimitsPtr->getDepth(),
                 perftResults[searchLimitsPtr->getDepth()]);
      sendStringToEngine(
        fmt::format("Perft test failed: {:n} leaf nodes at depth {} - should have been {:n}",
                    searchStats.leafPositionsEvaluated, searchLimitsPtr->getDepth(),
                    perftResults[searchLimitsPtr->getDepth()]));
    }
  }

  if (timerThread.joinable()) { timerThread.join(); }
  _isRunning = false;
  searchSemaphore.reset();
  LOG__TRACE(Logger::get().SEARCH_LOG, "Search thread ended.");
}

/**
 * Generates root moves and calls search in a loop increasing depth
 * with each iteration.
 *
 * Detects mate if started on a mate position.
 * @param position
 * @return a SearchResult
 */
SearchResult Search::iterativeDeepening(Position &position) {

  // prepare search result
  SearchResult searchResult = SearchResult();

  // check repetition and 50 moves
  if (checkDrawRepAnd50<ROOT>(position)) {
    LOG__WARN(Logger::get().SEARCH_LOG, "Search called when DRAW by Repetition or 50-moves-rule");
    searchResult.bestMove = MOVE_NONE;
    searchResult.bestMoveValue = VALUE_DRAW;
    return searchResult;
  }

  // no legal root moves - game already ended!
  if (!MoveGenerator::hasLegalMove(position)) {
    if (position.hasCheck()) {
      searchResult.bestMove = MOVE_NONE;
      searchResult.bestMoveValue = -VALUE_CHECKMATE;
      LOG__WARN(Logger::get().SEARCH_LOG, "Search called on a CHECKMATE position");
    }
    else {
      searchResult.bestMove = MOVE_NONE;
      searchResult.bestMoveValue = VALUE_DRAW;
      LOG__WARN(Logger::get().SEARCH_LOG, "Search called on a STALEMATE position");
    }
    return searchResult;
  }

  Depth iterationDepth = searchLimitsPtr->getStartDepth();

  // generate all legal root moves
  rootMoves = generateRootMoves(position);

  // print search setup for debugging
  LOG__INFO(Logger::get().SEARCH_LOG, "Searching in position: {}", position.printFen());
  LOG__DEBUG(Logger::get().SEARCH_LOG, "Root moves: {}", printMoveList(rootMoves));
  LOG__INFO(Logger::get().SEARCH_LOG, "Searching these moves: {}", printMoveList(rootMoves));
  LOG__INFO(Logger::get().SEARCH_LOG, "Search mode: {}", searchLimitsPtr->str());
  LOG__INFO(Logger::get().SEARCH_LOG, "Time Management: {} time limit: {:n}", (searchLimitsPtr->isTimeControl() ? "ON" : "OFF"), timeLimit);
  LOG__INFO(Logger::get().SEARCH_LOG, "Start Depth: {} Max Depth: {}", iterationDepth, searchLimitsPtr->getMaxDepth());
  LOG__DEBUG(Logger::get().SEARCH_LOG, "Starting iterative deepening now...");

  // max window search - preparation for aspiration window search
  Value alpha = VALUE_MIN;
  Value beta = VALUE_MAX;

  // check search requirements
  assert(!rootMoves.empty() && "No root moves to search");
  assert(iterationDepth > 0 && "iterationDepth <= 0");

  // ###########################################
  // ### BEGIN Iterative Deepening
  do {
    LOG__TRACE(Logger::get().SEARCH_LOG, "Iteration Depth {} START", iterationDepth);

    currentIterationDepth = iterationDepth;
    searchStats.currentSearchDepth = static_cast<Ply>(iterationDepth);
    if (searchStats.currentExtraSearchDepth < static_cast<Ply>(iterationDepth)) {
      searchStats.currentExtraSearchDepth = static_cast<Ply>(iterationDepth);
    }
    searchStats.bestMoveChanges = 0;
    searchStats.nodesVisited++;

    // protect the TT from being resized or cleared during search
    tt_lock.lock();

    // ###########################################
    // ### CALL SEARCH for iterationDepth
    if (searchLimitsPtr->isPerft()) {
      search<PERFT, PV>(position, iterationDepth, PLY_ROOT, alpha, beta, Do_Null_Move);
    }
    else {
      search<ROOT, PV>(position, iterationDepth, PLY_ROOT, alpha, beta, Do_Null_Move);
    }
    // ###########################################

    // release lock on TT
    tt_lock.unlock();

    // check the result  - we should have a result at his point
    if (!_stopSearchFlag && !searchLimitsPtr->isPerft()) {
      if (pv[PLY_ROOT].empty() || pv[PLY_ROOT].at(0) == MOVE_NONE) {
        LOG__ERROR(Logger::get().SEARCH_LOG, "{}:{} Best root move missing after iteration: pv[0] size {}", __func__, __LINE__, pv[PLY_ROOT].size());
      }
      if (!pv[PLY_ROOT].empty() && valueOf(pv[PLY_ROOT].at(0)) == VALUE_NONE) {
        LOG__ERROR(Logger::get().SEARCH_LOG, "{}:{} Best root move has no value after iteration (pv size={})", __func__, __LINE__, pv[PLY_ROOT].size());
      }
    }

    // break on stop signal or time
    if (stopConditions()) { break; }

    // sort root moves based on value for the next iteration
    std::stable_sort(rootMoves.begin(), rootMoves.end(), rootMovesSort);

    // update UCI GUI
    sendIterationEndInfoToEngine();

    LOG__TRACE(Logger::get().SEARCH_LOG, "Iteration Depth={} END", iterationDepth);

  } while (++iterationDepth <= searchLimitsPtr->getMaxDepth());
  // ### END OF Iterative Deepening
  // ###########################################

  // check the result  - we should have a result at his point
  if (!searchLimitsPtr->isPerft()) {
    if (pv[PLY_ROOT].empty() || pv[PLY_ROOT].at(0) == MOVE_NONE) {
      LOG__ERROR(Logger::get().SEARCH_LOG, "{}:{} Best root move missing after search: pv[0] size {}", __func__, __LINE__, pv[PLY_ROOT].size());
    }
    if (!pv[PLY_ROOT].empty() && valueOf(pv[PLY_ROOT].at(0)) == VALUE_NONE) {
      LOG__ERROR(Logger::get().SEARCH_LOG, "{}:{}Best root move has no value!( pv size={}", __func__, __LINE__, pv[PLY_ROOT].size());
    }
  }

  // update searchResult here
  searchResult.bestMove = pv[PLY_ROOT].empty() ? MOVE_NONE : pv[PLY_ROOT].at(0);
  searchResult.bestMoveValue = pv[PLY_ROOT].empty() ? VALUE_NONE : valueOf(pv[PLY_ROOT].at(0));
  searchResult.ponderMove = pv[PLY_ROOT].size() > 1 ? pv[PLY_ROOT].at(1) : MOVE_NONE;
  searchResult.depth = searchStats.currentSearchDepth;
  searchResult.extraDepth = searchStats.currentExtraSearchDepth;

  // search is finished - stop timer
  stopTime = now();
  searchStats.lastSearchTime = elapsedTime(startTime, stopTime);

  return searchResult;
}

/**
 * This is the templated search function for root, non-root and quiescence
 * searches. Through the template the compiler will generate specialized
 * versions of this method for each case.
 */
template<Search::Search_Type ST, Search::Node_Type NT>
Value Search::search(Position &position, Depth depth, Ply ply, Value alpha,
                     Value beta, Do_Null doNull) {

  assert(alpha >= VALUE_MIN && beta <= VALUE_MAX && "alpha/beta out if range");
  LOG__TRACE(Logger::get().SEARCH_LOG, "{:>{}}Search {} in ply {} for depth {}: START alpha={} beta={} currline={}", "", ply, (ST == ROOT ? "ROOT" : ST == NONROOT ? "NONROOT" : "QUIESCENCE"), ply, depth, alpha, beta, printMoveListUCI(currentVariation));

  // Check if search should be stopped
  if (stopConditions()) { return VALUE_NONE; }

  // Leaf node handling
  switch (ST) {
    case ROOT: // fall through
    case NONROOT:
      if (depth <= DEPTH_NONE || ply >= PLY_MAX - 1) {
        if (SearchConfig::USE_QUIESCENCE) {
          return search<QUIESCENCE, NT>(position, depth, ply, alpha, beta, doNull);
        }
        else {
          Value eval = evaluate(position);
          LOG__TRACE(Logger::get().SEARCH_LOG, "{:>{}} Evaluation: {} {}", " ", ply, printMoveListUCI(currentVariation), eval);
          return eval;
        }
      }
      break;

    case QUIESCENCE:
      // limit max quiescence depth
      if (ply > static_cast<Ply>(currentIterationDepth + SearchConfig::MAX_EXTRA_QDEPTH)
          || ply >= PLY_MAX - 1) {
        Value eval = evaluate(position);
        LOG__TRACE(Logger::get().SEARCH_LOG, "{:>{}} Evaluation: {} {}", " ", ply, printMoveListUCI(currentVariation), eval);
        return eval;
      }
      if (searchStats.currentExtraSearchDepth < ply) {
        searchStats.currentExtraSearchDepth = ply;
      }
      break;

    case PERFT:
      if (depth <= DEPTH_NONE || ply >= PLY_MAX - 1) {
        Value eval = evaluate(position);
        LOG__TRACE(Logger::get().SEARCH_LOG, "{:>{}} Evaluation: {} {}", " ", ply, printMoveListUCI(currentVariation), eval);
        return eval;
      }
  }

  // ###############################################
  // Mate Distance Pruning
  // Did we already find a shorter mate then ignore
  // this one.
  if (SearchConfig::USE_MDP && ST != ROOT && ST != PERFT) {
    if (alpha < -VALUE_CHECKMATE + ply) { alpha = -VALUE_CHECKMATE + ply; }
    if (beta > VALUE_CHECKMATE - ply) { beta = VALUE_CHECKMATE - ply; }
    if (alpha >= beta) {
      assert(isCheckMateValue(alpha));
      searchStats.mateDistancePrunings++;
      LOG__TRACE(Logger::get().SEARCH_LOG, "{:>{}}Search in ply %d for depth %d: MDP CUT", "", ply, ply, depth);
      return alpha;
    }
  }
  // ###############################################

  // prepare node search
  Value bestNodeValue = VALUE_NONE;
  Move ttStoreMove = MOVE_NONE;
  Move ttMove = MOVE_NONE;
  Value_Type ttType = TYPE_ALPHA;
  moveGenerators[ply].resetOnDemand();
  if (ST == ROOT || (ST == PERFT && ply == PLY_ROOT)) { currentMoveIndex = 0; }
  else { pv[ply].clear(); }

  // ###############################################
  // TT Lookup
  const TT::Entry* ttEntryPtr = nullptr;
  if (SearchConfig::USE_TT &&
      (SearchConfig::USE_TT_QSEARCH || ST != QUIESCENCE)
      && ST != PERFT
      && ST != ROOT
    ) {
    /* TT PROBE
     *  If this is a PV node and value is an EXACT value of a fully
     *  searched node we can cut off the search immediately.
     *  In non PV nodes the value needs to be EXACT or outside
     *  of our current alpha/beta bounds with the corresponding
     *  type stored in the TT for a cut. (for ALPHA value<=alpha,
     *  for BETA value>=beta)

     *  We might have a best move to try first from previous searches of this
     *  node (e.g. lower or equal depth, beta cut off).
     *  We also might have a mateThreat flag from previous null move searches
     *  we can use.
     */
    ttEntryPtr = tt->probe(position.getZobristKey());
    if (ttEntryPtr) {
      ttMove = ttEntryPtr->move;
      mateThreat[ply] = ttEntryPtr->mateThreat;
      // use value only if tt depth was equal or deeper
      if (ttEntryPtr->depth >= depth) {
        assert(ttEntryPtr->value != VALUE_NONE);
        Value ttValue = valueFromTT(ttEntryPtr->value, ply);
        // determine if we can cut based on tt value
        bool cut = false;
        if (ttEntryPtr->type == TYPE_EXACT) {
          cut = true;
        }
        else if (NT == NonPV) {
          if (ttEntryPtr->type == TYPE_ALPHA && ttValue <= alpha) {
            cut = true;
          }
          else if (ttEntryPtr->type == TYPE_ALPHA && ttValue < beta) {
            // should actually not happen
            LOG__ERROR(Logger::get().SEARCH_LOG, "TT ALPHA type smaller beta - should not happen");
            beta = ttValue;
          }
          else if (ttEntryPtr->type == TYPE_BETA && ttValue >= beta) {
            cut = true;
          }
          else if (ttEntryPtr->type == TYPE_BETA && ttValue > alpha) {
            // should actually not happen
            LOG__ERROR(Logger::get().SEARCH_LOG, "TT BETA type greater alpha - should not happen");
            alpha = ttValue;
          }
        }
        if (cut) {
          getPVLine(position, pv[ply], depth);
          searchStats.tt_Cuts++;
          return ttValue;
        }
        else {
          searchStats.tt_NoCuts++;
        }
      }
    }
  }
  // End TT Lookup
  // ###############################################

  // if we are not in check we allow prunings and search tree reductions
  if (!position.hasCheck() && ST != PERFT) {

    // get an evaluation for the position
    Value staticEval = evaluate(position);

    // ###############################################
    // Quiescence StandPat
    // Use evaluation as a standing pat (lower bound)
    // https://www.chessprogramming.org/Quiescence_Search#Standing_Pat
    // Assumption is that there is at least on move which would improve the
    // current position. So if we are already >beta we don't need to look at it.
    if (SearchConfig::USE_QS_STANDPAT_CUT
        && ST == QUIESCENCE
      ) {
      if (staticEval >= beta) {
        if (SearchConfig::USE_TT_QSEARCH) {
          storeTT(position, staticEval, TYPE_BETA, DEPTH_NONE, ply, MOVE_NONE,
                  mateThreat[ply]);
        }
        LOG__TRACE(Logger::get().SEARCH_LOG, "{:>{}}Quiescence in ply {}: STANDPAT CUT ({} > {} beta)", "", ply, ply, staticEval, beta);
        searchStats.qStandpatCuts++;
        return staticEval; // fail-hard: beta, fail-soft: statEval
      }
      if (staticEval > alpha) {
        alpha = staticEval;
      }
      bestNodeValue = staticEval;
      LOG__TRACE(Logger::get().SEARCH_LOG, "{:>{}}Quiescence in ply {}: STANDPAT {}", "", ply, ply, staticEval);
    }
    // ###############################################

    // ###############################################
    // FORWARD PRUNING BETA

    // ###############################################
    // NULL MOVE PRUNING
    // https://www.chessprogramming.org/Null_Move_Pruning
    // Under the assumption the in most chess position it would be better
    // do make a move than to not make a move we can assume that if
    // our positional value after a null move is already above beta (>beta)
    // it would be above beta when doing a move in any case.
    // Certain situations need to be considered though:
    // - Zugzwang - it would be better not to move
    // - in check - this would lead to an illegal situation where the king is
    // captured
    // - recursive null moves should be avoided
    if (SearchConfig::USE_NMP && ply > 1        // start with my color
        && NT == NonPV
        && depth >= SearchConfig::NMP_DEPTH     // don't do it too close to leaf nodes
        && doNull                               // don't do recursive null moves
        && position.getMaterialNonPawn(position.getNextPlayer()) // to avoid Zugzwang
        && ST == NONROOT // NMP will be removed from the compiler for qsearch
      ) {

      Depth newDepth = depth - SearchConfig::NMP_REDUCTION;

      // do a null move search with a null window
      position.doNullMove();
      Value nullValue =
        -search<NONROOT, NonPV>(position, newDepth, ply + 1, -beta, -beta + 1, No_Null_Move);
      position.undoNullMove();

      if (SearchConfig::NMP_VERIFICATION
          && depth > SearchConfig::NMP_V_REDUCTION
          && nullValue >= beta) {
        searchStats.nullMoveVerifications++;
        newDepth = depth - SearchConfig::NMP_V_REDUCTION;
        // confirm >beta by doing a shallow normal search on the position
        nullValue = search<NONROOT, PV>(position, newDepth, ply, alpha, beta, No_Null_Move);
      }

      // Check for mate threat and do not return an unproven mate value
      if ((mateThreat[ply] = isCheckMateValue(nullValue))) nullValue = VALUE_CHECKMATE_THRESHOLD;;

      if (nullValue >= beta) { // cut off node
        searchStats.nullMovePrunings++;
        LOG__TRACE(Logger::get().SEARCH_LOG, "{:>{}}Search in ply {} for depth {}: NULL CUT", "", ply, ply, depth);
        storeTT(position, nullValue, TYPE_BETA, newDepth, ply, MOVE_NONE, mateThreat[ply]);
        return nullValue;
      }
    }
    // ###############################################

  } // not check and not perft
  // ###############################################

  // FORWARD PRUNING BETA
  // ###############################################

  // ###############################################
  // IID
  // If we are here without a ttMove to search first
  // we try to find a good move to try first by doing
  // a shallow search. This is most effective with bad
  // move ordering. If move ordering is quite good
  // this might be a waste of search time.
  if (SearchConfig::USE_IID
    && ST != PERFT
    && ST != QUIESCENCE
    && NT == PV
    && ttMove == MOVE_NONE
    && depth > 4
    ) {
    searchStats.iidSearches++;
    const Depth iidDepth = depth - SearchConfig::IID_REDUCTION;
    if (iidDepth <= DEPTH_NONE) { ;
      search<QUIESCENCE, PV>(position, iidDepth, ply, alpha, beta, doNull);
    }
    else {
      search<NONROOT, PV>(position, iidDepth, ply, alpha, beta, doNull);
    }

    const TT::Entry* iidTTEntryPtr = tt->probe(position.getZobristKey());
    if (iidTTEntryPtr != nullptr && iidTTEntryPtr->move != MOVE_NONE) {
      ttMove = iidTTEntryPtr->move;
      LOG__DEBUG(Logger::get().SEARCH_LOG, "{:>{}}Search in ply {} for depth {}: IID SUCCESS: ttMove={}", "", ply, ply, depth, printMoveVerbose(ttMove));
    } else {
      LOG__DEBUG(Logger::get().SEARCH_LOG, "{:>{}}Search in ply {} for depth {}: IID FAILED", "", ply, ply, depth);
      if (pv[ply].empty()) {
        LOG__DEBUG(Logger::get().SEARCH_LOG, "{:>{}}Search in ply {} for depth {}: IID PV FAILED", "", ply, ply, depth);
      }
      else {
        ttMove = pv[ply][0];
        LOG__DEBUG(Logger::get().SEARCH_LOG, "{:>{}}Search in ply {} for depth {}: IID PV SUCCESS: ttMove={}", "", ply, ply, depth, printMoveVerbose(ttMove));
      };

    }
  }
  // IID
  // ###############################################


  // ###############################################
  // PV MOVE SORT
  // make sure the pv move is returned first by the move generator
  if (SearchConfig::USE_PV_MOVE_SORT && ST != ROOT && ST != PERFT) {
    if (ttMove != MOVE_NONE) {
      assert(moveGenerators[ply].validateMove(position, ttMove));
      moveGenerators[ply].setPV(ttMove);
      searchStats.pv_sortings++;
    }
    else {
      searchStats.no_moveForPVsorting++;
    }
  }
  // ###############################################

  // prepare move loop
  Value value = VALUE_NONE;
  Move move = MOVE_NONE;
  int movesSearched = 0; // to detect mate situations
  int moveNumber = 0; // to count where cutoffs take place

  // ###########################################################################
  // MOVE LOOP
  while ((move = getMove<ST>(position, ply)) != MOVE_NONE) {

    if (ST == ROOT) { LOG__TRACE(Logger::get().SEARCH_LOG, "Root Move {} START", printMove(move)); }
    else { LOG__TRACE(Logger::get().SEARCH_LOG, "{:>{}}Depth {} cv {} move {} START", "", ply, ply, printMoveListUCI(currentVariation), printMove(move)); }

    // reduce number of moves searched in quiescence
    // by looking at good captures only
    if (ST == QUIESCENCE
        && !position.hasCheck()
        && !goodCapture(position, move)) {
      continue;
    }

    // ###############################################
    // Minor Promotion Pruning
    // Skip non queen or knight promotion as they are
    // redundant. Exception would be stale mate situations
    // which we ignore.
    if (SearchConfig::USE_MPP
        && ST != ROOT
        && ST != PERFT
        && typeOf(move) == PROMOTION
        && promotionType(move) != QUEEN
        && promotionType(move) != KNIGHT) {
      searchStats.minorPromotionPrunings++;
      LOG__TRACE(Logger::get().SEARCH_LOG, "{:>{}}Search in ply {} for depth {}: Move {} MPP CUT", "", ply, ply, depth, printMove(move));
      continue;
    }
    // ###############################################

    // ###############################################
    // EXTENSIONS
    Depth extension = DEPTH_NONE;
    if (SearchConfig::USE_EXTENSIONS
      && ST != QUIESCENCE
      && depth <= DEPTH_FRONTIER // to limit search extensions and avoid search explosion
    ) {
      if ( // position has check is implicit in quiescence
        // move gives check
        position.givesCheck(move)
        // move is close to promotion
        || (typeOf(position.getPiece(getFromSquare(move))) == PieceType::PAWN
            && (position.getNextPlayer() == WHITE
                ? rankOf(getToSquare(move)) == RANK_7   // WHITE
                : rankOf(getToSquare(move)) == RANK_2)) // BLACK
        // promotion
        || typeOf(move) == MoveType::PROMOTION
        || mateThreat[ply] // mate threat from null move search or TT
        // Recapture?
        // Single Reply?
        // Pawn Endgame?
        ) {
        ++extension;
        searchStats.extensions++;
        LOG__TRACE(Logger::get().SEARCH_LOG, "{:>{}}Search in ply {} for depth {}: EXTENSION Move: {} ST={} NT={} mate={} castling={} prom={} preprom={} givecheck={}",
                   "", ply, ply, depth, printMoveVerbose(move), ST, NT, mateThreat[ply], typeOf(move) == MoveType::CASTLING, typeOf(move) == MoveType::PROMOTION, (typeOf(position.getPiece(getFromSquare(move))) == PieceType::PAWN && (position.getNextPlayer() == WHITE ? rankOf(getToSquare(move)) == RANK_7 : rankOf(getToSquare(move)) == RANK_2)), position.givesCheck(move));
      }
    }
    // EXTENSIONS
    // ###############################################

    // ###############################################
    // FORWARD PRUNING ALPHA
    // Avoid making the move on the position if we can
    // deduct that it is not worth examining.
    // Will not be done when in a pvNode search or when
    // already any search extensions has been determined.
    // Also not when in check.
    if (NT != PV
        && !position.hasCheck()
        && !extension
      ) {

    }

    // FORWARD PRUNING ALPHA
    // ###############################################

    // ###############################################
    // Execute move
    position.doMove(move);
    TT_PREFETCH;   // if available tell the cpu prefetch the tt entry into cpu cache
    EVAL_PREFETCH;
    searchStats.nodesVisited++;
    if (position.isLegalPosition()) {
      currentVariation.push_back(move);
      sendSearchUpdateToEngine();


      // check for repetition or 50-move-rule draws
      if (checkDrawRepAnd50<ST>(position)) {
        value = VALUE_DRAW;
      }
      else {

        // ROOT is used only at the start - changes directly to NONROOT
        const Search::Search_Type nextST = ST == ROOT ? NONROOT : ST;

        // reduce depth by 1 in the next search and add extension for this move
        Depth newDepth = depth - DEPTH_ONE + extension;

        // in quiescence we do not have depth any more
        if (ST == QUIESCENCE || newDepth < DEPTH_NONE) newDepth = DEPTH_NONE;

        if (!SearchConfig::USE_PVS || movesSearched == 0 || ST == PERFT) {
          // AlphaBeta Search or initial search in PVS
          value = -search<nextST, PV>(position, newDepth, ply + 1, -beta, -alpha, doNull);
        }
        else {
          // #############################
          // PVS Search /START
          value = -search<nextST, NonPV>(position, newDepth, ply + 1, -alpha - 1, -alpha, doNull);
          if (value > alpha && value < beta && !stopConditions()) {
            if (ST == ROOT) { searchStats.pvs_root_researches++; }
            else { searchStats.pvs_researches++; }
            value = -search<nextST, PV>(position, newDepth, ply + 1, -beta, -alpha, doNull);
          }
          else {
            if (ST == ROOT) { searchStats.pvs_root_cutoffs++; }
            else { searchStats.pvs_cutoffs++; }
          }
          // PVS Search /END
          // #############################
        }
      }
      assert((value != VALUE_NONE || _stopSearchFlag) && "Value should not be NONE at this point.");

      movesSearched++;
      currentVariation.pop_back();
    } // if (position.isLegalPosition())
    position.undoMove();
    //  ###############################################

    if (stopConditions()) { return VALUE_NONE; }

    // For root moves encode value into the move
    // so we can sort the move before the next iteration
    if (ST == ROOT) {
      setValue(rootMoves.at(currentMoveIndex++), value);
    }

    // In PERFT we can ignore values and pruning
    if (ST == PERFT) {
      if (ply == PLY_ROOT) { currentMoveIndex++; }
      value = VALUE_ONE;
      continue;
    }

    // Did we find a better move for this node (not ply)?
    // For the first move this is always the case.
    if (value > bestNodeValue) {

      // these are only valid for this node
      // not for all of the ply (not yet clear if >alpha)
      bestNodeValue = value;

      if (ST == ROOT) {
        searchStats.bestMoveChanges++;
        searchStats.bestMoveDepth = depth;
      }

      // AlphaBeta
      if (SearchConfig::USE_ALPHABETA) {

        /*
        Did we find a better move than in previous nodes in ply
        then this is our new PV and best move for this ply.
        If we never find a better alpha this means all moves in
        this node are worse then other moves in other nodes which
        raised alpha - meaning we have a better move from another
        node we would play. We will return alpha and store a alpha
        node in TT with no best move for TT.
        */
        if (value > alpha) {
          ttStoreMove = move;

          /*
           If we found a move that is better or equal than beta
           this means that the opponent can/will avoid this
           position altogether so we can stop search this node.
           We will not know if our best move is really the
           best move or how good it really is (value is an lower bound)
           as we cut off the rest of the search of the node here.
           We will safe the move as a killer to be able to search it
           earlier in another node of the ply.
          */
          if (value >= beta) {
            if (SearchConfig::USE_KILLER_MOVES && !position.isCapturingMove(move)) {
              moveGenerators[ply].storeKiller(move, SearchConfig::NO_KILLER_MOVES);
            }
            searchStats.prunings++;
            searchStats.betaCutOffs[moveNumber]++;
            ttType = TYPE_BETA; // store the beta value into the TT later
            LOG__TRACE(Logger::get().SEARCH_LOG, "{:>{}}Search in ply {} for depth {}: CUT NODE {} >= {} (beta)", "", ply, ply, depth, value, beta);
            break; // get out of loop and return the value at the end
          }
          else {
            /*
            We found a move between alpha and beta which means we
            really have found the best move so far in the ply which
            can be forced (opponent can't avoid it).
            We raise alpha so the successive searches in this ply
            need to find even better moves or dismiss the moves.
            */
            searchStats.alphaImprovements[moveNumber]++;
            alpha = value;
            ttType = TYPE_EXACT;
            setValue(ttStoreMove, bestNodeValue);
            savePV(ttStoreMove, pv[ply + 1], pv[ply]);
            LOG__TRACE(Logger::get().SEARCH_LOG, "{:>{}}Search in ply {} for depth {}: NEW PV {} ({}) (alpha) PV: {}", "", ply, ply, depth, printMove(move), value, printMoveListUCI(pv[ply]));
          }
        }
      } // AlphaBeta
      else { // Minimax
        setValue(move, value);
        savePV(move, pv[ply + 1], pv[ply]);
        ttType = TYPE_EXACT;
        LOG__TRACE(Logger::get().SEARCH_LOG, "{:>{}}Search in ply {} for depth {}: NEW PV {} ({}) PV: {}", "", ply, ply, depth, printMove(move), value, printMoveListUCI(pv[ply]));
      }
    }

    if (ST == ROOT) {
      LOG__TRACE(Logger::get().SEARCH_LOG, "Root Move {} END", printMove(move));
    }
    else {
      LOG__TRACE(Logger::get().SEARCH_LOG, "{:>{}}Depth {} cv {} move {} END", " ", ply, ply, printMoveListUCI(currentVariation), printMove(move));
    }

    moveNumber++;
  }
  // ##### Iterate through all available moves
  // ###########################################################################

  // do some checks
  ASSERT_START
    if (ST != PERFT && SearchConfig::USE_ALPHABETA) {
      // In an EXACT node we should have a best move and a PV
      if (ttType == TYPE_EXACT) {
        assert(ttStoreMove);
        assert(!pv[ply].empty());
        assert(alpha <= bestNodeValue && bestNodeValue <= beta);
      }

      // In a BETA node we should have a best move for the TT (might not be the
      // best due to cut off)
      if (ttType == TYPE_BETA) {
        assert(ttStoreMove);
        assert(bestNodeValue >= beta);
      }

      // We should not have found a best move in an ALPHA node (all moves were
      // worse than alpha)
      if (ttType == TYPE_ALPHA) {
        assert(ttStoreMove == MOVE_NONE);
        assert(bestNodeValue <= alpha);
      }
    }
  ASSERT_END

  // if we did not have at least one legal move
  // then we might have a mate or in quiescence
  // only quite moves
  if (!movesSearched && !stopConditions()) {
    searchStats.nonLeafPositionsEvaluated++;
    assert(ttType == TYPE_ALPHA);
    LOG__TRACE(Logger::get().SEARCH_LOG, "{:>{}}Depth {} cv {} NO LEGAL MOVES", "", ply, ply, printMoveListUCI(currentVariation));
    if (position.hasCheck()) {
      /* If the position has check we have a mate even in
         quiescence as we will have generated all moves
         because of the check. Return a -CHECKMATE. */
      bestNodeValue = -VALUE_CHECKMATE + ply;
      ttType = TYPE_EXACT;
      assert(ttStoreMove == MOVE_NONE);
      LOG__TRACE(Logger::get().SEARCH_LOG, "{:>{}} Search in ply {} for depth {}: {} CHECKMATE", " ", ply, ply, depth, bestNodeValue);
    }
    else if (ST != QUIESCENCE) {
      /* If not in quiescence we have a stale mate.
         Return the draw value. */
      bestNodeValue = VALUE_DRAW;
      ttType = TYPE_EXACT;
      assert(ttStoreMove == MOVE_NONE);
      LOG__TRACE(Logger::get().SEARCH_LOG, "{:>{}} Search in ply {} for depth {}: {} STALEMATE", " ", ply, ply, depth, bestNodeValue);
    }
    /* In quiescence having searched no moves while
       not in check means that there were only quiet
       moves which we ignored on purpose and return
       the StandPat */
  }

  LOG__TRACE(Logger::get().SEARCH_LOG, "{:>{}}Search {} in ply {} for depth {}: END value={} ({} moves searched) ({})",
             "", ply, (ST == ROOT ? "ROOT" : ST == NONROOT ? "NONROOT" : "QUIESCENCE"), ply, depth,
             bestNodeValue, movesSearched, printMoveListUCI(currentVariation));

  // best value should in any case not be VALUE_NONE any more
  assert(ST == PERFT || (bestNodeValue >= VALUE_MIN && bestNodeValue <= VALUE_MAX && "bestNodeValue should not be MIN/MAX here"));

  // store TT data
  switch (ST) {
    case NONROOT:
      if (SearchConfig::USE_TT) {
        LOG__TRACE(Logger::get().SEARCH_LOG, "{:>{}}Search storing into TT: {} {} {} {} {} {} {}", "",
                   ply, position.getZobristKey(), bestNodeValue, TT::str(ttType),
                   depth, printMove(ttStoreMove), false, position.printFen());
        storeTT(position, bestNodeValue, ttType, depth, ply, ttStoreMove, mateThreat[ply]);
      }
      break;
    case QUIESCENCE:
      if (SearchConfig::USE_TT && SearchConfig::USE_TT_QSEARCH) {
        LOG__TRACE(Logger::get().SEARCH_LOG, "{:>{}}Quiescence storing into TT: {} {} {} {} {} {} {}",
                   "", ply, position.getZobristKey(), bestNodeValue,
                   TT::str(ttType), depth, printMove(ttStoreMove), false,
                   position.printFen());
        storeTT(position, bestNodeValue, ttType, DEPTH_NONE, ply, ttStoreMove, mateThreat[ply]);
      }
      break;
    case ROOT: // no TT storing in root
    case PERFT:
      break;
  }

  return bestNodeValue;
}

Value Search::evaluate(Position &position) {
  // count all leaf nodes evaluated
  searchStats.leafPositionsEvaluated++;

  // PERFT stats
  if (searchLimitsPtr->isPerft()) {
    //    Move lastMove = position.getLastMove();
    //    if (position.getLastCapturedPiece() != PIECE_NONE)
    //    searchStats.captureCounter++; if (typeOf(lastMove) == ENPASSANT)
    //    searchStats.enPassantCounter++; if (position.hasCheck())
    //    searchStats.checkCounter++; if (position.hasCheckMate())
    //    searchStats.checkMateCounter++;
    return VALUE_ONE;
  }

  return pEvaluator->evaluate(position);
}

/**
 * This templated method returns the next move depending on the Search_Type.
 * For ROOT it wil return the next pre generated root move.
 * For NONROOT it will return the next move from the on demand move generator.
 * For QUIESCENCE it will return only quiescence moves from the on demand
 * generator.
 */
template<Search::Search_Type ST>
Move Search::getMove(Position &position, int ply) {
  LOG__TRACE(Logger::get().SEARCH_LOG, "{:>{}}Get move for position {} in ply {}", "", ply, position.getZobristKey(), ply);
  Move move = MOVE_NONE;
  switch (ST) {
    case ROOT:
      if (currentMoveIndex < rootMoves.size()) {
        move = rootMoves.at(currentMoveIndex);
        searchStats.currentRootMove = move;
      }
      break;
    case NONROOT:
      move = moveGenerators[ply].getNextPseudoLegalMove<MoveGenerator::GENALL>(position);
      break;
    case QUIESCENCE:
      if (position.hasCheck()) { // if in check look at all moves in quiescence
        move = moveGenerators[ply].getNextPseudoLegalMove<MoveGenerator::GENALL>(position);
      }
      else { // if not in check only look at captures
        move = moveGenerators[ply].getNextPseudoLegalMove<MoveGenerator::GENCAP>(position);
      }
      break;
    case PERFT:
      if (ply == PLY_ROOT) {
        if (currentMoveIndex < rootMoves.size()) {
          move = rootMoves.at(currentMoveIndex);
          searchStats.currentRootMove = move;
        }
      }
      else {
        move = moveGenerators[ply].getNextPseudoLegalMove<MoveGenerator::GENALL>(position);
      }
      break;
  }
  searchStats.movesGenerated++;
  return move;
}

/**
 * Simple "good capture" determination
 *
 * OBS: move must be a capture otherwise too many false positives
 * TODO: Improve, add SEE
 */
bool Search::goodCapture(Position &position, Move move) {
  ASSERT_START
    if (!position.isCapturingMove(move)) {
      LOG__ERROR(Logger::get().SEARCH_LOG, "move send to goodCapture should be capturing {:<30s} {}", printMoveVerbose(move), position.printFen());
    }
  ASSERT_END
  return

    // all pawn captures - they never loose material
    // typeOf(position.getPiece(getFromSquare(move))) == PAWN

    // Lower value piece captures higher value piece
    // With a margin to also look at Bishop x Knight
    (valueOf(position.getPiece(getFromSquare(move))) + 50) <
    valueOf(position.getPiece(getToSquare(move)))

    // all recaptures should be looked at
    || (position.getLastMove() != MOVE_NONE &&
        getToSquare(position.getLastMove()) == getToSquare(move) &&
        position.getLastCapturedPiece() != PIECE_NONE)

    // undefended pieces captures are good
    // If the defender is "behind" the attacker this will not be recognized
    // here This is not too bad as it only adds a move to qsearch which we
    // could otherwise ignore
    || !position.isAttacked(getToSquare(move), ~position.getNextPlayer());
}

inline void Search::storeTT(Position &position, Value value, Value_Type ttType,
                            Depth depth, Ply ply, Move move, bool _mateThreat) {

  if (!SearchConfig::USE_TT || searchLimitsPtr->isPerft() || _stopSearchFlag) {
    return;
  }

  assert((value >= VALUE_MIN && value <= VALUE_MAX));

  // store the position in the TT
  // correct the value for mate distance and remove the value from the move to
  // later be able to easier compare it wh read from TT
  tt->put(position.getZobristKey(), depth, move, valueToTT(value, ply),
          ttType, _mateThreat);
}

inline Value Search::valueToTT(Value value, Ply ply) {
  assert(value != VALUE_NONE);
  return isCheckMateValue(value) ? Value(value > 0 ? value + ply : value - ply) : value;
}

inline Value Search::valueFromTT(Value value, Ply ply) {
  return isCheckMateValue(value) ? Value(value > 0 ? value - ply : value + ply) : value;
}

template<Search::Search_Type ST>
bool Search::checkDrawRepAnd50(Position &position) const {
  // for quiescence search we stop at 1 repetition already which should not
  // loose too much precision
  constexpr int allowedRepetitions = (ST == QUIESCENCE ? 1 : 2);
  if (position.checkRepetitions(allowedRepetitions)) {
    LOG__TRACE(Logger::get().SEARCH_LOG, "DRAW because of repetition for move {} in variation {}",
               printMove(position.getLastMove()),
               printMoveListUCI(this->currentVariation));
    return true;
  }
  if (position.getHalfMoveClock() >= 100) {
    LOG__TRACE(Logger::get().SEARCH_LOG, "DRAW because 50-move rule",
               printMove(position.getLastMove()),
               printMoveListUCI(this->currentVariation));
    return true;
  }
  return false;
}

inline bool Search::stopConditions() {
  if (pv[PLY_ROOT].empty()) return false; // search at least until we have a best move
  if (_stopSearchFlag) { return true; }
  if (searchLimitsPtr->getNodes() && searchStats.nodesVisited >= searchLimitsPtr->getNodes()) {
    _stopSearchFlag = true;
  }
  return _stopSearchFlag;
}

void Search::configureTimeLimits() {
  if (searchLimitsPtr->getMoveTime() > 0) { // mode time per move
    timeLimit = searchLimitsPtr->getMoveTime();
  }
  else { // remaining time - estimated time per move

    // retrieve time left from search mode
    assert((searchLimitsPtr->getWhiteTime() && searchLimitsPtr->getBlackTime()) && "move times must be > 0");
    MilliSec timeLeft = (myColor == WHITE) ? searchLimitsPtr->getWhiteTime() : searchLimitsPtr->getBlackTime();

    // Give some overhead time so that in games with very low available time we
    // do not run out of time
    timeLeft -= 1'000; // this should do

    // when we know the move to go (until next time control) use them otherwise
    // assume 40
    int movesLeft = searchLimitsPtr->getMovesToGo() > 0 ? searchLimitsPtr->getMovesToGo() : 40;

    // when we have a time increase per move we estimate the additional time we
    // should have
    if (myColor == WHITE) {
      timeLeft += 40 * searchLimitsPtr->getWhiteInc();
    }
    else {
      timeLeft += 40 * searchLimitsPtr->getBlackInc();
    }

    // for timed games with remaining time
    timeLimit = timeLeft / movesLeft;
  }

  // limits for very short available time
  if (timeLimit < 100) {
    addExtraTime(0.9);
  }
}

void Search::addExtraTime(const double d) {
  if (!searchLimitsPtr->getMoveTime()) {
    extraTime += static_cast<MilliSec>(timeLimit * (d - 1));
    LOG__DEBUG(Logger::get().SEARCH_LOG, "Time added/reduced {:n} ms to {:n} ms", extraTime, timeLimit + extraTime);
  }
}

void Search::startTimer() {
  this->timerThread = std::thread([&] {
    Logger::get().SEARCH_LOG->debug("Timer thread started with time limit of {:n} ms", timeLimit);
    // relaxed busy wait
    while (elapsedTime(startTime) < timeLimit + extraTime) {
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    this->_stopSearchFlag = true;
    Logger::get().SEARCH_LOG->debug("Timer thread stopped search after wall time: {:n} ms (time limit {:n} ms and extra time {:n})", elapsedTime(this->startTime), timeLimit, extraTime);
  });
}

inline MilliSec Search::elapsedTime(const MilliSec t) {
  return elapsedTime(t, now());
}

inline MilliSec Search::elapsedTime(const MilliSec t1, const MilliSec t2) {
  return t2 - t1;
}

inline MilliSec Search::now() {
#if defined (__APPLE__)
  // this C function is much faster than c++ chrono
return clock_gettime_nsec_np(CLOCK_UPTIME_RAW_APPROX) / 1'000'000;
#else
  const std::chrono::time_point timePoint = std::chrono::high_resolution_clock::now();
  const std::chrono::duration timeSinceEpoch
    = std::chrono::duration_cast<std::chrono::milliseconds>(timePoint.time_since_epoch());
  return timeSinceEpoch.count();
#endif
}

inline uint64_t Search::getNps() const {
  return 1000 * searchStats.nodesVisited / (elapsedTime(startTime) + 1); // +1 to avoid division by zero};
}

inline void Search::savePV(Move move, MoveList &src, MoveList &dest) {
  dest = src;
  dest.push_front(move);
}

void Search::getPVLine(Position &position, MoveList &pvRoot,
                       const Depth depth) {
  // Recursion-less reading of the chain of pv moves
  pvRoot.clear();
  int counter = 0;
  const TT::Entry* ttMatchPtr = tt->getMatch(position.getZobristKey());
  while (ttMatchPtr != nullptr && ttMatchPtr->move != MOVE_NONE && counter < depth) {
    pvRoot.push_back(ttMatchPtr->move);
    position.doMove(ttMatchPtr->move);
    ttMatchPtr = tt->getMatch(position.getZobristKey());
    counter++;
  }
  for (int i = 0; i < counter; ++i) {
    position.undoMove();
  }
}

MoveList Search::generateRootMoves(Position &position) {
  moveGenerators[PLY_ROOT].reset();
  const MoveList* legalMoves =
    moveGenerators[PLY_ROOT].generateLegalMoves<MoveGenerator::GENALL>(
      position);

  // for (Move m : *legalMoves) LOG__TRACE(Logger::get().SEARCH_LOG, "before: {} {}",
  // printMoveVerbose(m), m);
  MoveList moveList;
  if (searchLimitsPtr->getMoves().empty()) { // if UCI searchmoves is empty then add all
    for (auto legalMove : *legalMoves) {
      setValue(legalMove, VALUE_NONE);
      moveList.push_back(legalMove);
    }
  }
  else { // only add if in the UCI searchmoves list
    for (auto legalMove : *legalMoves) {
      for (auto move : searchLimitsPtr->getMoves()) {
        if (moveOf(move) == moveOf(legalMove)) {
          setValue(legalMove, VALUE_NONE);
          moveList.push_back(legalMove);
        }
      }
    }
  }
  return moveList;
}

bool Search::rootMovesSort(Move m1, Move m2) {
  return (valueOf(m1) > valueOf(m2));
}

void Search::clearHash() {
  LOG__TRACE(Logger::get().SEARCH_LOG, "Search: Clear Hash command received!");
  std::chrono::milliseconds timeout(2500);
  if (tt_lock.try_lock_for(timeout)) {
    tt->clear();
    tt_lock.unlock();
  }
  else {
    LOG__WARN(Logger::get().SEARCH_LOG, "Could not clear hash while searching.");
  }
}

void Search::setHashSize(int sizeInMB) {
  LOG__TRACE(Logger::get().SEARCH_LOG, "Search: Set HashSize to {} MB command received!", sizeInMB);
  std::chrono::milliseconds timeout(2500);
  if (tt_lock.try_lock_for(timeout)) {
    tt->resize(sizeInMB * TT::MB);
    tt_lock.unlock();
  }
  else {
    LOG__WARN(Logger::get().SEARCH_LOG, "Could not set hash size while searching.");
  }
}

void Search::sendIterationEndInfoToEngine() const {
  ASSERT_START
    if (pv[PLY_ROOT].empty()) {
      LOG__ERROR(Logger::get().SEARCH_LOG, "{}:{} pv[PLY_ROOT] is empty here and it should not be",
                 __func__, __LINE__);
    }
  ASSERT_END

  if (!pEngine) {
    LOG__INFO(
      Logger::get().SEARCH_LOG, "UCI >> depth {} seldepth {} multipv 1 {} nodes {:n} nps {:n} time {:n} pv {}",
      searchStats.currentSearchDepth,
      searchStats.currentExtraSearchDepth,
      (searchLimitsPtr->isPerft()
       ? VALUE_ZERO
       : valueOf(pv[PLY_ROOT].empty() ? MOVE_NONE
                                      : pv[PLY_ROOT].at(0))),
      searchStats.nodesVisited, getNps(), elapsedTime(startTime),
      printMoveListUCI(pv[PLY_ROOT]));
  }
  else {
    pEngine->sendIterationEndInfo(
      searchStats.currentSearchDepth, searchStats.currentExtraSearchDepth,
      searchLimitsPtr->isPerft()
      ? VALUE_ZERO
      : valueOf(pv[PLY_ROOT].empty() ? MOVE_NONE : pv[PLY_ROOT].at(0)),
      searchStats.nodesVisited, getNps(), elapsedTime(startTime),
      pv[PLY_ROOT]);
  }
}

void Search::sendCurrentRootMoveToEngine() const {
  if (!pEngine) {
    LOG__TRACE(Logger::get().SEARCH_LOG, "UCI >> currmove {} currmovenumber {}",
               printMove(searchStats.currentRootMove), currentMoveIndex + 1);
  }
  else {
    pEngine->sendCurrentRootMove(searchStats.currentRootMove,
                                 currentMoveIndex + 1);
  }
}

void Search::sendSearchUpdateToEngine() {
  if (elapsedTime(lastUciUpdateTime) > UCI_UPDATE_INTERVAL) {
    lastUciUpdateTime = now();

    LOG__INFO(Logger::get().SEARCH_LOG, "Search statistics: {}", searchStats.str());
    LOG__INFO(Logger::get().SEARCH_LOG, "Eval   statistics: {}", pEvaluator->pawnTableStats());
    LOG__INFO(Logger::get().SEARCH_LOG, "TT     statistics: {}", tt->str());

    if (!pEngine) {
      LOG__INFO(
        Logger::get().SEARCH_LOG, "UCI >> depth {} seldepth {} nodes {:n} nps {:n} time {:n} hashfull {}",
        searchStats.currentSearchDepth,
        searchStats.currentExtraSearchDepth, searchStats.nodesVisited,
        getNps(), elapsedTime(startTime), tt->hashFull());
    }
    else {
      pEngine->sendSearchUpdate(
        searchStats.currentSearchDepth, searchStats.currentExtraSearchDepth,
        searchStats.nodesVisited, getNps(), elapsedTime(startTime),
        tt->hashFull());
    }

    sendCurrentRootMoveToEngine();

    if (!pEngine) {
      LOG__TRACE(
        Logger::get().SEARCH_LOG, "UCI >> currline {}", printMoveListUCI(currentVariation));
    }
    else {
      pEngine->sendCurrentLine(currentVariation);
    }
  }
}

void Search::sendResultToEngine() const {
  LOG__INFO(
    Logger::get().SEARCH_LOG, "UCI >> Engine got Best Move: {} ({}) [Ponder {}] from depth {}",
    printMove(lastSearchResult.bestMove),
    printValue(lastSearchResult.bestMoveValue),
    printMove(lastSearchResult.ponderMove),
    searchStats.bestMoveDepth);
  if (pEngine) {
    pEngine->sendResult(lastSearchResult.bestMove,
                        lastSearchResult.bestMoveValue,
                        lastSearchResult.ponderMove);
  }
}

void Search::sendStringToEngine(const std::string &anyString) const {
  LOG__INFO(Logger::get().SEARCH_LOG, "UCI >> Info {}", anyString);
  if (pEngine) {
    pEngine->sendString(anyString);
  }
}
