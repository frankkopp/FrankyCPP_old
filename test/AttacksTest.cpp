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


#include "Bitboards.h"
#include "Logging.h"
#include "types.h"
#include <Attacks.h>
#include <Position.h>
#include <gtest/gtest.h>
#include <misc.h>
using testing::Eq;

class AttacksTest : public ::testing::Test {
public:
  static void SetUpTestSuite() {
    NEWLINE;
    INIT::init();
    NEWLINE;
    Logger::get().TEST_LOG->set_level(spdlog::level::debug);
    Logger::get().SEARCH_LOG->set_level(spdlog::level::debug);
  }

protected:
  void SetUp() override {}
  void TearDown() override {}
};

TEST_F(AttacksTest, attacksTo) {
  Position position("2brr1k1/1pq1b1p1/p1np1p1p/P1p1p2n/1PNPPP2/2P1BNP1/4Q1BP/R2R2K1 w - -");
  Bitboard attacksTo = Attacks::attacksTo(position, SQ_E5, WHITE);
  fprint("{}", Bitboards::print(attacksTo));
  fprintln("{}", Bitboards::printFlat(attacksTo));
  EXPECT_EQ(740294656, attacksTo);

  attacksTo = Attacks::attacksTo(position, SQ_E5, BLACK);
  fprint("{}", Bitboards::print(attacksTo));
  fprintln("{}", Bitboards::printFlat(attacksTo));
  EXPECT_EQ(48378511622144, attacksTo);

  attacksTo = Attacks::attacksTo(position, SQ_D4, WHITE);
  fprint("{}", Bitboards::print(attacksTo));
  fprintln("{}", Bitboards::printFlat(attacksTo));
  EXPECT_EQ(3407880, attacksTo);

  attacksTo = Attacks::attacksTo(position, SQ_D4, BLACK);
  fprint("{}", Bitboards::print(attacksTo));
  fprintln("{}", Bitboards::printFlat(attacksTo));
  EXPECT_EQ(4483945857024, attacksTo);

  position  = Position("r3k2r/1ppn3p/2q1q1n1/4P3/2q1Pp2/6R1/pbp2PPP/1R4K1 b kq e3");
  attacksTo = Attacks::attacksTo(position, SQ_E5, BLACK);
  fprint("{}", Bitboards::print(attacksTo));
  fprintln("{}", Bitboards::printFlat(attacksTo));
  EXPECT_EQ(2339760743907840, attacksTo);

  attacksTo = Attacks::attacksTo(position, SQ_A3, BLACK);
  fprint("{}", Bitboards::print(attacksTo));
  fprintln("{}", Bitboards::printFlat(attacksTo));
  EXPECT_EQ(72057594037928448, attacksTo);
}

TEST_F(AttacksTest, revealedAttacks) {
  // 1k1r3q/1ppn3p/p4b2/4p3/8/P2N2P1/1PP1R1BP/2K1Q3 w - - ; Nxe5?;
  Position position("1k1r3q/1ppn3p/p4b2/4p3/8/P2N2P1/1PP1R1BP/2K1Q3 w - -");
  Bitboard occupiedBitboard = position.getOccupiedBB();

  const Square square = SQ_E5;

  Bitboard attacksTo = Attacks::attacksTo(position, square, BLACK) | Attacks::attacksTo(position, square, WHITE);
  fprintln("Direkt:");
  fprint("{}", Bitboards::print(attacksTo));
  fprintln("{}", Bitboards::printFlat(attacksTo));
  EXPECT_EQ(2286984186302464, attacksTo);

  // take away bishop on f6
  attacksTo ^= Bitboards::squareBB[SQ_F6];        // reset bit in set to traverse
  occupiedBitboard ^= Bitboards::squareBB[SQ_F6]; // reset bit in temporary occupancy (for x-Rays)

  attacksTo |= Attacks::revealedAttacks(position, square, occupiedBitboard, BLACK)
               | Attacks::revealedAttacks(position, square, occupiedBitboard, WHITE);

  fprintln("Revealed after removing bishop on f6:");
  fprint("{}", Bitboards::print(attacksTo));
  fprintln("{}", Bitboards::printFlat(attacksTo));
  EXPECT_EQ(9225623836668989440, attacksTo);

  // take away rook on e2
  attacksTo ^= Bitboards::squareBB[SQ_E2];        // reset bit in set to traverse
  occupiedBitboard ^= Bitboards::squareBB[SQ_E2]; // reset bit in temporary occupancy (for x-Rays)

  attacksTo |= Attacks::revealedAttacks(position, square, occupiedBitboard, BLACK)
               | Attacks::revealedAttacks(position, square, occupiedBitboard, WHITE);

  fprintln("Revealed after removing rook on e2:");
  fprint("{}", Bitboards::print(attacksTo));
  fprintln("{}", Bitboards::printFlat(attacksTo));
  EXPECT_EQ(9225623836668985360, attacksTo);
}

TEST_F(AttacksTest, leastValuablePiece) {
  Position position("r3k2r/1ppn3p/2q1q1n1/4P3/2q1Pp2/6R1/pbp2PPP/1R4K1 b kq e3");
  Bitboard attacksTo = Attacks::attacksTo(position, SQ_E5, BLACK);

  fprintln("All attackers");
  fprint("{}", Bitboards::print(attacksTo));
  fprintln("{}", Bitboards::printFlat(attacksTo));
  fprintln("{}", position.printBoard());

  Square lva = Attacks::getLeastValuablePiece(position, attacksTo, BLACK);
  fprintln("Least valuable attacker: {}", squareLabel(lva));
  EXPECT_EQ(SQ_G6, lva);

  // remove the attacker
  attacksTo ^= lva;

  lva = Attacks::getLeastValuablePiece(position, attacksTo, BLACK);
  fprintln("Least valuable attacker: {}", squareLabel(lva));
  EXPECT_EQ(SQ_D7, lva);

  // remove the attacker
  attacksTo ^= lva;

  lva = Attacks::getLeastValuablePiece(position, attacksTo, BLACK);
  fprintln("Least valuable attacker: {}", squareLabel(lva));
  EXPECT_EQ(SQ_B2, lva);

  // remove the attacker
  attacksTo ^= lva;

  lva = Attacks::getLeastValuablePiece(position, attacksTo, BLACK);
  fprintln("Least valuable attacker: {}", squareLabel(lva));
  EXPECT_EQ(SQ_E6, lva);

  // remove the attacker
  attacksTo ^= lva;

  lva = Attacks::getLeastValuablePiece(position, attacksTo, BLACK);
  fprintln("Least valuable attacker: {}", squareLabel(lva));
  EXPECT_EQ(SQ_NONE, lva);
}

TEST_F(AttacksTest, seeTest) {
  Attacks attacks{};
  // 1k1r3q/1ppn3p/p4b2/4p3/8/P2N2P1/1PP1R1BP/2K1Q3 w - - ; Nxe5?
  Position position("1k1r3q/1ppn3p/p4b2/4p3/8/P2N2P1/1PP1R1BP/2K1Q3 w - -");
  Move     move     = Misc::getMoveFromUCI(position, "d3e5");
  Value    seeScore = attacks.see(position, move);
  LOG__DEBUG(Logger::get().TEST_LOG, "See score = {}", seeScore);
  EXPECT_EQ(-220, seeScore);

  // 1k1r4/1pp4p/p7/4p3/8/P5P1/1PP4P/2K1R3 w - - ; Rxe5?
  position = Position("1k1r4/1pp4p/p7/4p3/8/P5P1/1PP4P/2K1R3 w - -");
  move     = Misc::getMoveFromUCI(position, "e1e5");
  seeScore = attacks.see(position, move);
  LOG__DEBUG(Logger::get().TEST_LOG, "See score = {}", seeScore);
  EXPECT_EQ(100, seeScore);

  // 5q1k/8/8/8/RRQ2nrr/8/8/K7 w - - 0 1
  position = Position("5q1k/8/8/8/RRQ2nrr/8/8/K7 w - -");
  move     = Misc::getMoveFromUCI(position, "c4f4");
  seeScore = attacks.see(position, move);
  LOG__DEBUG(Logger::get().TEST_LOG, "See score = {}", seeScore);
  EXPECT_EQ(-580, seeScore);

  // k6q/3n1n2/3b4/4p3/3P1P2/3N1N2/8/K7 w - -
  position = Position("k6q/3n1n2/3b4/4p3/3P1P2/3N1N2/8/K7 w - -");
  move     = Misc::getMoveFromUCI(position, "d3e5");
  seeScore = attacks.see(position, move);
  LOG__DEBUG(Logger::get().TEST_LOG, "See score = {}", seeScore);
  EXPECT_EQ(100, seeScore);

  // r3k2r/1ppn3p/2q1q1n1/4P3/2q1Pp2/6R1/pbp2PPP/1R2R1K1 b kq e3 0 1
  position = Position("r3k2r/1ppn3p/2q1q1n1/4P3/2q1Pp2/6R1/pbp2PPP/1R2R1K1 b kq e3 0 1 ");
  move     = Misc::getMoveFromUCI(position, "a2b1Q");
  seeScore = attacks.see(position, move);
  LOG__DEBUG(Logger::get().TEST_LOG, "See score = {}", seeScore);
  EXPECT_EQ(500, seeScore);

}
