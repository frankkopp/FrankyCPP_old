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
    LOGGING::init();
    INIT::init();
    NEWLINE;
  }

  std::shared_ptr<spdlog::logger> TEST_LOG = spdlog::get("Test_Logger");
  std::shared_ptr<spdlog::logger> MAIN_LOG = spdlog::get("Main_Logger");
  std::shared_ptr<spdlog::logger> SEARCH_LOG = spdlog::get("Search_Logger");
  std::shared_ptr<spdlog::logger> ENGINE_LOG = spdlog::get("Engine_Logger");
  std::shared_ptr<spdlog::logger> UCI_LOG = spdlog::get("UCI_Logger");
protected:
  void SetUp() override {
  }

  void TearDown() override {}
};

TEST_F(LoggingTest, macro) {
  LOG__CRITICAL(TEST_LOG, "CRITICAL {:n}", 1234567890);
  LOG__ERROR(TEST_LOG, "ERROR {:n}", 1234567890);
  LOG__WARN(TEST_LOG, "WARN {:n}", 1234567890);
  LOG__INFO(TEST_LOG, "INFO {:n}", 1234567890);
  LOG__DEBUG(TEST_LOG, "DEBUG {:n}", 1234567890);
  LOG__TRACE(TEST_LOG, "TRACE {:n}", 1234567890);
}


TEST_F(LoggingTest, decimal) {
  auto s = fmt::format(deLocale, "{:n}", 1234567890);
  std::cout << "Direct cout  : " << s << std::endl;
  fprintln("Macro with ln: {:n}", 1234567890);
  LOG__INFO(TEST_LOG, "INFO {:n}", 1234567890);
  ASSERT_EQ("1.234.567.890", s);
  s = fmt::format("{:n}", 1234567890);
  std::cout << "Direct cout  : " << s << std::endl;
  LOG__INFO(TEST_LOG, "INFO {:n}", 1234567890);
  ASSERT_EQ("1.234.567.890", s);

  //s = fmt::format("{:n}", std::numeric_limits<__int128_t>::max());
  //std::cout << "Direct cout  : " << s << std::endl;
  //LOG__INFO(TEST_LOG, "INFO {:n}", std::numeric_limits<__int128_t>::max());
  //ASSERT_EQ("170.141.183.460.469.231.731.687.303.715.884.105.727", s);
}

TEST_F(LoggingTest, basic) {
  // Logger test
  LOG__INFO(TEST_LOG, "TEST LOGGER:");
 LOG__CRITICAL(TEST_LOG, "CRITICAL");
  LOG__ERROR(TEST_LOG, "ERROR");
  LOG__WARN(TEST_LOG, "WARN");
  LOG__INFO(TEST_LOG, "INFO");
  LOG__DEBUG(TEST_LOG, "DEBUG");
  LOG__TRACE(TEST_LOG, "TRACE");
  LOG__TRACE(TEST_LOG, "TRACE {}",  "MARCO");
  LOG__INFO(TEST_LOG, "INFO {}", 123456789);
  LOG__INFO(TEST_LOG, "INFO {:d}", 123456789);
  LOG__INFO(TEST_LOG, "INFO {:n}", 123456789);
  LOG__INFO(TEST_LOG, "INFO {:.5n}", double(123456789.12345));

  // Logger test
  LOG__INFO(MAIN_LOG, "MAIN LOGGER TESTS:");
 LOG__CRITICAL(MAIN_LOG, "CRITICAL");
  LOG__ERROR(MAIN_LOG, "ERROR");
  LOG__WARN(MAIN_LOG, "WARN");
  LOG__INFO(MAIN_LOG, "INFO");
  LOG__DEBUG(MAIN_LOG, "DEBUG");
  LOG__TRACE(MAIN_LOG, "TRACE");
  LOG__TRACE(MAIN_LOG, "TRACE {}",  "MARCO");
  LOG__INFO(MAIN_LOG, "INFO {}", 123456789);
  LOG__INFO(MAIN_LOG, "INFO {:d}", 123456789);
  LOG__INFO(MAIN_LOG, "INFO {:n}", 123456789);
  LOG__INFO(MAIN_LOG, "INFO {:.5n}", double(123456789.12345));

  // Logger test
  LOG__INFO(ENGINE_LOG, "ENGINE LOGGER TESTS:");
  LOG__CRITICAL(ENGINE_LOG, "CRITICAL");
  LOG__ERROR(ENGINE_LOG, "ERROR");
  LOG__WARN(ENGINE_LOG, "WARN");
  LOG__INFO(ENGINE_LOG, "INFO");
  LOG__DEBUG(ENGINE_LOG, "DEBUG");
  LOG__TRACE(ENGINE_LOG, "TRACE");
  LOG__TRACE(ENGINE_LOG, "TRACE {}",  "MARCO");
  LOG__INFO(ENGINE_LOG, "INFO {}", 123456789);
  LOG__INFO(ENGINE_LOG, "INFO {:d}", 123456789);
  LOG__INFO(ENGINE_LOG, "INFO {:n}", 123456789);
  LOG__INFO(ENGINE_LOG, "INFO {:.5n}", double(123456789.12345));

  // Logger test
  LOG__INFO(SEARCH_LOG, "SEARCH LOGGER TESTS:");
  LOG__CRITICAL(SEARCH_LOG, "CRITICAL");
  LOG__ERROR(SEARCH_LOG, "ERROR");
  LOG__WARN(SEARCH_LOG, "WARN");
  LOG__INFO(SEARCH_LOG, "INFO");
  LOG__DEBUG(SEARCH_LOG, "DEBUG");
  LOG__TRACE(SEARCH_LOG, "TRACE");
  LOG__TRACE(SEARCH_LOG, "TRACE {}",  "MARCO");
  LOG__INFO(SEARCH_LOG, "INFO {}", 123456789);
  LOG__INFO(SEARCH_LOG, "INFO {:d}", 123456789);
  LOG__INFO(SEARCH_LOG, "INFO {:n}", 123456789);
  LOG__INFO(SEARCH_LOG, "INFO {:.5n}", double(123456789.12345));

  // Logger test
  LOG__INFO(UCI_LOG, "UCI LOGGER TESTS:");
  LOG__CRITICAL(UCI_LOG, "CRITICAL");
  LOG__ERROR(UCI_LOG, "ERROR");
  LOG__WARN(UCI_LOG, "WARN");
  LOG__INFO(UCI_LOG, "INFO");
  LOG__DEBUG(UCI_LOG, "DEBUG");
  LOG__TRACE(UCI_LOG, "TRACE");
  LOG__TRACE(UCI_LOG, "TRACE {}",  "MARCO");
  LOG__INFO(UCI_LOG, "INFO {}", 123456789);
  LOG__INFO(UCI_LOG, "INFO {:d}", 123456789);
  LOG__INFO(UCI_LOG, "INFO {:n}", 123456789);
  LOG__INFO(UCI_LOG, "INFO {:.5n}", double(123456789.12345));
}
