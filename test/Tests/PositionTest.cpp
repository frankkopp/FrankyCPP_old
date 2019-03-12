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

#include "../../src/globals.h"
#include "../../src/Position.h"
#include "../../src/Bitboards.h"

using namespace std;

using testing::Eq;

TEST(PositionTest, ZobristTest) {
  Position::init();
  // NEWLINE  ;
  Key z = 0ULL;

  z ^= Zobrist::pieces[WHITE_KING][SQ_E1];
  z ^= Zobrist::pieces[BLACK_KING][SQ_E8];
  z ^= Zobrist::castlingRights[ANY_CASTLING];
  z ^= Zobrist::enPassantFile[FILE_NONE];
  Key expected = z;
  // cout << "Zobrist= " << z << std::endl;
  ASSERT_EQ(3127863183353006913, z);

  z ^= Zobrist::pieces[WHITE_KING][SQ_E1];
  z ^= Zobrist::pieces[WHITE_KING][SQ_E2];
  // cout << "Zobrist= " << z << std::endl;

  z ^= Zobrist::pieces[WHITE_KING][SQ_E2];
  z ^= Zobrist::pieces[WHITE_KING][SQ_E1];
  // cout << "Zobrist= " << z << std::endl;
  ASSERT_EQ(expected, z);

  z ^= Zobrist::castlingRights[WHITE_CASTLING];
  // cout << "Zobrist= " << z << std::endl;

  z ^= Zobrist::castlingRights[WHITE_CASTLING];
  // cout << "Zobrist= " << z << std::endl;
  ASSERT_EQ(expected, z);

  z ^= Zobrist::castlingRights[WHITE_OO];
  // cout << "Zobrist= " << z << std::endl;

  z ^= Zobrist::castlingRights[WHITE_OO];
  // cout << "Zobrist= " << z << std::endl;
  ASSERT_EQ(expected, z);

  z ^= Zobrist::enPassantFile[fileOf(SQ_D3)];
  // cout << "Zobrist= " << z << std::endl;

  z ^= Zobrist::enPassantFile[fileOf(SQ_D3)];
  // cout << "Zobrist= " << z << std::endl;
  ASSERT_EQ(expected, z);

  z ^= Zobrist::nextPlayer;
  // cout << "Zobrist= " << z << std::endl;

  z ^= Zobrist::nextPlayer;
  // cout << "Zobrist= " << z << std::endl;
  ASSERT_EQ(expected, z);
}

TEST(PositionTest, Setup) {
  Bitboards::init();
  Position::init();
  Values::init();
  // NEWLINE  ;
  string fen;

  Position position;
  ASSERT_EQ(WHITE, position.getNextPlayer());
  ASSERT_EQ(BLACK, ~position.getNextPlayer());
  ASSERT_EQ(position.getMaterial(WHITE), position.getMaterial(BLACK));
  ASSERT_EQ(24, position.getGamePhase());
  ASSERT_EQ(position.getMgPosValue(WHITE), position.getMgPosValue(BLACK));
  ASSERT_EQ(-225, position.getMgPosValue(WHITE));
  ASSERT_EQ(-225, position.getMgPosValue(BLACK));
  ASSERT_EQ(WHITE_KING, position.getPiece(SQ_E1));
  ASSERT_EQ(BLACK_KING, position.getPiece(SQ_E8));
  ASSERT_EQ(WHITE_KNIGHT, position.getPiece(SQ_B1));
  ASSERT_EQ(BLACK_KNIGHT, position.getPiece(SQ_B8));

  fen = string("r3k2r/1ppn3p/2q1q1n1/8/2q1Pp2/6R1/p1p2PPP/1R4K1 b kq e3 10 113");
  position = Position(fen.c_str());
  ASSERT_EQ(fen, position.printFen());
  ASSERT_EQ(SQ_E3, position.getEnPassantSquare());
  ASSERT_EQ(BLACK, position.getNextPlayer());
  ASSERT_EQ(3400, position.getMaterial(WHITE));
  ASSERT_EQ(6940, position.getMaterial(BLACK));
  ASSERT_EQ(22, position.getGamePhase());
  ASSERT_EQ(90, position.getMgPosValue(WHITE));
  ASSERT_EQ(7, position.getMgPosValue(BLACK));
  ASSERT_EQ(WHITE_KING, position.getPiece(SQ_G1));
  ASSERT_EQ(BLACK_KING, position.getPiece(SQ_E8));
  ASSERT_EQ(WHITE_ROOK, position.getPiece(SQ_G3));
  ASSERT_EQ(BLACK_QUEEN, position.getPiece(SQ_C6));

  fen = string("r1bqkb1r/pppp1ppp/2n2n2/3Pp3/8/8/PPP1PPPP/RNBQKBNR w - e6 0 1");
  position = Position(fen.c_str());
  ASSERT_EQ(fen, position.printFen());
  ASSERT_EQ(SQ_E6, position.getEnPassantSquare());
  ASSERT_EQ(WHITE, position.getNextPlayer());
}

TEST(PositionTest, Output) {
  Bitboards::init();
  Position::init();
  // NEWLINE  ;

  ostringstream expected;
  ostringstream actual;

  // start pos
  Position position;
  ASSERT_EQ(START_POSITION_FEN, position.printFen());
  expected << "  +---+---+---+---+---+---+---+---+\n"
              "8 | r | n | b | q | k | b | n | r |\n"
              "  +---+---+---+---+---+---+---+---+\n"
              "7 | p | p | p | p | p | p | p | p |\n"
              "  +---+---+---+---+---+---+---+---+\n"
              "6 |   |   |   |   |   |   |   |   |\n"
              "  +---+---+---+---+---+---+---+---+\n"
              "5 |   |   |   |   |   |   |   |   |\n"
              "  +---+---+---+---+---+---+---+---+\n"
              "4 |   |   |   |   |   |   |   |   |\n"
              "  +---+---+---+---+---+---+---+---+\n"
              "3 |   |   |   |   |   |   |   |   |\n"
              "  +---+---+---+---+---+---+---+---+\n"
              "2 | P | P | P | P | P | P | P | P |\n"
              "  +---+---+---+---+---+---+---+---+\n"
              "1 | R | N | B | Q | K | B | N | R |\n"
              "  +---+---+---+---+---+---+---+---+\n"
              "    A   B   C   D   E   F   G   H  \n\n";
  actual << position.printBoard();
  ASSERT_EQ(expected.str(), actual.str());

  string fen("r3k2r/1ppn3p/2q1q1n1/8/2q1Pp2/6R1/p1p2PPP/1R4K1 b kq e3 10 113");
  position = Position(fen.c_str());
  ASSERT_EQ(fen, position.printFen());

  fen = string("r1b1k2r/pp2ppbp/2n3p1/q7/3pP3/2P1BN2/P2Q1PPP/2R1KB1R w Kkq - 0 11");
  position = Position(fen.c_str());
  ASSERT_EQ(fen, position.printFen());

  fen = string("rnbqkbnr/1ppppppp/8/p7/Q1P5/8/PP1PPPPP/RNB1KBNR b KQkq - 1 2");
  position = Position(fen.c_str());
  ASSERT_EQ(fen, position.printFen());
}

TEST(PositionTest, Copy) {
  Bitboards::init();
  Position::init();
  // NEWLINE  ;

  string fen("r3k2r/1ppn3p/2q1q1n1/8/2q1Pp2/6R1/p1p2PPP/1R4K1 b kq e3 10 113");
  Position position(fen.c_str());
  Position copy(position);
  ASSERT_EQ(position.getZobristKey(), copy.getZobristKey());
  ASSERT_EQ(position.printFen(), copy.printFen());
  ASSERT_EQ(position.printBoard(), copy.printBoard());
  ASSERT_EQ(position.getOccupiedBB(WHITE), copy.getOccupiedBB(WHITE));
  ASSERT_EQ(position.getOccupiedBB(BLACK), copy.getOccupiedBB(BLACK));
  ASSERT_EQ(SQ_E3, copy.getEnPassantSquare());
  ASSERT_EQ(BLACK, copy.getNextPlayer());
}

TEST(PositionTest, PosValue) {
  Bitboards::init();
  Position::init();
  Values::init();
  // NEWLINE  ;

  Position position("8/8/8/8/8/8/8/8 w - - 0 1");
  //  cout << position.str() << endl;

  position.putPiece(WHITE_KING, SQ_E1);
  position.putPiece(BLACK_KING, SQ_E8);
  position.putPiece(WHITE_KNIGHT, SQ_E4);
  position.putPiece(BLACK_KNIGHT, SQ_D5);
  // cout << position.str() << endl;
  ASSERT_EQ(2, position.getGamePhase());
  ASSERT_EQ(2320, position.getMaterial(WHITE));
  ASSERT_EQ(2320, position.getMaterial(BLACK));
  ASSERT_EQ(0, position.getMgPosValue(WHITE));
  ASSERT_EQ(0, position.getMgPosValue(BLACK));
  ASSERT_EQ(-10, position.getEgPosValue(WHITE));
  ASSERT_EQ(-10, position.getEgPosValue(BLACK));
}

TEST(PositionTest, doUndoMoveNormal) {
  Bitboards::init();
  Position::init();
  Values::init();
  // NEWLINE  ;

  Position position;
  //  cout << position.str() << endl;

  // do move tests

  position.doMove(createMove<NORMAL>(SQ_E2, SQ_E4));
  //  cout << position.str() << endl;
  ASSERT_EQ(SQ_E3, position.getEnPassantSquare());
  ASSERT_EQ("rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1", position.printFen());

  position.doMove(createMove<NORMAL>(SQ_D7, SQ_D5));
  //  cout << position.str() << endl;
  ASSERT_EQ(SQ_D6, position.getEnPassantSquare());
  ASSERT_EQ("rnbqkbnr/ppp1pppp/8/3p4/4P3/8/PPPP1PPP/RNBQKBNR w KQkq d6 0 2", position.printFen());

  position.doMove(createMove<NORMAL>(SQ_E4, SQ_D5));
  //  cout << position.str() << endl;
  ASSERT_EQ(SQ_NONE, position.getEnPassantSquare());
  ASSERT_EQ(BLACK, position.getNextPlayer());
  ASSERT_EQ(5900, position.getMaterial(BLACK));
  ASSERT_EQ("rnbqkbnr/ppp1pppp/8/3P4/8/8/PPPP1PPP/RNBQKBNR b KQkq - 0 2", position.printFen());

  // undo move tests

  position.undoMove();
  ASSERT_EQ("rnbqkbnr/ppp1pppp/8/3p4/4P3/8/PPPP1PPP/RNBQKBNR w KQkq d6 0 2", position.printFen());
  ASSERT_EQ(SQ_D6, position.getEnPassantSquare());
  ASSERT_EQ(WHITE, position.getNextPlayer());
  ASSERT_EQ(6000, position.getMaterial(BLACK));

  position.undoMove();
  ASSERT_EQ(SQ_E3, position.getEnPassantSquare());
  ASSERT_EQ("rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1", position.printFen());

  position.undoMove();
  ASSERT_EQ(Position().printFen(), position.printFen());
}

TEST(PositionTest, doUndoMovePromotion) {
  Bitboards::init();
  Position::init();
  Values::init();
  // NEWLINE  ;

  Position position("6k1/P7/8/8/8/8/8/3K4 w - - 0 1");
  //  cout << position.str() << endl;

  // do move

  position.doMove(createMove<PROMOTION>(SQ_A7, SQ_A8, QUEEN));
  //  cout << position.str() << endl;
  ASSERT_EQ(BLACK, position.getNextPlayer());
  ASSERT_EQ(2900, position.getMaterial(WHITE));
  ASSERT_EQ("Q5k1/8/8/8/8/8/8/3K4 b - - 0 1", position.printFen());

  // undo move

  position.undoMove();
  ASSERT_EQ(WHITE, position.getNextPlayer());
  ASSERT_EQ(2100, position.getMaterial(WHITE));
  ASSERT_EQ("6k1/P7/8/8/8/8/8/3K4 w - - 0 1", position.printFen());
}

TEST(PositionTest, doUndoMoveEnPassantCapture) {
  Bitboards::init();
  Position::init();
  Values::init();
  // NEWLINE  ;

  // do move
  Position position("rnbqkbnr/ppp1pppp/8/8/3pP3/2N2N2/PPPP1PPP/R1BQKB1R b KQkq e3 0 3");
  //  cout << position.str() << endl;
  position.doMove(createMove<ENPASSANT>(SQ_D4, SQ_E3));
  //  cout << position.str() << endl;
  ASSERT_EQ(WHITE, position.getNextPlayer());
  ASSERT_EQ(5900, position.getMaterial(WHITE));
  ASSERT_EQ("rnbqkbnr/ppp1pppp/8/8/8/2N1pN2/PPPP1PPP/R1BQKB1R w KQkq - 0 4", position.printFen());

  // undo move
  position.undoMove();
  ASSERT_EQ(BLACK, position.getNextPlayer());
  ASSERT_EQ(6000, position.getMaterial(WHITE));
  ASSERT_EQ("rnbqkbnr/ppp1pppp/8/8/3pP3/2N2N2/PPPP1PPP/R1BQKB1R b KQkq e3 0 3", position.printFen());

  // do move
  position = Position("r1bqkb1r/pppp1ppp/2n2n2/3Pp3/8/8/PPP1PPPP/RNBQKBNR w KQkq e6 0 1");
  //  cout << position.str() << endl;
  position.doMove(createMove<ENPASSANT>(SQ_D5, SQ_E6));
  //  cout << position.str() << endl;
  ASSERT_EQ(BLACK, position.getNextPlayer());
  ASSERT_EQ(5900, position.getMaterial(BLACK));
  ASSERT_EQ("r1bqkb1r/pppp1ppp/2n1Pn2/8/8/8/PPP1PPPP/RNBQKBNR b KQkq - 0 1", position.printFen());

  // undo move
  position.undoMove();
  ASSERT_EQ(WHITE, position.getNextPlayer());
  ASSERT_EQ(6000, position.getMaterial(WHITE));
  ASSERT_EQ("r1bqkb1r/pppp1ppp/2n2n2/3Pp3/8/8/PPP1PPPP/RNBQKBNR w KQkq e6 0 1", position.printFen());
}

TEST(PositionTest, doMoveCastling) {
  Bitboards::init();
  Position::init();
  Values::init();
  // NEWLINE  ;

  // do move
  Position position("r3k2r/pppqbppp/2np1n2/1B2p1B1/4P1b1/2NP1N2/PPPQ1PPP/R3K2R w KQkq -");
  // cout << position.str() << endl;
  position.doMove(createMove<CASTLING>(SQ_E1, SQ_G1));
  // cout << position.str() << endl;
  ASSERT_EQ("r3k2r/pppqbppp/2np1n2/1B2p1B1/4P1b1/2NP1N2/PPPQ1PPP/R4RK1 b kq - 1 1",
            position.printFen());

  // undo move
  position.undoMove();
  ASSERT_EQ("r3k2r/pppqbppp/2np1n2/1B2p1B1/4P1b1/2NP1N2/PPPQ1PPP/R3K2R w KQkq - 0 1",
            position.printFen());

  // do move
  position = Position("r3k2r/pppqbppp/2np1n2/1B2p1B1/4P1b1/2NP1N2/PPPQ1PPP/R3K2R w KQkq -");
  // cout << position.str() << endl;
  position.doMove(createMove<CASTLING>(SQ_E1, SQ_C1));
  // cout << position.str() << endl;
  ASSERT_EQ("r3k2r/pppqbppp/2np1n2/1B2p1B1/4P1b1/2NP1N2/PPPQ1PPP/2KR3R b kq - 1 1",
            position.printFen());

  // undo move
  position.undoMove();
  ASSERT_EQ("r3k2r/pppqbppp/2np1n2/1B2p1B1/4P1b1/2NP1N2/PPPQ1PPP/R3K2R w KQkq - 0 1",
            position.printFen());

  // do move
  position = Position("r3k2r/pppqbppp/2np1n2/1B2p1B1/4P1b1/2NP1N2/PPPQ1PPP/R3K2R b KQkq -");
  // cout << position.str() << endl;
  position.doMove(createMove<CASTLING>(SQ_E8, SQ_G8));
  // cout << position.str() << endl;
  ASSERT_EQ("r4rk1/pppqbppp/2np1n2/1B2p1B1/4P1b1/2NP1N2/PPPQ1PPP/R3K2R w KQ - 1 1",
            position.printFen());

  // undo move
  position.undoMove();
  ASSERT_EQ("r3k2r/pppqbppp/2np1n2/1B2p1B1/4P1b1/2NP1N2/PPPQ1PPP/R3K2R b KQkq - 0 1",
            position.printFen());

  // do move
  position = Position("r3k2r/pppqbppp/2np1n2/1B2p1B1/4P1b1/2NP1N2/PPPQ1PPP/R3K2R b KQkq -");
  // cout << position.str() << endl;
  position.doMove(createMove<CASTLING>(SQ_E8, SQ_C8));
  // cout << position.str() << endl;
  ASSERT_EQ("2kr3r/pppqbppp/2np1n2/1B2p1B1/4P1b1/2NP1N2/PPPQ1PPP/R3K2R w KQ - 1 1",
            position.printFen());

  // undo move
  position.undoMove();
  ASSERT_EQ("r3k2r/pppqbppp/2np1n2/1B2p1B1/4P1b1/2NP1N2/PPPQ1PPP/R3K2R b KQkq - 0 1",
            position.printFen());

  // do move
  position = Position("r3k2r/pppqbppp/2np1n2/1B2p1B1/4P1b1/2NP1N2/PPPQ1PPP/R3K2R w KQkq -");
  // cout << position.str() << endl;
  position.doMove(createMove<NORMAL>(SQ_E1, SQ_F1));
  // cout << position.str() << endl;
  ASSERT_EQ("r3k2r/pppqbppp/2np1n2/1B2p1B1/4P1b1/2NP1N2/PPPQ1PPP/R4K1R b kq - 1 1",
            position.printFen());

  // undo move
  position.undoMove();
  ASSERT_EQ("r3k2r/pppqbppp/2np1n2/1B2p1B1/4P1b1/2NP1N2/PPPQ1PPP/R3K2R w KQkq - 0 1",
            position.printFen());

  // do move
  position = Position("r3k2r/pppqbppp/2np1n2/1B2p1B1/4P1b1/2NP1N2/PPPQ1PPP/R3K2R w KQkq -");
  // cout << position.str() << endl;
  position.doMove(createMove<NORMAL>(SQ_H1, SQ_F1));
  // cout << position.str() << endl;
  ASSERT_EQ("r3k2r/pppqbppp/2np1n2/1B2p1B1/4P1b1/2NP1N2/PPPQ1PPP/R3KR2 b Qkq - 1 1",
            position.printFen());

  // undo move
  position.undoMove();
  ASSERT_EQ("r3k2r/pppqbppp/2np1n2/1B2p1B1/4P1b1/2NP1N2/PPPQ1PPP/R3K2R w KQkq - 0 1",
            position.printFen());

  // do move
  position = Position("r3k2r/pppqbppp/2np1n2/1B2p1B1/4P1b1/2NP1N2/PPPQ1PPP/R3K2R b KQkq -");
  // cout << position.str() << endl;
  position.doMove(createMove<NORMAL>(SQ_A8, SQ_C8));
  // cout << position.str() << endl;
  ASSERT_EQ("2r1k2r/pppqbppp/2np1n2/1B2p1B1/4P1b1/2NP1N2/PPPQ1PPP/R3K2R w KQk - 1 1",
            position.printFen());

  // undo move
  position.undoMove();
  ASSERT_EQ("r3k2r/pppqbppp/2np1n2/1B2p1B1/4P1b1/2NP1N2/PPPQ1PPP/R3K2R b KQkq - 0 1",
            position.printFen());

  // do move
  position = Position("r3k2r/1ppqbppp/2np1n2/1B2p1B1/4P1b1/2NP1N2/1PPQ1PPP/R3K2R b KQkq - 0 1");
  // cout << position.str() << endl;
  position.doMove(createMove<NORMAL>(SQ_A8, SQ_A1));
  // cout << position.str() << endl;
  ASSERT_EQ("4k2r/1ppqbppp/2np1n2/1B2p1B1/4P1b1/2NP1N2/1PPQ1PPP/r3K2R w Kk - 0 2",
            position.printFen());

  // undo move
  position.undoMove();
  ASSERT_EQ("r3k2r/1ppqbppp/2np1n2/1B2p1B1/4P1b1/2NP1N2/1PPQ1PPP/R3K2R b KQkq - 0 1",
            position.printFen());
}
