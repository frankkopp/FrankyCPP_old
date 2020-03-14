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

#include "SearchStats.h"
#include <sstream>

std::ostream& operator<<(std::ostream& os, const SearchStats& stats) {
  os << stats.str();
  return os;
}

std::ostream& operator<<(std::ostream& os, const std::array<uint64_t, MAX_MOVES>& array) {
  for (auto n : array) { os << n << " "; };
  return os;
}

std::string SearchStats::printPercentages(std::vector<double> p) {
  std::stringstream os;
  for (double d : p) { os << fmt::format("{0:.2f}%", d * 100) << " "; };
  return os.str();
}

std::string SearchStats::str() const {
  std::stringstream os;
  os.imbue(deLocale);
  os
      << "nodesVisited: " << nodesVisited
      << " movesGenerated: " << movesGenerated
      << " leafPositionsEvaluated: " << leafPositionsEvaluated
      << " nonLeafPositionsEvaluated: " << nonLeafPositionsEvaluated
      << " tt_Cuts: " << tt_Cuts
      << " tt_NoCuts: " << tt_NoCuts
      << " quiescenceStandpatCuts: " << qStandpatCuts
      << " prunings: " << prunings
      << " pvs_cutoffs: " << pvs_cutoffs
      << " pvs_researches: " << pvs_researches
      << " pvs_root_cutoffs: " << pvs_root_cutoffs
      << " pvs_root_researches: " << pvs_root_researches
      << " pv_sortings: " << pv_sortings
      << " noTTMoveForPVsorting: " << no_moveForPVsorting
      << " nullMovePrunings: " << nullMovePrunings
      << " nullMoveVerifications: " << nullMoveVerifications
      << " minorPromotionPrunings: " << minorPromotionPrunings
      << " mateDistancePrunings: " << mateDistancePrunings
      << " extensions: " << extensions
      << "   "
      << " checkCounter: " << checkCounter
      << " checkMateCounter: " << checkMateCounter
      << " captureCounter: " << captureCounter
      << " enPassantCounter: " << enPassantCounter
      << " positionsNonQuiet: " << positionsNonQuiet
      << "   "
      // not used/implemented
      //    << " aspirationResearches: " << aspirationResearches
      //    << " razorReductions: " << razorReductions
      //    << " iidSearches: " << iidSearches
      //    << " rfpPrunings: " << rfpPrunings
      //    << " fpPrunings: " << fpPrunings
      //    << " efpPrunings: " << efpPrunings
      //    << " qfpPrunings: " << qfpPrunings
      //    << " lrReductions: " << lrReductions
      //    << " lmpPrunings: " << lmpPrunings
      //    << " lmrReductions: " << lmrReductions
      //    << " deltaPrunings: " << deltaPrunings
      //    << "   "
      << " bestMoveChanges: " << bestMoveChanges
      << " currentRootMove: " << currentRootMove
      << " lastSearchTime: " << lastSearchTime
      << " currentSearchDepth: " << currentSearchDepth
      << " currentExtraSearchDepth: " << currentExtraSearchDepth
      << "   "
      << " betaCutOffs: " << printPercentages(getPercentages(betaCutOffs.begin(), betaCutOffs.end(), 15))
      << " alphaImprovements: " << printPercentages(getPercentages(alphaImprovements.begin(), alphaImprovements.end(), 15));

  return os.str();
}
