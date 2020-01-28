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

#include <string>
#include <iostream>
#include <fstream>
#include "version.h"
#include "types.h"
#include "Logging.h"
#include "Engine.h"
#include "UCIHandler.h"

#include "boost/program_options.hpp"
namespace po = boost::program_options;

inline po::variables_map programOptions;

int main(int argc, char* argv[]) {

  std::string appName = "FrankCPP";
  appName
    .append(" v")
    .append(std::to_string(FrankyCPP_VERSION_MAJOR))
    .append(".")
    .append(std::to_string(FrankyCPP_VERSION_MINOR));
  std::cout << appName << std::endl;

  ASSERT_START
    std::cout << "DEBUG ASSERTION TESTS ON" << std::endl;
  ASSERT_END

  std::string config_file;

  // Command line options
  try {
    // Declare a group of options that will be allowed only on command line
    po::options_description generic("Generic options");
    generic.add_options()
             ("help,?", "produce help message")
             ("version,v", "print version string")
             ("config,c", po::value<std::string>(&config_file)->default_value("FrankyCPP.cfg"), "name of a file of a configuration.");

    // Declare a group of options that will be  allowed both on command line
    // and in config file
    po::options_description config("Configuration");
    config.add_options()
            ("log_lvl,l", po::value<std::string>()->default_value("warn"), "set log level critical|error|warn|info|debug|trace>")
            ("search_log_lvl,s", po::value<std::string>()->default_value("warn"), "set log level for search <critical|error|warn|info|debug|trace>");

    // Hidden options, will be allowed both on command line and in config file,
    // but will not be shown to the user.
    po::options_description hidden("Hidden options");
    hidden.add_options()
            ("test,t", po::value<std::string>(), "test_hidden");

    po::options_description cmdline_options;
    cmdline_options.add(generic).add(config).add(hidden);

    po::options_description config_file_options;
    config_file_options.add(config).add(hidden);

    po::options_description visible("Allowed options");
    visible.add(generic).add(config);

    po::positional_options_description p;
    p.add("input-file", -1);

    store(po::command_line_parser(argc, argv).options(cmdline_options).positional(p).run(), programOptions);
    notify(programOptions);

    if (programOptions.count("help")) {
      std::cout << visible << "\n";
      return 0;
    }

    if (programOptions.count("version")) {
      std::cout << "Version: " << appName << "\n";
      return 0;
    }

    std::ifstream ifs(config_file.c_str());
    if (!ifs) {
      std::cerr << "could not open config file: " << config_file << "\n";
    }
    else {
      store(parse_config_file(ifs, config_file_options), programOptions);
      notify(programOptions);
    }

    if (programOptions.count("test")) {
      std::cout << programOptions["test"].as<std::string>() << "\n";
    }
  }
  catch (std::exception &e) {
    std::cerr << "error: " << e.what() << "\n";
    return 1;
  }
  catch (...) {
    std::cerr << "Exception of unknown type!\n";
    return 1;
  }

  // Init all pre calculated data structures
  INIT::init();

  // start engine and UCI loop
  Engine engine;
  UCI_Handler uci(&engine);
  uci.loop();

  return 0;
}