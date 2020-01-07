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

#ifndef FRANKYCPP_SEARCHCONFIG_H
#define FRANKYCPP_SEARCHCONFIG_H

namespace SearchConfig {

  // basic search strategies and features
  inline bool USE_QUIESCENCE = true; // use quiescence search
  inline Depth MAX_EXTRA_QDEPTH = static_cast<Depth>(20);
  inline bool USE_ALPHABETA = true; // use ALPHA_BETA instead of MinMax
  inline bool USE_PVS = true; // use PVS null window search

  // Transposition Table
  inline bool USE_TT = true; // use transposition table
  inline bool USE_TT_QSEARCH = true; // use transposition table also in quiescence search
  inline int TT_SIZE_MB = 64; // size of TT in MB

  // Move Sorting Features
  inline bool USE_KILLER_MOVES = true; // Store refutation moves (>beta) for move ordering
  inline int NO_KILLER_MOVES = 2; // number of killers stored
  inline bool USE_PV_MOVE_SORTING = true; // tell the move gen the current pv to return first
  inline bool USE_IID = true; // internal iterative deepening if we did not get a TT move
  inline Depth IID_REDUCTION = static_cast<Depth>(4); // reduction of search depth for IID

  // Pruning features
  inline bool USE_MDP = true; // mate distance pruning
  inline bool USE_MPP = true; // minor promotion pruning
  inline bool USE_RFP = true; // Reverse Futility Pruning
  inline Value RFP_MARGIN = static_cast<Value>(300);
  inline bool USE_RAZOR_PRUNING = true; // Razoring - bad move direct into qs
  inline Depth RAZOR_DEPTH = static_cast<Depth>(2);
  inline Value RAZOR_MARGIN = static_cast<Value>(600);
  inline bool USE_NMP = true;
  inline Depth NMP_DEPTH = static_cast<Depth>(2);
  inline bool USE_VERIFY_NMP = true;
  inline Depth NMP_VERIFICATION_DEPTH = static_cast<Depth>(1); // depth - NMP_DEPTH > NMP_VERIFICATION_DEPTH

  // tactical features
  inline bool USE_EXTENSIONS = true;

}

#endif //FRANKYCPP_SEARCHCONFIG_H
