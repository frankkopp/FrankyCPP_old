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
#include "../../src/Values.h"
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
  NEWLINE;
  string fen;

  Position position;
  ASSERT_EQ(WHITE, position.getNextPlayer());
  ASSERT_EQ(BLACK, ~position.getNextPlayer());
  ASSERT_EQ(position.getMaterial(WHITE), position.getMaterial(BLACK));
  ASSERT_EQ(24, position.getGamePhase());
  ASSERT_FLOAT_EQ(1.0, position.getGamePhaseFactor());
  ASSERT_EQ(position.getMidPosValue(WHITE), position.getMidPosValue(BLACK));
  ASSERT_EQ(-225, position.getMidPosValue(WHITE));
  ASSERT_EQ(-225, position.getMidPosValue(BLACK));
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
  ASSERT_FLOAT_EQ((22.0/24), position.getGamePhaseFactor());
  ASSERT_EQ(90, position.getMidPosValue(WHITE));
  ASSERT_EQ(7, position.getMidPosValue(BLACK));
  ASSERT_EQ(WHITE_KING, position.getPiece(SQ_G1));
  ASSERT_EQ(BLACK_KING, position.getPiece(SQ_E8));
  ASSERT_EQ(WHITE_ROOK, position.getPiece(SQ_G3));
  ASSERT_EQ(BLACK_QUEEN, position.getPiece(SQ_C6));

  fen = string("r1bqkb1r/pppp1ppp/2n2n2/3Pp3/8/8/PPP1PPPP/RNBQKBNR w - e6 0 1");
  position = Position(fen.c_str());
  ASSERT_EQ(fen, position.printFen());
  ASSERT_EQ(SQ_E6, position.getEnPassantSquare());
  ASSERT_EQ(WHITE, position.getNextPlayer());

  fen = string("R6R/3Q4/1Q4Q1/4Q3/2Q4Q/Q4Q2/pp1Q4/kBNN1KB1 w - - 0 1");
  position = Position(fen.c_str());
  ASSERT_EQ(fen, position.printFen());
  ASSERT_EQ(WHITE, position.getNextPlayer());
  ASSERT_EQ(24, position.getGamePhase());

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
              "7 | * | * | * | * | * | * | * | * |\n"
              "  +---+---+---+---+---+---+---+---+\n"
              "6 |   |   |   |   |   |   |   |   |\n"
              "  +---+---+---+---+---+---+---+---+\n"
              "5 |   |   |   |   |   |   |   |   |\n"
              "  +---+---+---+---+---+---+---+---+\n"
              "4 |   |   |   |   |   |   |   |   |\n"
              "  +---+---+---+---+---+---+---+---+\n"
              "3 |   |   |   |   |   |   |   |   |\n"
              "  +---+---+---+---+---+---+---+---+\n"
              "2 | O | O | O | O | O | O | O | O |\n"
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
  ASSERT_EQ(0, position.getMidPosValue(WHITE));
  ASSERT_EQ(0, position.getMidPosValue(BLACK));
}


TEST(PositionTest, Bitboards) {
  Bitboards::init();
  Position::init();
//  NEWLINE;

  string fen("r3k2r/1ppn3p/2q1q1n1/8/2q1Pp2/6R1/p1p2PPP/1R4K1 b kq e3 10 113");
  Position position(fen.c_str());

  Bitboard bb, expected;

  bb = position.getPieceBB(WHITE, KING);
//  cout << Bitboards::print(bb);
  ASSERT_EQ(Bitboards::squareBB[SQ_G1], bb);

  bb = position.getPieceBB(BLACK, KING);
//  cout << Bitboards::print(bb);
  ASSERT_EQ(Bitboards::squareBB[SQ_E8], bb);

  bb = position.getPieceBB(WHITE, ROOK);
//  cout << Bitboards::print(bb);
  expected = Bitboards::squareBB[SQ_B1] | Bitboards::squareBB[SQ_G3];
  ASSERT_EQ(expected, bb);

  bb = position.getPieceBB(BLACK, ROOK);
//  cout << Bitboards::print(bb);
  expected = Bitboards::squareBB[SQ_A8] | Bitboards::squareBB[SQ_H8];
  ASSERT_EQ(expected, bb);

  bb = position.getPieceBB(WHITE, PAWN);
//  cout << Bitboards::print(bb);
  expected = Bitboards::squareBB[SQ_E4] | Bitboards::squareBB[SQ_F2] | Bitboards::squareBB[SQ_G2] |
             Bitboards::squareBB[SQ_H2];
  ASSERT_EQ(expected, bb);

  bb = position.getPieceBB(BLACK, KNIGHT);
//  cout << Bitboards::print(bb);
  expected = Bitboards::squareBB[SQ_D7] | Bitboards::squareBB[SQ_G6];
  ASSERT_EQ(expected, bb);
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
  ASSERT_EQ("rnbqkbnr/ppp1pppp/8/8/3pP3/2N2N2/PPPP1PPP/R1BQKB1R b KQkq e3 0 3",
            position.printFen());

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
  ASSERT_EQ("r1bqkb1r/pppp1ppp/2n2n2/3Pp3/8/8/PPP1PPPP/RNBQKBNR w KQkq e6 0 1",
            position.printFen());
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

TEST(PositionTest, doNullMove) {
  Bitboards::init();
  Position::init();
  Values::init();
  NEWLINE;

  // do move
  Position position("rnbqkbnr/ppp1pppp/8/8/3pP3/2N2N2/PPPP1PPP/R1BQKB1R b KQkq e3");
  cout << position.str() << endl;

  position.doNullMove();
  cout << position.str() << endl;
  ASSERT_EQ("rnbqkbnr/ppp1pppp/8/8/3pP3/2N2N2/PPPP1PPP/R1BQKB1R w KQkq - 0 1",
            position.printFen());

  position.undoNullMove();
  cout << position.str() << endl;
  ASSERT_EQ("rnbqkbnr/ppp1pppp/8/8/3pP3/2N2N2/PPPP1PPP/R1BQKB1R b KQkq e3 0 1",
            position.printFen());
}

TEST(PositionTest, repetitionSimple) {
  Position::init();
  Bitboards::init();
  // NEWLINE;

  Position position;

  position.doMove(createMove(SQ_E2, SQ_E4));
  position.doMove(createMove(SQ_E7, SQ_E5));

  // cout << "Repetitions: " << position.countRepetitions() << endl;
  ASSERT_EQ(0, position.countRepetitions());

  // Simple repetition
  // takes 3 loops to get to repetition
  for (int i = 0; i <= 2; i++) {
    position.doMove(createMove(SQ_G1, SQ_F3));
    position.doMove(createMove(SQ_B8, SQ_C6));
    position.doMove(createMove(SQ_F3, SQ_G1));
    position.doMove(createMove(SQ_C6, SQ_B8));
    //cout << "Repetitions: " << position.countRepetitions() << endl;
  }

  // cout << "3-Repetitions: " << position.countRepetitions() << endl;
  ASSERT_EQ(2, position.countRepetitions());
  ASSERT_TRUE(position.checkRepetitions(2));
}

TEST(PositionTest, repetitionAdvanced) {
  Position::init();
  Bitboards::init();
  NEWLINE;

  Position position("6k1/p3q2p/1n1Q2pB/8/5P2/6P1/PP5P/3R2K1 b - -");

  position.doMove(createMove(SQ_E7, SQ_E3));
  position.doMove(createMove(SQ_G1, SQ_G2));

  cout << "Repetitions: " << position.countRepetitions() << endl;
  ASSERT_EQ(0, position.countRepetitions());

  // takes 2 loops to get to repetition
  for (int i = 0; i < 2; i++) {
    position.doMove(createMove(SQ_E3, SQ_E2));
    position.doMove(createMove(SQ_G2, SQ_G1));
    position.doMove(createMove(SQ_E2, SQ_E3));
    position.doMove(createMove(SQ_G1, SQ_G2));
    cout << "Repetitions: " << position.countRepetitions() << endl;
  }

  cout << "3-Repetitions: " << position.countRepetitions() << endl;
  ASSERT_EQ(2, position.countRepetitions());
  ASSERT_TRUE(position.checkRepetitions(2));
}

TEST(PositionTest, insufficientMaterial) {
  Position::init();
  Bitboards::init();
  NEWLINE;

  string fen;
  Position position;

  // KK
  fen = "8/3k4/8/8/8/8/4K3/8 w - -";
  position = Position(fen);
  ASSERT_TRUE(position.checkInsufficientMaterial());

  // KQK
  fen = "8/3k4/8/8/8/8/4KQ2/8 w - -";
  position = Position(fen);
  ASSERT_FALSE(position.checkInsufficientMaterial());

  // KNK
  fen = "8/3k4/8/8/8/8/4KN2/8 w - -";
  position = Position(fen);
  ASSERT_TRUE(position.checkInsufficientMaterial());

  // KNNK
  fen = "8/3k4/8/8/8/8/4KNN1/8 w - -";
  position = Position(fen);
  ASSERT_TRUE(position.checkInsufficientMaterial());

  // KKN
  fen = "8/2nk4/8/8/8/8/4K3/8 w - -";
  position = Position(fen);
  ASSERT_TRUE(position.checkInsufficientMaterial());

  // KNNK
  fen = "8/1nnk4/8/8/8/8/4K3/8 w - -";
  position = Position(fen);
  ASSERT_TRUE(position.checkInsufficientMaterial());

  // KBKB - B same field color
  fen = "8/3k1b2/8/8/8/8/4K1B1/8 w - -";
  position = Position(fen);
  ASSERT_TRUE(position.checkInsufficientMaterial());

  // KBKB - B different field color
  fen = "8/3k2b1/8/8/8/8/4K1B1/8 w - -";
  position = Position(fen);
  ASSERT_FALSE(position.checkInsufficientMaterial());
}

TEST(PositionTest, rotatedBB) {
  Position::init();
  Bitboards::init();
  NEWLINE;

  string fen;
  Position position;

  fen = "r3k2r/1ppn3p/2q1q1n1/8/2q1Pp2/6R1/p1p2PPP/1R4K1 b kq e3";
  position = Position(fen);

  ASSERT_EQ(Bitboards::rotateR90(position.getOccupiedBB()), position.getOccupiedBBR90());
  ASSERT_EQ(Bitboards::rotateL90(position.getOccupiedBB()), position.getOccupiedBBL90());
  ASSERT_EQ(Bitboards::rotateR45(position.getOccupiedBB()), position.getOccupiedBBR45());
  ASSERT_EQ(Bitboards::rotateL45(position.getOccupiedBB()), position.getOccupiedBBL45());

}

TEST(PositionTest, hasCheck) {
  Position::init();
  Bitboards::init();
  NEWLINE;

  string fen;
  Position position;

  fen = "r3k2r/1ppn3p/2q1qNn1/8/2q1Pp2/6R1/p1p2PPP/1R4K1 b kq e3";
  position = Position(fen);

  ASSERT_TRUE(position.isAttacked(SQ_E8, WHITE));
  ASSERT_TRUE(position.hasCheck());
}

TEST(PositionTest, isAttacked) {
  Position::init();
  Bitboards::init();
  NEWLINE;

  string fen;
  Position position;

  fen = "r3k2r/1ppn3p/2q1q1n1/8/2q1Pp2/6R1/p1p2PPP/1R4K1 b kq e3";
  position = Position(fen);

  // pawns
  ASSERT_TRUE(position.isAttacked(SQ_G3, WHITE));
  ASSERT_TRUE(position.isAttacked(SQ_E3, WHITE));
  ASSERT_TRUE(position.isAttacked(SQ_B1, BLACK));
  ASSERT_TRUE(position.isAttacked(SQ_E4, BLACK));
  ASSERT_TRUE(position.isAttacked(SQ_E3, BLACK));

  // knight
  ASSERT_TRUE (position.isAttacked(SQ_E5, BLACK));
  ASSERT_TRUE (position.isAttacked(SQ_F4, BLACK));
  ASSERT_FALSE(position.isAttacked(SQ_G1, BLACK));

  // sliding
  ASSERT_TRUE(position.isAttacked(SQ_G6, WHITE));
  ASSERT_TRUE(position.isAttacked(SQ_A5, BLACK));

  fen = "rnbqkbnr/1ppppppp/8/p7/Q1P5/8/PP1PPPPP/RNB1KBNR b KQkq - 1 2";
  position = Position(fen);

  // king
  ASSERT_TRUE (position.isAttacked(SQ_D1, WHITE));
  ASSERT_FALSE(position.isAttacked(SQ_E1, BLACK));

  // rook
  ASSERT_TRUE (position.isAttacked(SQ_A5, BLACK));
  ASSERT_FALSE(position.isAttacked(SQ_A4, BLACK));

  // queen
  ASSERT_FALSE(position.isAttacked(SQ_E8, WHITE));
  ASSERT_TRUE (position.isAttacked(SQ_D7, WHITE));
  ASSERT_FALSE(position.isAttacked(SQ_E8, WHITE));

  // bug tests
  fen = "r1bqk1nr/pppp1ppp/2nb4/1B2B3/3pP3/8/PPP2PPP/RN1QK1NR b KQkq -";
  position = Position(fen);
  ASSERT_FALSE(position.isAttacked(SQ_E8, WHITE));
  ASSERT_FALSE(position.isAttacked(SQ_E1, BLACK));

  fen = "rnbqkbnr/ppp1pppp/8/1B6/3Pp3/8/PPP2PPP/RNBQK1NR b KQkq -";
  position = Position(fen);
  ASSERT_TRUE (position.isAttacked(SQ_E8, WHITE));
  ASSERT_FALSE(position.isAttacked(SQ_E1, BLACK));

}

TEST(PositionTest, giveCheck) {
  Position::init();
  Bitboards::init();
  NEWLINE;

  string fen;
  Position position;
  Move move;

  fen = "r3k2r/1ppn3p/2q1q1n1/8/2q1Pp2/6R1/p1p2PPP/1R4K1 b kq e3";
  position = Position(fen);

  // DIRECT CHECKS

  // Pawns
  position = Position("4r3/1pn3k1/4p1b1/p1Pp1P1r/3P2NR/1P3B2/3K2P1/4R3 w - -");
  move = createMove("f5f6");
  ASSERT_TRUE(position.givesCheck(move));

  position = Position("5k2/4pp2/1N2n1p1/r3P2p/P5PP/2rR1K2/P7/3R4 b - -");
  move = createMove("h5g4");
  ASSERT_TRUE(position.givesCheck(move));

  // Knights
  position = Position("5k2/4pp2/1N2n1p1/r3P2p/P5PP/2rR1K2/P7/3R4 w - -");
  move = createMove("b6d7");
  ASSERT_TRUE(position.givesCheck(move));

  position = Position("5k2/4pp2/1N2n1p1/r3P2p/P5PP/2rR1K2/P7/3R4 b - -");
  move = createMove("e6d4");
  ASSERT_TRUE(position.givesCheck(move));

  // Rooks
  position = Position("5k2/4pp2/1N2n1pp/r3P3/P5PP/2rR4/P3K3/3R4 w - -");
  move = createMove("d3d8");
  ASSERT_TRUE(position.givesCheck(move));

  position = Position("5k2/4pp2/1N2n1pp/r3P3/P5PP/2rR4/P3K3/3R4 b - -");
  move = createMove("c3c2");
  ASSERT_TRUE(position.givesCheck(move));

  // blocked opponent piece - no check
  position = Position("5k2/4pp2/1N2n1pp/r3P3/P5PP/2rR4/P2RK3/8 b - -");
  move = createMove("c3c2");
  ASSERT_FALSE(position.givesCheck(move));

  // blocked own piece - no check
  position = Position("5k2/4pp2/1N2n1pp/r3P3/P5PP/2rR4/P2nK3/3R4 b - -");
  move = createMove("c3c2");
  ASSERT_FALSE(position.givesCheck(move));

  // Bishop
  position = Position("6k1/3q2b1/p1rrnpp1/P3p3/2B1P3/1p1R3Q/1P4PP/1B1R3K w - -");
  move = createMove("c4e6");
  ASSERT_TRUE(position.givesCheck(move));

  // Queen
  position = Position("5k2/4pp2/1N2n1pp/r3P3/P5PP/2qR4/P3K3/3R4 b - -");
  move = createMove("c3c2");
  ASSERT_TRUE(position.givesCheck(move));

  position = Position("6k1/3q2b1/p1rrnpp1/P3p3/2B1P3/1p1R3Q/1P4PP/1B1R3K w - -");
  move = createMove("h3e6");
  ASSERT_TRUE(position.givesCheck(move));

  position = Position("6k1/p3q2p/1n1Q2pB/8/5P2/6P1/PP5P/3R2K1 b - -");
  move = createMove("e7e3");
  ASSERT_TRUE(position.givesCheck(move));

  // no check
  position = Position("6k1/p3q2p/1n1Q2pB/8/5P2/6P1/PP5P/3R2K1 b - -");
  move = createMove("e7e4");
  ASSERT_FALSE(position.givesCheck(move));

  // promotion
  position = Position("1k3r2/1p1bP3/2p2p1Q/Ppb5/4Rp1P/2q2N1P/5PB1/6K1 w - -");
  move = createMove<PROMOTION>("e7f8q");
  ASSERT_TRUE(position.givesCheck(move));

  position = Position("1r3r2/1p1bP2k/2p2n2/p1Pp4/P2N1PpP/1R2p3/1P2P1BP/3R2K1 w - -");
  move = createMove<PROMOTION>("e7f8n");
  ASSERT_TRUE(position.givesCheck(move));

  // Castling checks
  position = Position("r4k1r/8/8/8/8/8/8/R3K2R w KQ -");
  move = createMove<CASTLING>("e1g1");
  ASSERT_TRUE(position.givesCheck(move));

  position = Position("r2k3r/8/8/8/8/8/8/R3K2R w KQ -");
  move = createMove<CASTLING>("e1c1");
  ASSERT_TRUE(position.givesCheck(move));

  position = Position("r3k2r/8/8/8/8/8/8/R4K1R b kq -");
  move = createMove<CASTLING>("e8g8");
  ASSERT_TRUE(position.givesCheck(move));

  position = Position("r3k2r/8/8/8/8/8/8/R2K3R b kq -");
  move = createMove<CASTLING>("e8c8");
  ASSERT_TRUE(position.givesCheck(move));

  position = Position("r6r/8/8/8/8/8/8/2k1K2R w K -");
  move = createMove<CASTLING>("e1g1");
  ASSERT_TRUE(position.givesCheck(move));

  // en passant checks
  position = Position("8/3r1pk1/p1R2p2/1p5p/r2Pp3/PRP3P1/4KP1P/8 b - d3");
  move = createMove<ENPASSANT>("e4d3");
  ASSERT_TRUE(position.givesCheck(move));

  // REVEALED CHECKS
  position = Position("6k1/8/3P1bp1/2BNp3/8/1Q3P1q/7r/1K2R3 w - -");
  move = createMove("d5e7");
  ASSERT_TRUE(position.givesCheck(move));

  position = Position("6k1/8/3P1bp1/2BNp3/8/1Q3P1q/7r/1K2R3 w - -");
  move = createMove("d5c7");
  ASSERT_TRUE(position.givesCheck(move));

  position = Position("6k1/8/3P1bp1/2BNp3/8/1B3P1q/7r/1K2R3 w - -");
  move = createMove("d5c7");
  ASSERT_TRUE(position.givesCheck(move));

  position = Position("6k1/8/3P1bp1/2BNp3/8/1Q3P1q/7r/1K2R3 w - -");
  move = createMove("d5e7");
  ASSERT_TRUE(position.givesCheck(move));

  position = Position("1Q1N2k1/8/3P1bp1/2B1p3/8/5P1q/7r/1K2R3 w - -");
  move = createMove("d8e6");
  ASSERT_TRUE(position.givesCheck(move));

  position = Position("1R1N2k1/8/3P1bp1/2B1p3/8/5P1q/7r/1K2R3 w - -");
  move = createMove("d8e6");
  ASSERT_TRUE(position.givesCheck(move));

  // revealed by en passant capture
  position = Position("8/b2r1pk1/p1R2p2/1p5p/r2Pp3/PRP3P1/5K1P/8 b - d3");
  move = createMove<ENPASSANT>("e4d3");
  ASSERT_TRUE(position.givesCheck(move));

  // test where we had bugs
  position = Position("2r1r3/pb1n1kpn/1p1qp3/6p1/2PP4/8/P2Q1PPP/3R1RK1 w - -");
  move = createMove("f2f4");
  ASSERT_FALSE(position.givesCheck(move));
  position = Position("2r1r1k1/pb3pp1/1p1qpn2/4n1p1/2PP4/6KP/P2Q1PP1/3RR3 b - -");
  move = createMove("e5d3");
  ASSERT_TRUE(position.givesCheck(move));
  position = Position("R6R/3Q4/1Q4Q1/4Q3/2Q4Q/Q1NNQQ2/1p6/qk3KB1 b - -");
  move = createMove("b1c2");
  ASSERT_TRUE(position.givesCheck(move));
  position = Position("8/8/8/8/8/5K2/R7/7k w - -");
  move = createMove("a2h2");
  ASSERT_TRUE(position.givesCheck(move));
  position = Position("r1bqkb1r/ppp1pppp/2n2n2/1B1P4/8/8/PPPP1PPP/RNBQK1NR w KQkq -");
  move = createMove("d5c6");
  ASSERT_FALSE(position.givesCheck(move));
  position = Position("rnbq1bnr/pppkpppp/8/3p4/3P4/3Q4/PPP1PPPP/RNB1KBNR w KQ -");
  move = createMove("d3h7");
  ASSERT_FALSE(position.givesCheck(move));
}


TEST(PositionTest, isLegalMove) {
  Position::init();
  Bitboards::init();
  NEWLINE;

  string fen;
  Position position;

  // no o-o castling
  fen = "r3k2r/1ppn3p/2q1q1n1/8/2q1Pp2/B5R1/p1p2PPP/1R4K1 b kq e3";
  position = Position(fen);
  ASSERT_FALSE(position.isLegalMove(createMove<CASTLING>(SQ_E8, SQ_G8)));
  ASSERT_TRUE(position.isLegalMove(createMove<CASTLING>(SQ_E8, SQ_C8)));

  // in check - no castling at all
  fen = "r3k2r/1ppn3p/2q1qNn1/8/2q1Pp2/B5R1/p1p2PPP/1R4K1 b kq e3";
  position = Position(fen);
  ASSERT_FALSE(position.isLegalMove(createMove<CASTLING>(SQ_E8, SQ_G8)));
  ASSERT_FALSE(position.isLegalMove(createMove<CASTLING>(SQ_E8, SQ_C8)));

}

TEST(PositionTest, isLegalPosition) {
  Position::init();
  Bitboards::init();
  NEWLINE;

  string fen;
  Position position;

  // no o-o castling
  fen = "r3k2r/1ppn3p/2q1q1n1/8/2q1Pp2/B5R1/p1p2PPP/1R4K1 b kq e3";
  position = Position(fen);

  position.doMove(createMove<CASTLING>(SQ_E8, SQ_G8));
  ASSERT_FALSE(position.isLegalPosition());
  position.undoMove();

  position.doMove(createMove<CASTLING>(SQ_E8, SQ_C8));
  ASSERT_TRUE(position.isLegalPosition());
  position.undoMove();

  // in check - no castling at all
  fen = "r3k2r/1ppn3p/2q1qNn1/8/2q1Pp2/B5R1/p1p2PPP/1R4K1 b kq e3";
  position = Position(fen);

  position.doMove(createMove<CASTLING>(SQ_E8, SQ_G8));
  ASSERT_FALSE(position.isLegalPosition());
  position.undoMove();

  position.doMove(createMove<CASTLING>(SQ_E8, SQ_C8));
  ASSERT_FALSE(position.isLegalPosition());
  position.undoMove();


}