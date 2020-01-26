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

#include <iostream>
#include "types.h"
#include "Logging.h"

#include "boost/program_options.hpp"
namespace po = boost::program_options;

extern po::variables_map programOptions;

void Logger::init() {
  try {
    std::locale::global(deLocale);
  }
  catch (...) {
    std::cerr << "failed to set locale" << std::endl;
  }

  // default log level
  auto logLvL = programOptions["log_lvl"].as<std::string>();
  const auto logLevel = [&] {
    if (logLvL == "warn") {
      return spdlog::level::warn;
    }
    else if (logLvL == "info") {
      return spdlog::level::info;
    }
    else if (logLvL == "debug") {
      return spdlog::level::debug;
    }
    else {
      std::cerr << "unknown log level '" << logLvL << "' - using default.\n";
      logLvL = "warn";
      return spdlog::level::warn;
    }
  }();

  const auto flushLevel = spdlog::level::warn;

  // global log level
  spdlog::set_level(logLevel);

  // default pattern
  spdlog::set_pattern(defaultPattern);

  // Shared file sink
  sharedFileSink->set_level(logLevel);

  // Main Logger
  MAIN_LOG->sinks().push_back(sharedFileSink);
  MAIN_LOG->set_pattern(defaultPattern);
  MAIN_LOG->set_level(logLevel);
  MAIN_LOG->flush_on(flushLevel);

  ENGINE_LOG->sinks().push_back(sharedFileSink);
  ENGINE_LOG->set_pattern(defaultPattern);
  ENGINE_LOG->set_level(logLevel);
  ENGINE_LOG->flush_on(flushLevel);

  SEARCH_LOG->sinks().push_back(sharedFileSink);
  SEARCH_LOG->set_pattern(defaultPattern);
  SEARCH_LOG->set_level(SEARCH_LOG_LEVEL);
  SEARCH_LOG->flush_on(flushLevel);

  TSUITE_LOG->sinks().push_back(sharedFileSink);
  TSUITE_LOG->set_pattern(defaultPattern);
  TSUITE_LOG->set_level(logLevel);
  TSUITE_LOG->flush_on(flushLevel);

  MOVEGEN_LOG->sinks().push_back(sharedFileSink);
  MOVEGEN_LOG->set_pattern(defaultPattern);
  MOVEGEN_LOG->set_level(logLevel);
  MOVEGEN_LOG->flush_on(flushLevel);

  EVAL_LOG->sinks().push_back(sharedFileSink);
  EVAL_LOG->set_pattern(defaultPattern);
  EVAL_LOG->set_level(logLevel);
  EVAL_LOG->flush_on(flushLevel);

  TT_LOG->sinks().push_back(sharedFileSink);
  TT_LOG->set_pattern(defaultPattern);
  TT_LOG->set_level(logLevel);
  TT_LOG->flush_on(flushLevel);

  UCIHAND_LOG->sinks().push_back(sharedFileSink);
  UCIHAND_LOG->set_pattern(defaultPattern);
  UCIHAND_LOG->set_level(logLevel);
  UCIHAND_LOG->flush_on(flushLevel);

  UCI_LOG->sinks().push_back(uciOutSink);
  UCI_LOG->set_pattern("[%H:%M:%S:%f] %L %v");
  UCI_LOG->set_level(logLevel);
  UCI_LOG->flush_on(flushLevel);

  // Logger for Unit Tests
  TEST_LOG->set_pattern(defaultPattern);
  TEST_LOG->set_level(logLevel);
  TEST_LOG->flush_on(flushLevel);

  std::cout << "Logger initialized (" << logLvL << ")" << std::endl;
}


