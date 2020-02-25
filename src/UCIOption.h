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

#ifndef FRANKYCPP_UCIOPTION_H
#define FRANKYCPP_UCIOPTION_H

#include <sstream>

static const char* optionTypeStrings[] = {"check", "spin", "combo", "button", "std::string"};

/**
 * UCI Option data class to store te available UCI options.
 */
class UCI_Option {

public:
  /**
 * UCI Option can have these types
 */
  enum UCI_OptionType {
    CHECK, SPIN, COMBO, BUTTON, STRING
  };

private:

  const std::string nameID;
  const UCI_OptionType type;
  const std::string defaultValue;
  const std::string minValue;
  const std::string maxValue;
  const std::string varValue;

  std::string currentValue;

public:

  explicit UCI_Option(const char* nameID);
  UCI_Option(const char* nameID, bool value);
  UCI_Option(const char* nameID, int def, int min, int max);
  UCI_Option(const char* nameID, const char* str);
  UCI_Option(const char* nameID, const char* var, const char* def);
  UCI_Option(const UCI_Option &o);

  friend std::ostream &operator<<(std::ostream &os, const UCI_Option &option);

  const std::string &getNameID() const { return nameID; }
  const UCI_OptionType &getType() const { return type; }
  std::string getTypeString() const { return optionTypeStrings[type]; }
  const std::string &getDefaultValue() const { return defaultValue; }
  const std::string &getMinValue() const { return minValue; }
  const std::string &getMaxValue() const { return maxValue; }
  const std::string &getVarValue() const { return varValue; }
  std::string getCurrentValue() const { return currentValue; }
  void setCurrentValue(std::string value) { currentValue = std::move(value); }

};


#endif //FRANKYCPP_UCIOPTION_H
