/*
 * MIT License
 *
 * Copyright (c) 2020 Frank Kopp
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
using testing::Eq;

#include <iostream>

#include <boost/timer/timer.hpp>

TEST(BOOST, cpu_timer) {

  uint64_t i = 0;
  const uint64_t iterations = 10;
  const int repetitions = 1'000'000;

  boost::timer::cpu_timer timer;
  timer.stop();

  std::cout << std::endl;
  std::cout << timer.format() << std::endl;

  while (i++ < iterations) {
    for (int j = 0; j < repetitions; ++j) {
      timer.resume();
      if (!((i * j) % 10'000)) i=i;
      timer.stop();
    }
  }

  std::cout << std::endl;
  std::cout << timer.format() << std::endl;

}

//#include <boost/log/core.hpp>
//#include <boost/log/trivial.hpp>
//#include <boost/log/expressions.hpp>
//#include <boost/log/sinks/basic_sink_frontend.hpp>
//#include <boost/log/utility/setup/common_attributes.hpp>
//#include <boost/log/sources/severity_channel_logger.hpp>
//#include <boost/log/sources/record_ostream.hpp>
//
//namespace logging = boost::log;
//namespace src = boost::log::sources;
//namespace sinks = boost::log::sinks;
//namespace keywords = boost::log::keywords;
//
//TEST(SuiteName, Logging) {
//  logging::core::get()->set_filter(logging::trivial::severity >= logging::trivial::trace);
//
//  logging::add_common_attributes();
//
//  using namespace logging::trivial;
//  src::severity_channel_logger<severity_level> lg;
//
//  BOOST_LOG_CHANNEL_SEV(lg, "net", trace) << "A trace severity message";
//  BOOST_LOG_CHANNEL_SEV(lg, "uci", trace) << "A trace severity message";
//  BOOST_LOG_SEV(lg, debug) << "A debug severity message";
//  BOOST_LOG_SEV(lg, info) << "An informational severity message";
//  BOOST_LOG_SEV(lg, warning) << "A warning severity message";
//  BOOST_LOG_SEV(lg, error) << "An error severity message";
//  BOOST_LOG_SEV(lg, fatal) << "A fatal severity message";
//}
