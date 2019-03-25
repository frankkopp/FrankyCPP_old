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

#ifndef FRANKYCPP_UCIPROTOCOLHANDLER_H
#define FRANKYCPP_UCIPROTOCOLHANDLER_H

#include <thread>
#include <iostream>
#include "Semaphore.h"
#include "Engine.h"

using namespace std;

namespace UCI {

  class Handler {

    Semaphore mySemaphore;
    Engine engine;

  public:

    /** Constructor */
    Handler();
    /** Destructor */
    virtual ~Handler();

    /** Starts the handler loop */
    void loop();

    void uciCommand();
    void isReadyCommand();
    void setOptionCommand(istringstream &inStream);
    void uciNewGameCommand();
    void positionCommand(istringstream &inStream);
    void goCommand(istringstream &inStream);
    void stopCommand();
    void ponderHitCommand();
    void registerCommand(istringstream &inStream);
    void debugCommand(istringstream &inStream);

  private:

    void send(string toSend) const;
  };
}
#endif //FRANKYCPP_UCIPROTOCOLHANDLER_H
