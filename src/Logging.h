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

#ifndef FRANKYCPP_LOGGING_H
#define FRANKYCPP_LOGGING_H

#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/basic_file_sink.h"

// uncomment this if tracing macros are needed.
#define SEARCH_LOG_LEVEL spdlog::level::trace

#define CRITICAL__LVL 1
#define ERROR__LVL 2
#define WARN__LVL 3
#define INFO__LVL 4
#define DEBUG__LVL 5
#define TRACE__LVL 6

#define LOG__LEVEL DEBUG__LVL

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

namespace LOGGING {
  extern void init();
}


#endif //FRANKYCPP_LOGGING_H
