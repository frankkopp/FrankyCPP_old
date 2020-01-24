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

#include <iosfwd>
#include "types.h"
#include "gtest/gtest_prod.h"

#ifndef FRANKYCPP_TT_H
#define FRANKYCPP_TT_H

// pre-fetching of TT entries into CPU caches
#ifdef __GNUC__
#define TT_ENABLE_PREFETCH
#endif

#ifdef TT_ENABLE_PREFETCH
#include <emmintrin.h>
#define TT_PREFETCH tt->prefetch(position.getZobristKey());
#else
#define TT_PREFETCH void(0);
#endif

/**
 * Simple TT implementation using heap memory and simple hash for entries.
 * The number of entries are always a power of two fitting into the given size.
 * It is not yet thread safe as it has no synchronization.
 *
 * Tests have shown that an implementation with a struct and bitfields is the
 * more efficient than using only one 64-bit data field with manual bit shifting
 * and masking (~9% slower)
 * Also using buckets has not shown significant improvements and is much
 * slower (~20% slower).
 */
class TT {
public:

  static constexpr int CacheLineSize = 64;
  static constexpr uint64_t KB = 1024;
  static constexpr uint64_t MB = KB * KB;
  static constexpr uint64_t DEFAULT_TT_SIZE = 2 * MB; // byte

  struct Entry {
    // sorted by size to achieve smallest struct size
    // using bitfield for smallest size
    Key key = 0; // 64 bit
    Move move = MOVE_NONE; // 32 bit
    Value value = VALUE_NONE; // 8 bit signed
    Depth depth:7; // 0-127
    uint8_t age:3; // 0-7
    Value_Type type:2; // 4 values
    bool mateThreat:1; // 1-bit bool
    friend std::ostream &operator<<(std::ostream &os, const Entry &entry);
  };

  // struct Entry has 16 Byte
  static constexpr uint64_t ENTRY_SIZE = sizeof(Entry);

  static_assert(CacheLineSize % ENTRY_SIZE == 0, "Cluster size incorrect");

private:

  // threads for clearing hash
  unsigned int noOfThreads = 1;

  // size and fill info
  uint64_t sizeInByte = 0;
  std::size_t maxNumberOfEntries = 0;
  std::size_t hashKeyMask = 0;
  std::size_t numberOfEntries = 0;

  // statistics
  mutable uint64_t numberOfPuts = 0;
  mutable uint64_t numberOfCollisions = 0;
  mutable uint64_t numberOfOverwrites = 0;
  mutable uint64_t numberOfUpdates = 0;
  mutable uint64_t numberOfProbes = 0;
  mutable uint64_t numberOfHits = 0; // entries with identical key found
  mutable uint64_t numberOfMisses = 0; // no entry with key found

  // this array hold the actual entries for the transposition table
  Entry* _data = new Entry[0]; // default initialization

public:

  // TT default size is 2 MB
  TT() : TT(DEFAULT_TT_SIZE) {}

  /** @param newSizeInBytes Size of TT in bytes which will be reduced to the next
   * lowest power of 2 size */
  explicit TT(uint64_t newSizeInBytes);

  ~TT() {
    delete[] _data;
  }

  // disallow copies
  TT(TT const &tt) = delete; // copy
  TT &operator=(const TT &) = delete; // copy assignment
  TT(TT const &&tt) = delete; // move
  TT &operator=(const TT &&) = delete; // move assignment

  /**
   * Changes the size of the transposition table and clears all entries.
   * @param newSizeInByte in Byte which will be reduced to the next
   * lowest power of 2 size
   */
  void resize(uint64_t newSizeInByte);

  /** Clears the transposition table be resetting all entries to 0. */
  void clear();

  /**
    * Stores the node value and the depth it has been calculated at.
    * Also stores the best move for the node.
    * OBS: move will be stripped of any value before storing as we store value
    * separately and it may be surprising that a MOVE_NONE has a value.
    * @param forced when true skips age check (mostly for unit testing)
    * @param key Position key (usually Zobrist key)
    * @param depth 0-DEPTH_MAX (usually 127)
    * @param move best move of the node (when BETA best move until cut off)
    * @param value Value of the position between VALUE_MIN and VALUE_MAX
    * @param type EXACT, ALPHA or BETA
    * @param mateThreat node had a mate threat in the ply
    */
  void
  put(Key key, Depth depth, Move move, Value value, Value_Type type, bool mateThreat, bool forced);

  /**
    * Stores the node value and the depth it has been calculated at.
    * @param key Position key (usually Zobrist key)
    * @param depth 0-DEPTH_MAX (usually 127)
    * @param move best move of the node (when BETA best move until cut off)
    * @param value Value of the position between VALUE_MIN and VALUE_MAX
    * @param type EXACT, ALPHA or BETA
    * @param mateThreat node had a mate threat in the ply
    */
  void put(Key key, Depth depth, Move move, Value value, Value_Type type, bool mateThreat) {
    put(key, depth, move, value, type, mateThreat, false);
  }

  /**
    * Stores the node value and the depth it has been calculated at.
    *
    * @param key Position key (usually Zobrist key)
    * @param depth 0-DEPTH_MAX (usually 127)
    * @param value Value of the position between VALUE_MIN and VALUE_MAX
    * @param type EXACT, ALPHA or BETA
    */
  void put(Key key, Depth depth, Value value, Value_Type type) {
    put(key, depth, MOVE_NONE, value, type, false);
  }

  /**
   * This retrieves a copy of the entry of this node from cache.
   *
   * @param key Position key (usually Zobrist key)
   * @return Entry for key or 0 if not found
   */
  inline const TT::Entry* getMatch(const Key key) const {
    const Entry* const entryPtr = getEntryPtr(key);
    return entryPtr->key == key ? entryPtr : nullptr;
  }

  /**
   * Looks up and returns a result using get(Key key).
   * Result is a logical TT result. HIT means we can cut the search of the node.
   * MISS means we need to be searching on.
   * In both cases we might have a ttMove.
   * A HIT is returned when entry type is either EXACT or ALPHA and value<alpha
   * or BETA and value>beta. In a PV node only EXACT values are a HIT. 
   *
   * May write to ttValue and ttMove.
   *
   * @tparam NT true for a PV node, false for NonPV
   * @param key Position key (usually Zobrist key)
   * @param depth 1-DEPTH_MAX (127)
   * @param alpha current alpha when probing
   * @param beta current beta when probing
   * @param ttValue TT value will be stored into this
   * @param ttMove TT move will be stored into this
   * @param isPVNode current node type when probing
   * @return A result of the probe with value and move from the TT in case of hit.
   */
  const TT::Entry* probe(const Key &key);

  /** Age all entries by 1 */
  void ageEntries();

  /** Returns how full the transposition table is in permill as per UCI */
  inline int hashFull() const {
    if (!maxNumberOfEntries) return 0;
    return static_cast<int>(1000 *
                            (static_cast<double>(numberOfEntries) /
                             static_cast<double>(maxNumberOfEntries)));
  };

  std::string str() {
    return fmt::format(
      "TT: size {:n} MB max entries {:n} of size {:n} Bytes entries {:n} ({:n}%) puts {:n} "
      "updates {:n} collisions {:n} overwrites {:n} probes {:n} hits {:n} ({:n}%) misses {:n} ({:n}%)",
      sizeInByte / MB, maxNumberOfEntries, sizeof(Entry), numberOfEntries, hashFull() / 10,
      numberOfPuts, numberOfUpdates, numberOfCollisions, numberOfOverwrites, numberOfProbes,
      numberOfHits, numberOfProbes ? (numberOfHits * 100) / numberOfProbes : 0,
      numberOfMisses, numberOfProbes ? (numberOfMisses * 100) / numberOfProbes : 0);
  }

private:

  static void
  writeEntry(Entry* entryPtr, Key key, Depth depth, Move move,
             Value value, Value_Type type, bool mateThreat, uint8_t age);


  /* generates the index hash key from the position key  */
  inline std::size_t getHash(const Key key) const {
    return key & hashKeyMask;
  }

  /* This retrieves a direct pointer to the entry of this node from cache */
  inline TT::Entry* getEntryPtr(const Key key) const {
    return &_data[getHash(key)];
  }

  /** GETTER and SETTER */
public:

  uint64_t getSizeInByte() const {
    return sizeInByte;
  }

  std::size_t getMaxNumberOfEntries() const {
    return maxNumberOfEntries;
  }

  std::size_t getNumberOfEntries() const {
    return numberOfEntries;
  }

  uint64_t getNumberOfPuts() const {
    return numberOfPuts;
  }

  uint64_t getNumberOfCollisions() const {
    return numberOfCollisions;
  }

  uint64_t getNumberOfOverwrites() const {
    return numberOfOverwrites;
  }

  uint64_t getNumberOfUpdates() const {
    return numberOfUpdates;
  }

  uint64_t getNumberOfProbes() const {
    return numberOfProbes;
  }

  uint64_t getNumberOfHits() const {
    return numberOfHits;
  }

  uint64_t getNumberOfMisses() const {
    return numberOfMisses;
  }

  int getThreads() const {
    return noOfThreads;
  }

  void setThreads(int threads) {
    TT::noOfThreads = threads;
  }

  static inline std::string str(const Value_Type type) {
    switch (type) {
      case TYPE_NONE:
        return "NONE";
      case TYPE_EXACT:
        return "EXACT";
      case TYPE_ALPHA:
        return "ALPHA";
      case TYPE_BETA:
        return "BETA";
    }
    return "";
  }

  FRIEND_TEST(TT_Test, put);
  FRIEND_TEST(TT_Test, put);
  FRIEND_TEST(TT_Test, get);
  FRIEND_TEST(TT_Test, probe);

  // using prefetch improves probe lookup speed significantly
  inline void prefetch(const Key key) {
#ifdef TT_ENABLE_PREFETCH
    _mm_prefetch(&_data[(key & hashKeyMask)], _MM_HINT_T0);
#endif
  }
};

#endif //FRANKYCPP_TT_H
