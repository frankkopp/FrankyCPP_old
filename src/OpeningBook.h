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

#ifndef FRANKYCPP_OPENINGBOOK_H
#define FRANKYCPP_OPENINGBOOK_H

#include <map>
#include "gtest/gtest_prod.h"
#include "PGN_Reader.h"
#include "Position.h"

class MoveGenerator;

struct BookEntry {
  Key key{};
  std::string fen{};
  int counter{0};
  std::vector<Move> moves{};
  std::vector<BookEntry*> ptrNextPosition{};

  BookEntry(const Key &zobrist, const std::string &fenString) : key(zobrist), fen(fenString), counter{1} {}
  std::string str();
};

class OpeningBook {
public:

  enum class BookFormat {
    SIMPLE,
    SAN,
    PGN
  };

private:
  std::mutex bookMutex;
  bool isInitialized = false;
  uint64_t fileSize{};
  BookFormat bookFormat;
  std::string bookFilePath{};
  std::unordered_map<Key, BookEntry> bookMap{};

  uint64_t gamesTotal = 0;
  uint64_t gamesProcessed = 0;

public:
  explicit OpeningBook(const std::string &bookPath, const BookFormat &bFormat);

  void initialize();
  uint64_t size() { return bookMap.size(); }
  Move getRandomMove(Key zobrist);

private:
  void readBookFromFile(const std::string &filePath);
  std::vector<std::string> getLinesFromFile(std::ifstream &ifstream);
  void processAllLines(std::vector<std::string> &lines);
  void processLine(std::string &line);
  void processSimpleLine(std::string &line);
  void processSANLine(std::string &line);
  void processPGNFileFifo(std::vector<std::string> &lines);
  void processPGNFile(std::vector<std::string> &lines);
  void processGames(std::vector<PGN_Game>* ptrGames);
  void processGame(PGN_Game &game);
  void addToBook(Position &currentPosition, const Move &move);
};


#endif //FRANKYCPP_OPENINGBOOK_H
