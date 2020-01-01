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
  inline bool USE_QUIESCENCE = true;
  inline bool USE_ALPHABETA = true;
  inline bool USE_PVS = true;

  // Transposition Table
  inline bool USE_TT = true;
  inline bool USE_TT_QSEARCH = true;
  inline int TT_SIZE_MB = 64;

  // Move Sorting Features
  inline bool USE_KILLER_MOVES = true;
  inline int NO_KILLER_MOVES = 2;
  inline bool USE_PV_MOVE_SORTING = true;

  // Pruning features
  inline bool USE_MDP = true; // mate distance pruning
  inline bool USE_MPP = true; // minor promotion pruning

}

#endif //FRANKYCPP_SEARCHCONFIG_H
