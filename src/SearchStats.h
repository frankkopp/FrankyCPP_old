/*
 * MIT License
 *
 * Copyright (c) 2018 Frank Kopp
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

#ifndef FRANKYCPP_SEARCHSTATS_H
#define FRANKYCPP_SEARCHSTATS_H

#include <ostream>
#include "types.h"

/** data structure to cluster all search statistical values */
struct SearchStats {

  // counter for cut off to measure quality of move ordering
  // long[] betaCutOffs = new long[MAX_MOVES];

  // Search info values
  Ply currentSearchDepth = PLY_ROOT;
  Ply currentExtraSearchDepth = PLY_ROOT;
  Move currentRootMove = MOVE_NONE;
  int bestMoveChanges = 0;
  int bestMoveDepth = 0;
  MilliSec lastSearchTime = 0;

  // performance statistics
  uint64_t movesGenerated = 0;
  uint64_t nodesVisited = 0;

  // PERFT Values
  uint64_t leafPositionsEvaluated = 0;
  uint64_t nonLeafPositionsEvaluated = 0;
  uint64_t checkCounter = 0;
  uint64_t checkMateCounter = 0;
  uint64_t captureCounter = 0;
  uint64_t enPassantCounter = 0;

  // TT Statistics
  uint64_t tt_Cuts = 0;
  uint64_t tt_NoCuts = 0;

  // Optimization Values
  uint64_t aspirationResearches = 0;
  uint64_t prunings = 0;
  uint64_t pvs_root_researches = 0;
  uint64_t pvs_root_cutoffs = 0;
  uint64_t pvs_researches = 0;
  uint64_t pvs_cutoffs = 0;
  uint64_t positionsNonQuiet = 0;
  uint64_t minorPromotionPrunings = 0;
  uint64_t mateDistancePrunings = 0;
  uint64_t rfpPrunings = 0;
  uint64_t nullMovePrunings = 0;
  uint64_t nullMoveVerifications = 0;
  uint64_t razorReductions = 0;
  uint64_t iidSearches = 0;
  uint64_t lrReductions = 0;
  uint64_t efpPrunings = 0;
  uint64_t fpPrunings = 0;
  uint64_t qfpPrunings = 0;
  uint64_t lmpPrunings = 0;
  uint64_t lmrReductions = 0;
  uint64_t deltaPrunings = 0;

  std::string str() const;
  friend std::ostream &operator<<(std::ostream &os, const SearchStats &stats);

};


#endif //FRANKYCPP_SEARCHSTATS_H
