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
#include <chrono>
#include <iostream>
#include <fstream>
#include <mutex>
#include <regex>
#include <thread>
#include <random>
#include "OpeningBook.h"
#include "types.h"
#include "Logging.h"
#include "misc.h"
#include "Fifo.h"
#include "ThreadPool.h"
#include "Position.h"
#include "PGN_Reader.h"

#include <boost/thread/thread_functors.hpp>

// BOOST Serialization
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

#define PARALLEL_LINE_PROCESSING
#define PARALLEL_GAME_PROCESSING
#define FIFO_PROCESSING

#ifdef HAS_EXECUTION_LIB
#include <execution>
#endif

#ifdef HAS_FILESYSTEM_LIB
#include <filesystem>
#endif

OpeningBook::OpeningBook(const std::string &bookPath, const BookFormat &bFormat)
  : bookFilePath(bookPath), bookFormat(bFormat) {
}

void OpeningBook::initialize() {
  if (isInitialized) return;
  LOG__INFO(Logger::get().BOOK_LOG, "Opening book initialization.");

  const auto start = std::chrono::high_resolution_clock::now();

  // set root entry
  Position position;
  bookMap.emplace(position.getZobristKey(),
                  BookEntry(position.getZobristKey(), position.printFen()));

  // read book from file
  readBookFromFile(bookFilePath);

  const auto stop = std::chrono::high_resolution_clock::now();
  const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
  LOG__INFO(Logger::get().BOOK_LOG, "Opening book initialized in ({:n} ms). {:n} positions", elapsed.count(), bookMap.size());
  isInitialized = true;
}

void OpeningBook::readBookFromFile(const std::string &filePath) {

  // get file size
#ifdef HAS_FILESYSTEM_LIB
  std::filesystem::path p{filePath};
  fileSize = std::filesystem::file_size(std::filesystem::canonical(p));
#else
  std::ifstream in(filePath, std::ifstream::ate | std::ifstream::binary);
  fileSize = in.tellg();
#endif

  std::ifstream file(filePath);
  if (file.is_open()) {
    LOG__DEBUG(Logger::get().BOOK_LOG, "Open book '{}' with {:n} kB successful.", filePath, fileSize / 1024);
    std::vector<std::string> lines = getLinesFromFile(file);
    file.close();
    processAllLines(lines);
  }
  else {
    LOG__ERROR(Logger::get().BOOK_LOG, "Open book '{}' failed.", filePath);
    return;
  }
}

void OpeningBook::processAllLines(std::vector<std::string> &lines) {
  LOG__DEBUG(Logger::get().BOOK_LOG, "Processing file.");

  const auto start = std::chrono::high_resolution_clock::now();
  LOG__DEBUG(Logger::get().BOOK_LOG, "Creating internal book...");

  switch (bookFormat) {
    case BookFormat::SIMPLE:
    case BookFormat::SAN: {
#ifdef PARALLEL_LINE_PROCESSING
#ifdef HAS_EXECUTION_LIB
      std::for_each(std::execution::par_unseq, lines.begin(), lines.end(),
                    [&](auto &&item) { processLine(item); });
#else
      const auto noOfThreads = std::thread::hardware_concurrency() == 0 ?
                               4 : std::thread::hardware_concurrency();
      const auto maxNumberOfEntries = lines.size();
      std::vector<std::thread> threads;
      threads.reserve(noOfThreads);
      for (unsigned int t = 0; t < noOfThreads; ++t) {
        threads.emplace_back([&, this, t]() {
          auto range = maxNumberOfEntries / noOfThreads;
          auto start = t * range;
          auto end = start + range;
          if (t == noOfThreads - 1) end = maxNumberOfEntries;
          for (std::size_t i = start; i < end; ++i) {
            processLine(lines[i]);
          }
        });
      }
      for (std::thread &th: threads) th.join();
#endif
#else // no parallel execution
      for (auto l : lines) {
        processLine(l);
      }
#endif
      break;
    }
    case BookFormat::PGN:
#ifdef FIFO_PROCESSING
      processPGNFileFifo(lines);
#else
      processPGNFile(lines);
#endif
      break;
  }

  const auto stop = std::chrono::high_resolution_clock::now();
  const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
  LOG__DEBUG(Logger::get().BOOK_LOG, "Internal book created {:n} positions in {:n} ms.", bookMap.size(), elapsed.count());
}

std::vector<std::string> OpeningBook::getLinesFromFile(std::ifstream &ifstream) {
  LOG__DEBUG(Logger::get().BOOK_LOG, "Reading lines from book.");
  const auto start = std::chrono::high_resolution_clock::now();
  std::vector<std::string> lines;
  lines.reserve(fileSize / 40);
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

  // clean up line
  const std::regex whiteSpaceTrim(R"(^\s*(.*)\s*$)");
  line = std::regex_replace(line, whiteSpaceTrim, "$1");

  switch (bookFormat) {
    case BookFormat::SIMPLE:
      processSimpleLine(line);
      break;
    case BookFormat::SAN:
      processSANLine(line);
      break;
    case BookFormat::PGN:
      LOG__ERROR(Logger::get().BOOK_LOG, "PNG format can't be processed by line");
      break;
  }
}

void OpeningBook::processSimpleLine(std::string &line) {
  std::smatch matcher;

  // check if line starts with move
  const std::regex startPattern(R"(^[a-h][1-8][a-h][1-8].*$)");
  if (!std::regex_match(line, matcher, startPattern)) {
    LOG__TRACE(Logger::get().BOOK_LOG, "Line ignored: {}", line);
    return;
  }

  // iterate over all found pattern matches (aka moves)
  const std::regex movePattern(R"([a-h][1-8][a-h][1-8])");
  auto move_begin = std::sregex_iterator(line.begin(), line.end(), movePattern);
  auto move_end = std::sregex_iterator();
  LOG__TRACE(Logger::get().BOOK_LOG, "Found {} moves in line: {}", std::distance(move_begin, move_end), line);

  Position currentPosition; // start position
  for (auto i = move_begin; i != move_end; ++i) {
    const std::string &moveStr = (*i).str();
    LOG__TRACE(Logger::get().BOOK_LOG, "Moves {}", moveStr);

    // create and validate the move
    Move move = Misc::getMoveFromUCI(currentPosition, moveStr);
    if (!isMove(move)) {
      LOG__WARN(Logger::get().BOOK_LOG, "Not a valid move {} on this position {}", moveStr, currentPosition.printFen());
      return;
    }

    addToBook(currentPosition, move);
  }
}

void OpeningBook::processSANLine(std::string &line) {
  std::smatch matcher;

  // check if line starts valid
  const std::regex startLineRegex(R"(^\d+\. )");
  if (std::regex_match(line, matcher, startLineRegex)) {
    LOG__TRACE(Logger::get().BOOK_LOG, "Line ignored: {}", line);
    return;
  }

  // ignore patterns
  const std::regex numberRegex(R"(^\d+\.)");
  const std::regex resultRegex(R"((1/2|1|0)-(1/2|1|0))");

  //1. f4 d5 2. Nf3 Nf6 3. e3 g6 4. b3 Bg7 5. Bb2 O-O 6. Be2 c5 7. O-O Nc6 8. Ne5 Qc7 1/2-1/2
  //1. f4 d5 2. Nf3 Nf6 3. e3 Bg4 4. Be2 e6 5. O-O Bd6 6. b3 O-O 7. Bb2 c5 1/2-1/2
  // split at every whitespace and iterate through items
  const std::regex splitRegex(R"(\s+)");
  const std::sregex_token_iterator iter(line.begin(), line.end(), splitRegex, -1);
  const std::sregex_token_iterator end;

  Position currentPosition; // start position
  LOG__TRACE(Logger::get().BOOK_LOG, "Found {} items in line: {}", std::distance(iter, end), line);
  for (auto i = iter; i != end; ++i) {
    const std::string &moveStr = (*i).str();
    LOG__TRACE(Logger::get().BOOK_LOG, "Item {}", moveStr);
    if (std::regex_match(moveStr, matcher, numberRegex)) { continue; }
    if (std::regex_match(moveStr, matcher, resultRegex)) { continue; }
    LOG__TRACE(Logger::get().BOOK_LOG, "SAN Move {}", moveStr);

    // create and validate the move
    Move move = Misc::getMoveFromSAN(currentPosition, moveStr);
    if (move == MOVE_NONE) {
      LOG__WARN(Logger::get().BOOK_LOG, "Not a valid move {} on this position {}", moveStr, currentPosition.printFen());
      return;
    }
    LOG__TRACE(Logger::get().BOOK_LOG, "Move found {}", printMoveVerbose(move));

    addToBook(currentPosition, move);
  }
}

void OpeningBook::processPGNFileFifo(std::vector<std::string> &lines) {
  LOG__DEBUG(Logger::get().BOOK_LOG, "Process lines from PGN file with FIFO...");
  // reading pgn and get a list of games
  PGN_Reader pgnReader(lines);
  // prepare FIFO for storing the games
  Fifo<PGN_Game> gamesFifo;
  // prepare thread pool
  const auto numberOfThreads = std::thread::hardware_concurrency() == 0 ?
                               4 : std::thread::hardware_concurrency();
  ThreadPool threadPool(numberOfThreads);
  bool finished = false;
  // prepare worker for processing found games
  for (unsigned int i = 0; i < numberOfThreads; i++) {
    threadPool.enqueue([&] {
      while (!gamesFifo.isClosed()) {
        LOG__TRACE(Logger::get().BOOK_LOG, "Get game...");
        auto game = gamesFifo.pop_wait();
        if (game.has_value()) { // no value means pop_wait has been canceled
          LOG__TRACE(Logger::get().BOOK_LOG, "Got game...");
          processGame(game.value());
          LOG__TRACE(Logger::get().BOOK_LOG, "Processed game...Book now at {:n} entries.", bookMap.size());
        }
        else {
          LOG__TRACE(Logger::get().BOOK_LOG, "Game NULL");
        }
      }
    });
  }

  // start finding games and putting them into the FIFO
  std::future<bool> future = std::async(std::launch::async, [&] {
    LOG__DEBUG(Logger::get().BOOK_LOG, "Start finding games");
    finished = pgnReader.process(gamesFifo);
    LOG__DEBUG(Logger::get().BOOK_LOG, "Finished finding games {}", finished);
    return finished;
  });

  // wait until all games have been put into the FIFO
  if (future.get()) {
    // busy wait until FIFO is empty
    while (!gamesFifo.empty()) {
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    LOG__DEBUG(Logger::get().BOOK_LOG, "Finished processing games.");
    gamesFifo.close();
    LOG__DEBUG(Logger::get().BOOK_LOG, "Closed down ThreadPool");
  }

}

void OpeningBook::processPGNFile(std::vector<std::string> &lines) {
  LOG__DEBUG(Logger::get().BOOK_LOG, "Process lines from PGN file...");

  // reading pgn and get a list of games
  PGN_Reader pgnReader(lines);
  if (!pgnReader.process()) {
    LOG__ERROR(Logger::get().BOOK_LOG, "Could not process lines from PGN file.");
    return;
  }
  auto ptrGames = &pgnReader.getGames();
  gamesTotal = ptrGames->size();

  // process all games
  processGames(ptrGames);
}

void OpeningBook::processGames(std::vector<PGN_Game>* ptrGames) {// processing games
  LOG__DEBUG(Logger::get().BOOK_LOG, "Processing {:n} games", ptrGames->size());
  const auto startTime = std::chrono::high_resolution_clock::now();

#ifdef PARALLEL_GAME_PROCESSING
#ifdef HAS_EXECUTION_LIB
  std::for_each(std::execution::par_unseq, ptrGames->begin(), ptrGames->end(),
                [&](auto game) { processGame(game); });
#else
  const auto noOfThreads = std::thread::hardware_concurrency() == 0 ?
                           4 : std::thread::hardware_concurrency();
  const unsigned int maxNumberOfEntries = ptrGames->size();
  std::vector<std::thread> threads;
  threads.reserve(noOfThreads);
  for (unsigned int t = 0; t < noOfThreads; ++t) {
    threads.emplace_back([&, this, t]() {
      auto range = maxNumberOfEntries / noOfThreads;
      auto start = t * range;
      auto end = start + range;
      if (t == noOfThreads - 1) end = maxNumberOfEntries;
      for (std::size_t i = start; i < end; ++i) {
        processGame((*ptrGames)[i]);
      }
    });
  }
  for (std::thread &th: threads) th.join();
#endif
#else
  auto iterEnd = ptrGames->end();
  for (auto iter = ptrGames->begin(); iter < iterEnd; iter++) {
    processGame(*iter);
    LOG__TRACE(Logger::get().BOOK_LOG, "Process game finished - games={:n} book={:n}", gamesProcessed, bookMap.size());
  }
#endif

  const auto stopTime = std::chrono::high_resolution_clock::now();
  const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(stopTime - startTime);
  LOG__INFO(Logger::get().BOOK_LOG, "Processed {:n} games in {:n} ms", gamesTotal, elapsed.count());
}


void OpeningBook::processGame(PGN_Game &game) {
  std::smatch matcher;
  const std::regex UCIRegex(R"(([a-h][1-8][a-h][1-8])([NBRQnbrq])?)");
  const std::regex SANRegex(R"(([NBRQK])?([a-h])?([1-8])?x?([a-h][1-8]|O-O-O|O-O)(=?([NBRQ]))?([!?+#]*)?)");

  Position currentPosition; // start position
  for (auto moveStr : game.moves) {
    Move move = MOVE_NONE;

    // check the notation format
    // Per PGN it must be SAN but some files have UCI notation
    // As UCI is pattern wise a subset of SAN we test for UCI first.  
    if (std::regex_match(moveStr, matcher, UCIRegex)) {
      //      LOG__DEBUG(Logger::get().BOOK_LOG, "Game move {} is UCI", moveStr);
      move = Misc::getMoveFromUCI(currentPosition, moveStr);
    }
    else if (std::regex_match(moveStr, matcher, SANRegex)) {
      //      LOG__DEBUG(Logger::get().BOOK_LOG, "Game move {} is SAN", moveStr);
      move = Misc::getMoveFromSAN(currentPosition, moveStr);
    }

    // create and validate the move
    if (move == MOVE_NONE) {
      LOG__WARN(Logger::get().BOOK_LOG, "Not a valid move {} on this position {}", moveStr, currentPosition.printFen());
      //move = Misc::getMoveFromUCIDEBUG(currentPosition, moveStr);
      return;
    }
    LOG__TRACE(Logger::get().BOOK_LOG, "Move found {}", printMoveVerbose(move));

    addToBook(currentPosition, move);

  }
  gamesProcessed++;
#ifndef FIFO_PROCESSING
  // x % 0 is undefined in c++
  // avgLinesPerGameTimesProgressSteps = 12*15 as 12 is avg game lines and 15 steps
  const uint64_t progressInterval = 1 + (gamesTotal / 15);
  if (gamesProcessed % progressInterval == 0) {
    LOG__DEBUG(Logger::get().BOOK_LOG, "Process games: {:s}", Misc::printProgress(static_cast<double>(gamesProcessed) / gamesTotal));
  }
#endif
}

void OpeningBook::addToBook(Position &currentPosition, const Move &move) {
  // remember previous position
  const Key lastKey = currentPosition.getZobristKey();
  // make move on position to get new position
  currentPosition.doMove(move);
  const Key currentKey = currentPosition.getZobristKey();
  const std::string currentFen = currentPosition.printFen();

  // get the lock on the data map
  const std::scoped_lock<std::mutex> lock(bookMutex);

  if (bookMap.count(currentKey)) {
    // pointer to entry already in book
    BookEntry* existingEntry = &bookMap.at(currentKey);
    existingEntry->counter++;
    LOG__TRACE(Logger::get().BOOK_LOG, "Position already existed {} times: {}", existingEntry->counter, existingEntry->fen);
  }
  else {
    // new position
    bookMap.emplace(currentKey, BookEntry(currentKey, currentFen));
    LOG__TRACE(Logger::get().BOOK_LOG, "Position new", currentKey);
  }

  // add move to the last book entry's move list
  BookEntry* lastEntry = &bookMap.at(lastKey);
  if (std::find(lastEntry->moves.begin(), lastEntry->moves.end(), move) == lastEntry->moves.end()) {
    lastEntry->moves.emplace_back(move);
    lastEntry->ptrNextPosition.emplace_back(std::make_shared<BookEntry>(bookMap.at(currentKey)));
    LOG__TRACE(Logger::get().BOOK_LOG, "Added to last entry.");
  }

}

Move OpeningBook::getRandomMove(Key zobrist) {
  Move bookMove = MOVE_NONE;
  if (bookMap.find(zobrist) != bookMap.end()) {
    BookEntry bookEntry = bookMap.at(zobrist);
    if (!bookEntry.moves.empty()) {
      std::random_device rd;
      std::uniform_int_distribution<std::size_t> random(0, bookEntry.moves.size() - 1);
      bookMove = bookEntry.moves[random(rd)];
      //    for (int i = 0; i < bookEntry.moves.size(); i++) {
      //      fprintln("{}. {} ({})", i+1, printMoveVerbose(bookEntry.moves[i]), bookEntry.ptrNextPosition[i]->counter);
      //    }
      //    fprintln("Book move: {}", printMoveVerbose(bookMove));
    }
  }
  return bookMove;
}

void OpeningBook::saveToCache() {
  const std::scoped_lock<std::mutex> lock(bookMutex);

  LOG__DEBUG(Logger::get().BOOK_LOG, "Saving book to cache.");

  { // save data to archive
    const auto start = std::chrono::high_resolution_clock::now();

    const std::basic_string<char> &serCacheFile = bookFilePath + ".cache.bin";
    LOG__DEBUG(Logger::get().BOOK_LOG, "Saving to cache file {}", serCacheFile);

    // create and open a binary archive for output
    std::ofstream ofsBin(serCacheFile, std::fstream::binary | std::fstream::out);
    boost::archive::binary_oarchive oa(ofsBin);
    // write class instance to archive
    oa << BOOST_SERIALIZATION_NVP(bookMap);

    const auto stop = std::chrono::high_resolution_clock::now();
    const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
    LOG__INFO(Logger::get().BOOK_LOG, "Opening book save to bin cache in ({:n} ms) ({})", elapsed.count(), serCacheFile);
  } // archive and stream closed when destructors are called

  // this is redundant for testing purposes
//  { // save data to archive
//    const auto start = std::chrono::high_resolution_clock::now();
//
//    const std::basic_string<char> &txtCacheFile = bookFilePath + ".cache.txt";
//    LOG__DEBUG(Logger::get().BOOK_LOG, "Saving to cache file {}", txtCacheFile);
//
//    // create and open a character archive for output
//    std::ofstream ofsTXT(txtCacheFile);
//    boost::archive::text_oarchive oa(ofsTXT);
//    // write class instance to archive
//    oa << BOOST_SERIALIZATION_NVP(bookMap);
//
//    const auto stop = std::chrono::high_resolution_clock::now();
//    const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
//    LOG__INFO(Logger::get().BOOK_LOG, "Opening book save to txt cache in ({:n} ms) ({})", elapsed.count(), txtCacheFile);
//  } // archive and stream closed when destructors are called
//
//  // this is redundant for testing purposes
//  { // save data to archive
//    const auto start = std::chrono::high_resolution_clock::now();
//
//    const std::basic_string<char> &xmlCacheFile = bookFilePath + ".cache.xml";
//    LOG__DEBUG(Logger::get().BOOK_LOG, "Saving to cache file {}", xmlCacheFile);
//
//    // create and open a character archive for output
//    std::ofstream ofsXML(xmlCacheFile);
//    boost::archive::xml_oarchive oa(ofsXML);
//    // write class instance to archive
//    oa << BOOST_SERIALIZATION_NVP(bookMap);
//
//    const auto stop = std::chrono::high_resolution_clock::now();
//    const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
//    LOG__INFO(Logger::get().BOOK_LOG, "Opening book saved to xml cache in ({:n} ms) ({})", elapsed.count(), xmlCacheFile);
//  } // archive and stream closed when destructors are called
}

void OpeningBook::loadFromCache() {
  const std::scoped_lock<std::mutex> lock(bookMutex);

  LOG__DEBUG(Logger::get().BOOK_LOG, "Loading book from cache.");

  std::unordered_map<Key, BookEntry> binMap;

  { // load data from archive
    const auto start = std::chrono::high_resolution_clock::now();

    const std::basic_string<char> &serCacheFile = bookFilePath + ".cache.bin";
    LOG__DEBUG(Logger::get().BOOK_LOG, "Loading from cache file {}", serCacheFile);
    // create and open a binary archive for input
    std::ifstream ifsBin(serCacheFile, std::fstream::binary | std::fstream::in);
    if (!ifsBin.is_open() || !ifsBin.good()) {
      LOG__ERROR(Logger::get().BOOK_LOG, "Loading from cache file {} failed", serCacheFile);
      return;
    }
    boost::archive::binary_iarchive ia(ifsBin);
    // write archive to class instance
    ia >> BOOST_SERIALIZATION_NVP(binMap);

    const auto stop = std::chrono::high_resolution_clock::now();
    const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
    LOG__INFO(Logger::get().BOOK_LOG,
              "Opening book loaded from bin cache with {:n} entries in ({:n} ms) ({})",
              binMap.size(), elapsed.count(), serCacheFile);
  }

//  std::unordered_map<Key, BookEntry> txtMap;
//  {
//    const auto start = std::chrono::high_resolution_clock::now();
//
//    // create and open a txt archive for input
//    const std::basic_string<char> &txtCacheFile = bookFilePath + ".cache.txt";
//    LOG__DEBUG(Logger::get().BOOK_LOG, "Loading from cache file {}", txtCacheFile);
//    std::ifstream ifsTXT(txtCacheFile);
//    if (!ifsTXT.good()) {
//      LOG__ERROR(Logger::get().BOOK_LOG, "Loading from cache file {} failed", txtCacheFile);
//      return;
//    }
//    boost::archive::text_iarchive ia(ifsTXT);
//    // write archive to class instance
//    ia >> BOOST_SERIALIZATION_NVP(txtMap);
//
//    const auto stop = std::chrono::high_resolution_clock::now();
//    const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
//    LOG__INFO(Logger::get().BOOK_LOG,
//              "Opening book loaded from txt cache with {:n} entries in ({:n} ms) ({})",
//              txtMap.size(), elapsed.count(), txtCacheFile);
//  }
//
//  std::unordered_map<Key, BookEntry> xmlMap;
//  {
//    const auto start = std::chrono::high_resolution_clock::now();
//
//    // create and open a xml archive for input
//    const std::basic_string<char> &xmlCacheFile = bookFilePath + ".cache.xml";
//    LOG__DEBUG(Logger::get().BOOK_LOG, "Loading from cache file {}", xmlCacheFile);
//    std::ifstream ifsXML(xmlCacheFile);
//    if (!ifsXML.good()) {
//      LOG__ERROR(Logger::get().BOOK_LOG, "Loading from cache file {} failed", xmlCacheFile);
//      return;
//    }
//    boost::archive::xml_iarchive ia(ifsXML);
//    // write archive to class instance
//    ia >> BOOST_SERIALIZATION_NVP(xmlMap);
//
//    const auto stop = std::chrono::high_resolution_clock::now();
//    const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
//    LOG__INFO(Logger::get().BOOK_LOG,
//              "Opening book loaded from xml cache with {:n} entries in ({:n} ms) ({})",
//              xmlMap.size(), elapsed.count(), xmlCacheFile);
//  }

  bookMap = std::move(binMap);

} // archive and stream closed when destructors are called

void OpeningBook::reset() {
  const std::scoped_lock<std::mutex> lock(bookMutex);
  bookMap.clear();
  gamesTotal = 0;
  gamesProcessed = 0;
  isInitialized = false;
  LOG__DEBUG(Logger::get().TEST_LOG, "Opening book reset: {:n}", bookMap.size());

}

std::string BookEntry::str() {
  std::ostringstream os;
  os << this->fen << " (" << this->counter << ") ";
  for (int i = 0; i < moves.size(); i++) {
    os << "[" << printMove(this->moves[i]) << " (" << this->ptrNextPosition[i]->counter << ")] ";
  }
  return os.str();
}

