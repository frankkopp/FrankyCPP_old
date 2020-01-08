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

#include <sstream>
#include "SearchStats.h"

std::ostream &operator<<(std::ostream &os, const SearchStats &stats) {
  os << stats.str();
  return os;
}

std::string SearchStats::str() const {
  std::stringstream os;
  os.imbue(deLocale);
  os
    << "nodesVisited: " << nodesVisited
    << " tt_Cutss: " << tt_Cuts
    << " tt_NoCuts: " << tt_NoCuts
    << " leafPositionsEvaluated: " << leafPositionsEvaluated
    << " nonLeafPositionsEvaluated: " << nonLeafPositionsEvaluated
    << " prunings: " << prunings
    << " pvs_root_researches: " << pvs_root_researches
    << " pvs_root_cutoffs: " << pvs_root_cutoffs
    << " pvs_researches: " << pvs_researches
    << " pvs_cutoffs: " << pvs_cutoffs
    << " pv_sortings: " << pv_sortings
    << " noTTMoveForPVsorting: " << no_moveForPVsorting
    << " quiescenceStandpatCuts: " << qStandpatCuts
    << " iid searches: " << iidSearches
    << " movesGenerated: " << movesGenerated
    << " currentSearchDepth: " << currentSearchDepth
    << " currentExtraSearchDepth: " << currentExtraSearchDepth
    << " lastSearchTime: " << lastSearchTime
    << " minorPromotionPrunings: " << minorPromotionPrunings
    << " mateDistancePrunings: " << mateDistancePrunings
    << " currentRootMove: " << currentRootMove
    << " bestMoveChanges: " << bestMoveChanges
    << " checkCounter: " << checkCounter
    << " checkMateCounter: " << checkMateCounter
    << " captureCounter: " << captureCounter
    << " enPassantCounter: " << enPassantCounter
    << " aspirationResearches: " << aspirationResearches
    << " positionsNonQuiet: " << positionsNonQuiet
    << " rfpPrunings: " << rfpPrunings
    << " nullMovePrunings: " << nullMovePrunings
    << " nullMoveVerifications: " << nullMoveVerifications
    << " razorReductions: " << razorReductions
    << " iidSearches: " << iidSearches
    << " lrReductions: " << lrReductions
    << " efpPrunings: " << efpPrunings
    << " fpPrunings: " << fpPrunings
    << " qfpPrunings: " << qfpPrunings
    << " lmpPrunings: " << lmpPrunings
    << " lmrReductions: " << lmrReductions
    << " deltaPrunings: " << deltaPrunings;
  return os.str();
}

