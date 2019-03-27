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

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "../../src/datatypes.h"
#include "../../src/UCIHandler.h"

using namespace std;
using testing::Eq;

thread startHandlerThread(UCI::Handler &uciHandler) {
  thread myThread(&UCI::Handler::loop, uciHandler);
  myThread.detach();
  return myThread;
}

TEST(UCITest, baseTest) {
  INIT::init();
  NEWLINE

  println("Creating UCIHandler")

  istringstream is;
  ostringstream os;

  UCI::Handler uciHandler;
  thread th = startHandlerThread(uciHandler);

  while (true) {
    //cout << "UI SENDING..." << endl;
    //is."isready";
    sleep(1);
    //cout << "UI RECEIVED:" << os.str() << endl;
    //os.clear();
  }

}
