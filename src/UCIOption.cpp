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

#include "types.h"
#include "UCIOption.h"


UCI_Option::UCI_Option(const char* name)
  : nameID(name), type(BUTTON), defaultValue(boolStr(false)) {}

UCI_Option::UCI_Option(const char* name, bool value)
  : nameID(name), type(CHECK), defaultValue(boolStr(value)), currentValue(boolStr(value)) {}

UCI_Option::UCI_Option(const char* name, int def, int min, int max)
  : nameID(name), type(SPIN), defaultValue(std::to_string(def)), minValue(std::to_string(min)),
    maxValue(std::to_string(max)), currentValue(std::to_string(def)) {}

UCI_Option::UCI_Option(const char* name, const char* str)
  : nameID(name), type(STRING), defaultValue(str), currentValue(str) {}

UCI_Option::UCI_Option(const char* name, const char* val, const char* def)
  : nameID(name), type(STRING), defaultValue(val), currentValue(def) {}

UCI_Option::UCI_Option(const UCI_Option &o) = default;

std::ostream &operator<<(std::ostream &os, const UCI_Option &option) {
  os << "Option = nameID: " << option.nameID << " type: " << option.getTypeString()
     << " currentValue: " << option.currentValue
     << " defaultValue: " << option.defaultValue << " minValue: " << option.minValue
     << " maxValue: "
     << option.maxValue << " varValue: " << option.varValue;
  return os;
}

