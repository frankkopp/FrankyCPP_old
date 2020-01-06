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

TEST_F(LoggingTest, decimal) {
  auto s = fmt::format(deLocale, "{:n}", 1234567890);
  std::cout << "Direct cout  : " << s << std::endl;
  fprintln("Macro with ln: {:n}", 1234567890);
  TEST_LOG->info("INFO {:n}", 1234567890);
  ASSERT_EQ("1.234.567.890", s);
  s = fmt::format("{:n}", 1234567890);
  std::cout << "Direct cout  : " << s << std::endl;
  TEST_LOG->info("INFO {:n}", 1234567890);
  ASSERT_EQ("1.234.567.890", s);

  s = fmt::format("{:n}", std::numeric_limits<__int128_t>::max());
  std::cout << "Direct cout  : " << s << std::endl;
  TEST_LOG->info("INFO {:n}", std::numeric_limits<__int128_t>::max());
  ASSERT_EQ("170.141.183.460.469.231.731.687.303.715.884.105.727", s);
}

TEST_F(LoggingTest, basic) {
  // Logger test
  TEST_LOG->info("TEST LOGGER:");
  TEST_LOG->critical("CRITICAL");
  TEST_LOG->error("ERROR");
  TEST_LOG->warn("WARN");
  TEST_LOG->info("INFO");
  TEST_LOG->debug("DEBUG");
  SPDLOG_DEBUG(TEST_LOG, "DEBUG {}", "MARCO");
  TEST_LOG->trace("TRACE");
  TRACE(TEST_LOG, "TRACE {}", "MARCO");
  TEST_LOG->info("INFO {}", 123456789);
  TEST_LOG->info("INFO {:d}", 123456789);
  TEST_LOG->info("INFO {:n}", 123456789);
  TEST_LOG->info("INFO {:.5n}", double(123456789.12345));

  // Logger test
  MAIN_LOG->info("MAIN LOGGER TESTS:");
  MAIN_LOG->critical("CRITICAL");
  MAIN_LOG->error("ERROR");
  MAIN_LOG->warn("WARN");
  MAIN_LOG->info("INFO");
  MAIN_LOG->debug("DEBUG");
  SPDLOG_DEBUG(MAIN_LOG, "DEBUG {}", "MARCO");
  MAIN_LOG->trace("TRACE");
  TRACE(MAIN_LOG, "TRACE {}", "MARCO");
  MAIN_LOG->info("INFO {}", 123456789);
  MAIN_LOG->info("INFO {:d}", 123456789);
  MAIN_LOG->info("INFO {:n}", 123456789);
  MAIN_LOG->info("INFO {:.5n}", double(123456789.12345));

  // Logger test
  ENGINE_LOG->info("ENGINE LOGGER TESTS:");
  ENGINE_LOG->critical("CRITICAL");
  ENGINE_LOG->error("ERROR");
  ENGINE_LOG->warn("WARN");
  ENGINE_LOG->info("INFO");
  ENGINE_LOG->debug("DEBUG");
  SPDLOG_DEBUG(ENGINE_LOG, "DEBUG {}", "MARCO");
  ENGINE_LOG->trace("TRACE");
  TRACE(ENGINE_LOG, "TRACE {}", "MARCO");
  ENGINE_LOG->info("INFO {}", 123456789);
  ENGINE_LOG->info("INFO {:d}", 123456789);
  ENGINE_LOG->info("INFO {:n}", 123456789);
  ENGINE_LOG->info("INFO {:.5n}", double(123456789.12345));

  // Logger test
  SEARCH_LOG->info("SEARCH LOGGER TESTS:");
  SEARCH_LOG->critical("CRITICAL");
  SEARCH_LOG->error("ERROR");
  SEARCH_LOG->warn("WARN");
  SEARCH_LOG->info("INFO");
  SEARCH_LOG->debug("DEBUG");
  SPDLOG_DEBUG(SEARCH_LOG, "DEBUG {}", "MARCO");
  SEARCH_LOG->trace("TRACE");
  TRACE(SEARCH_LOG, "TRACE {}", "MARCO");
  SEARCH_LOG->info("INFO {}", 123456789);
  SEARCH_LOG->info("INFO {:d}", 123456789);
  SEARCH_LOG->info("INFO {:n}", 123456789);
  SEARCH_LOG->info("INFO {:.5n}", double(123456789.12345));

  // Logger test
  UCI_LOG->info("UCI LOGGER TESTS:");
  UCI_LOG->critical("CRITICAL");
  UCI_LOG->error("ERROR");
  UCI_LOG->warn("WARN");
  UCI_LOG->info("INFO");
  UCI_LOG->debug("DEBUG");
  SPDLOG_DEBUG(UCI_LOG, "DEBUG {}", "MARCO");
  UCI_LOG->trace("TRACE");
  TRACE(UCI_LOG, "TRACE {}", "MARCO");
  UCI_LOG->info("INFO {}", 123456789);
  UCI_LOG->info("INFO {:d}", 123456789);
  UCI_LOG->info("INFO {:n}", 123456789);
  UCI_LOG->info("INFO {:.5n}", double(123456789.12345));
}
