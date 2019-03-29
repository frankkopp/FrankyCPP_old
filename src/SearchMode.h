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

#ifndef FRANKYCPP_UCISEARCHMODE_H
#define FRANKYCPP_UCISEARCHMODE_H

#include <ostream>
#include "datatypes.h"

class SearchMode {

public:
  
  // defaults
  int          whiteTime = 0;
  int          blackTime = 0;
  int          whiteInc  = 0;
  int          blackInc  = 0;
  int          movesToGo = 0;
  int          depth     = 0;
  long         nodes     = 0;
  int          mate      = 0;
  int          movetime  = 0;
  MoveList     moves;

  bool ponder   = false;
  bool infinite = false;
  bool perft    = false;

  friend ostream &operator<<(ostream &os, const SearchMode &mode);

};


#endif //FRANKYCPP_UCISEARCHMODE_H
