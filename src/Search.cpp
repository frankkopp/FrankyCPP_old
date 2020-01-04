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

////////////////////////////////////////////////
///// CONSTRUCTORS

Search::Search() : Search(nullptr) {
}

Search::Search(Engine* pEng) {
  pEngine = pEng;
  if (SearchConfig::USE_TT) {
    tt.setThreads(4);
    int hashSize = SearchConfig::TT_SIZE_MB;
    if (pEngine) {
      int tmp = pEngine->getHashSize();
      if (tmp) hashSize = tmp;
    }
    tt.resize(hashSize * TT::MB);
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
    LOG->error("Start Search: Search already running");
    return;
  }

  // make sure we have a semaphore available
  searchSemaphore.reset();

  searchLimits = limits;

  // join() previous thread
  if (myThread.joinable()) myThread.join();
  stopSearchFlag = false;

  // start search in a separate thread
  LOG->debug("Starting search in separate thread.");
  myThread = std::thread(&Search::run, this, pos);

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
  LOG->debug("Stopping search.");

  // stop pondering if we are
  if (searchLimits.isPonder()) {
    if (!isRunning()) {
      // Ponder search has finished before we stopped it
      // Per UCI protocol we need to send the result anyway although a miss
      LOG->info("Pondering has been stopped after ponder search has finished. Send obsolete result");
      LOG->info("Search result was: {} PV {}", lastSearchResult.str(), printMoveListUCI(pv[PLY_ROOT]));
      sendResultToEngine();
    }
    else {
      LOG->info("Pondering has been stopped. Ponder Miss!");
    }
    searchLimits.ponderStop();
  }

  // set stop flag - search needs to check regularly and stop accordingly
  stopSearchFlag = true;

  // Wait for the thread to die
  if (myThread.joinable()) myThread.join();
  waitWhileSearching();

  assert(!running);
  LOG->info("Search stopped.", running);
}

bool Search::isRunning() const {
  return running;
}

void Search::waitWhileSearching() {
  TRACE(LOG, "Wait while searching");
  if (!running) return;
  searchSemaphore.getOrWait();
  searchSemaphore.reset();
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
        LOG->debug("Time Management: {} Time limit: {:n}", (searchLimits.isTimeControl() ? "ON"
                                                                                         : "OFF"), timeLimit);
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
 * @param position the position to be searched given as value (copied)
 */
void Search::run(Position position) {
  TRACE(LOG, "Search thread started.");

  // get the search lock
  searchSemaphore.getOrWait();
  running = true;

  // store the start time of the search
  startTime = lastUciUpdateTime = now();

  // Initialize for new search
  myColor = position.getNextPlayer();
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
  lastSearchResult = iterativeDeepening(position);

  // if the mode still is ponder at this point we finished the ponder
  // search early before a miss or hit has been signaled. We need to
  // wait with sending the result until we get a miss (stop) or a hit.
  if (searchLimits.isPonder()) {
    LOG->info("Ponder Search finished! Waiting for Ponderhit to send result");
    return;
  }

  sendResultToEngine();

  // print result of the search
  LOG->info("Search statistics: {}", searchStats.str());
  if (SearchConfig::USE_TT) LOG->info(tt.str());
  LOG->info("Search Depth was {} ({})", searchStats.currentIterationDepth, searchStats.currentExtraSearchDepth);
  LOG->info("Search took {},{:03} sec ({:n} nps)",
            (searchStats.lastSearchTime % 1'000'000) / 1'000, (searchStats.lastSearchTime % 1'000),
            (searchStats.nodesVisited * 1'000) / (searchStats.lastSearchTime + 1));

  running = false;
  searchSemaphore.reset();
  TRACE(LOG, "Search thread ended.");
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

  // print search setup for debugging
  LOG->info("Searching in position: {}", position.printFen());
  LOG->debug("Root moves: {}", printMoveList(rootMoves));
  LOG->info("Searching these moves: {}", printMoveList(rootMoves));
  LOG->info("Search mode: {}", searchLimits.str());
  LOG->info("Time Management: {} time limit: {:n}", (searchLimits.isTimeControl() ? "ON"
                                                                                  : "OFF"), timeLimit);
  LOG->info("Start Depth: {} Max Depth: {}", iterationDepth, searchLimits.getMaxDepth());
  LOG->debug("Starting iterative deepening now...");

  // max window search - preparation for aspiration window search
  Value alpha = VALUE_MIN;
  Value beta = VALUE_MAX;

  // check search requirements
  assert (!rootMoves.empty() && "No root moves to search");
  assert (iterationDepth > 0 && "iterationDepth <= 0");

  // ###########################################
  // ### BEGIN Iterative Deepening
  do {
    TRACE(LOG, "Iteration Depth {} START", iterationDepth);

    searchStats.currentIterationDepth = iterationDepth;
    searchStats.bestMoveChanges = 0;
    searchStats.nodesVisited++;

    // protect the TT from being resized or cleared during search
    tt_lock.lock();

    // ###########################################
    // ### CALL SEARCH for iterationDepth
    search<ROOT, PV>(position, iterationDepth, PLY_ROOT, alpha, beta);
    // ###########################################

    // release lock on TT
    tt_lock.unlock();

    // here should have a best root move with a value.
    if (!stopSearchFlag) {
      assert (
        searchLimits.isPerft() || (pv[PLY_ROOT].at(0) != MOVE_NONE && "Best root move missing!"));
      assert (searchLimits.isPerft() ||
              (valueOf(pv[PLY_ROOT].at(0)) != VALUE_NONE && "Best root move has no value!"));
    }

    // break on stop signal or time
    if (stopConditions(true)) break;

    sendIterationEndInfoToEngine();

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
  searchResult.bestMove = pv[PLY_ROOT].empty() ? MOVE_NONE : pv[PLY_ROOT].at(0);
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
 *
 * @tparam T
 * @param position
 * @param depth
 */
template<Search::Search_Type ST, Search::Node_Type NT>
Value Search::search(Position &position, Depth depth, Ply ply, Value alpha, Value beta) {
  assert (alpha >= VALUE_MIN && beta <= VALUE_MAX && "alpha/beta out if range");

  const bool PERFT = searchLimits.isPerft();

  TRACE(LOG, "{:>{}}Search {} in ply {} for depth {}: START alpha={} beta={} currline={}", "", ply, (
    ST == ROOT ? "ROOT" : ST == NONROOT ? "NONROOT"
                                        : "QUIESCENCE"), ply, depth, alpha, beta, printMoveListUCI(currentVariation));

  // Check if search should be stopped
  if (stopConditions(shouldTimeCheck())) {
    return VALUE_NONE;
  }

  // Leaf node handling
  switch (ST) {
    case ROOT: // fall through
    case NONROOT:
      searchStats.currentSearchDepth = std::max(searchStats.currentSearchDepth, ply);
      searchStats.currentExtraSearchDepth = std::max(searchStats.currentExtraSearchDepth, ply);
      if (depth <= DEPTH_NONE || ply >= PLY_MAX - 1) {
        if (SearchConfig::USE_QUIESCENCE && !PERFT) {
          return search<QUIESCENCE, NT>(position, depth, ply, alpha, beta);
        }
        else {
          Value eval = evaluate(position);
          TRACE(LOG, "{:>{}} Evaluation: {} {}", " ", ply, printMoveListUCI(currentVariation), eval);
          return eval;
        }
      }
      break;
    case QUIESCENCE:
      if (ply >= PLY_MAX - 1) {
        Value eval = evaluate(position);
        TRACE(LOG, "{:>{}} Evaluation: {} {}", " ", ply, printMoveListUCI(currentVariation), eval);
        return eval;
      }
      searchStats.currentExtraSearchDepth = std::max(searchStats.currentExtraSearchDepth, ply);
      break;
  }

  // ###############################################
  // Mate Distance Pruning
  // Did we already find a shorter mate then ignore
  // this one.
  if (SearchConfig::USE_MDP
      && ST != ROOT && !PERFT) {
    alpha = std::max(-VALUE_CHECKMATE + ply, alpha);
    beta = std::min(VALUE_CHECKMATE - ply, beta);
    if (alpha >= beta) {
      assert (isCheckMateValue(alpha));
      searchStats.mateDistancePrunings++;
      TRACE(LOG, "{:>{}}Search in ply %d for depth %d: MDP CUT", "", ply, ply, depth);
      return alpha;
    }
  }
  // ###############################################

  // prepare node search
  Value bestNodeValue = VALUE_NONE;
  Value ttValue = VALUE_NONE;
  Move ttStoreMove = MOVE_NONE;
  Move ttMove = MOVE_NONE;
  TT::EntryType ttType = TT::TYPE_ALPHA;
  moveGenerators[ply].resetOnDemand();
  if (ST != ROOT) {
    pv[ply].clear();
  }
  else {
    currentMoveIndex = 0;
  }

  // ###############################################
  // TT Lookup
  if (SearchConfig::USE_TT
      && (SearchConfig::USE_TT_QSEARCH || ST != QUIESCENCE) && !PERFT) {

    // probe the TT and set ttValue and ttMove in case of HIT
    TT::Result result =
      tt.probe(position.getZobristKey(), depth, alpha, beta, ttValue, ttMove, NT == PV);

    /* TT PROBE
     * tt.probe has two results:
     * TT_HIT:
     *  If this is a PV node a TT_HIT is an EXACT value of a fully
     *  searched node. In non PV nodes it is also a HIT if it is a value
     *  outside  of our current alpha/beta bound with the corresponding
     *  type stored in the TT. (for ALPHA value<=alpha, for BETA value>=beta)
     *  In case of a TT_HIT we can immediately stop the search and return the
     *  value.
     *  If we are in the ROOT ply this means we have already searched this
     *  position to the depth (or deeper) of the current depth to go.
     *  We will then try to get the PV line from the TT to be sent to the UI.
     * TT_MISS:
     *  If we have a miss we can't use the value for a cut off and have to
     *  search the node.
     *  We might have a best move to try first from previous searches of this
     *  node (e.g. lower or equal depth, beta cut off).
     */
    switch (result) {
      case TT::TT_HIT:
        searchStats.tt_Hits++;
        // correct mate values
        ttValue = valueFromTT(ttValue, ply);
        if (ST == ROOT) {
          getPVLine(position, pv[PLY_ROOT]);
          setValue(rootMoves.at(currentMoveIndex), ttValue);
          setValue(pv[PLY_ROOT].at(0), ttValue);
        }
        return ttValue;
      case TT::TT_MISS:
        // no usable value
        searchStats.tt_Misses++;
        break;
    }
  }
  // End TT Lookup
  // ###############################################

  // ###############################################
  // Quiescence StandPat
  // Use evaluation as a standing pat (lower bound)
  // https://www.chessprogramming.org/Quiescence_Search#Standing_Pat
  // Assumption is that there is at least on move which would improve the
  // current position. So if we are already >beta we don't need to look at it.
  if (ST == QUIESCENCE && !position.hasCheck()) {
    Value standPat = evaluate(position);
    bestNodeValue = standPat;
    TRACE(LOG, "{:>{}}Quiescence in ply {}: STANDPAT {}", "", ply, ply, standPat);
    if (standPat >= beta) {
      TRACE(LOG, "{:>{}}Quiescence in ply {}: STANDPAT CUT ({} > {} beta)", "", ply, ply, standPat, beta);
      if (SearchConfig::USE_TT_QSEARCH) {
        storeTT(position, standPat, TT::TYPE_BETA, DEPTH_NONE, ply, MOVE_NONE, false);
      }
      return standPat; // fail-hard: beta, fail-soft: statEval
    }
    if (standPat > alpha) {
      alpha = standPat;
    }
  }
  // ###############################################

  // ###############################################
  // INTERNAL ITERATIVE DEEPENING
  // If we didn't get a best move from the TT to play
  // first (PV) then do a shallow search to find
  // one. This is most effective with bad move ordering.
  // If move ordering is quite good this might be
  // a waste of search time.
  // TODO: look at this - doesn't seem to help
  // @formatter:off
  if (SearchConfig::USE_IID
      && !PERFT
      && NT == PV
      && !ttMove 
      && depth > SearchConfig::IID_REDUCTION) {
    // @formatter:on
    //    fprintln("\n**IID SEARCH");
    //    fprintln(
    //      "**ST={:<10} NT={:<5} depth={:<2} ply={:<2} alpha={:>6} beta={:>6} ttValue={:>6} ttMove={:<30} ",
    //      ST == ROOT ? "ROOT" : ST == NONROOT ? "NONROOT" : "QUIESCENCE", NT == PV ? "PV" : "NonPV",
    //      depth, ply, alpha, beta, ttValue, printMoveVerbose(ttMove));
    searchStats.iidSearches++;
    Depth iidDepth = depth - SearchConfig::IID_REDUCTION;
    // do the iterative search which will eventually
    // fill the pv list and the TT
    search<ST, PV>(position, iidDepth, ply, alpha, beta);
    // no we look in the pv list if we have a best move
    //tt.probe(position.getZobristKey(), depth, alpha, beta, ttValue, ttMove);
    ttMove = pv[ply].empty() ? MOVE_NONE : pv[ply].at(0);
    //    fprintln("****IID SEARCH RESULT: pv={} pv[{}] = {}",
    //             printMoveVerbose(pv[ply].empty() ? MOVE_NONE : pv[ply].at(0)), ply,
    //             printMoveList(pv[ply]));
    //    fprintln("****IID SEARCH RESULT: ttMove={} ttValue={}\n", printMoveVerbose(ttMove), ttValue);
  }
  // ###############################################

  // make sure the pv move is returned first
  if (ST != ROOT && SearchConfig::USE_PV_MOVE_SORTING && ttMove) {
    moveGenerators[ply].setPV(ttMove);
  }

  // prepare move loop
  Value value = VALUE_NONE;
  Move move = MOVE_NONE;
  int movesSearched = 0; // to detect mate situations

  // ###########################################################################
  // MOVE LOOP
  while ((move = getMove<ST>(position, ply)) != MOVE_NONE) {

    if (ST == ROOT) {
      TRACE(LOG, "Root Move {} START", printMove(move));
      if (depth > 5) { // avoid protocol flooding
        sendCurrentRootMoveToEngine();
      }
    }
    else {
      TRACE(LOG, "{:>{}}Depth {} cv {} move {} START", "", ply, ply, printMoveListUCI(currentVariation), printMove(move));
    }

    // reduce number of moves searched in quiescence
    // by looking at good captures only
    if (ST == QUIESCENCE && !position.hasCheck() && !goodCapture(position, move)) {
      continue;
    }

    // ###############################################
    // Minor Promotion Pruning
    // Skip non queen or knight promotion as they are
    // redundant. Exception would be stale mate situations
    // which we ignore.
    if (SearchConfig::USE_MPP
        && !PERFT && ST != ROOT) {
      if (typeOf(move) == PROMOTION && promotionType(move) != QUEEN &&
          promotionType(move) != KNIGHT) {
        searchStats.minorPromotionPrunings++;
        TRACE(LOG, "{:>{}}Search in ply {} for depth {}: Move {} MPP CUT", "", ply, ply, depth, printMove(move));
        continue;
      }
    }
    // ###############################################

    // ************************************
    // Execute move
    position.doMove(move);
    searchStats.nodesVisited++;
    /* IDEA: (credit to Robert Hyatt) Instead of testing each move
        for legality we could simply go ahead and recurse into each node and
        if there is a king capture in one of the succeeding nodes we jump back
        and dismiss this move. */
    if (position.isLegalPosition()) {
      currentVariation.push_back(move);
      sendSearchUpdateToEngine();

      // check for repetition or 50-move-rule draws
      if (checkDrawRepAnd50<ST>(position)) {
        value = VALUE_DRAW;
      }
      else {
        // reduce depth by 1 in the next search
        Depth newDepth = ST == QUIESCENCE ? DEPTH_NONE : depth - 1;

        // ROOT is used only at the start - changes directly to NONROOT
        const Search::Search_Type nextST = ST == ROOT ? NONROOT : ST;

        if (!SearchConfig::USE_PVS || movesSearched == 0 || PERFT ) {
          // AlphaBeta Search or initial search in PVS
          value = -search<nextST, PV>(position, newDepth, ply + 1, -beta, -alpha);
        }
        else {
          // #############################
          // PVS Search /START
          value = -search<nextST, NonPV>(position, newDepth, ply + 1, -alpha - 1, -alpha);
          if (value > alpha && value < beta && !stopSearchFlag) {
            if (ST == ROOT) { searchStats.pvs_root_researches++; }
            else { searchStats.pvs_researches++; }
            value = -search<nextST, PV>(position, newDepth, ply + 1, -beta, -alpha);
          }
          else {
            if (ST == ROOT) { searchStats.pvs_root_cutoffs++; }
            else { searchStats.pvs_cutoffs++; }
          }
          // PVS Search /END
          // #############################
        }
      }
      assert ((value != VALUE_NONE || stopSearchFlag) && "Value should not be NONE at this point.");

      movesSearched++;
      currentVariation.pop_back();
    } // if (position.isLegalPosition())
    position.undoMove();
    // ************************************

    if (stopConditions(false)) {
      return VALUE_NONE;
    }

    // For root moves encode value into the move
    // so we can sort the move before the next iteration
    if (ST == ROOT) {
      setValue(rootMoves.at(currentMoveIndex++), value);
    }

    // In PERFT we can ignore values and pruning
    if (PERFT) {
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
            ttType = TT::TYPE_BETA; // store the beta value into the TT later
            TRACE(LOG, "{:>{}} Search in ply {} for depth {}: CUT NODE {} >= {} (beta)", " ", ply, ply, depth, value, beta);
            break; // get out of loop and return the value at the end
          }
          else {
            /*
            We found a move between alpha and beta which means we
            really have found a the best move so far in the ply which
            we can force (opponent can't avoid it).
            We raise alpha so the successive searches in this ply
            need to find even better moves or dismiss the moves.
            */
            alpha = value;
            ttType = TT::TYPE_EXACT;
            setValue(ttStoreMove, bestNodeValue);
            savePV(ttStoreMove, pv[ply + 1], pv[ply]);
            TRACE(LOG, "{:>{}}Search in ply {} for depth {}: NEW PV {} ({}) (alpha) PV: {}", "", ply, ply, depth, printMove(move), value, printMoveListUCI(pv[ply]));
          }
        }
      }
      else { // Minimax
        setValue(move, value);
        savePV(move, pv[ply + 1], pv[ply]);
        ttType = TT::TYPE_EXACT;
        TRACE(LOG, "{:>{}}Search in ply {} for depth {}: NEW PV {} ({}) PV: {}", "", ply, ply, depth, printMove(move), value, printMoveListUCI(pv[ply]));
      }
    }

    if (ST == ROOT) {
      TRACE(LOG, "Root Move {} END", printMove(move));
    }
    else {
      TRACE(LOG, "{:>{}}Depth {} cv {} move {} END", " ", ply, ply, printMoveListUCI(currentVariation), printMove(move));
    }
  }
  // ##### Iterate through all available moves
  // ###########################################################################

  // do some checks
  if (!PERFT && SearchConfig::USE_ALPHABETA) {
    // In an EXACT node we should have a best move and a PV
    if (ttType == TT::TYPE_EXACT) {
      assert (ttStoreMove);
      assert (!pv[ply].empty());
      assert (alpha <= bestNodeValue && bestNodeValue <= beta);
    }

    // In a BETA node we should have a best move for the TT (might not be the best due to cut off)
    if (ttType == TT::TYPE_BETA) {
      assert (ttStoreMove);
      assert (bestNodeValue >= beta);
    }

    // We should not have found a best move in an ALPHA node (all moves were worse than alpha)
    if (ttType == TT::TYPE_ALPHA) {
      assert (ttStoreMove == MOVE_NONE);
      assert (bestNodeValue <= alpha);
    }
  }

  // if we did not have at least one legal move
  // then we might have a mate or in quiescence
  // only quite moves
  if (!movesSearched && !stopSearchFlag) {
    searchStats.nonLeafPositionsEvaluated++;
    assert (ttType == TT::TYPE_ALPHA);
    TRACE(LOG, "{:>{}}Depth {} cv {} NO LEGAL MOVES", "", ply, ply, printMoveListUCI(currentVariation));
    if (position.hasCheck()) {
      /* If the position has check we have a mate even in
         quiescence as we will have generated all moves
         because of the check. Return a -CHECKMATE. */
      bestNodeValue = -VALUE_CHECKMATE + ply;
      ttType = TT::TYPE_EXACT;
      assert (ttStoreMove == MOVE_NONE);
      TRACE(LOG, "{:>{}} Search in ply {} for depth {}: {} CHECKMATE", " ", ply, ply, depth, bestNodeValue);
    }
    else if (ST != QUIESCENCE) {
      /* If not in quiescence we have a stale mate.
         Return the draw value. */
      bestNodeValue = VALUE_DRAW;
      ttType = TT::TYPE_EXACT;
      assert (ttStoreMove == MOVE_NONE);
      TRACE(LOG, "{:>{}} Search in ply {} for depth {}: {} STALEMATE", " ", ply, ply, depth, bestNodeValue);
    }
    /* In quiescence having searched no moves while
       not in check means that there were only quiet
       moves which we ignored on purpose and return
       the StandPat */
  }

  TRACE(LOG, "{:>{}}Search {} in ply {} for depth {}: END value={} ({} moves searched) ({})", "", ply, (ST == ROOT ? "ROOT" : ST == NONROOT ? "NONROOT" : "QUIESCENCE"), ply, depth, bestNodeValue, movesSearched, printMoveListUCI(currentVariation));

  // best value should in any case not be VALUE_NONE any more
  assert ((bestNodeValue >= VALUE_MIN && bestNodeValue <= VALUE_MAX && "bestNodeValue should not be MIN/MAX here"));

  // store TT data
  switch (ST) {
    case ROOT: // fall through
    case NONROOT:
      if (!PERFT && SearchConfig::USE_TT) {
        TRACE(LOG, "{:>{}}Search storing into TT: {} {} {} {} {} {} {}", "", ply, position.getZobristKey(), bestNodeValue, TT::str(ttType), depth, printMove(ttStoreMove), false, position.printFen());
        storeTT(position, bestNodeValue, ttType, depth, ply, ttStoreMove, false);
      }
      break;
    case QUIESCENCE:
      if (!PERFT && SearchConfig::USE_TT && SearchConfig::USE_TT_QSEARCH) {
        TRACE(LOG, "{:>{}}Quiescence storing into TT: {} {} {} {} {} {} {}", "", ply, position.getZobristKey(), bestNodeValue, TT::str(ttType), depth, printMove(ttStoreMove), false, position.printFen());
        storeTT(position, bestNodeValue, ttType, DEPTH_NONE, ply, ttStoreMove, false);
      }
      break;
  }

  return bestNodeValue;
}

Value Search::evaluate(Position &position) {
  // count all leaf nodes evaluated
  searchStats.leafPositionsEvaluated++;

  // PERFT stats
  if (searchLimits.isPerft()) {
    //    Move lastMove = position.getLastMove();
    //    if (position.getLastCapturedPiece() != PIECE_NONE) searchStats.captureCounter++;
    //    if (typeOf(lastMove) == ENPASSANT) searchStats.enPassantCounter++;
    //    if (position.hasCheck()) searchStats.checkCounter++;
    //    if (position.hasCheckMate()) searchStats.checkMateCounter++;
    return VALUE_ONE;
  }

  return evaluator.evaluate(position);
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
 * Simple "good capture" determination
 *
 * OBS: move must be a capture otherwise too many false positives
 * TODO: Improve, add SEE
 */
bool Search::goodCapture(Position &position, Move move) {
  assert (position.isCapturingMove(move));
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
    // If the defender is "behind" the attacker this will not be recognized here
    // This is not too bad as it only adds a move to qsearch which we could otherwise ignore
    || !position.isAttacked(getToSquare(move), ~position.getNextPlayer());
}

inline void
Search::storeTT(Position &position, Value value, TT::EntryType ttType, Depth depth, Ply ply,
                Move bestMove, bool mateThreat) {

  if (!SearchConfig::USE_TT || searchLimits.isPerft() || stopSearchFlag) return;

  assert (depth >= 0 && depth <= DEPTH_MAX);
  assert ((value >= VALUE_MIN && value <= VALUE_MAX));

  // store the position in the TT
  // correct the value for mate distance and remove the value from the move to
  // later be able to easier compare it wh read from TT
  tt.put(position.getZobristKey(), depth, moveOf(bestMove), valueToTT(value, ply), ttType, mateThreat);

}

inline Value Search::valueToTT(Value value, Ply ply) {
  assert (value != VALUE_NONE);
  return isCheckMateValue(value) ? Value(value > 0 ? value + ply : value - ply) : value;
}

inline Value Search::valueFromTT(Value value, Ply ply) {
  return isCheckMateValue(value) ? Value(value > 0 ? value - ply : value + ply) : value;
}

inline bool Search::stopConditions(bool shouldTimeCheck) {
  if (stopSearchFlag) return true;
  if ((shouldTimeCheck && timeLimitReached()) ||
      (searchLimits.getNodes() && searchStats.nodesVisited >= searchLimits.getNodes())) {
    stopSearchFlag = true;
  }
  return stopSearchFlag;
}

inline bool Search::shouldTimeCheck() const {
  return !(searchStats.nodesVisited & TIME_CHECK_FREQ);
}

template<Search::Search_Type ST>
bool Search::checkDrawRepAnd50(Position &position) const {
  // for quiescence search we stop at 1 repetition already which should not
  // loose too much precision
  constexpr int allowedRepetitions = (ST == QUIESCENCE ? 1 : 2);
  if (position.checkRepetitions(allowedRepetitions)) {
    TRACE(LOG, "DRAW because of repetition for move {} in variation {}", printMove(position.getLastMove()), printMoveListUCI(this->currentVariation));
    return true;
  }
  if (position.getHalfMoveClock() >= 100) {
    TRACE(LOG, "DRAW because 50-move rule", printMove(position.getLastMove()), printMoveListUCI(this->currentVariation));
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
    assert (
      (searchLimits.getWhiteTime() && searchLimits.getBlackTime()) && "move times must be > 0");
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

void Search::addExtraTime(const double d) {
  if (!searchLimits.getMoveTime()) {
    extraTime += timeLimit * (d - 1);
    LOG->debug("Time added {:n} ms to {:n} ms", extraTime, timeLimit + extraTime);
  }
}

inline bool Search::timeLimitReached() {
  if (!searchLimits.isTimeControl()) return false;
  return elapsedTime(startTime) >= timeLimit + extraTime;
}

inline MilliSec Search::elapsedTime(const MilliSec t) {
  return elapsedTime(t, now());
}

inline MilliSec Search::elapsedTime(const MilliSec t1, const MilliSec t2) {
  return t2 - t1;
}

inline MilliSec Search::now() {
  // this C function is much faster than c++ chrono
  return clock_gettime_nsec_np(CLOCK_UPTIME_RAW_APPROX) / 1'000'000;
}

inline MilliSec Search::getNps() const {
  return 1000 * searchStats.nodesVisited /
         (elapsedTime(startTime) + 1); // +1 to avoid division by zero};
}

inline void Search::savePV(Move move, MoveList &src, MoveList &dest) {
  dest = src;
  dest.push_front(move);
}

void Search::getPVLine(Position &position, MoveList &pvRoot) {
  // Recursion-less reading of the chain of pv moves
  pvRoot.clear();
  int counter = 0;
  TT::Entry ttEntry;
  while ((ttEntry = tt.getEntry(position.getZobristKey())).key != 0 && ttEntry.move != MOVE_NONE) {
    pvRoot.push_back(ttEntry.move);
    position.doMove(ttEntry.move);
    counter++;
  }
  for (int i = 0; i < counter; ++i) {
    position.undoMove();
  }
}

MoveList Search::generateRootMoves(Position &position) {
  moveGenerators[PLY_ROOT].reset();
  const MoveList* legalMoves = moveGenerators[PLY_ROOT].generateLegalMoves<MoveGenerator::GENALL>(position);

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
  TRACE(LOG, "Search: Clear Hash command received!");
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
  TRACE(LOG, "Search: Set HashSize to {} MB command received!", sizeInMB);
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
  if (!pEngine) {
    LOG->info("UCI >> {}", fmt::format("depth {} seldepth {} multipv 1 {} nodes {:n} nps {:n} time {:n} pv {}", searchStats.currentIterationDepth, searchStats.currentExtraSearchDepth, (searchLimits.isPerft()
                                                                                                                                                                                         ? VALUE_ZERO
                                                                                                                                                                                         : valueOf(pv[PLY_ROOT].at(0))), searchStats.nodesVisited, getNps(), elapsedTime(startTime), printMoveListUCI(pv[PLY_ROOT])));
  }
  else {
    pEngine->sendIterationEndInfo(searchStats.currentIterationDepth, searchStats.currentExtraSearchDepth, searchLimits.isPerft()
                                                                                                          ? VALUE_ZERO
                                                                                                          : valueOf(pv[PLY_ROOT].at(0)), searchStats.nodesVisited, getNps(), elapsedTime(startTime), pv[PLY_ROOT]);
  }
}

void Search::sendCurrentRootMoveToEngine() const {
  if (!pEngine) {
    TRACE(LOG, "UCI >> {}", fmt::format("currmove {} currmovenumber {}", printMove(searchStats.currentRootMove),
                                        currentMoveIndex + 1));
  }
  else {
    pEngine->sendCurrentRootMove(searchStats.currentRootMove, currentMoveIndex + 1);
  }
}

void Search::sendSearchUpdateToEngine() {
  if (elapsedTime(lastUciUpdateTime) > UCI_UPDATE_INTERVAL) {
    lastUciUpdateTime = now();
    if (!pEngine) {
      LOG->info("UCI >> {}", fmt::format("depth {} seldepth {} nodes {:n} nps {:n} time {:n} hashfull {}", searchStats.currentIterationDepth, searchStats.currentExtraSearchDepth, searchStats.nodesVisited, getNps(), elapsedTime(startTime), tt.hashFull()));
    }
    else {
      pEngine->sendSearchUpdate(searchStats.currentIterationDepth, searchStats.currentExtraSearchDepth, searchStats.nodesVisited, getNps(), elapsedTime(startTime), 0);
    }

    if (!pEngine) {
      TRACE(LOG, "UCI >> {}", fmt::format("currline {}", printMoveListUCI(currentVariation)));
    }
    else {
      pEngine->sendCurrentLine(currentVariation);
    }
  }
}

void Search::sendResultToEngine() const {
  if (!pEngine) {
    LOG->info("UCI >> {}", fmt::format("Engine got Best Move: {} ({}) [Ponder {}] from depth {}", printMove(lastSearchResult.bestMove), printValue(valueOf(lastSearchResult.bestMove)), printMove(lastSearchResult.ponderMove), searchStats.bestMoveDepth));
  }
  else {
    pEngine->sendResult(lastSearchResult.bestMove, lastSearchResult.ponderMove);
  }

}


















