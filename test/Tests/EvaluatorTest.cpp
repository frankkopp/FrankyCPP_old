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
#include "../../src/Logging.h"
#include "../../src/Evaluator.h"
#include "../../src/Position.h"


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

  int value = evaluator.evaluate(position);
  ASSERT_EQ(0, value);

  position.doMove(createMove("e2e4"));
  value = evaluator.evaluate(position);
  ASSERT_TRUE(value == -55);

  position.doMove(createMove("d7d5"));
  value = evaluator.evaluate(position);
  ASSERT_TRUE(value == 0);

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
