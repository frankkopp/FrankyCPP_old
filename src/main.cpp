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

#include "version.h"
#include "logging.h"
#include "Engine.h"
#include "UCIHandler.h"

int main() {
  try {
    std::locale::global(std::locale("de_DE.UTF-8"));
  }
  catch (...) {
    std::cerr << "failed to set locale" << std::endl;
  }
  setlocale(LC_ALL, "de_DE.UTF-8");
  setlocale(LC_NUMERIC, "de_DE.UTF-8");
  std::cout << std::locale().name() << std::endl;
  std::locale::global(std::locale(""));
  std::cout << std::locale().name() << std::endl;
  std::locale::global(std::locale("C"));
  std::cout << std::locale().name() << std::endl;

  std::cout << "FrankCPP v" << FrankyCPP_VERSION_MAJOR << "." << FrankyCPP_VERSION_MINOR << std::endl;

  // initializes and configures logging - only needed once in main()
  LOGGING::init();

  auto LOG = spdlog::get("Main_Logger");
  
  LOG->info("FrankyCPP STARTED");
  INIT::init();
  Engine engine;
  UCI::Handler uci(&engine);
  uci.loop();
  LOG->info("FrankyCPP ENDED");

  return 0;
}