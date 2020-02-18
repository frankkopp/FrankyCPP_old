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
#include "Logging.h"
#include "types.h"

using testing::Eq;

class LoggingTest : public ::testing::Test {
public:
  static void SetUpTestSuite() {
    NEWLINE;
    INIT::init();
    NEWLINE;
  }

protected:
  void SetUp() override {
  }

  void TearDown() override {}
};

TEST_F(LoggingTest, macro) {
  LOG__CRITICAL(Logger::get().TEST_LOG, "CRITICAL {:n}", 1234567890);
  LOG__ERROR(Logger::get().TEST_LOG, "ERROR {:n}", 1234567890);
  LOG__WARN(Logger::get().TEST_LOG, "WARN {:n}", 1234567890);
  LOG__INFO(Logger::get().TEST_LOG, "INFO {:n}", 1234567890);
  LOG__DEBUG(Logger::get().TEST_LOG, "DEBUG {:n}", 1234567890);
  LOG__TRACE(Logger::get().TEST_LOG, "TRACE {:n}", 1234567890);
}


TEST_F(LoggingTest, decimal) {
  auto s = fmt::format("{:n}", 1234567890);
  EXPECT_EQ("1234567890", s);
  std::cout << "Direct cout with fmt::format(\"{:n}\",           1234567890): " << s << std::endl;
  s = fmt::format(deLocale, "{:n}", 1234567890);
  EXPECT_EQ("1.234.567.890", s);
  std::cout << "Direct cout with fmt::format(deLocale, \"{:n}\", 1234567890): " << s << std::endl;
  Logger::get().TEST_LOG->error("SPDLOG ERROR {:n}", 1234567890);
  LOG__ERROR(Logger::get().TEST_LOG, "SPDLOG ERROR {:n}", 1234567890);


  //s = fmt::format("{:n}", std::numeric_limits<__int128_t>::max());
  //std::cout << "Direct cout  : " << s << std::endl;
  //LOG__INFO(Logger::get().TEST_LOG, "INFO {:n}", std::numeric_limits<__int128_t>::max());
  //ASSERT_EQ("170.141.183.460.469.231.731.687.303.715.884.105.727", s);
}

TEST_F(LoggingTest, basic) {
  // Logger test
  LOG__INFO(Logger::get().TEST_LOG, "TEST LOGGER:");
 LOG__CRITICAL(Logger::get().TEST_LOG, "CRITICAL");
  LOG__ERROR(Logger::get().TEST_LOG, "ERROR");
  LOG__WARN(Logger::get().TEST_LOG, "WARN");
  LOG__INFO(Logger::get().TEST_LOG, "INFO");
  LOG__DEBUG(Logger::get().TEST_LOG, "DEBUG");
  LOG__TRACE(Logger::get().TEST_LOG, "TRACE");
  LOG__TRACE(Logger::get().TEST_LOG, "TRACE {}",  "MARCO");
  LOG__INFO(Logger::get().TEST_LOG, "INFO {}", 123456789);
  LOG__INFO(Logger::get().TEST_LOG, "INFO {:d}", 123456789);
  LOG__INFO(Logger::get().TEST_LOG, "INFO {:n}", 123456789);
  LOG__INFO(Logger::get().TEST_LOG, "INFO {:.5n}", double(123456789.12345));

  // Logger test
  LOG__INFO(Logger::get().MAIN_LOG, "MAIN LOGGER TESTS:");
 LOG__CRITICAL(Logger::get().MAIN_LOG, "CRITICAL");
  LOG__ERROR(Logger::get().MAIN_LOG, "ERROR");
  LOG__WARN(Logger::get().MAIN_LOG, "WARN");
  LOG__INFO(Logger::get().MAIN_LOG, "INFO");
  LOG__DEBUG(Logger::get().MAIN_LOG, "DEBUG");
  LOG__TRACE(Logger::get().MAIN_LOG, "TRACE");
  LOG__TRACE(Logger::get().MAIN_LOG, "TRACE {}",  "MARCO");
  LOG__INFO(Logger::get().MAIN_LOG, "INFO {}", 123456789);
  LOG__INFO(Logger::get().MAIN_LOG, "INFO {:d}", 123456789);
  LOG__INFO(Logger::get().MAIN_LOG, "INFO {:n}", 123456789);
  LOG__INFO(Logger::get().MAIN_LOG, "INFO {:.5n}", double(123456789.12345));

  // Logger test
  LOG__INFO(Logger::get().ENGINE_LOG, "ENGINE LOGGER TESTS:");
  LOG__CRITICAL(Logger::get().ENGINE_LOG, "CRITICAL");
  LOG__ERROR(Logger::get().ENGINE_LOG, "ERROR");
  LOG__WARN(Logger::get().ENGINE_LOG, "WARN");
  LOG__INFO(Logger::get().ENGINE_LOG, "INFO");
  LOG__DEBUG(Logger::get().ENGINE_LOG, "DEBUG");
  LOG__TRACE(Logger::get().ENGINE_LOG, "TRACE");
  LOG__TRACE(Logger::get().ENGINE_LOG, "TRACE {}",  "MARCO");
  LOG__INFO(Logger::get().ENGINE_LOG, "INFO {}", 123456789);
  LOG__INFO(Logger::get().ENGINE_LOG, "INFO {:d}", 123456789);
  LOG__INFO(Logger::get().ENGINE_LOG, "INFO {:n}", 123456789);
  LOG__INFO(Logger::get().ENGINE_LOG, "INFO {:.5n}", double(123456789.12345));

  // Logger test
  LOG__INFO(Logger::get().SEARCH_LOG, "SEARCH LOGGER TESTS:");
  LOG__CRITICAL(Logger::get().SEARCH_LOG, "CRITICAL");
  LOG__ERROR(Logger::get().SEARCH_LOG, "ERROR");
  LOG__WARN(Logger::get().SEARCH_LOG, "WARN");
  LOG__INFO(Logger::get().SEARCH_LOG, "INFO");
  LOG__DEBUG(Logger::get().SEARCH_LOG, "DEBUG");
  LOG__TRACE(Logger::get().SEARCH_LOG, "TRACE");
  LOG__TRACE(Logger::get().SEARCH_LOG, "TRACE {}",  "MARCO");
  LOG__INFO(Logger::get().SEARCH_LOG, "INFO {}", 123456789);
  LOG__INFO(Logger::get().SEARCH_LOG, "INFO {:d}", 123456789);
  LOG__INFO(Logger::get().SEARCH_LOG, "INFO {:n}", 123456789);
  LOG__INFO(Logger::get().SEARCH_LOG, "INFO {:.5n}", double(123456789.12345));

  // Logger test
  LOG__INFO(Logger::get().UCI_LOG, "UCI LOGGER TESTS:");
  LOG__CRITICAL(Logger::get().UCI_LOG, "CRITICAL");
  LOG__ERROR(Logger::get().UCI_LOG, "ERROR");
  LOG__WARN(Logger::get().UCI_LOG, "WARN");
  LOG__INFO(Logger::get().UCI_LOG, "INFO");
  LOG__DEBUG(Logger::get().UCI_LOG, "DEBUG");
  LOG__TRACE(Logger::get().UCI_LOG, "TRACE");
  LOG__TRACE(Logger::get().UCI_LOG, "TRACE {}",  "MARCO");
  LOG__INFO(Logger::get().UCI_LOG, "INFO {}", 123456789);
  LOG__INFO(Logger::get().UCI_LOG, "INFO {:d}", 123456789);
  LOG__INFO(Logger::get().UCI_LOG, "INFO {:n}", 123456789);
  LOG__INFO(Logger::get().UCI_LOG, "INFO {:.5n}", double(123456789.12345));
}
