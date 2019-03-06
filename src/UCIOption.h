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

#ifndef FRANKYCPP_UCIOPTION_H
#define FRANKYCPP_UCIOPTION_H

#include <iostream>
#include <sstream>

namespace UCI {

  using namespace std;

  /**
   * UCI Option can have these types
   */
  enum OptionType { CHECK, SPIN, COMBO, BUTTON, STRING };
  static const char* optionTypeStrings[] = {"check", "spin", "combo", "button", "string"};

  /**
   * UCI Option class
   */
  class Option {

    string nameID;
    OptionType type;
    string currentValue;
    string defaultValue;
    string minValue;
    string maxValue;

    string varValue;

  public:
    explicit Option(const char *nameID);

    Option(const char *nameID, bool value);

    Option(const char *nameID, int def, int min, int max);

    Option(const char *nameID, const char *str);

    Option(const char *nameID, const char *var, const char *def);

    Option(const Option &o);

    friend ostream &operator<<(ostream &os, const Option &option);

    const string &getNameID() const {
      return nameID;
    }

    OptionType getType() const {
      return type;
    }

    std::string getTypeString() const {
      return optionTypeStrings[type];
    }

    const string &getCurrentValue() const {
      return currentValue;
    }

    const string &getDefaultValue() const {
      return defaultValue;
    }

    const string &getMinValue() const {
      return minValue;
    }

    const string &getMaxValue() const {
      return maxValue;
    }

    const string &getVarValue() const {
      return varValue;
    }

  };

  // print engine config

}

#endif //FRANKYCPP_UCIOPTION_H
