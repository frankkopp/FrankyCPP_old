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

#include <algorithm>
#include <iostream>
#include <fstream>
#include <chrono>
#include <regex>
#include <mutex>
#include <execution>
#include "types.h"
#include "Logging.h"
#include "misc.h"
#include "OpeningBook.h"
#include "Position.h"
#include "MoveGenerator.h"

#define MULTITHREADED

OpeningBook::OpeningBook(std::string bookPath, BookFormat bFormat)
  : bookFilePath(bookPath), bookFormat(bFormat) {
  mg = std::make_shared<MoveGenerator>();
}

void OpeningBook::initialize() {
  if (isInitialized) return;
  LOG__DEBUG(Logger::get().BOOK_LOG, "Opening book initialization.");

  const auto start = std::chrono::high_resolution_clock::now();

  // set root entry
  bookMap.insert(std::make_pair(START_POSITION_FEN, BookEntry(START_POSITION_FEN)));

  // read book from file
  readBookFromFile(bookFilePath);

  const auto stop = std::chrono::high_resolution_clock::now();
  const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
  LOG__DEBUG(Logger::get().BOOK_LOG, "Opening book initialized ({:n} ms)", elapsed.count());
  isInitialized = true;
}

void OpeningBook::readBookFromFile(const std::string filePath) {
  std::ifstream file(filePath);
  if (file.is_open()) {
    LOG__DEBUG(Logger::get().BOOK_LOG, "Open book '{}' successful.", filePath);
    processAllLines(file);
    file.close();
  }
  else {
    LOG__ERROR(Logger::get().BOOK_LOG, "Open book '{}' failed.", filePath);
    return;
  }
}

void OpeningBook::processAllLines(std::ifstream &ifstream) {
  LOG__DEBUG(Logger::get().BOOK_LOG, "Processing file.");
  std::vector<std::string> lines = getLinesFromFile(ifstream);

  const auto start = std::chrono::high_resolution_clock::now();
  LOG__DEBUG(Logger::get().BOOK_LOG, "Creating internal book...");

#ifdef MULTITHREADED
  std::for_each(
    std::execution::par_unseq,
    lines.begin(),
    lines.end(),
    [&](auto&& item) {
      processLine(item);
    });
#else
  for (auto line : lines) {
    processLine(line);
  }
#endif

  const auto stop = std::chrono::high_resolution_clock::now();
  const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
  LOG__DEBUG(Logger::get().BOOK_LOG, "Internal book created {:n} positions in {:n} ms.", bookMap.size(), elapsed.count());
}

std::vector<std::string> OpeningBook::getLinesFromFile(std::ifstream &ifstream) {
  LOG__DEBUG(Logger::get().BOOK_LOG, "Reading lines from book.");
  const auto start = std::chrono::high_resolution_clock::now();
  std::vector<std::string> lines;
  std::string line;
  while (std::getline(ifstream, line)) {
    if (!line.empty()) lines.push_back(line);
  }
  const auto stop = std::chrono::high_resolution_clock::now();
  const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
  LOG__DEBUG(Logger::get().BOOK_LOG, "Read {:n} lines in {:n} ms.", lines.size(), elapsed.count());
  return lines;
}

void OpeningBook::processLine(std::string &line) {
  LOG__TRACE(Logger::get().BOOK_LOG, "Processing line: {}", line);
  switch (bookFormat) {
    case BookFormat::SIMPLE:
      processSimpleLine(line);
      break;
    case BookFormat::SAN:
      LOG__ERROR(Logger::get().BOOK_LOG, "SAN format not yet implemented.");
      break;
    case BookFormat::PNG:
      LOG__ERROR(Logger::get().BOOK_LOG, "PNG format not yet implemented.");
      break;
  }
}

void OpeningBook::processSimpleLine(std::string &line) {
  // clean up line
  std::regex whiteSpaceTrim(R"(^\s*(.*)\s*$)");
  line = std::regex_replace(line, whiteSpaceTrim, "$1");

  std::smatch matcher;

  // check if line starts with move
  std::regex startPattern(R"(^[a-h][1-8][a-h][1-8].*$)");
  if (!std::regex_match(line, matcher, startPattern)) {
    LOG__TRACE(Logger::get().BOOK_LOG, "Line ignored: {}", line);
    return;
  }

  // iterate over all found pattern matches (aka moves)
  std::regex movePattern(R"([a-h][1-8][a-h][1-8])");
  auto move_begin = std::sregex_iterator(line.begin(), line.end(), movePattern);
  auto move_end = std::sregex_iterator();
  LOG__TRACE(Logger::get().BOOK_LOG, "Found {} moves in line: {}", std::distance(move_begin, move_end), line);
  Position currentPosition; // start position

  for (std::sregex_iterator i = move_begin; i != move_end; ++i) {
    const std::string &moveStr = (*i).str();
    LOG__TRACE(Logger::get().BOOK_LOG, "Moves {}", moveStr);

    // create and validate the move
    Move move = Misc::getMoveFromUCI(currentPosition, moveStr);
    if (!isMove(move)) {
      LOG__WARN(Logger::get().BOOK_LOG, "Not a valid move {} on this position {}", moveStr, currentPosition.printFen());
      return;
    }

    // remember the last position as fen
    const std::string lastFen = currentPosition.printFen();

    // make move on position
    currentPosition.doMove(move);
    const std::string fen = currentPosition.printFen();

    // adding entry to book
    addToBook(move, lastFen, fen);

  }
}
void OpeningBook::addToBook(const Move &move, const std::string &lastFen, const std::string &fen) {
#ifdef MULTITHREADED
  const std::lock_guard<std::mutex> lock(synchMutex);
#endif
  
  if (bookMap.count(fen)) {
    // pointer to entry already in book
    BookEntry* existingEntry = &bookMap.at(fen);
    existingEntry->counter++;
    LOG__TRACE(Logger::get().BOOK_LOG, "Position already existed {} times: {}", existingEntry->counter, existingEntry->position);
  }
  else {
    // new position
    BookEntry newEntry(fen);
    bookMap.insert(std::make_pair(fen, newEntry));
    LOG__TRACE(Logger::get().BOOK_LOG, "Position new {} ", newEntry.position);
  }

  // add move to the last book entry's move list
  BookEntry* lastEntry = &bookMap.at(lastFen);
  if (std::find(lastEntry->moves.begin(), lastEntry->moves.end(), move) == lastEntry->moves.end()) {
    lastEntry->moves.push_back(move);
    lastEntry->ptrNextPosition.push_back(&bookMap.at(fen));
    LOG__TRACE(Logger::get().BOOK_LOG, "Added to last entry.");
  }
}

std::string BookEntry::str() {
  std::ostringstream os;
  os << this->position << " (" << this->counter << ") ";
  for (int i = 0; i < moves.size(); i++) {
    os << "[" << printMove(this->moves[i]) << " (" << this->ptrNextPosition[i]->counter << ")] ";
  }
  return os.str();
}

#undef MULTITHREADED
