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

#include "UCIOption.h"

using namespace std;

namespace UCI {

  Option::Option(const char *name)
    : type(BUTTON), nameID(name) {}

  Option::Option(const char *name, bool value)
    : type(CHECK), nameID(name) { defaultValue = currentValue = value ? "true" : "false"; }

  Option::Option(const char *name, int def, int min, int max)
    : type(SPIN), nameID(name), minValue(std::to_string(min)),
      maxValue(std::to_string(max)) { defaultValue = currentValue = std::to_string(def); }

  Option::Option(const char *name, const char *str)
    : type(STRING), nameID(name) { defaultValue = currentValue = str; }

  Option::Option(const char *name, const char *val, const char *def)
    : type(STRING), nameID(name), defaultValue(val), currentValue(def) {}

  Option::Option(const Option &o)
  : nameID(o.nameID), type(o.type), currentValue(o.currentValue), defaultValue(o.defaultValue),
  minValue(o.minValue), maxValue(o.maxValue), varValue(o.varValue) {}

  ostream &operator<<(ostream &os, const Option &option) {
    os << "Option = nameID: " << option.nameID << " type: " << option.getTypeString()
       << " currentValue: " << option.currentValue
       << " defaultValue: " << option.defaultValue << " minValue: " << option.minValue
       << " maxValue: "
       << option.maxValue << " varValue: " << option.varValue;
    return os;
  }

}
