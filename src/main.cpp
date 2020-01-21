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

#include <thread>
#include "version.h"
#include "types.h"
#include "Logging.h"
//#include "Engine.h"
//#include "UCIHandler.h"

//class Busybody {
//public:
//  Busybody() {
//    Logger::get().MAIN_LOG->info("Busybody CTOR");
//  }
//
//  void loop() {
//    int counter = 0;
//
//    while (counter < 100) {
//      Logger::get().MAIN_LOG->info("Busybody {}", counter++);
//      std::chrono::milliseconds timespan(1000); // or whatever
//      std::this_thread::sleep_for(timespan);
//    }
//  }
//};

int main() {

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

  Logger::get();
  INIT::init();

  //Busybody bb;
  //bb.loop();

  //  Engine engine;
  //  UCI_Handler uci(&engine);
  //  uci.loop();

  return 0;
}