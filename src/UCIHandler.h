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

#ifndef FRANKYCPP_UCIPROTOCOLHANDLER_H
#define FRANKYCPP_UCIPROTOCOLHANDLER_H

#include <thread>
#include <iosfwd>
#include "types.h"

class Engine;

  class UCI_Handler {

    Engine *pEngine;

    std::istream *pInputStream;
    std::ostream *pOutputStream;

  public:

    /** Constructor */
    explicit UCI_Handler(Engine *ptr);
    /** Constructor */
    UCI_Handler(Engine *pEng, std::istream *pIstream, std::ostream *pOstream);

    /** Starts the handler loop with the istream provided when creating the
     * instance */
    void loop();

    /** Starts the handler loop  with the given istream (mainly for testing) */
    void loop(std::istream *pIstream);

    void uciCommand() const;
    void isReadyCommand() const;
    void setOptionCommand(std::istringstream &inStream) const;
    void uciNewGameCommand() const;
    void positionCommand(std::istringstream &inStream) const;
    void goCommand(std::istringstream &inStream);
    void stopCommand() const;
    void ponderHitCommand() const;
    static void registerCommand() ;
    static void debugCommand() ;
    void send(const std::string &toSend) const;

    ///////////////////
    //// GETTER
    void sendIterationEndInfo(int depth, int seldepth, Value value, uint64_t nodes,
                              uint64_t nps, MilliSec time, const MoveList &pv) const;
    void sendCurrentRootMove(Move currmove, unsigned long movenumber) const;
    void sendSearchUpdate(int depth, int seldepth, uint64_t nodes, uint64_t nps,
                          MilliSec time, int hashfull) const;
    void sendCurrentLine(const MoveList &moveList) const;
    void sendResult(Move bestMove, Move ponderMove) const;
    void sendString(const std::string &anyString) const;
  };

#endif //FRANKYCPP_UCIPROTOCOLHANDLER_H
