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
  position = pos;
  searchLimits = limits;
  myColor = position.getNextPlayer();

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
  if (searchLimits.ponder) {
    if (!isRunning()) {
      // Ponder search has finished before we stopped it
      // Per UCI protocol we need to send the result anyway although a miss
      LOG->info(
        "Pondering has been stopped after ponder search has finished. Send obsolete result");
      LOG->info("Search result was: {} PV {}", lastSearchResult.str(),
                printMoveListUCI(pv[ROOT_PLY]));
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
  if (searchLimits.ponder) {
    LOG->info("****** PONDERHIT *******");
    if (isRunning()) {
      LOG->info("Ponderhit when ponder search still running. Continue searching.");
      // store the start time of the search
      startTime = now();
      searchLimits.ponderHit();
      // if time based game setup the time soft and hard time limits
      if (searchLimits.timeControl) {
        configureTimeLimits();
        LOG->debug("Time Management: {} time limit: {:n}",
                   (searchLimits.timeControl ? "ON" : "OFF"),
                   timeLimit);
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
  for (int i = 0; i < MAX_SEARCH_DEPTH; i++) {
    moveGenerators[i] = MoveGenerator();
    pv[i] = MoveList();
  }

  // search mode
  if (searchLimits.perft) {
    LOG->info("Search Mode: PERFT SEARCH ({})", searchLimits.maxDepth);
  }
  if (searchLimits.infinite) {
    LOG->info("Search Mode: INFINITE SEARCH");
  }
  if (searchLimits.ponder) {
    LOG->info("Search Mode: PONDER SEARCH");
  }
  if (searchLimits.mate) {
    LOG->info("Search Mode: MATE SEARCH ({})", searchLimits.mate);
  }

  // initialization done
  initSemaphore.release();

  // if time based game setup the soft and hard time limits
  if (searchLimits.timeControl) configureTimeLimits();

  // start iterative deepening
  lastSearchResult = iterativeDeepening(&position);

  // if the mode still is ponder at this point we finished the ponder
  // search early before a miss or hit has been signaled. We need to
  // wait with sending the result until we get a miss (stop) or a hit.
  if (searchLimits.ponder) {
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
 * @param pPosition
 * @return a SearchResult
 */
SearchResult Search::iterativeDeepening(Position* pPosition) {

  // prepare search result
  SearchResult searchResult = SearchResult();

  // no legal root moves - game already ended!
  if (!moveGenerators[ROOT_PLY].hasLegalMove(&position)) {
    if (position.hasCheck()) {
      searchResult.bestMove = NOMOVE;
      setValue(searchResult.bestMove, -VALUE_CHECKMATE);
    }
    else {
      searchResult.bestMove = NOMOVE;
      setValue(searchResult.bestMove, VALUE_DRAW);
    }
    return searchResult;
  }

  int iterationDepth = searchLimits.startDepth;

  // current search iterationDepth
  searchStats.currentSearchDepth = ROOT_PLY;
  searchStats.currentExtraSearchDepth = ROOT_PLY;

  // generate all legal root moves
  rootMoves = generateRootMoves(&position);

  // make sure we have a temporary best move
  // when using TT this will already be set
  pv[ROOT_PLY].push_back(rootMoves.front());

  // print search setup for debugging
  if (LOG->should_log(spdlog::level::debug)) {
    LOG->debug("Root moves: {}", printMoveList(rootMoves));
    LOG->debug("Searching in position: {}", position.printFen());
    LOG->debug("Searching these moves: {}", printMoveList(rootMoves));
    LOG->debug("Search mode: {}", searchLimits.str());
    LOG->debug("Time Management: {} time limit: {:n}",
               (searchLimits.timeControl ? "ON" : "OFF"),
               timeLimit);
    LOG->debug("Start Depth: {} Max Depth: {}", iterationDepth, searchLimits.maxDepth);
    LOG->debug("Starting iterative deepening now...");
  }

  // max window search - preparation for aspiration window search
  int alpha = VALUE_MIN;
  int beta = VALUE_MAX;

  // check search requirements
  assert (!rootMoves.empty() && "No root moves to search");
  assert (iterationDepth > 0 && "iterationDepth <= 0");

  // first iteration update before start
  sendIterationEndInfoToEngine();

  // ###########################################
  // ### BEGIN Iterative Deepening
  do {
    assert (pv[ROOT_PLY].front() != NOMOVE && "No best root move");

    TRACE(LOG, "Iteration Depth {} START", iterationDepth);

    searchStats.currentIterationDepth = iterationDepth;
    searchStats.bestMoveChanges = 0;
    searchStats.nodesVisited++;

    // ###########################################
    // ### CALL SEARCH for iterationDepth
    search<ROOT>(pPosition, iterationDepth, ROOT_PLY, alpha, beta);
    // ###########################################
  
    sendIterationEndInfoToEngine();

    // break on stop signal or time
    if (stopConditions()) break;

    // sort root moves based on value for the next iteration
    std::stable_sort(rootMoves.begin(), rootMoves.end(), std::greater());

    TRACE(LOG, "Iteration Depth={} END", iterationDepth);

  } while (++iterationDepth <= searchLimits.maxDepth);
  // ### ENDOF Iterative Deepening
  // ###########################################

  // update searchResult here
  searchResult.bestMove = pv[ROOT_PLY].front();
  searchResult.ponderMove = pv[ROOT_PLY].size() > 1 ? pv[ROOT_PLY].at(1) : NOMOVE;
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
    LOG->debug("Search took {}",
               fmt::format("{},{:03} sec",
                           (searchStats.lastSearchTime % 1'000'000) / 1'000,
                           (searchStats.lastSearchTime % 1'000)));
  }

  return searchResult;
}

/**
 * This is the templated search function for root, non-root and quiescence
 * searches. Through the template the compiler will generate specialized
 * versions of this method for each case.
 * 
 * @tparam T
 * @param pPosition
 * @param depth
 */
template<Search::Search_Type ST>
Value Search::search(Position* pPosition, int depth, int ply, int alpha, int beta) {
  assert (alpha >= VALUE_MIN && beta <= VALUE_MAX && "alpha/beta out if range");

  constexpr bool root = ST == ROOT;
  constexpr bool nonroot = ST == NONROOT;
  constexpr bool quiescence = ST == QUIESCENCE;

  TRACE(LOG, "{:>{}}Search {} in ply {} for depth {}: START alpha={} beta={} currline={}",
        "", ply, (root ? "ROOT" : nonroot ? "NONROOT" : "QUIESCENCE"),
        ply, depth, alpha, beta, printMoveListUCI(currentVariation));

  if (stopConditions()) return VALUE_NONE;

  // Leaf node handling
  if (!quiescence) { // normal search
    searchStats.currentSearchDepth = std::max(searchStats.currentSearchDepth, ply);
    searchStats.currentExtraSearchDepth = std::max(searchStats.currentExtraSearchDepth, ply);
    if (depth <= 0 || ply >= MAX_SEARCH_DEPTH - 1) {
      return search<QUIESCENCE>(pPosition, depth, ply, alpha, beta);
    }
  }
  else { // quiescence search
    if (searchLimits.perft || ply >= MAX_SEARCH_DEPTH - 1 || !SearchConfig::USE_QUIESCENCE) {
      return evaluate(pPosition, ply);
    }
    searchStats.currentExtraSearchDepth = std::max(searchStats.currentExtraSearchDepth, ply);
  }

  // to detect mate situations
  int movesSearched = 0;

  // prepare move loop
  Value bestNodeValue = VALUE_MIN;
  Value value = VALUE_NONE;
  Move move = NOMOVE;
  moveGenerators[ply].resetOnDemand();
  if (root) {
    currentMoveIndex = 0;
  }

  // ###############################################
  // Quiescence StandPat
  // Use evaluation as a standing pat (lower bound)
  // https://www.chessprogramming.org/Quiescence_Search#Standing_Pat
  // Assumption is that there is at least on move which would improve the
  // current position. So if we are already >beta we don't need to look at it.
  if (quiescence && !pPosition->hasCheck()) {
    Value standPat = evaluate(pPosition, ply);
    bestNodeValue = standPat;
    TRACE(LOG, "{:>{}}Quiescence in ply {}: STANDPAT {}", "", ply, ply, standPat);
    if (standPat >= beta) {
      TRACE(LOG, "{:>{}}Quiescence in ply {}: STANDPAT CUT ({} > {} beta)",
            "", ply, ply, standPat, beta);
      return standPat; // fail-hard: beta, fail-soft: statEval
    }
    if (standPat > alpha) alpha = standPat;
  }
  // ###############################################

  // ###############################################
  // MOVE LOOP
  while ((move = getMove<ST>(pPosition, ply)) != NOMOVE) {

    if (root) {
      TRACE(LOG, "Root Move {} START", printMove(move));
      if (depth > 4) sendCurrentRootMoveToEngine(); // avoid protocol flooding
    }
    else {
      TRACE(LOG, "{:>{}}Depth {} cv {} move {} START",
            "", ply, ply, printMoveListUCI(currentVariation), printMove(move));
    }

    // reduce number of moves searched in quiescence
    // by looking at good captures only
    if (quiescence && !goodCapture(pPosition, move)) {
      move = getMove<QUIESCENCE>(pPosition, ply);
      continue;
    }

    // ************************************
    // Execute move
    pPosition->doMove(move);
    if (pPosition->isLegalPosition()) {
      currentVariation.push_back(move);
      searchStats.nodesVisited++;
      movesSearched++;
      sendSearchUpdateToEngine();

      // check for repetition ot 50-move-rule draws
      if (checkDrawRepAnd50<ST>(pPosition))
        value = VALUE_DRAW;
      else {
        // recurse deeper into the search tree
        if (root) {
          value = -search<NONROOT>(&position, depth - 1, ply + 1, -beta, -alpha);
        }
        else {
          value = -search<ST>(&position, depth - 1, ply + 1, -beta, -alpha);
        }
      }
      assert ((value != VALUE_NONE || stopSearchFlag) && "Value should not be NONE at this point.");

      currentVariation.pop_back();
    }
    pPosition->undoMove();
    // ************************************

    if (stopConditions()) return VALUE_NONE;

    // In PERFT we can ignore values and pruning
    if (searchLimits.perft) continue;

    // for root moves encode value into the move
    // so we can sort the move before the next iteration
    if (root) setValue(rootMoves.at(currentMoveIndex), value);

    // Did we find a better move for this node?
    // For the first move this is always the case.
    if (value > bestNodeValue) {

      bestNodeValue = value;

      // AlphaBeta
      if (SearchConfig::USE_ALPHABETA) {

        // If we found a move that is better or equal than beta this means that the
        // opponent can/will avoid this position altogether so we can stop search
        // this node
        if (value >= beta) { // fail-high
          // TODO: Add Killer handling
          searchStats.prunings++;
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
          savePV(move, pv[ply + 1], pv[ply]);
          alpha = value;
          if (root) searchStats.bestMoveChanges++;
          TRACE(LOG, "{:>{}}Search in ply {} for depth {}: NEW PV {} ({}) (alpha) PV: {}",
                "", ply, ply, depth, printMove(move), value, printMoveListUCI(pv[ply]));
        }
      }
      else { // Minimax
        savePV(move, pv[ply + 1], pv[ply]);
        bestNodeValue = value;
        if (ST == ROOT)
          TRACE(LOG, "NEW BEST ROOT move {} value {}",
                printMoveListUCI(pv[ROOT_PLY]), value);
        else
          TRACE(LOG, "{:>{}} NEW BEST MOVE for node {}  move={} old best={} value={} pv={}",
                " ", ply, printMoveListUCI(currentVariation), printMove(move),
                bestNodeValue, value, printMoveListUCI(pv[ROOT_PLY]));
      }
    }

    if (root) {
      currentMoveIndex++;
      TRACE(LOG, "Root Move {} END", printMove(move));
    }
    else {
      TRACE(LOG, "{:>{}} Depth {} cv {} move {} END",
            " ", ply, ply, printMoveListUCI(currentVariation), printMove(move));
    }
  }
  // ##### Iterate through all available moves
  // ##########################################################

  // if we did not have at least one legal move then we might have a mate or
  // in quiescence only quite moves
  if (!movesSearched && !stopSearchFlag) {
    searchStats.nonLeafPositionsEvaluated++;
    if (position.hasCheck()) {
      // if the position has check we have a mate even in quiescence as we will
      // have generated all moves because of the check.
      // Return a -CHECKMATE.
      bestNodeValue = -VALUE_CHECKMATE + Value(ply);
      TRACE(LOG, "{:>{}} Search in ply {} for depth {}: CHECKMATE", " ", ply, ply, depth);
    }
    else if (!quiescence) {
      // if not in quiescence we have a stale mate. Return the draw value.
      bestNodeValue = VALUE_DRAW;
      TRACE(LOG, "{:>{}} Search in ply {} for depth {}: STALEMATE", " ", ply, ply, depth);
    }
  }

  TRACE(LOG, "{:>{}}Search {} in ply {} for depth {}: END value={} ({} moves searched) ({})",
        "", ply, (root ? "ROOT" : nonroot ? "NONROOT" : "QUIESCENCE"),
        ply, depth, alpha, movesSearched, printMoveListUCI(currentVariation));

  assert (bestNodeValue > VALUE_MIN && bestNodeValue < VALUE_MAX
          && "bestNodeValue should not be MIN/MAX here");
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
Move Search::getMove(Position* pPosition, int ply) {
  TRACE(LOG, "{:>{}}Get move for position {} in ply {}", "", ply, pPosition->getZobristKey(), ply);
  Move move = NOMOVE;
  constexpr Search_Type type = ST;
  switch (type) {
    case ROOT:
      if (currentMoveIndex < rootMoves.size()) { ;
        move = rootMoves.at(currentMoveIndex);
        searchStats.currentRootMove = move;
      }
      break;
    case NONROOT:
      move = moveGenerators[ply].getNextPseudoLegalMove<MoveGenerator::GENALL>(pPosition);
      break;
    case QUIESCENCE:
      if (pPosition->hasCheck()) { // if in check look at all moves in quiescence
        move = moveGenerators[ply].getNextPseudoLegalMove<MoveGenerator::GENALL>(pPosition);
      }
      else { // if not in check only look at captures
        move = moveGenerators[ply].getNextPseudoLegalMove<MoveGenerator::GENCAP>(pPosition);
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
bool Search::goodCapture(Position* pPosition, Move move) {
  return
    // only captures
    pPosition->getPiece(getToSquare(move)) != PIECE_NONE
    // all pawn captures - they never loose material
    && (typeOf(pPosition->getPiece(getFromSquare(move))) == PAWN
        // Lower value piece captures higher value piece
        // With a margin to also look at Bishop x Knight
        || (valueOf(pPosition->getPiece(getFromSquare(move))) + 50) <=
           valueOf(pPosition->getPiece(getToSquare(move)))
        // all recaptures should be looked at
        || (pPosition->getLastMove() != NOMOVE
            && getToSquare(pPosition->getLastMove()) == getToSquare(move)
            && pPosition->getLastCapturedPiece() != PIECE_NONE)
        // undefended pieces captures are good
        // If the defender is "behind" the attacker this will not be recognized here
        // This is not too bad as it only adds a move to qsearch which we could otherwise ignore
        || !pPosition->isAttacked(getToSquare(move), ~pPosition->getNextPlayer()));
}

//
///**
// * Called for root position as root moves are generated separately and need to
// * take care of best moves and sorting root moves.
// * @param pPosition
// * @param depth
// * @param ply
// * @return
// */
//void Search::searchRoot(Position *pPosition, const int depth) {
//
//  // prepare move loop
//  Value bestNodeValue = VALUE_NONE;
//  Value value = VALUE_NONE;
//
//  int i = 0;
//  for (auto &move : rootMoves) {
//    TRACE(LOG, "Root Move {} START", printMove(move));
//
//    searchStats.currentRootMove = move;
//    searchStats.currentRootMoveNumber = ++i;
//    if (depth > 4) sendUCICurrentRootMove(); // avoid protocol flooding
//
//    if (true) {}; // prevent warnings
//
//    Value value1 = VALUE_NONE;
//    pPosition->doMove(move);
//    if (pPosition->isLegalPosition()) {
//      searchStats.nodesVisited++;
//      currentVariation.push_back(move);
//      sendUCISearchUpdate();
//
//      // check for repetition ot 50-move-rule draws
//      if (checkDrawRepAnd50(pPosition))
//        value1 = VALUE_DRAW;
//        // recurse deeper into the search tree
//      else
//        value1 = -searchNonRoot(&position, depth - 1, ROOT_PLY + 1);
//
//      currentVariation.pop_back();
//      assert (
//        (value1 != VALUE_NONE || stopSearchFlag) && "Value should not be NONE at this point.");
//    }
//    pPosition->undoMove();
//    value = value1;
//
//    if (stopConditions()) return;
//
//    // In PERFT we can ignore values and pruning
//    if (searchLimits.perft) continue;
//
//    // encode value into the move
//    setValue(move, value);
//
//    if (value > bestNodeValue) {
//      savePV(move, pv[1], pv[ROOT_PLY]);
//      bestNodeValue = value;
//      TRACE(LOG, "NEW BEST ROOT move {} value {}", printMoveListUCI(pv[ROOT_PLY]), value);
//    }
//    TRACE(LOG, "Root Move {} END", printMove(move));
//  }
//}
//
///**
// * Recursive search for all non root positions with on the fly move generation.
// * @param pPosition
// * @param depth
// * @param ply
// * @return
// */
//Value Search::searchNonRoot(Position *pPosition, const int depth, const int ply) {
//
//  if (stopConditions()) return VALUE_NONE; // value does not matter because of top flag
//
//  // update current search depth stats
//  searchStats.currentSearchDepth = std::max(searchStats.currentSearchDepth, ply);
//  searchStats.currentExtraSearchDepth = std::max(searchStats.currentExtraSearchDepth, ply);
//
//  // On leaf node call qsearch
//  if (depth <= 0 || ply >= MAX_SEARCH_DEPTH - 1) {
//    return qsearch(pPosition, ply);
//  }
//
//  // to detect mate situations
//  int numberOfSearchedMoves = 0;
//
//  // prepare move loop
//  Value bestNodeValue = VALUE_NONE;
//  Value value = VALUE_NONE;
//  Move move = NOMOVE;
//  moveGenerators[ply].resetOnDemand();
//
//  // ###############################################
//  // MOVE LOOP
//  // Search all generated moves using the onDemand move generator.
//  while ((move = moveGenerators[ply].getNextPseudoLegalMove(GENALL, pPosition)) != NOMOVE) {
//    TRACE(LOG, "{:>{}} Depth {} cv {} move {} START", " ", ply, ply,
//          printMoveListUCI(currentVariation), printMove(move));
//
//    if (false) {}; // prevent warnings
//
//    Value value1 = VALUE_NONE;
//    pPosition->doMove(move);
//    if (pPosition->isLegalPosition()) {
//      searchStats.nodesVisited++;
//      currentVariation.push_back(move);
//      sendUCISearchUpdate();
//
//      // check for repetition ot 50-move-rule draws
//      if (checkDrawRepAnd50(pPosition))
//        value1 = VALUE_DRAW;
//        // recurse deeper into the search tree
//      else
//        value1 = -searchNonRoot(&position, depth - 1, ply + 1);
//
//      currentVariation.pop_back();
//      assert (
//        (value1 != VALUE_NONE || stopSearchFlag) && "Value should not be NONE at this point.");
//    }
//    pPosition->undoMove();
//    value = value1;
//
//    if (stopConditions()) return VALUE_NONE; // value does not matter because of top flag
//
//    if (value == -std::abs(VALUE_NONE)) continue; // was illegal move
//
//    if (searchLimits.perft) continue; // In PERFT we can ignore values and pruning
//
//    numberOfSearchedMoves++; // legal move
//
//    // Did we find a better move for this node?
//    // For the first move this is always the case.
//    if (value > bestNodeValue) {
//      savePV(move, pv[ply + 1], pv[ply]);
//      bestNodeValue = value;
//      TRACE(LOG, "{:>{}} NEW BEST MOVE for node {}  move={} old best={} value={} pv={}",
//            " ", ply, printMoveListUCI(currentVariation), printMove(move),
//            bestNodeValue, value, printMoveListUCI(pv[ROOT_PLY]));
//    }
//    TRACE(LOG, "{:>{}} Depth {} cv {} move {} END", " ", ply, ply,
//          printMoveListUCI(currentVariation), printMove(move));
//  }
//  // ##### Iterate through all available moves
//  // ##########################################################
//
//  // if we did not have at least one legal move then we have a mate
//  if (numberOfSearchedMoves == 0 && !stopSearchFlag) {
//    searchStats.nonLeafPositionsEvaluated++;
//    if (position.hasCheck()) {
//      // We have a check mate. Return a -CHECKMATE.
//      bestNodeValue = static_cast<Value>(-VALUE_CHECKMATE + ply);
//    }
//    else {
//      // We have a stale mate. Return the draw value.
//      bestNodeValue = VALUE_DRAW;
//    }
//  }
//
//  return bestNodeValue;
//}

//Value Search::qsearch(Position *pPosition, const int ply) {
//  // update current search depth stats
//  searchStats.currentExtraSearchDepth = std::max(searchStats.currentExtraSearchDepth, ply);
//
//  // if PERFT or MAX SEARCH DEPTH reached return with eval to count all captures etc.
//  if (searchLimits.perft
//      || ply >= MAX_SEARCH_DEPTH - 1
//      || !SearchConfig::USE_QUIESCENCE)
//    return evaluate(pPosition, ply);
//
//
//  return evaluate(pPosition, ply);
//}

Value Search::evaluate(Position* pPosition, const int ply) {
  if (ply) {};

  // count all leaf nodes evaluated
  searchStats.leafPositionsEvaluated++;

  // PERFT stats
  // TODO: more perft stats
  if (searchLimits.perft) {
    return VALUE_ONE;
  }

  Value value = evaluator.evaluate(pPosition);
  TRACE(LOG, "{:>{}} Evaluation: {} {}", " ", ply, printMoveListUCI(currentVariation), value);
  return value;
}

inline bool Search::stopConditions() {
  if (hardTimeLimitReached()
      || (searchLimits.nodes
          && searchStats.nodesVisited >= searchLimits.nodes))
    stopSearchFlag = true;
  return stopSearchFlag;
}

template<Search::Search_Type ST>
bool Search::checkDrawRepAnd50(Position* pPosition) const {
  // for quiescence search we stop at 1 repetition already which should not
  // loose too much precision
  constexpr int allowedRepetitions = (ST == QUIESCENCE ? 1 : 2);
  if (pPosition->checkRepetitions(allowedRepetitions)) {
    this->LOG->debug("DRAW because of repetition for move {} in variation {}",
                     printMove(pPosition->getLastMove()), printMoveListUCI(this->currentVariation));
    return true;
  }
  if (pPosition->getHalfMoveClock() >= 100) {
    this->LOG->debug("DRAW because 50-move rule",
                     printMove(pPosition->getLastMove()), printMoveListUCI(this->currentVariation));
    return true;
  }
  return false;
}

void Search::configureTimeLimits() {
  if (searchLimits.moveTime > 0) { // mode time per move
    timeLimit = searchLimits.moveTime;
  }
  else { // remaining time - estimated time per move

    // retrieve time left from search mode
    assert ((searchLimits.whiteTime && searchLimits.blackTime) && "move times must be > 0");
    MilliSec timeLeft = (myColor == WHITE) ? searchLimits.whiteTime : searchLimits.blackTime;

    // Give some overhead time so that in games with very low available time we
    // do not run out of time
    timeLeft -= 1'000; // this should do

    // when we know the move to go (until next time control) use them otherwise assume 40
    int movesLeft = searchLimits.movesToGo > 0 ? searchLimits.movesToGo : 40;

    // when we have a time increase per move we estimate the additional time we should have
    if (myColor == WHITE) {
      timeLeft += 40 * searchLimits.whiteInc;
    }
    else {
      timeLeft += 40 * searchLimits.blackInc;
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
  if (searchLimits.moveTime == 0) {
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
bool Search::hardTimeLimitReached() {
  if (!searchLimits.timeControl) return false;
  if (elapsedTime(startTime) >= timeLimit + extraTime) stopSearchFlag = true;
  return stopSearchFlag;
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
 * @param pPosition
 * @return UCI filtered root moves
 */
MoveList Search::generateRootMoves(Position* pPosition) {
  const MoveList* const legalMoves = moveGenerators[ROOT_PLY].generateLegalMoves<MoveGenerator::GENALL>(
    pPosition);
  //for (Move m : *legalMoves) TRACE(LOG, "before: {} {}", printMoveVerbose(m), m);
  MoveList moveList;
  if (searchLimits.moves.empty()) { // if UCI searchmoves is empty then add all
    for (auto legalMove : *legalMoves) {
      setValue(legalMove, VALUE_NONE);
      moveList.push_back(legalMove);
    }
  }
  else { // only add if in the UCI searchmoves list
    for (auto legalMove : *legalMoves) {
      for (auto move : searchLimits.moves) {
        if (moveOf(move) == moveOf(legalMove)) {
          setValue(legalMove, VALUE_NONE);
          moveList.push_back(legalMove);
        }
      }
    }
  }
  return moveList;
}

void Search::sendIterationEndInfoToEngine() const {

  MilliSec result;
  result = elapsedTime(startTime);
  std::string infoString =
    fmt::format(
      "depth {} seldepth {} multipv 1 {} nodes {} nps {} time {} pv {}",
      searchStats.currentIterationDepth,
      searchStats.currentExtraSearchDepth,
      valueOf(pv[ROOT_PLY].front()),
      searchStats.nodesVisited,
      getNps(),
      result,
      printMoveListUCI(pv[ROOT_PLY]));

  if (pEngine == nullptr) LOG->warn("<no engine> >> {}", infoString);
  else {
    pEngine->sendIterationEndInfo(searchStats.currentIterationDepth,
                                  searchStats.currentExtraSearchDepth,
                                  valueOf(pv[ROOT_PLY].front()),
                                  searchStats.nodesVisited,
                                  getNps(),
                                  elapsedTime(startTime),
                                  pv[ROOT_PLY]);
  }
}

void Search::sendCurrentRootMoveToEngine() const {
  std::string infoString =
    fmt::format(
      "currmove {} currmovenumber {}",
      printMove(searchStats.currentRootMove),
      currentMoveIndex + 1);

  if (pEngine == nullptr) LOG->warn("<no engine> >> {}", infoString);
  else
    pEngine->sendCurrentRootMove(searchStats.currentRootMove,
                                 currentMoveIndex + 1);
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
      0); // TODO implement Hash

    //(int) (1000 * ((float) transpositionTable.getNumberOfEntries() / transpositionTable.getMaxEntries())));
    if (pEngine == nullptr) LOG->warn("<no engine> >> {}", infoString);
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
    if (pEngine == nullptr) LOG->warn("<no engine> >> {}", infoString);
    else pEngine->sendCurrentLine(currentVariation);
  }
}

void Search::sendResultToEngine() const {
  std::string infoString =
    fmt::format(
      "Engine got Best Move: {} ({}) [Ponder {}]",
      printMove(lastSearchResult.bestMove),
      printValue(valueOf(lastSearchResult.bestMove)),
      printMove(lastSearchResult.ponderMove));

  if (pEngine == nullptr) LOG->warn("<no engine> >> {}", infoString);
  else pEngine->sendResult(lastSearchResult.bestMove, lastSearchResult.ponderMove);
}













