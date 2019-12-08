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

#include "logging.h"

namespace LOGGING {
  void init() {

    // test if already initialized
    if (spdlog::get("Main_Logger")) return;
    
    // Logger setup
    auto LOG = spdlog::stdout_color_mt("Main_Logger");
    LOG->set_level(spdlog::level::trace);

    auto UCI_LOG = spdlog::stdout_color_mt("UCI_Logger");
    UCI_LOG->set_level(spdlog::level::trace);

    auto ENGINE_LOG = spdlog::stdout_color_mt("Engine_Logger");
    ENGINE_LOG->set_level(spdlog::level::trace);

    auto SEARCH_LOG = spdlog::stdout_color_mt("Search_Logger");
    SEARCH_LOG->set_level(spdlog::level::trace);
    SEARCH_LOG->set_pattern("[%H:%M:%S:%f] [%n] [%l] [thread %t] %v");

    LOG->info("Logging initialized.");
    
    // Logger test
//    LOG->critical("CRITICAL");
//    LOG->error("ERROR");
//    LOG->warn("WARN");
//    LOG->info("INFO");
//    LOG->debug("DEBUG");
//    SPDLOG_DEBUG(LOG, "DEBUG {}", "MARCO");
//    LOG->trace("TRACE");
//    SPDLOG_TRACE(LOG, "TRACE {}", "MARCO");
  }
}
