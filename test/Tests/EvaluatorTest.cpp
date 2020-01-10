/*
 * MIT License
 *
 * Copyright (c) 2019 Frank Kopp
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
#include <EvaluatorConfig.h>
#include <boost/timer/timer.hpp>
#include "types.h"
#include "Logging.h"
#include "Evaluator.h"
#include "Position.h"

#include <boost/timer/timer.hpp>

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
  EvaluatorConfig::USE_MOBILITY = false;
  EvaluatorConfig::USE_PAWNEVAL = false;

  int value = evaluator.evaluate(position);
  ASSERT_EQ(0, value);

  position.doMove(createMove("e2e4"));
  value = evaluator.evaluate(position);
  ASSERT_EQ(-55, value);

  position.doMove(createMove("d7d5"));
  value = evaluator.evaluate(position);
  ASSERT_EQ(0, value);

}


TEST_F(EvaluatorTest, total) {

  Position position;
  Evaluator evaluator;

  EvaluatorConfig::USE_MOBILITY = true;
  EvaluatorConfig::USE_PAWNEVAL = true;

  // only test basic material and position

  int value = evaluator.evaluate(position);
  ASSERT_EQ(0, value);

  position.doMove(createMove("e2e4"));
  value = evaluator.evaluate(position);
  ASSERT_EQ(-100, value);

  position.doMove(createMove("e7e5"));
  value = evaluator.evaluate(position);
  ASSERT_EQ(0, value);

}


TEST_F(EvaluatorTest, evaluatePieceMobility) {
  Position position;
  Evaluator evaluator;
  std::string fen;
  int actual;

  // turn off all other piece evaluations but mobility
  EvaluatorConfig::USE_MOBILITY = true;

  // start position
  actual = evaluator.evaluatePiece<WHITE, KNIGHT>(position);
  ASSERT_EQ(4 * EvaluatorConfig::MOBILITY_WEIGHT, actual);
  actual = evaluator.evaluatePiece<BLACK, KNIGHT>(position);
  ASSERT_EQ(4 * EvaluatorConfig::MOBILITY_WEIGHT, actual);
  actual = evaluator.evaluatePiece<WHITE, BISHOP>(position);
  ASSERT_EQ(0 * EvaluatorConfig::MOBILITY_WEIGHT, actual);
  actual = evaluator.evaluatePiece<BLACK, BISHOP>(position);
  ASSERT_EQ(0 * EvaluatorConfig::MOBILITY_WEIGHT, actual);
  actual = evaluator.evaluatePiece<WHITE, ROOK>(position);
  ASSERT_EQ(0 * EvaluatorConfig::MOBILITY_WEIGHT, actual);
  actual = evaluator.evaluatePiece<BLACK, ROOK>(position);
  ASSERT_EQ(0 * EvaluatorConfig::MOBILITY_WEIGHT, actual);
  actual = evaluator.evaluatePiece<WHITE, QUEEN>(position);
  ASSERT_EQ(0 * EvaluatorConfig::MOBILITY_WEIGHT, actual);
  actual = evaluator.evaluatePiece<BLACK, QUEEN>(position);
  ASSERT_EQ(0 * EvaluatorConfig::MOBILITY_WEIGHT, actual);

  // total
  actual = 0;
  actual += evaluator.evaluatePiece<WHITE, KNIGHT>(position) - evaluator.evaluatePiece<BLACK, KNIGHT>(position);
  actual += evaluator.evaluatePiece<WHITE, BISHOP>(position) - evaluator.evaluatePiece<BLACK, BISHOP>(position);
  actual += evaluator.evaluatePiece<WHITE, ROOK>(position) - evaluator.evaluatePiece<BLACK, ROOK>(position);
  actual += evaluator.evaluatePiece<WHITE, QUEEN>(position) - evaluator.evaluatePiece<BLACK, QUEEN>(position);
  ASSERT_EQ(0 * EvaluatorConfig::MOBILITY_WEIGHT, actual);

  // complex pos
  fen = "r3k2r/1ppn3p/2q1q1nb/4P2N/2q1Pp2/B5R1/pbp2PPP/1R4K1 w kq - 0 1";
  position = Position(fen);
  actual = evaluator.evaluatePiece<WHITE, KNIGHT>(position);
  ASSERT_EQ(3 * EvaluatorConfig::MOBILITY_WEIGHT, actual);
  actual = evaluator.evaluatePiece<BLACK, KNIGHT>(position);
  ASSERT_EQ(10 * EvaluatorConfig::MOBILITY_WEIGHT, actual);
  actual = evaluator.evaluatePiece<WHITE, BISHOP>(position);
  ASSERT_EQ(6 * EvaluatorConfig::MOBILITY_WEIGHT, actual);
  actual = evaluator.evaluatePiece<BLACK, BISHOP>(position);
  ASSERT_EQ(9 * EvaluatorConfig::MOBILITY_WEIGHT, actual);
  actual = evaluator.evaluatePiece<WHITE, ROOK>(position);
  ASSERT_EQ(15 * EvaluatorConfig::MOBILITY_WEIGHT, actual);
  actual = evaluator.evaluatePiece<BLACK, ROOK>(position);
  ASSERT_EQ(10 * EvaluatorConfig::MOBILITY_WEIGHT, actual);
  actual = evaluator.evaluatePiece<WHITE, QUEEN>(position);
  ASSERT_EQ(0 * EvaluatorConfig::MOBILITY_WEIGHT, actual);
  actual = evaluator.evaluatePiece<BLACK, QUEEN>(position);
  ASSERT_EQ(31 * EvaluatorConfig::MOBILITY_WEIGHT, actual);

  // total
  actual = 0;
  actual += evaluator.evaluatePiece<WHITE, KNIGHT>(position) - evaluator.evaluatePiece<BLACK, KNIGHT>(position);
  actual += evaluator.evaluatePiece<WHITE, BISHOP>(position) - evaluator.evaluatePiece<BLACK, BISHOP>(position);
  actual += evaluator.evaluatePiece<WHITE, ROOK>(position) - evaluator.evaluatePiece<BLACK, ROOK>(position);
  actual += evaluator.evaluatePiece<WHITE, QUEEN>(position) - evaluator.evaluatePiece<BLACK, QUEEN>(position);
  ASSERT_EQ(-36 * EvaluatorConfig::MOBILITY_WEIGHT, actual);
}


TEST_F(EvaluatorTest, evaluatePawn) {
  Position position;
  Evaluator evaluator;
  std::string fen;
  int actual;

  EvaluatorConfig::USE_PAWNEVAL = true;

  // start position
  actual = evaluator.evaluatePawn<WHITE>(position);
  ASSERT_EQ(-277 * EvaluatorConfig::PAWNEVAL_WEIGHT, actual);
  actual = evaluator.evaluatePawn<BLACK>(position);
  ASSERT_EQ(-277 * EvaluatorConfig::PAWNEVAL_WEIGHT, actual);

  // total
  actual = 0;
  actual += evaluator.evaluatePawn<WHITE>(position) - evaluator.evaluatePawn<BLACK>(position);
  ASSERT_EQ(0, actual);

  // complex pos
  fen = "r3k2r/1ppn3p/2q1q1nb/4P2N/2q1Pp2/B5RP/pbp2PP1/1R4K1 w kq - 0 1";
  position = Position(fen);
  actual = evaluator.evaluatePawn<WHITE>(position);
  ASSERT_EQ(-376 * EvaluatorConfig::PAWNEVAL_WEIGHT, actual);
  actual = evaluator.evaluatePawn<BLACK>(position);
  ASSERT_EQ(-895 * EvaluatorConfig::PAWNEVAL_WEIGHT, actual);

  // total
  actual = 0;
  ASSERT_EQ(0 * EvaluatorConfig::PAWNEVAL_WEIGHT, actual);

}

TEST_F(EvaluatorTest, debugging) {

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

TEST_F(EvaluatorTest, PERFT_eps) {
  using namespace boost::timer;
  std::string fen;
  Evaluator e;
  Position position;
  const int nano_sec = 1'000'000'000;

  fen = "r3k2r/1ppn3p/2q1q1nb/4P2N/2q1Pp2/B5RP/pbp2PP1/1R4K1 w kq - 0 1";
  position = Position(fen);
  const uint64_t iterations = 10'000'000;
  const uint64_t rounds = 10;

  for (uint64_t round = 0; round < rounds; ++round) {
    fprintln("ROUND: {}", round);
    auto timer = cpu_timer();
    for (uint64_t i = 0; i < iterations; ++i) {
      e.evaluate(position);
    }
    timer.stop();
    const nanosecond_type cpuTime = timer.elapsed().user + timer.elapsed().system;
    fprintln("WALL Time: {:n} ns ({:3f} sec)", timer.elapsed().wall, static_cast<double>(timer.elapsed().wall)/nano_sec);
    fprintln("CPU  Time: {:n} ns ({:3f} sec)", cpuTime, static_cast<double>(cpuTime)/nano_sec);
    fprintln("EPS:       {:n} eps", (iterations * nano_sec) / cpuTime);
    fprintln("TPE:       {:n} ns", cpuTime / iterations);
    NEWLINE;
  }
}