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

#include <iostream>
#include <sstream>

#include "UCIHandler.h"
#include "Search.h"

using namespace std;

namespace UCI {

  Handler::Handler() = default;
  Handler::~Handler() = default;

  void Handler::loop() {
    string cmd, token;
    do {

      cout << "WAIT FOR COMMAND:" << endl;

      // Block here waiting for input or EOF
      if (!getline(cin, cmd)) cmd = "quit";

      // create the stream object
      istringstream inStream(cmd);

      // clear possible previous entries
      token.clear();

      // read word from stream delimiter is whitespace
      // to get line use inStream.str()
      inStream >> skipws >> token;
      cout << "RECEIVED: " << token << endl;

      if (token == "quit") break;
      else if (token == "go") goCommand(inStream);

      cout << "COMMAND PROCESSED: " << token << endl;

    } while (token != "quit");
  }

  void Handler::goCommand(istringstream &inStream) {
    Search search;
    search.start();
  }

}