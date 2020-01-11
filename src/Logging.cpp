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

#include <iostream>
#include "types.h"
#include "Logging.h"

namespace LOGGING {
  void init() {
    try {
      std::locale::global(deLocale);
    }
    catch (...) {
      std::cerr << "failed to set locale" << std::endl;
    }

    std::string defaultPattern = fmt::format("[%H:%M:%S:%f] [t:%t] [%-14n] [%-8l]: %v");
    spdlog::set_pattern(defaultPattern);

    // Shared file sink
    auto sharedFileSink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("FrankyCPP.log");
    sharedFileSink->set_level(spdlog::level::trace);

    // Main Logger
    auto MAIN_LOG = spdlog::stdout_color_mt("Main_Logger");
    MAIN_LOG->sinks().push_back(sharedFileSink);
    MAIN_LOG->set_pattern(defaultPattern);
    MAIN_LOG->set_level(spdlog::level::trace);
    MAIN_LOG->flush_on(spdlog::level::trace);

    auto ENGINE_LOG = spdlog::stdout_color_mt("Engine_Logger");
    ENGINE_LOG->sinks().push_back(sharedFileSink);
    ENGINE_LOG->set_pattern(defaultPattern);
    ENGINE_LOG->set_level(spdlog::level::trace);
    ENGINE_LOG->flush_on(spdlog::level::trace);

    auto SEARCH_LOG = spdlog::stdout_color_mt("Search_Logger");
    SEARCH_LOG->sinks().push_back(sharedFileSink);
    SEARCH_LOG->set_pattern(defaultPattern);
    SEARCH_LOG->set_level(SEARCH_LOG_LEVEL);
    SEARCH_LOG->flush_on(spdlog::level::trace);

    auto MOVEGEN_LOG = spdlog::stdout_color_mt("MoveGen_Logger");
    MOVEGEN_LOG->sinks().push_back(sharedFileSink);
    MOVEGEN_LOG->set_pattern(defaultPattern);
    MOVEGEN_LOG->set_level(spdlog::level::trace);
    MOVEGEN_LOG->flush_on(spdlog::level::trace);

    auto EVAL_LOG = spdlog::stdout_color_mt("Eval_Logger");
    EVAL_LOG->sinks().push_back(sharedFileSink);
    EVAL_LOG->set_pattern(defaultPattern);
    EVAL_LOG->set_level(spdlog::level::trace);
    EVAL_LOG->flush_on(spdlog::level::trace);

    auto TT_LOG = spdlog::stdout_color_mt("TT_Logger");
    TT_LOG->sinks().push_back(sharedFileSink);
    TT_LOG->set_pattern(defaultPattern);
    TT_LOG->set_level(spdlog::level::trace);
    TT_LOG->flush_on(spdlog::level::trace);

    auto UCIHANDLER_LOG = spdlog::stdout_color_mt("UCIHandler_Logger");
    UCIHANDLER_LOG->sinks().push_back(sharedFileSink);
    UCIHANDLER_LOG->set_pattern(defaultPattern);
    UCIHANDLER_LOG->set_level(spdlog::level::trace);
    UCIHANDLER_LOG->flush_on(spdlog::level::trace);

    auto uciOutSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    auto UCI_LOG = spdlog::basic_logger_mt("UCI_Logger", "FrankyCPP_uci.log");
    UCI_LOG->sinks().push_back(uciOutSink);
    UCI_LOG->set_pattern("[%H:%M:%S:%f] %L %v");
    UCI_LOG->set_level(spdlog::level::trace);
    UCI_LOG->flush_on(spdlog::level::trace);

    // Logger for Unit Tests
    auto TEST_LOG = spdlog::stdout_color_mt("Test_Logger");
    TEST_LOG->set_pattern(defaultPattern);
    TEST_LOG->set_level(spdlog::level::trace);
    TEST_LOG->flush_on(spdlog::level::trace);

    LOG__INFO(MAIN_LOG, "Logging initialized.");
    LOG__INFO(MAIN_LOG, "Locale is set to: {}", std::locale().name());

  }
}
