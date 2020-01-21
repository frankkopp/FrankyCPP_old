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

#ifndef FRANKYCPP_EVALUATOR_H
#define FRANKYCPP_EVALUATOR_H

#include <vector>
//#include "gtest/gtest_prod.h"
#include "Logging.h"
#include "types.h"
#include "EvaluatorConfig.h"

// pre-fetching of TT entries into CPU caches
#ifdef __GNUC__
#define EVAL_ENABLE_PREFETCH
#endif

#ifdef EVAL_ENABLE_PREFETCH
#include <emmintrin.h>
#define EVAL_PREFETCH evaluator.prefetch(position.getPieceBB(WHITE, PAWN) | position.getPieceBB(BLACK, PAWN));
#else
#define EVAL_PREFETCH void(0);
#endif


// forward declared dependencies
class Position;

class Evaluator {

//  std::shared_ptr<spdlog::logger> LOG = spdlog::get("Eval_Logger");

  /** Entry class for the eval cache */
  struct Entry {
    Bitboard pawnBitboard = 0;
    int midvalue = 0;
    int endvalue = 0;

    std::string str() const {
      return fmt::format("id {} midvalue {} endvalue {}", pawnBitboard, midvalue, endvalue);
    }

    std::ostream &operator<<(std::ostream &os) {
      os << this->str();
      return os;
    }
  };

  /** eval cache */
  typedef std::vector<Entry> Table;
  Table pawnTable;
  /** if eval cache is turned off this holds the pawn eval */
  Entry defaultEntry{0, 0, 0};

  inline std::size_t getTableIndex(const Bitboard pawnsBitboard) const {
    return pawnsBitboard & (config.PAWN_TABLE_SIZE - 1);
  }

  /** stats for cache */
  std::size_t cacheEntries = 0;
  std::size_t cacheHits = 0;
  std::size_t cacheMisses = 0;
  std::size_t cacheReplace = 0;

public:

  Evaluator(); // constructor
  explicit Evaluator(std::size_t pawnEvalCacheSize); // constructor
  ~Evaluator() = default;
  Evaluator(const Evaluator &) = delete; // copy constructor
  Evaluator(Evaluator &&) = delete; // move constructor
  Evaluator &operator=(const Evaluator &) = delete; // assignment constructor
  Evaluator &operator=(const Evaluator &&) = delete; // assignment constructor

  EvaluatorConfig config{};

  void resizePawnTable(size_t size);

  Value evaluate(const Position &position);

  std::string pawnTableStats() const {
    return fmt::format("Cache stats: capacity {:n} entries {:n} hits {:n} "
                       "misses {:n} replace {:n}",
                       pawnTable.capacity(), cacheEntries, cacheHits,
                       cacheMisses, cacheReplace);
  }

  inline void prefetch(const Bitboard &pawnBitboard) const {
#ifdef EVAL_ENABLE_PREFETCH
    _mm_prefetch(&pawnTable[getTableIndex(pawnBitboard)], _MM_HINT_T0);
#endif
  }

private:

  int pawnEval(const Position &position);

  void evaluatePawns(const Position &position, Entry* entry);

  template<Color C, PieceType PT>
  int evaluatePiece(const Position &position);

  template<Color C, PieceType PT>
  int mobility(const Position &position, Square sq);

  template<Color C>
  int evaluateKing(const Position &position);

  template<Color C>
  int kingCastleSafety(const Position &position);

//  FRIEND_TEST(EvaluatorTest, evaluatePieceMobility);
//  FRIEND_TEST(EvaluatorTest, evaluatePawns);

};


#endif //FRANKYCPP_EVALUATOR_H
