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

#include "UCISearchMode.h"

using namespace std;

class Engine;

namespace UCI {

  class Handler {

    Engine *pEngine;

    istream *pInputStream = &cin;
    ostream *pOutputStream = &cout;

    // stores the last search mode we read in from UCI protocoll
    UCISearchMode searchMode;

  public:

    /** Constructor */
    explicit Handler(Engine *pEng);
    /** Constructor */
    Handler(Engine *pEng, istream *pIstream, ostream *pOstream);
    /** Destructor */
    virtual ~Handler();

    /** Starts the handler loop with the istream provided when creating the instance */
    void loop();

    /** Starts the handler loop  with the given istream (mainly for testing) */
    void loop(istream *pIstream);

    void uciCommand();
    void isReadyCommand();
    void setOptionCommand(istringstream &inStream);
    void uciNewGameCommand();
    void positionCommand(istringstream &inStream);
    void goCommand(istringstream &inStream);
    void stopCommand();
    void ponderHitCommand();
    void registerCommand();
    void debugCommand();
    void send(const string& toSend) const;

    ///////////////////
    //// GETTER
    const UCISearchMode &getSearchMode() const { return searchMode; };

    void sendResult(Move bestMove, Move ponderMove);

  };
}

#endif //FRANKYCPP_UCIPROTOCOLHANDLER_H
