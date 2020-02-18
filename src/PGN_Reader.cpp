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

#include <string>
#include <sstream>
#include <execution>
#include "PGN_Reader.h"
#include "misc.h"

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/regex.hpp>
using namespace boost;

using namespace std::string_literals;

PGN_Reader::PGN_Reader(std::vector<std::string> lines)
  : inputLines{lines} {}

bool PGN_Reader::process() {
  LOG__TRACE(Logger::get().BOOK_LOG, "Processing {} lines.", inputLines.size());
  const auto start = std::chrono::high_resolution_clock::now();
  // loop over all input lines
  uint64_t c = 0;
  VectorIterator linesIter = inputLines.begin();
  while (linesIter < inputLines.end()) {
    LOG__TRACE(Logger::get().BOOK_LOG, "Processing game {}", games.size() + 1);
    processOneGame(linesIter);
  }
  const auto stop = std::chrono::high_resolution_clock::now();
  const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
  LOG__TRACE(Logger::get().BOOK_LOG, "Found {} games in {:n} ms", games.size(), elapsed.count());
  return true;
}

void PGN_Reader::processOneGame(VectorIterator &iterator) {
  bool gameEndReached = false;
  PGN_Game game{};
  boost::regex e1;
  do {
    // clean up line
    trim(*iterator);

    // ignore comment lines
    if (starts_with(*iterator, "%")) continue;

    // trailing comments
    e1.assign(R"(;.*$)");
    erase_regex(*iterator, e1);

    // ignore meta data tags for now
    e1.assign(R"(\[\w+ +".*"\])");
    if (find_regex(*iterator, e1)) continue;

    LOG__TRACE(Logger::get().BOOK_LOG, "Process line: {}    (length={})", *iterator, iterator->size());

    // eliminate double whitespace
    e1.assign(R"(\s+)");
    replace_all_regex(*iterator, e1, " "s);

    // process move section
    e1.assign(R"(^(\d+.)|([KQRBN]?[a-h][1-8]))");
    if (find_regex(*iterator, e1)) {
      handleMoveSection(iterator, game);
      gameEndReached = true;
    }

  } while (++iterator < inputLines.end() && !gameEndReached);
  const uint64_t dist = inputLines.size() - std::distance(iterator, inputLines.end());
  if (games.size() % 10'000 == 0) Misc::printProgress(static_cast<double>(dist) / inputLines.size());
  games.push_back(game);
}

void PGN_Reader::handleMoveSection(VectorIterator &iterator, PGN_Game &game) {
  LOG__TRACE(Logger::get().BOOK_LOG, "Move section line: {}    (length={})", *iterator, iterator->size());

  // read and concatenate all lines belonging to the move section of  one game
  std::ostringstream os;
  boost::regex e1;
  do {
    trim(*iterator);
    // ignore comment lines
    if (starts_with(*iterator, "%")) continue;
    // trailing comments
    e1.assign(R"(;.*$)");
    erase_regex(*iterator, e1);
    // append line
    os << *iterator << " ";
    // look for end pattern
    e1.assign(R"(.*((1-0)|(0-1)|(1/2-1/2)|\*)$)");
    if (find_regex(*iterator, e1)) {
      break;
    }
  } while (++iterator < inputLines.end());

  std::string moveSection = os.str();
  LOG__TRACE(Logger::get().BOOK_LOG, "Move section: {} (length={})", moveSection, moveSection.size());

  // eliminate unwanted stuff
  e1.assign(R"((\$\d{1,3}))"); // no NAG annotation supported
  replace_all_regex(moveSection, e1, " "s);
  e1.assign(R"(\{[^{}]*\})"); // bracket comments
  replace_all_regex(moveSection, e1, " "s);
  e1.assign(R"(<[^<>]*>)"); // reserved symbols < >
  replace_all_regex(moveSection, e1, " "s);

  // handle nested RAV variation comments
  e1.assign(R"(\([^()]*\))"); // reserved symbols < >
  do { // no RAV variations supported (could be nested)
    replace_all_regex(moveSection, e1, " "s);
  } while (find_regex(moveSection, e1));

  // remove result from line
  e1.assign(R"(((1-0)|(0-1)|(1/2-1/2)|\*))");
  replace_all_regex(moveSection, e1, ""s);

  // remove move numbers
  e1.assign(R"(\d{1,3}( )*(\.{1,3}))");
  replace_all_regex(moveSection, e1, " "s);

  // eliminate double whitespace
  e1.assign(R"(\s+)");
  replace_all_regex(moveSection, e1, " "s);
  trim(moveSection);

  LOG__TRACE(Logger::get().BOOK_LOG, "Move section clean (length={}): {} ", moveSection.size(), moveSection);

  // add to game
  std::vector<std::string> moves{};
  //  e1.assign(R"( +)");
  split(moves, moveSection, is_space(), token_compress_on);

  LOG__TRACE(Logger::get().BOOK_LOG, "Moves extracted: {} ", moves.size());

  e1.assign(R"(([NBRQK])?([a-h])?([1-8])?x?([a-h][1-8]|O-O-O|O-O)(=([NBRQ]))?([!?+#]*)?)");
  for (auto m : moves) {
    if (find_regex(m, e1)) {
      LOG__TRACE(Logger::get().BOOK_LOG, "Move: {} ", m);
      game.moves.push_back(m);
    }
  }
}

/**
 * PGN Specification:
 * =============================================================================
    <PGN-database> ::= <PGN-game> <PGN-database>
                       <empty>

    <PGN-game> ::= <tag-section> <movetext-section>

    <tag-section> ::= <tag-pair> <tag-section>
                      <empty>

    <tag-pair> ::= [ <tag-name> <tag-value> ]

    <tag-name> ::= <identifier>

    <tag-value> ::= <string>

    <movetext-section> ::= <element-sequence> <game-termination>

    <element-sequence> ::= <element> <element-sequence>
                           <recursive-variation> <element-sequence>
                           <empty>

    <element> ::= <move-number-indication>
                  <SAN-move>
                  <numeric-annotation-glyph>

    <recursive-variation> ::= ( <element-sequence> )

    <game-termination> ::= 1-0
                           0-1
                           1/2-1/2
                           *

    <empty> ::=
 * =============================================================================
 */


