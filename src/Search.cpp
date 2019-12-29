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
#include "Search.h"
#include "SearchConfig.h"
#include "Engine.h"
#include "TT.h"

std::timed_mutex Search::tt_lock;

////////////////////////////////////////////////
///// CONSTRUCTORS

Search::Search() : Search(nullptr) {
}

Search::Search(Engine* pEng) {
  pEngine = pEng;
  if (SearchConfig::USE_TT) {
    int hashSize = SearchConfig::TT_SIZE_MB;
    if (pEngine) {
      int tmp = pEngine->getHashSize();
      if (tmp) hashSize = tmp;
    }
    tt.resize(hashSize * TT::MB);
    tt.setThreads(4);
  }
  else {
    tt.resize(0);
  }
}

Search::~Search() {
  // necessary to avoid err message: terminate called without an active exception
  if (myThread.joinable()) myThread.join();
}

////////////////////////////////////////////////
///// PUBLIC

void Search::startSearch(const Position &pos, SearchLimits &limits) {
  if (running) {
    LOG->error("Search already running");
    return;
  }

  // make sure we have a semaphore available
  searchSemaphore.reset();

  // pos is a deep copy of the position parameter to not change
  // the original position given
  myPosition = pos;
  searchLimits = limits;
  myColor = myPosition.getNextPlayer();

  // join() previous thread
  if (myThread.joinable()) myThread.join();
  stopSearchFlag = false;

  // start search in a separate thread
  LOG->debug("Starting search in separate thread.");
  myThread = std::thread(&Search::run, this);

  // wait until thread is initialized before returning to caller
  initSemaphore.getOrWait();
  assert(running);
  LOG->info("Search started.");
}

void Search::stopSearch() {
  if (!running) {
    LOG->warn("Stop search called when search was not running");
    return;
  }
  LOG->info("Stopping search.");

  // stop pondering if we are
  if (searchLimits.isPonder()) {
    if (!isRunning()) {
      // Ponder search has finished before we stopped it
      // Per UCI protocol we need to send the result anyway although a miss
      LOG->info(
        "Pondering has been stopped after ponder search has finished. Send obsolete result");
      LOG->info("Search result was: {} PV {}", lastSearchResult.str(),
                printMoveListUCI(pv[PLY_ROOT]));
      sendResultToEngine();
    }
    else {
      LOG->info("Pondering has been stopped. Ponder Miss!");
    }
    searchLimits.ponderStop();
  }

  // set stop flag - search needs to check regularly and stop accordingly
  stopSearchFlag = true;
  LOG->debug("Stop flag has been set to: {}!", stopSearchFlag);

  // Wait for the thread to die
  if (myThread.joinable()) myThread.join();
  waitWhileSearching();

  LOG->info("Search stopped.", running);
  assert(!running);
}

bool Search::isRunning() const {
  return running;
}

void Search::waitWhileSearching() {
  LOG->trace("Wait while searching");
  if (!running) return;
  LOG->trace("Waiting for search semaphore");
  searchSemaphore.getOrWait();
  LOG->trace("Got search semaphore");
  searchSemaphore.reset();
  LOG->trace("Reset search semaphore");
}

void Search::ponderhit() {
  if (searchLimits.isPonder()) {
    LOG->info("****** PONDERHIT *******");
    if (isRunning()) {
      LOG->info("Ponderhit when ponder search still running. Continue searching.");
      // store the start time of the search
      startTime = now();
      configureTimeLimits();
      searchLimits.ponderHit();
      // if time based game setup the time limits
      if (searchLimits.isTimeControl()) {
        LOG->debug("Time Management: {} time limit: {:n}",
                   (searchLimits.isTimeControl() ? "ON" : "OFF"), timeLimit);
      }
    }
    else {
      LOG->info("Ponderhit when ponder search already ended. Sending result.");
      LOG->info("Search Result: {}", lastSearchResult.str());
      if (pEngine) {
        pEngine->sendResult(lastSearchResult.bestMove, lastSearchResult.ponderMove);
      }
    }
  }
  else {
    LOG->warn("Ponderhit when not pondering!");
  }
}

////////////////////////////////////////////////
///// PRIVATE

/**
 * Called when the new search thread is started.
 * Initializes the search.
 * Calls <code>iterativeDeepening()</code> when search is initialized.
 * <p>
 * After the search has stopped calls <code>Engine.sendResult(searchResult)</code>
 * to store/hand over the result. After storing the result the search is ended
 * and the thread terminated.<br>
 */
void Search::run() {

  // get the search lock
  searchSemaphore.getOrWait();
  running = true;

  LOG->debug("Search thread started.");

  // store the start time of the search
  startTime = lastUciUpdateTime = now();

  // Initialize for new search
  lastSearchResult = SearchResult();
  timeLimit = extraTime = 0;
  searchStats = SearchStats();

  // Initialize ply based data
  // Each depth in search gets it own global field to avoid object creation
  // during search.
  for (int i = DEPTH_NONE; i < DEPTH_MAX; i++) {
    moveGenerators[i] = MoveGenerator();
    pv[i].clear();
  }

  // age TT entries
  tt.ageEntries();

  // search mode
  if (searchLimits.isPerft()) {
    LOG->info("Search Mode: PERFT SEARCH ({})", searchLimits.getMaxDepth());
  }
  if (searchLimits.isInfinite()) {
    LOG->info("Search Mode: INFINITE SEARCH");
  }
  if (searchLimits.isPonder()) {
    LOG->info("Search Mode: PONDER SEARCH");
  }
  if (searchLimits.getMate()) {
    LOG->info("Search Mode: MATE SEARCH ({})", searchLimits.getMate());
  }

  // initialization done
  initSemaphore.release();

  // if time based game setup the soft and hard time limits
  if (searchLimits.isTimeControl()) configureTimeLimits();

  // start iterative deepening
  lastSearchResult = iterativeDeepening(myPosition);

  // if the mode still is ponder at this point we finished the ponder
  // search early before a miss or hit has been signaled. We need to
  // wait with sending the result until we get a miss (stop) or a hit.
  if (searchLimits.isPonder()) {
    LOG->info("Ponder Search finished! Waiting for Ponderhit to send result");
    return;
  }

  sendResultToEngine();

  running = false;
  searchSemaphore.reset();
  LOG->debug("Search thread ended.");
}

/**
 * Generates root moves and calls search in a loop increasing depth
 * with each iteration.
 * <p>
 * Detects mate if started on a mate position.
 * @param position
 * @return a SearchResult
 */
SearchResult Search::iterativeDeepening(Position &position) {

  // prepare search result
  SearchResult searchResult = SearchResult();

  // no legal root moves - game already ended!
  if (!MoveGenerator::hasLegalMove(position)) {
    if (position.hasCheck()) {
      searchResult.bestMove = MOVE_NONE;
      setValue(searchResult.bestMove, -VALUE_CHECKMATE);
    }
    else {
      searchResult.bestMove = MOVE_NONE;
      setValue(searchResult.bestMove, VALUE_DRAW);
    }
    return searchResult;
  }

  Depth iterationDepth = searchLimits.getStartDepth();

  // current search iterationDepth
  searchStats.currentSearchDepth = PLY_ROOT;
  searchStats.currentExtraSearchDepth = PLY_ROOT;

  // generate all legal root moves
  rootMoves = generateRootMoves(position);

  // make sure we have a temporary best move
  // when using TT this will already be set
  pv[PLY_ROOT].push_back(rootMoves.at(0));

  // print search setup for debugging
  if (LOG->should_log(spdlog::level::debug)) {
    LOG->debug("Root moves: {}", printMoveList(rootMoves));
    LOG->debug("Searching in position: {}", position.printFen());
    LOG->debug("Searching these moves: {}", printMoveList(rootMoves));
    LOG->debug("Search mode: {}", searchLimits.str());
    LOG->debug("Time Management: {} time limit: {:n}",
               (searchLimits.isTimeControl() ? "ON" : "OFF"),
               timeLimit);
    LOG->debug("Start Depth: {} Max Depth: {}", iterationDepth, searchLimits.getMaxDepth());
    LOG->debug("Starting iterative deepening now...");
  }

  // max window search - preparation for aspiration window search
  Value alpha = VALUE_MIN;
  Value beta = VALUE_MAX;

  // check search requirements
  assert (!rootMoves.empty() && "No root moves to search");
  assert (iterationDepth > 0 && "iterationDepth <= 0");

  // ###########################################
  // ### BEGIN Iterative Deepening
  do {
    assert (pv[PLY_ROOT].at(0) != MOVE_NONE && "No best root move");

    TRACE(LOG, "Iteration Depth {} START", iterationDepth);

    searchStats.currentIterationDepth = iterationDepth;
    searchStats.bestMoveChanges = 0;
    searchStats.nodesVisited++;

    // protect the TT from being resozed or cleared during search
    tt_lock.lock();

    // ###########################################
    // ### CALL SEARCH for iterationDepth
    search<ROOT>(position, iterationDepth, PLY_ROOT, alpha, beta);
    // ###########################################

    // release lock on TT
    tt_lock.unlock();

    sendIterationEndInfoToEngine();

    // break on stop signal or time
    if (stopConditions()) break;

    //    fmt::print("Before: ");
    //    for (auto m : rootMoves) {
    //      fmt::print("{} ", printMoveVerbose(m));
    //    }
    //    NEWLINE;

    // sort root moves based on value for the next iteration
    std::stable_sort(rootMoves.begin(), rootMoves.end(), rootMovesSort);

    //    fmt::print("After : ");
    //    for (auto m : rootMoves) {
    //      fmt::print("{} ", printMoveVerbose(m));
    //    }
    //    NEWLINE;

    TRACE(LOG, "Iteration Depth={} END", iterationDepth);

  } while (++iterationDepth <= searchLimits.getMaxDepth());
  // ### ENDOF Iterative Deepening
  // ###########################################

  // update searchResult here
  searchResult.bestMove = pv[PLY_ROOT].at(0);
  searchResult.ponderMove = pv[PLY_ROOT].size() > 1 ? pv[PLY_ROOT].at(1) : MOVE_NONE;
  searchResult.depth = searchStats.currentSearchDepth;
  searchResult.extraDepth = searchStats.currentExtraSearchDepth;

  // search is finished - stop timer
  stopTime = now();
  searchStats.lastSearchTime = elapsedTime(startTime, stopTime);

  // print result of the search
  if (LOG->should_log(spdlog::level::debug)) {
    LOG->debug("Search statistics: {}", searchStats.str());
    LOG->debug("Search Depth was {} ({})", searchStats.currentIterationDepth,
               searchStats.currentExtraSearchDepth);
    LOG->debug("Search took {},{:03} sec ({:n} nps)",
               (searchStats.lastSearchTime % 1'000'000) / 1'000,
               (searchStats.lastSearchTime % 1'000),
               (searchStats.nodesVisited * 1'000) / searchStats.lastSearchTime);
  }

  return searchResult;
}

/**
 * This is the templated search function for root, non-root and quiescence
 * searches. Through the template the compiler will generate specialized
 * versions of this method for each case.
 * 
 * @tparam T
 * @param position
 * @param depth
 */
template<Search::Search_Type ST>
Value Search::search(Position &position, Depth depth, Ply ply, Value alpha, Value beta) {
  assert (alpha >= VALUE_MIN && beta <= VALUE_MAX && "alpha/beta out if range");

  constexpr bool root = ST == ROOT;
  constexpr bool nonroot = ST == NONROOT;
  constexpr bool quiescence = ST == QUIESCENCE;

  const bool PERFT = searchLimits.isPerft();

  TRACE(LOG, "{:>{}}Search {} in ply {} for depth {}: START alpha={} beta={} currline={}",
        "", ply, (root ? "ROOT" : nonroot ? "NONROOT" : "QUIESCENCE"),
        ply, depth, alpha, beta, printMoveListUCI(currentVariation));

  if (stopConditions()) return VALUE_NONE;

  // Leaf node handling
  if (!quiescence) { // normal search
    searchStats.currentSearchDepth = std::max(searchStats.currentSearchDepth, ply);
    searchStats.currentExtraSearchDepth = std::max(searchStats.currentExtraSearchDepth, ply);
    if (depth <= DEPTH_NONE || ply >= PLY_MAX - 1) {
      return search<QUIESCENCE>(position, depth, ply, alpha, beta);
    }
  }
  else { // quiescence search
    if (PERFT || ply >= PLY_MAX - 1 || !SearchConfig::USE_QUIESCENCE) {
      return evaluate(position, ply);
    }
    searchStats.currentExtraSearchDepth = std::max(searchStats.currentExtraSearchDepth, ply);
  }

  // to detect mate situations
  int movesSearched = 0;

  // prepare move loop
  Value bestNodeValue = VALUE_NONE;
  Move bestNodeMove = MOVE_NONE;
  TT::EntryType ttType = TT::TYPE_ALPHA;
  moveGenerators[ply].resetOnDemand();
  if (!root) { pv[ply].clear(); }
  else { currentMoveIndex = 0; }
  Value value = VALUE_NONE;
  Move move = MOVE_NONE;

  // ###############################################
  // Quiescence StandPat
  // Use evaluation as a standing pat (lower bound)
  // https://www.chessprogramming.org/Quiescence_Search#Standing_Pat
  // Assumption is that there is at least on move which would improve the
  // current position. So if we are already >beta we don't need to look at it.
  if (quiescence && !position.hasCheck()) {
    Value standPat = evaluate(position, ply);
    bestNodeValue = standPat;
    TRACE(LOG, "{:>{}}Quiescence in ply {}: STANDPAT {}", "", ply, ply, standPat);
    if (standPat >= beta) {
      TRACE(LOG, "{:>{}}Quiescence in ply {}: STANDPAT CUT ({} > {} beta)",
            "", ply, ply, standPat, beta);
      //storeTT(position, standPat, TT::TYPE_BETA, DEPTH_NONE, MOVE_NONE, false);
      return standPat; // fail-hard: beta, fail-soft: statEval
    }
    if (standPat > alpha) alpha = standPat;
  }
  // ###############################################

  // ###########################################################################
  // MOVE LOOP
  while ((move = getMove<ST>(position, ply)) != MOVE_NONE) {

    if (root) {
      TRACE(LOG, "Root Move {} START", printMove(move));
      if (depth > 5) sendCurrentRootMoveToEngine(); // avoid protocol flooding
    }
    else {
      TRACE(LOG, "{:>{}}Depth {} cv {} move {} START",
            "", ply, ply, printMoveListUCI(currentVariation), printMove(move));
    }

    // reduce number of moves searched in quiescence
    // by looking at good captures only
    if (quiescence && !goodCapture(position, move)) continue;

    // ************************************
    // Execute move
    position.doMove(move);
    if (position.isLegalPosition()) {
      currentVariation.push_back(move);
      searchStats.nodesVisited++;
      movesSearched++;
      sendSearchUpdateToEngine();

      // check for repetition ot 50-move-rule draws
      if (checkDrawRepAnd50<ST>(position)) {
        value = VALUE_DRAW;
      }
      else {
        // recurse deeper into the search tree
        switch (ST) {
          case ROOT: // fall through
          case NONROOT:
            value = -search<NONROOT>(position, depth - 1, ply + 1, -beta, -alpha);
            break;
          case QUIESCENCE:
            value = -search<QUIESCENCE>(position, DEPTH_NONE, ply + 1, -beta, -alpha);
            break;
        }
      }
      assert ((value != VALUE_NONE || stopSearchFlag) && "Value should not be NONE at this point.");

      currentVariation.pop_back();
    }
    else {
      TRACE(LOG, "{:>{}}Depth {} cv {} move {} NOT LEGAL",
            "", ply, ply, printMoveListUCI(currentVariation), printMove(move));
    }

    position.undoMove();
    // ************************************

    if (stopConditions()) return VALUE_NONE;

    // For root moves encode value into the move
    // so we can sort the move before the next iteration
    if (root) setValue(rootMoves.at(currentMoveIndex++), value);

    // In PERFT we can ignore values and pruning
    if (PERFT) continue;

    // Did we find a better move for this node?
    // For the first move this is always the case.
    if (value > bestNodeValue) {

      bestNodeValue = value;
      bestNodeMove = move;

      // AlphaBeta
      if (SearchConfig::USE_ALPHABETA) {

        // If we found a move that is better or equal than beta this means that the
        // opponent can/will avoid this position altogether so we can stop search
        // this node
        if (value >= beta) { // fail-high
          // save killer moves so they will be searched earlier on following nodes
          if (SearchConfig::USE_KILLER_MOVES) {
            moveGenerators[ply].storeKiller(move, SearchConfig::NO_KILLER_MOVES);
          }
          searchStats.prunings++;
          ttType = TT::TYPE_BETA; // store the beta value into the TT later

          TRACE(LOG, "{:>{}} Search in ply {} for depth {}: CUT NODE {} >= {} (beta)",
                " ", ply, ply, depth, value, beta);
          break; // get out of loop and return the value at the end
        }

        // Did we find a better move than in previous nodes then this is our new
        // PV and best move for this ply.
        // If we never find a better alpha we do have a best move for this node
        // but not for the ply. We will return alpha and store a alpha node in
        // TT.
        if (value > alpha) { // NEW ALPHA => NEW PV NODE
          setValue(move, value);
          savePV(move, pv[ply + 1], pv[ply]);
          if (root) {
            searchStats.bestMoveChanges++;
            searchStats.bestMoveDepth = depth;
            setValue(pv[PLY_ROOT].at(0), value);
          }
          ttType = TT::TYPE_EXACT;
          alpha = value;
          TRACE(LOG, "{:>{}}Search in ply {} for depth {}: NEW PV {} ({}) (alpha) PV: {}",
                "", ply, ply, depth, printMove(move), value, printMoveListUCI(pv[ply]));
        }
      }
        // Minimax
      else {
        setValue(move, value);
        savePV(move, pv[ply + 1], pv[ply]);
        ttType = TT::TYPE_EXACT;
        TRACE(LOG, "{:>{}}Search in ply {} for depth {}: NEW PV {} ({}) PV: {}",
              "", ply, ply, depth, printMove(move), value, printMoveListUCI(pv[ply]));
      }
    }

    if (root) {
      TRACE(LOG, "Root Move {} END", printMove(move));
    }
    else {
      TRACE(LOG, "{:>{}}Depth {} cv {} move {} END",
            " ", ply, ply, printMoveListUCI(currentVariation), printMove(move));
    }
  }
  // ##### Iterate through all available moves
  // ###########################################################################

  // if we did not have at least one legal move then we might have a mate or
  // in quiescence only quite moves
  if (!movesSearched && !stopSearchFlag) {
    searchStats.nonLeafPositionsEvaluated++;
    assert (ttType == TT::TYPE_ALPHA);
    TRACE(LOG, "{:>{}}Depth {} cv {} NO LEGAL MOVES", "", ply, ply,
          printMoveListUCI(currentVariation));
    if (myPosition.hasCheck()) {
      // if the position has check we have a mate even in quiescence as we will
      // have generated all moves because of the check.
      // Return a -CHECKMATE.
      bestNodeValue = -VALUE_CHECKMATE + Value(ply);
      assert (bestNodeMove == MOVE_NONE);
      ttType = TT::TYPE_EXACT;
      TRACE(LOG, "{:>{}} Search in ply {} for depth {}: {} CHECKMATE", " ", ply, ply, depth,
            bestNodeValue);
    }
    else if (!quiescence) {
      // if not in quiescence we have a stale mate. Return the draw value.
      bestNodeValue = VALUE_DRAW;
      assert (bestNodeMove == MOVE_NONE);
      ttType = TT::TYPE_EXACT;
      TRACE(LOG, "{:>{}} Search in ply {} for depth {}: {} STALEMATE", " ", ply, ply, depth,
            bestNodeValue);
    }
    // in quiescence having searched no moves while not in check means that
    // there were only quiet moves which we ignored on purpose.
  }

  TRACE(LOG, "{:>{}}Search {} in ply {} for depth {}: END value={} ({} moves searched) ({})",
        "", ply, (root ? "ROOT" : nonroot ? "NONROOT" : "QUIESCENCE"),
        ply, depth, bestNodeValue, movesSearched, printMoveListUCI(currentVariation));

  assert (PERFT || (bestNodeValue > VALUE_MIN && bestNodeValue < VALUE_MAX
                    && "bestNodeValue should not be MIN/MAX here"));

  if (!quiescence) {
    TRACE(LOG, "{:>{}}Storing into TT: {} {} {} {} {} {} {}", "", ply,
          position.getZobristKey(), bestNodeValue, TT::str(ttType), depth, printMove(bestNodeMove),
          false, position.printFen());
    storeTT(position, bestNodeValue, ttType, depth, bestNodeMove, false);
  }

  return bestNodeValue;
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
  TRACE(LOG, "{:>{}}Get move for position {} in ply {}", "", ply, position.getZobristKey(), ply);
  Move move = MOVE_NONE;
  constexpr Search_Type type = ST;
  switch (type) {
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
  }
  searchStats.movesGenerated++;
  return move;
}

/**
 * Simple good capture determination
 * TODO: Improve, add SEE
 */
bool Search::goodCapture(Position &position, Move move) {
  return
    // only captures
    position.getPiece(getToSquare(move)) != PIECE_NONE
    // all pawn captures - they never loose material
    && (typeOf(position.getPiece(getFromSquare(move))) == PAWN
        // Lower value piece captures higher value piece
        // With a margin to also look at Bishop x Knight
        || (valueOf(position.getPiece(getFromSquare(move))) + 50) <=
           valueOf(position.getPiece(getToSquare(move)))
        // all recaptures should be looked at
        || (position.getLastMove() != MOVE_NONE
            && getToSquare(position.getLastMove()) == getToSquare(move)
            && position.getLastCapturedPiece() != PIECE_NONE)
        // undefended pieces captures are good
        // If the defender is "behind" the attacker this will not be recognized here
        // This is not too bad as it only adds a move to qsearch which we could otherwise ignore
        || !position.isAttacked(getToSquare(move), ~position.getNextPlayer()));
}

Value Search::evaluate(Position &position, Ply ply) {
  // count all leaf nodes evaluated
  searchStats.leafPositionsEvaluated++;

  // PERFT stats
  // TODO: more perft stats
  if (searchLimits.isPerft()) {
    return VALUE_ONE;
  }

  Value value = evaluator.evaluate(position);
  TRACE(LOG, "{:>{}} Evaluation: {} {}", " ", ply, printMoveListUCI(currentVariation), value);
  return value;
}

inline void Search::storeTT(Position &position, Value value, TT::EntryType ttType,
                            Depth depth, Move bestMove, bool mateThreat) {
  if (!SearchConfig::USE_TT || searchLimits.isPerft() || stopSearchFlag) return;
  assert (depth >= 0 && depth <= DEPTH_MAX);
  assert ((value >= VALUE_MIN && value <= VALUE_MAX));

#ifdef TT_DEBUG
  tt.put(false, position.getZobristKey(), value, ttType, depth, bestMove, mateThreat, position.printFen());
#else
  tt.put(position.getZobristKey(), value, ttType, depth, bestMove, mateThreat);
#endif
}

inline bool Search::stopConditions() {
  if (timeLimitReached()
      || (searchLimits.getNodes() && searchStats.nodesVisited >= searchLimits.getNodes())) {
    stopSearchFlag = true;
  }
  return stopSearchFlag;
}

template<Search::Search_Type ST>
bool Search::checkDrawRepAnd50(Position &position) const {
  // for quiescence search we stop at 1 repetition already which should not
  // loose too much precision
  constexpr int allowedRepetitions = (ST == QUIESCENCE ? 1 : 2);
  if (position.checkRepetitions(allowedRepetitions)) {
    this->LOG->debug("DRAW because of repetition for move {} in variation {}",
                     printMove(position.getLastMove()),
                     printMoveListUCI(this->currentVariation));
    return true;
  }
  if (position.getHalfMoveClock() >= 100) {
    this->LOG->debug("DRAW because 50-move rule",
                     printMove(position.getLastMove()),
                     printMoveListUCI(this->currentVariation));
    return true;
  }
  return false;
}

void Search::configureTimeLimits() {
  if (searchLimits.getMoveTime() > 0) { // mode time per move
    timeLimit = searchLimits.getMoveTime();
  }
  else { // remaining time - estimated time per move

    // retrieve time left from search mode
    assert ((searchLimits.getWhiteTime() && searchLimits.getBlackTime())
            && "move times must be > 0");
    MilliSec timeLeft = (myColor == WHITE) ? searchLimits.getWhiteTime()
                                           : searchLimits.getBlackTime();

    // Give some overhead time so that in games with very low available time we
    // do not run out of time
    timeLeft -= 1'000; // this should do

    // when we know the move to go (until next time control) use them otherwise assume 40
    int movesLeft = searchLimits.getMovesToGo() > 0 ? searchLimits.getMovesToGo() : 40;

    // when we have a time increase per move we estimate the additional time we should have
    if (myColor == WHITE) {
      timeLeft += 40 * searchLimits.getWhiteInc();
    }
    else {
      timeLeft += 40 * searchLimits.getBlackInc();
    }

    // for timed games with remaining time
    timeLimit = timeLeft / movesLeft;
  }

  // limits for very short available time
  if (timeLimit < 100) {
    addExtraTime(0.9);
  }
}

/**
 * Changes the time limit by the given factor and also sets the soft time limit
 * to 0.8 of the hard time limit.
 * Factor 1 is neutral. <1 shortens the time, >1 adds time<br/>
 * Example: factor 0.8 is 20% less time. Factor 1.2 is 20% additional time
 * Always calculated from the initial time budget.
 *
 * @param d
 */
void Search::addExtraTime(const double d) {
  if (!searchLimits.getMoveTime()) {
    extraTime += timeLimit * (d - 1);
    LOG->debug("Time added {:n} ms to {:n} ms",
               extraTime,
               timeLimit + extraTime);
  }
}

/**
 * Hard time limit is used to check time regularly in the search to stop the search when
 * time is out
 * TODO instead of checking this regularly we could use a timer thread to set stopSearch to true.
 *
 * @return true if hard time limit is reached, false otherwise
 */
inline bool Search::timeLimitReached() {
  if (!searchLimits.isTimeControl()) return false;
  return elapsedTime(startTime) >= timeLimit + extraTime;
}

/**
 * @param t time point since the elapsed time
 * @return the elapsed time from the start of the search to the given t
 */
inline MilliSec Search::elapsedTime(const MilliSec t) {
  return elapsedTime(t, now());
}

/**
 * @param t1 Earlier time point
 * @param t2 Later time point
 * @return Duration between time points in milliseconds
 */
inline MilliSec Search::elapsedTime(const MilliSec t1, const MilliSec t2) {
  return t2 - t1;
}

/**
 * Returns the current time in ms
 * @return current time
 */
inline MilliSec Search::now() {
  // this C function is much faster than c++ chrono
  return clock_gettime_nsec_np(CLOCK_UPTIME_RAW_APPROX) / 1'000'000;
}

inline MilliSec Search::getNps() const {
  return
    1000 * searchStats.nodesVisited /
    (elapsedTime(startTime) + 1); // +1 to avoid division by zero};
}

inline void Search::savePV(Move move, MoveList &src, MoveList &dest) {
  dest = src;
  dest.push_front(move);
}

/**
 * Generates root moves and filters them according to the UCI searchmoves list
 * @param position
 * @return UCI filtered root moves
 */
MoveList Search::generateRootMoves(Position &position) {
  moveGenerators[PLY_ROOT].reset();
  const MoveList* legalMoves =
    moveGenerators[PLY_ROOT].generateLegalMoves<MoveGenerator::GENALL>(position);

  //for (Move m : *legalMoves) TRACE(LOG, "before: {} {}", printMoveVerbose(m), m);
  MoveList moveList;
  if (searchLimits.getMoves().empty()) { // if UCI searchmoves is empty then add all
    for (auto legalMove : *legalMoves) {
      setValue(legalMove, VALUE_NONE);
      moveList.push_back(legalMove);
    }
  }
  else { // only add if in the UCI searchmoves list
    for (auto legalMove : *legalMoves) {
      for (auto move : searchLimits.getMoves()) {
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
  LOG->debug("Search: Clear Hash command received!");
  std::chrono::milliseconds timeout(2500);
  if (tt_lock.try_lock_for(timeout)) {
    tt.clear();
    tt_lock.unlock();
  }
  else {
    LOG->warn("Could not clear hash while searching.");
  }
}

void Search::setHashSize(int sizeInMB) {
  LOG->debug("Search: Set HashSize to {} MB command received!", sizeInMB);
  std::chrono::milliseconds timeout(2500);
  if (tt_lock.try_lock_for(timeout)) {
    tt.resize(sizeInMB * TT::MB);
    tt_lock.unlock();
  }
  else {
    LOG->warn("Could not set hash size while searching.");
  }
}

void Search::sendIterationEndInfoToEngine() const {

  MilliSec result;
  result = elapsedTime(startTime);
  std::string infoString =
    fmt::format(
      "depth {} seldepth {} multipv 1 {} nodes {} nps {} time {} pv {}",
      searchStats.currentIterationDepth,
      searchStats.currentExtraSearchDepth,
      valueOf(pv[PLY_ROOT].at(0)),
      searchStats.nodesVisited,
      getNps(),
      result,
      printMoveListUCI(pv[PLY_ROOT]));

  if (!pEngine) { LOG->warn("<no engine> >> {}", infoString); }
  else {
    pEngine->sendIterationEndInfo(searchStats.currentIterationDepth,
                                  searchStats.currentExtraSearchDepth,
                                  valueOf(pv[PLY_ROOT].at(0)),
                                  searchStats.nodesVisited,
                                  getNps(),
                                  elapsedTime(startTime),
                                  pv[PLY_ROOT]);
  }
}

void Search::sendCurrentRootMoveToEngine() const {
  std::string infoString =
    fmt::format(
      "currmove {} currmovenumber {}",
      printMove(searchStats.currentRootMove),
      currentMoveIndex + 1);

  if (!pEngine) { LOG->warn("<no engine> >> {}", infoString); }
  else {
    pEngine->sendCurrentRootMove(searchStats.currentRootMove,
                                 currentMoveIndex + 1);
  }
}

void Search::sendSearchUpdateToEngine() {
  if (elapsedTime(lastUciUpdateTime) > UCI_UPDATE_INTERVAL) {
    lastUciUpdateTime = now();

    MilliSec result;
    result = elapsedTime(startTime);
    std::string infoString = fmt::format(
      "depth {} seldepth {} nodes {} nps {} time {} hashfull {}",
      searchStats.currentIterationDepth,
      searchStats.currentExtraSearchDepth,
      searchStats.nodesVisited,
      getNps(),
      result,
      tt.hashFull());

    //(int) (1000 * ((float) transpositionTable.getNumberOfEntries() / transpositionTable.getMaxEntries())));
    if (!pEngine) { LOG->warn("<no engine> >> {}", infoString); }
    else {
      pEngine->sendSearchUpdate(searchStats.currentIterationDepth,
                                searchStats.currentExtraSearchDepth,
                                searchStats.nodesVisited,
                                getNps(),
                                elapsedTime(startTime),
                                0);
    }

    infoString = fmt::format("currline {}",
                             printMoveListUCI(currentVariation));
    if (!pEngine) { LOG->warn("<no engine> >> {}", infoString); }
    else { pEngine->sendCurrentLine(currentVariation); }

    LOG->debug(tt.str());
  }


}

void Search::sendResultToEngine() const {
  std::string infoString =
    fmt::format(
      "Engine got Best Move: {} ({}) [Ponder {}] from depth {}",
      printMove(lastSearchResult.bestMove),
      printValue(valueOf(lastSearchResult.bestMove)),
      printMove(lastSearchResult.ponderMove),
      searchStats.bestMoveDepth);

  if (!pEngine) { LOG->warn("<no engine> >> {}", infoString); }
  else { pEngine->sendResult(lastSearchResult.bestMove, lastSearchResult.ponderMove); }

}


















