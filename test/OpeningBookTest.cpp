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
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
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

#include "types.h"
#include "Logging.h"
#include <gtest/gtest.h>
#include <OpeningBook.h>
using testing::Eq;

class OpeningBookTest : public ::testing::Test {
public:
  static void SetUpTestSuite() {
    NEWLINE;
    INIT::init();
    NEWLINE;
    Logger::get().TEST_LOG->set_level(spdlog::level::debug);
    Logger::get().MAIN_LOG->set_level(spdlog::level::trace);
    Logger::get().BOOK_LOG->set_level(spdlog::level::debug);
  }

protected:
  void SetUp() override {}
  void TearDown() override {}
};

void printEntry(BookEntry* next, int ply);

TEST_F(OpeningBookTest, initSimpleSmall) {
  std::string filePathStr = FrankyCPP_PROJECT_ROOT;
  filePathStr += +"/books/book_smalltest.txt";
  OpeningBook book(filePathStr, OpeningBook::BookFormat::SIMPLE);
  book.initialize();
  LOG__INFO(Logger::get().TEST_LOG, "Entries in book: {:n}", book.size());
  EXPECT_EQ(11'517, book.size());
}

TEST_F(OpeningBookTest, initSimple) {
  std::string filePathStr = FrankyCPP_PROJECT_ROOT;
  filePathStr += +"/books/book.txt";
  OpeningBook book(filePathStr, OpeningBook::BookFormat::SIMPLE);
  book.initialize();
  LOG__INFO(Logger::get().TEST_LOG, "Entries in book: {:n}", book.size());
  EXPECT_EQ(292'568, book.size());
}

TEST_F(OpeningBookTest, initSAN) {
  std::string filePathStr = FrankyCPP_PROJECT_ROOT;
  filePathStr += +"/books/book_graham.txt";
  OpeningBook book(filePathStr, OpeningBook::BookFormat::SAN);
  book.initialize();
  LOG__INFO(Logger::get().TEST_LOG, "Entries in book: {:n}", book.size());
  EXPECT_EQ(1'262, book.size());
}

TEST_F(OpeningBookTest, initPGN) {
  std::string filePathStr = FrankyCPP_PROJECT_ROOT;
  filePathStr += +"/books/pgn_test.pgn";
  OpeningBook book(filePathStr, OpeningBook::BookFormat::PNG);
  book.initialize();
  LOG__INFO(Logger::get().TEST_LOG, "Entries in book: {:n}", book.size());
  EXPECT_EQ(1'162, book.size());
}

TEST_F(OpeningBookTest, initPGNMedium) {
  std::string filePathStr = FrankyCPP_PROJECT_ROOT;
  filePathStr += +"/books/8moves_GM_LB.pgn";
  OpeningBook book(filePathStr, OpeningBook::BookFormat::PNG);
  book.initialize();
  LOG__INFO(Logger::get().TEST_LOG, "Entries in book: {:n}", book.size());
  //  EXPECT_EQ(216'070, book.size());
}

TEST_F(OpeningBookTest, initPGNLarge) {
  std::string filePathStr = FrankyCPP_PROJECT_ROOT;
  filePathStr += +"/books/superbook.pgn";
  OpeningBook book(filePathStr, OpeningBook::BookFormat::PNG);
  book.initialize();
  LOG__INFO(Logger::get().TEST_LOG, "Entries in book: {:n}", book.size());
//  EXPECT_EQ(216'070, book.size());
}


