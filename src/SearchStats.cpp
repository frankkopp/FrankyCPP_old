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

#include "SearchStats.h"

std::ostream &operator<<(std::ostream &os, const SearchStats &stats) {
  os << "currentIterationDepth: " << stats.currentIterationDepth << " currentSearchDepth: "
     << stats.currentSearchDepth << " currentExtraSearchDepth: " << stats.currentExtraSearchDepth
     << " currentRootMove: " << stats.currentRootMove << " currentRootMoveNumber: "
     << stats.currentRootMoveNumber << " lastSearchTime: " << stats.lastSearchTime
     << " bestMoveChanges: " << stats.bestMoveChanges << " leafPositionsEvaluated: "
     << stats.leafPositionsEvaluated << " nonLeafPositionsEvaluated: "
     << stats.nonLeafPositionsEvaluated << " checkCounter: " << stats.checkCounter
     << " checkMateCounter: " << stats.checkMateCounter << " captureCounter: "
     << stats.captureCounter << " enPassantCounter: " << stats.enPassantCounter
     << " aspirationResearches: " << stats.aspirationResearches << " prunings: " << stats.prunings
     << " pvs_root_researches: " << stats.pvs_root_researches << " pvs_root_cutoffs: "
     << stats.pvs_root_cutoffs << " pvs_researches: " << stats.pvs_researches << " pvs_cutoffs: "
     << stats.pvs_cutoffs << " positionsNonQuiet: " << stats.positionsNonQuiet << " tt_Hits: "
     << stats.tt_Hits << " tt_Misses: " << stats.tt_Misses << " tt_Cuts: " << stats.tt_Cuts
     << " tt_Ignored: " << stats.tt_Ignored << " movesGenerated: " << stats.movesGenerated
     << " nodesVisited: " << stats.nodesVisited << " minorPromotionPrunings: "
     << stats.minorPromotionPrunings << " mateDistancePrunings: " << stats.mateDistancePrunings
     << " rfpPrunings: " << stats.rfpPrunings << " nullMovePrunings: " << stats.nullMovePrunings
     << " nullMoveVerifications: " << stats.nullMoveVerifications << " razorReductions: "
     << stats.razorReductions << " iidSearches: " << stats.iidSearches << " lrReductions: "
     << stats.lrReductions << " efpPrunings: " << stats.efpPrunings << " fpPrunings: "
     << stats.fpPrunings << " qfpPrunings: " << stats.qfpPrunings << " lmpPrunings: "
     << stats.lmpPrunings << " lmrReductions: " << stats.lmrReductions << " deltaPrunings: "
     << stats.deltaPrunings;
  return os;
}

