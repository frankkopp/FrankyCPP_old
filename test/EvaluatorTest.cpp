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

#include <gtest/gtest.h>
#include <boost/timer/timer.hpp>
#include "types.h"
#include "Logging.h"
#include "Evaluator.h"
#include "Position.h"
#include "Test_Fens.h"

using testing::Eq;

class EvaluatorTest : public ::testing::Test {
public:
  static void SetUpTestSuite() {
    NEWLINE;
    LOGGING::init();
    INIT::init();
    NEWLINE;
    // turn off info and below logging in the application
    spdlog::set_level(spdlog::level::trace);
  }

  std::shared_ptr<spdlog::logger> LOG = spdlog::get("Test_Logger");
protected:
  void SetUp() override {
    LOG->set_level(spdlog::level::info);
  }

  void TearDown() override {}
};

TEST_F(EvaluatorTest, basic) {

  Position position;
  Evaluator evaluator;

  // only test basic material and position
  evaluator.config.USE_MOBILITY = false;
  evaluator.config.USE_PAWNEVAL = false;
  evaluator.config.USE_CHECK_BONUS = false;
  evaluator.config.USE_PIECE_BONI = false;

  int value = evaluator.evaluate(position);
  ASSERT_EQ(0 + evaluator.config.TEMPO, value);

  position.doMove(createMove("e2e4"));
  value = evaluator.evaluate(position);
  ASSERT_EQ(-55 + evaluator.config.TEMPO, value);

  position.doMove(createMove("d7d5"));
  value = evaluator.evaluate(position);
  ASSERT_EQ(0 + evaluator.config.TEMPO, value);
}

TEST_F(EvaluatorTest, evaluatePieceMobility) {
  Position position;
  Evaluator evaluator;
  std::string fen;
  int actual;

  // turn off all other piece evaluations but mobility
  evaluator.config.USE_MOBILITY = true;
  evaluator.config.USE_PIECE_BONI = false;

  // start position
  actual = evaluator.evaluatePiece<WHITE, KNIGHT>(position);
  ASSERT_EQ(4 * evaluator.config.MOBILITY_WEIGHT, actual);
  actual = evaluator.evaluatePiece<BLACK, KNIGHT>(position);
  ASSERT_EQ(4 * evaluator.config.MOBILITY_WEIGHT, actual);
  actual = evaluator.evaluatePiece<WHITE, BISHOP>(position);
  ASSERT_EQ(0 * evaluator.config.MOBILITY_WEIGHT, actual);
  actual = evaluator.evaluatePiece<BLACK, BISHOP>(position);
  ASSERT_EQ(0 * evaluator.config.MOBILITY_WEIGHT, actual);
  actual = evaluator.evaluatePiece<WHITE, ROOK>(position);
  ASSERT_EQ(0 * evaluator.config.MOBILITY_WEIGHT, actual);
  actual = evaluator.evaluatePiece<BLACK, ROOK>(position);
  ASSERT_EQ(0 * evaluator.config.MOBILITY_WEIGHT, actual);
  actual = evaluator.evaluatePiece<WHITE, QUEEN>(position);
  ASSERT_EQ(0 * evaluator.config.MOBILITY_WEIGHT, actual);
  actual = evaluator.evaluatePiece<BLACK, QUEEN>(position);
  ASSERT_EQ(0 * evaluator.config.MOBILITY_WEIGHT, actual);

  // total
  actual = 0;
  actual += evaluator.evaluatePiece<WHITE, KNIGHT>(position) - evaluator.evaluatePiece<BLACK, KNIGHT>(position);
  actual += evaluator.evaluatePiece<WHITE, BISHOP>(position) - evaluator.evaluatePiece<BLACK, BISHOP>(position);
  actual += evaluator.evaluatePiece<WHITE, ROOK>(position) - evaluator.evaluatePiece<BLACK, ROOK>(position);
  actual += evaluator.evaluatePiece<WHITE, QUEEN>(position) - evaluator.evaluatePiece<BLACK, QUEEN>(position);
  ASSERT_EQ(0 * evaluator.config.MOBILITY_WEIGHT, actual);

  // complex pos
  fen = "r3k2r/1ppn3p/2q1q1nb/4P2N/2q1Pp2/B5R1/pbp2PPP/1R4K1 w kq - 0 1";
  position = Position(fen);
  actual = evaluator.evaluatePiece<WHITE, KNIGHT>(position);
  ASSERT_EQ(3 * evaluator.config.MOBILITY_WEIGHT, actual);
  actual = evaluator.evaluatePiece<BLACK, KNIGHT>(position);
  ASSERT_EQ(10 * evaluator.config.MOBILITY_WEIGHT, actual);
  actual = evaluator.evaluatePiece<WHITE, BISHOP>(position);
  ASSERT_EQ(6 * evaluator.config.MOBILITY_WEIGHT, actual);
  actual = evaluator.evaluatePiece<BLACK, BISHOP>(position);
  ASSERT_EQ(9 * evaluator.config.MOBILITY_WEIGHT, actual);
  actual = evaluator.evaluatePiece<WHITE, ROOK>(position);
  ASSERT_EQ(15 * evaluator.config.MOBILITY_WEIGHT, actual);
  actual = evaluator.evaluatePiece<BLACK, ROOK>(position);
  ASSERT_EQ(10 * evaluator.config.MOBILITY_WEIGHT, actual);
  actual = evaluator.evaluatePiece<WHITE, QUEEN>(position);
  ASSERT_EQ(0 * evaluator.config.MOBILITY_WEIGHT, actual);
  actual = evaluator.evaluatePiece<BLACK, QUEEN>(position);
  ASSERT_EQ(31 * evaluator.config.MOBILITY_WEIGHT, actual);

  // total
  actual = 0;
  actual += evaluator.evaluatePiece<WHITE, KNIGHT>(position) - evaluator.evaluatePiece<BLACK, KNIGHT>(position);
  actual += evaluator.evaluatePiece<WHITE, BISHOP>(position) - evaluator.evaluatePiece<BLACK, BISHOP>(position);
  actual += evaluator.evaluatePiece<WHITE, ROOK>(position) - evaluator.evaluatePiece<BLACK, ROOK>(position);
  actual += evaluator.evaluatePiece<WHITE, QUEEN>(position) - evaluator.evaluatePiece<BLACK, QUEEN>(position);
  ASSERT_EQ(-36 * evaluator.config.MOBILITY_WEIGHT, actual);
}


TEST_F(EvaluatorTest, evaluatePawns) {
  Position position;
  Evaluator evaluator;
  std::string fen;
  int actual;

  evaluator.config.USE_PAWNEVAL = true;
  evaluator.config.USE_PAWN_TABLE = true;

  // start position
  actual = evaluator.pawnEval(position);
  ASSERT_EQ(0, actual);
  actual = evaluator.pawnEval(position);
  ASSERT_EQ(0, actual);

  NEWLINE;

  // complex pos
  fen = "r3k2r/1ppn3p/2q1q1nb/4P2N/2q1Pp2/B5RP/pbp2PP1/1R4K1 w kq - 0 1";
  position = Position(fen);
  actual = evaluator.pawnEval(position);
  ASSERT_EQ(-15, actual);
  actual = evaluator.pawnEval(position);
  ASSERT_EQ(-15, actual);
}

TEST_F(EvaluatorTest, pieceBoni) {
  Position position;
  Evaluator evaluator;
  std::string fen;
  int actual;

  evaluator.config.USE_MATERIAL = false;
  evaluator.config.USE_POSITION = false;
  evaluator.config.USE_PAWNEVAL = false;
  evaluator.config.USE_CHECK_BONUS = false;
  evaluator.config.USE_MOBILITY = false;
  evaluator.config.USE_PIECE_BONI = true;
  evaluator.config.USE_KING_CASTLE_SAFETY = false;
  evaluator.config.TEMPO = 0;

  // start position
  actual = evaluator.evaluate(position);
  ASSERT_EQ(0, actual);

  NEWLINE;

  // complex pos
  fen = "r3k2r/1ppn3p/2q1q1nb/4P2N/2q1Pp2/B5RP/pbp2PP1/1R4K1 w kq - 0 1";
  position = Position(fen);
  actual = evaluator.evaluate(position);
  ASSERT_EQ(-40, actual);
}


TEST_F(EvaluatorTest, kingCastleSafety) {
  Position position;
  Evaluator evaluator;
  std::string fen;
  int actual;

  evaluator.config.USE_MATERIAL = false;
  evaluator.config.USE_POSITION = false;
  evaluator.config.USE_PAWNEVAL = false;
  evaluator.config.USE_CHECK_BONUS = false;
  evaluator.config.USE_MOBILITY = false;
  evaluator.config.USE_PIECE_BONI = false;
  evaluator.config.USE_KING_CASTLE_SAFETY = true;
  evaluator.config.TEMPO = 0;

  // start position
  actual = evaluator.evaluate(position);
  ASSERT_EQ(0, actual);

  NEWLINE;

  // complex pos
  fen = "r3k2r/1ppn3p/2q1q1nb/4P2N/2q1Pp2/B5RP/pbp2PP1/1R4K1 w kq - 0 1";
  position = Position(fen);
  actual = evaluator.evaluate(position);
  ASSERT_EQ(50, actual);
}

TEST_F(EvaluatorTest, total) {

  Position position;
  Evaluator evaluator;

  // only test basic material and position

  int value = evaluator.evaluate(position);
  ASSERT_EQ(0 + evaluator.config.TEMPO, value);

  position.doMove(createMove("e2e4"));
  value = evaluator.evaluate(position);
  ASSERT_EQ(-70 + evaluator.config.TEMPO, value);

  position.doMove(createMove("e7e5"));
  value = evaluator.evaluate(position);
  ASSERT_EQ(0 + evaluator.config.TEMPO, value);
}


TEST_F(EvaluatorTest, fens) {
  using namespace boost::timer;
  Position position;
  auto fens = Test_Fens::getFENs();
  static constexpr int NUMBER_OF_FENS = 9999;

  Evaluator evaluator;
  evaluator.config.USE_MATERIAL = true;
  evaluator.config.USE_POSITION = true;
  evaluator.config.USE_PAWNEVAL = true;
  evaluator.config.USE_PAWN_TABLE = true;
  evaluator.config.PAWN_TABLE_SIZE = 2'097'152;
  evaluator.config.USE_CHECK_BONUS = true;
  evaluator.config.USE_MOBILITY = true;
  evaluator.config.USE_PIECE_BONI = true;
  evaluator.config.USE_KING_CASTLE_SAFETY = true;
  evaluator.resizePawnTable(evaluator.config.PAWN_TABLE_SIZE);

  auto iterEnd = NUMBER_OF_FENS > fens.size() ? fens.end() : fens.begin() + NUMBER_OF_FENS;
  auto timerTotal = cpu_timer();
  for (auto fen = fens.begin(); fen != iterEnd; ++fen) {
    auto timer = cpu_timer();
    position = Position(*fen);
    timer.stop();
    fprint("value = {:6}   {:67}   {}   {}", evaluator.evaluate(position), *fen, evaluator.pawnTableStats(), timer.format());
  }
  timerTotal.stop();
  fprint("{}", timerTotal.format());
}

TEST_F(EvaluatorTest, DISABLED_PERFT_eps) {
  using namespace boost::timer;
  std::string fen;
  Position position;
  const int nano_sec = 1'000'000'000;

  fen = "r3k2r/1ppn3p/2q1q1nb/4P2N/2q1Pp2/B5RP/pbp2PP1/1R4K1 w kq - 0 1";
  position = Position(fen);
  const uint64_t iterations = 50'000'000;
  const uint64_t rounds = 5;

  Evaluator evaluator;
  evaluator.config.USE_MATERIAL = true;
  evaluator.config.USE_POSITION = true;
  evaluator.config.USE_PAWNEVAL = true;
  evaluator.config.USE_PAWN_TABLE = true;
  evaluator.config.PAWN_TABLE_SIZE = 2'097'152;
  evaluator.config.USE_CHECK_BONUS = true;
  evaluator.config.USE_MOBILITY = true;
  evaluator.config.USE_PIECE_BONI = true;
  evaluator.config.USE_KING_CASTLE_SAFETY = true;
  evaluator.resizePawnTable(evaluator.config.PAWN_TABLE_SIZE);

  for (uint64_t round = 0; round < rounds; ++round) {
    fprintln("ROUND: {}", round + 1);
    auto timer = cpu_timer();
    for (uint64_t i = 0; i < iterations; ++i) {
      evaluator.evaluate(position);
    }
    timer.stop();
    const nanosecond_type cpuTime = timer.elapsed().user + timer.elapsed().system;
    fprintln("WALL Time: {:n} ns ({:3f} sec)", timer.elapsed().wall, static_cast<double>(timer.elapsed().wall) / nano_sec);
    fprintln("CPU  Time: {:n} ns ({:3f} sec)", cpuTime, static_cast<double>(cpuTime) / nano_sec);
    fprintln("EPS:       {:n} eps", (iterations * nano_sec) / cpuTime);
    fprintln("TPE:       {:n} ns", cpuTime / iterations);
    NEWLINE;
  }
}


TEST_F(EvaluatorTest, DISABLED_debugging) {

  Evaluator evaluator;
  Value value;
  Position position;

  //Storing into TT: 16558441573230445409 4352 EXACT 1  a2a1q b1a1 c2c1q
  //Storing into TT: 16558441573230445409 4365 EXACT 1  a2a1r b1a1 c2c1q -4365

  position = Position("r3k2r/1ppn3p/2q1q1n1/4P3/2q1Pp2/6R1/1b3PPP/R1q3K1 w kq - 0 3");
  value = evaluator.evaluate(position);
  fmt::print("Value {}\n", value);

  position = Position("r3k2r/1ppn3p/2q1q1n1/4P3/2q1Pp2/6R1/pbp2PPP/1R4K1 b kq e3 0 1");
  position.doMove(createMove<PROMOTION>("a2a1q"));
  position.doMove(createMove("b1a1"));
  position.doMove(createMove<PROMOTION>("c2c1q"));
  value = evaluator.evaluate(position);
  fmt::print("Value {}\n", value);

  position = Position("r3k2r/1ppn3p/2q1q1n1/4P3/2q1Pp2/6R1/pbp2PPP/1R4K1 b kq e3 0 1");
  position.doMove(createMove<PROMOTION>("a2a1r"));
  position.doMove(createMove("b1a1"));
  position.doMove(createMove<PROMOTION>("c2c1q"));
  value = evaluator.evaluate(position);
  fmt::print("Value {}\n", value);

}
