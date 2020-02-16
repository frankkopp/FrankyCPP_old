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

#ifndef FRANKYCPP_MISC_H
#define FRANKYCPP_MISC_H

#include "types.h"

// forward declaration
class Position;

namespace Misc {

  /**
   * Creates a move from the given SAN string.
   * Returns MOVE_NONE if the notation string can't be converted to
   * a legal Move on this position.
   */
  Move getMoveFromSAN(const Position &, const std::string &);

  /**
   * Creates a Move from the given position and the UCI notation string.
   * Returns MOVE_NONE if the notation string can't be converted to
   * a legal Move on this position.
   */
  Move getMoveFromUCI(Position &position, std::string moveStr);

  /* returns the given string with as lower case string */
  std::string toLowerCase(std::string str);

  /* returns the given string with as upper case string */
  std::string toUpperCase(std::string str);
}

#endif //FRANKYCPP_MISC_H
