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

#ifndef FRANKYCPP_LOGGING_H
#define FRANKYCPP_LOGGING_H

#include <iostream>
#include <spdlog/spdlog.h>
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/basic_file_sink.h"

#ifdef NDEBUG
#define ASSERT_START while(0) {
#define ASSERT_END }
#else
#define ASSERT_START while (1) {
#define ASSERT_END break; }
#endif

#define SEARCH_LOG_LEVEL spdlog::level::trace

#define ZERO__LVL 0
#define CRITICAL__LVL 1
#define ERROR__LVL 2
#define WARN__LVL 3
#define INFO__LVL 4
#define DEBUG__LVL 5
#define TRACE__LVL 6

#define LOG__LEVEL ZERO__LVL

#if LOG__LEVEL > 0
#define LOG__CRITICAL(logger, ...) logger->critical(__VA_ARGS__)
#else
#define LOG__CRITICAL(logger, ...) void(0)
#endif

#if LOG__LEVEL > 1
#define LOG__ERROR(logger, ...) logger->error(__VA_ARGS__)
#else
#define LOG__ERROR(logger, ...) void(0)
#endif

#if LOG__LEVEL > 2
#define LOG__WARN(logger, ...) logger->warn(__VA_ARGS__)
#else
#define LOG__WARN(logger, ...) void(0)
#endif

#if LOG__LEVEL > 3
#define LOG__INFO(logger, ...) logger->info(__VA_ARGS__)
#else
#define LOG__INFO(logger, ...) void(0)
#endif

#if LOG__LEVEL > 4
#define LOG__DEBUG(logger, ...) logger->debug(__VA_ARGS__)
#else
#define LOG__DEBUG(logger, ...) void(0)
#endif

#if LOG__LEVEL > 5
#define LOG__TRACE(logger, ...) logger->trace(__VA_ARGS__)
#else
#define LOG__TRACE(logger, ...) void(0)
#endif

/** Singleton class for Logger */
class Logger {
  Logger() {
    std::cout << "Logger Singleton created!" << std::endl;
  };
  ~Logger() = default;

  void init();

  static Logger* instance;

public :
  // disallow copies
  Logger(Logger const &) = delete; // copy
  Logger &operator=(const Logger &) = delete; // copy assignment
  Logger(Logger const &&) = delete; // move
  Logger &operator=(const Logger &&) = delete; // move assignment

public:
  static Logger* get() {
    if (!instance) {
      std::cout << "Creating Logger Singleton..." << std::endl;
      instance = new Logger();
      instance->init();
    }
    return instance;
  }

  const std::string defaultPattern = "[%H:%M:%S:%f] [t:%-10t] [%-17n] [%-8l]: %v";
  const std::shared_ptr<spdlog::sinks::basic_file_sink_mt> sharedFileSink =
    std::make_shared<spdlog::sinks::basic_file_sink_mt>("FrankyCPP.log");
  const std::shared_ptr<spdlog::sinks::stdout_color_sink_mt> uciOutSink
  = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();

  // @formatter:off
  const std::shared_ptr<spdlog::logger> MAIN_LOG    = spdlog::stdout_color_mt("Main_Logger");
  const std::shared_ptr<spdlog::logger> ENGINE_LOG  = spdlog::stdout_color_mt("Engine_Logger");
  const std::shared_ptr<spdlog::logger> SEARCH_LOG  = spdlog::stdout_color_mt("Search_Logger");
  const std::shared_ptr<spdlog::logger> TSUITE_LOG  = spdlog::stdout_color_mt("TSuite_Logger");
  const std::shared_ptr<spdlog::logger> MOVEGEN_LOG = spdlog::stdout_color_mt("MoveGen_Logger");
  const std::shared_ptr<spdlog::logger> EVAL_LOG    = spdlog::stdout_color_mt("Eval_Logger");
  const std::shared_ptr<spdlog::logger> TT_LOG      = spdlog::stdout_color_mt("TT_Logger");
  const std::shared_ptr<spdlog::logger> UCIHAND_LOG = spdlog::stdout_color_mt("UCIHandler_Logger");
  const std::shared_ptr<spdlog::logger> UCI_LOG     = spdlog::basic_logger_mt("UCI_Logger", "FrankyCPP_uci.log");
  const std::shared_ptr<spdlog::logger> TEST_LOG    = spdlog::stdout_color_mt("Test_Logger");
  // @formatter:on
};
#endif //FRANKYCPP_LOGGING_H
