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
#include "globals.h"

/** data structure to cluster all search statistical values */
class SearchStats {

public:

  // counter for cut off to measure quality of move ordering
  // long[] betaCutOffs = new long[MAX_MOVES];

  // Info values
  int currentIterationDepth = 0;
  int currentSearchDepth = 0;
  int currentExtraSearchDepth = 0;
  Move currentRootMove = NOMOVE;
  int currentRootMoveNumber = 0;
  int bestMoveChanges = 0;
  MilliSec lastSearchTime = 0;

  // PERFT Values
  long leafPositionsEvaluated = 0;
  long nonLeafPositionsEvaluated = 0;
  long checkCounter = 0;
  long checkMateCounter = 0;
  long captureCounter = 0;
  long enPassantCounter = 0;

  // Optimization Values
  int aspirationResearches = 0;
  long prunings = 0;
  long pvs_root_researches = 0;
  long pvs_root_cutoffs = 0;
  long pvs_researches = 0;
  long pvs_cutoffs = 0;
  long positionsNonQuiet = 0;
  long tt_Hits = 0;
  long tt_Misses = 0;
  long tt_Cuts = 0;
  long tt_Ignored = 0;
  long movesGenerated = 0;
  long nodesVisited = 0;
  int minorPromotionPrunings = 0;
  int mateDistancePrunings = 0;
  int rfpPrunings = 0;
  int nullMovePrunings = 0;
  int nullMoveVerifications = 0;
  int razorReductions = 0;
  int iidSearches = 0;
  int lrReductions = 0;
  int efpPrunings = 0;
  int fpPrunings = 0;
  int qfpPrunings = 0;
  int lmpPrunings = 0;
  int lmrReductions = 0;
  int deltaPrunings = 0;

  std::string str() const;
  friend std::ostream &operator<<(std::ostream &os, const SearchStats &stats);

};


#endif //FRANKYCPP_SEARCHSTATS_H
