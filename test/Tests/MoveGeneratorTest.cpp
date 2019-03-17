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
#include <ostream>
#include <string>

#include "../../src/MoveGenerator.h"

using namespace std;

using testing::Eq;

TEST(MoveGenTest, pawnMoves) {
  Position::init();
  Bitboards::init();
  NEWLINE;

  string fen;
  MoveGenerator mg;

  fen = "r3k2r/1ppn3p/2q1q1n1/8/2q1Pp2/6R1/p1p2PPP/1R4K1 b kq e3";

  Position position(fen);
  vector<Move> moves;

  cout << position.printBoard() << endl;

  GenMode genMode = GENCAP;
  mg.generatePawnMoves(genMode, &position, &moves);
  cout << "Moves CAP: " << moves.size() << endl;
  for (Move m : moves) {
    cout << printMove(m) << endl;
  }

  NEWLINE;
  moves.clear();
  genMode = GENNONCAP;
  mg.generatePawnMoves(genMode, &position, &moves);
  cout << "Moves NONCAP: " << moves.size() << endl;
  for (Move m : moves) {
    cout << printMove(m) << endl;
  }


}



