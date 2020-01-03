/*
 * MIT License
 *
 * Copyright (c) 2019 Frank Kopp
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

#include "Logging.h"
#include "types.h"

#ifndef FRANKYCPP_TT_H
#define FRANKYCPP_TT_H

/**
 * Simple TT implementation using heap memory and simple hash for entries.
 * The number of entries are always a power of two fitting into the given size.
 * It is not yet thread safe as it has no synchronization. 
 */
class TT {
public:

  static constexpr uint64_t KB = 1024;
  static constexpr uint64_t MB = KB * KB;
  static constexpr uint64_t DEFAULT_TT_SIZE = 2 * MB; // byte

  typedef uint64_t Entry;
  static constexpr uint64_t ENTRY_SIZE = sizeof(Key) + sizeof(Entry); // 16 byte

  enum EntryType : u_int8_t {
    TYPE_NONE = 0, TYPE_EXACT = 1, TYPE_ALPHA = 2, TYPE_BETA = 3,
  };

  enum Result {
    // TT probe has found an entry and the value leads to a cut off
    TT_HIT,
    // TT probe has not found an entry or the value does not lead to a cut off.
    TT_MISS
  };

private:

  std::shared_ptr<spdlog::logger> const LOG = spdlog::get("TT_Logger");

  // threads for clearing hash
  int noOfThreads = 1;

  // size and fill info
  uint64_t sizeInByte = 0L;
  std::size_t maxNumberOfEntries = 0L;
  std::size_t numberOfEntries = 0L;

  // statistics
  mutable uint64_t numberOfPuts = 0;
  mutable uint64_t numberOfCollisions = 0;
  mutable uint64_t numberOfOverwrites = 0;
  mutable uint64_t numberOfUpdates = 0;
  mutable uint64_t numberOfProbes = 0;
  mutable uint64_t numberOfHits = 0;
  mutable uint64_t numberOfMisses = 0;

  // these two arrays hold the actual entries for the transposition table
  Key* _keys = new Key[0]; // default initialization
  Entry* _data = new Entry[0]; // default initialization

public:

  TT() : TT(DEFAULT_TT_SIZE) {};
  TT(uint64_t size);
  ~TT();

  // disallow copies
  TT(TT const &tt) = delete;
  TT &operator=(const TT &) = delete;

  /**
   * Changes the soze of the transposition table and clears all entries. 
   * @param newSize in Byte
   */
  void resize(uint64_t newSize);

  /**
   * Clears the transposition table be resetting all entries to 0.
   */
  void clear();

  /**
    * Stores the node value and the depth it has been calculated at.
    * @param forced - when true skips age check
    * @param key
    * @param value
    * @param type
    * @param depth
    * @param bestMove
    * @param mateThreat
    */
  void put(bool forced, Key key, Value value, EntryType type, Depth depth, Move bestMove,
           bool mateThreat);

  /**
   * Stores the node value and the depth it has been calculated at.
   * @param key
   * @param value
   * @param type
   * @param depth
   * @param bestMove
   * @param mateThreat
   */
  void put(Key key, Value value, EntryType type, Depth depth, Move bestMove, bool mateThreat) {
    put(false, key, value, type, depth, bestMove, mateThreat);
  }

  /**
  * Stores the node value and the depth it has been calculated at.
  *
  * @param key
  * @param value
  * @param type
  * @param depth
  */
  void put(Key key, Value value, EntryType type, Depth depth) {
    put(key, value, type, depth, MOVE_NONE, false);
  }

  /**
   * This retrieves the cached value of this node from cache
   * Decreases the age of the entry found. The returned entry
   * has the unchanged age. 
   *
   * @param key
   * @return value for key or <tt>VALUE_NONE</tt> if not found
   */
  Entry get(Key key);

  /**
   * Looks up and returns a result using get(Key key).
   * May write to ttValue and ttMove.
   *
   * @param position
   * @param ttValue
   * @param ttMove
   * @return A result of the probe with value and move from the TT in case of hit.
   */
  TT::Result
  probe(const Key &key, const Depth &depth, const Value &alpha, const Value &beta, Value &ttValue,
        Move &ttMove, bool isPVNode);

  /**
   * Age all entries by 1
   */
  void ageEntries();

  /**
   * Returns how full the transposition table is in permill as per UCI
   */
  inline int hashFull() const {
    if (!maxNumberOfEntries) return 0;
    return static_cast<int>(1000 *
                            (static_cast<double>(numberOfEntries) / (maxNumberOfEntries)));
  };

  std::string str() {
    return fmt::format(
      "TT: size {:n} MB max entries {:n} hashfull {} entries {:n} puts {:n} updates {:n} collisions {:n} overwrites {:n} ",
      sizeInByte / MB, maxNumberOfEntries, hashFull(), numberOfEntries, numberOfPuts,
      numberOfUpdates, numberOfCollisions, numberOfOverwrites);
  }

private:

  /**
   * @param key
   * @return returns a hash key
   */
  std::size_t getHash(Key key);

  // ###########################################################################
  // Bit operations for data
  // data:       length position values
  // ----------------------------------------------------
  // move:       32 bit  0-31    (only positive integers)
  // Value:      16 bit 32-47    (only positive shorts)
  // Depth:       8 bit 48-55    (only positive bytes)
  // Age:         3 bit 56-58    (0-7)
  // Type:        2 bit 59-60    (0-3)
  // MateThread:  1 bit 61       (0-1)
  // Free:        2 bit
  //
  // 1111111111111111111111111111111111111111111111111111111111111111
  //   ||T|A |Depth  |Value          | Move                         |
  // 1111111111111111111111111111111111111111111111111111111111111111
  // ###########################################################################

  // MASKs
  static constexpr TT::Entry MOVE_bitMASK = 0b11111111111111111111111111111111;
  static constexpr TT::Entry VALUE_bitMASK = 0b1111111111111111;
  static constexpr TT::Entry DEPTH_bitMASK = 0b11111111;
  static constexpr TT::Entry AGE_bitMASK = 0b111;
  static constexpr TT::Entry TYPE_bitMASK = 0b11;
  static constexpr TT::Entry MATE_bitMASK = 0b1;

  // Bit operation values
  static constexpr TT::Entry TT_MOVE_MASK = MOVE_bitMASK;
  static constexpr TT::Entry TT_VALUE_SHIFT = 32;
  static constexpr TT::Entry TT_VALUE_MASK = VALUE_bitMASK << TT_VALUE_SHIFT;
  static constexpr TT::Entry TT_DEPTH_SHIFT = 48;
  static constexpr TT::Entry TT_DEPTH_MASK = DEPTH_bitMASK << TT_DEPTH_SHIFT;
  static constexpr TT::Entry TT_AGE_SHIFT = 56;
  static constexpr TT::Entry TT_AGE_MASK = AGE_bitMASK << TT_AGE_SHIFT;
  static constexpr TT::Entry TT_TYPE_SHIFT = 59;
  static constexpr TT::Entry TT_TYPE_MASK = TYPE_bitMASK << TT_TYPE_SHIFT;
  static constexpr TT::Entry TT_MATE_SHIFT = 61;
  static constexpr TT::Entry TT_MATE_MASK = MATE_bitMASK << TT_MATE_SHIFT;

public:

  static inline Entry setBestMove(Entry eData, Move bestMove) {
    eData &= ~TT_MOVE_MASK; // reset old move
    return eData | bestMove;
  }

  static inline Entry setValue(Entry entry, Value value) {
    if (value < VALUE_NONE) { value = VALUE_NONE; }
    else if (value > VALUE_INF) value = VALUE_INF;
    // shift to positive short to avoid signed values setting the negative bits
    value += -VALUE_NONE;
    assert (value >= 0);
    entry &= ~TT_VALUE_MASK;
    return entry | static_cast<Entry>(value) << TT_VALUE_SHIFT;
  }

  static inline Entry setDepth(Entry entry, Depth depth) {
    entry &= ~TT_DEPTH_MASK;
    return entry | static_cast<Entry>(depth) << TT_DEPTH_SHIFT;
  }
  static inline Entry setAge(Entry entry, uint8_t age) {
    if (age > 7) age = 7;
    entry &= ~TT_AGE_MASK;
    return entry | static_cast<Entry>(age) << TT_AGE_SHIFT;
  }
  static inline Entry resetAge(Entry entry) {
    return setAge(entry, (uint8_t) 1);
  }

  static inline Entry increaseAge(Entry entry) {
    return setAge(entry, std::min(7, getAge(entry) + 1));
  }

  static inline Entry decreaseAge(Entry entry) {
    return setAge(entry, std::max(0, getAge(entry) - 1));
  }

  static inline Entry setType(Entry entry, EntryType type) {
    if (type > 3) type = EntryType::TYPE_NONE;
    entry &= ~TT_TYPE_MASK;
    return entry | static_cast<Entry>(type) << TT_TYPE_SHIFT;
  }

  static inline Entry setMateThreat(Entry entry, bool mateThreat) {
    if (mateThreat) { return entry | static_cast<Entry>(1) << TT_MATE_SHIFT; }
    else { return entry & ~TT_MATE_MASK; }
  }

public:

  static inline Move getBestMove(Entry entry) {
    return static_cast<Move>(entry & TT_MOVE_MASK);
  }

  static inline Value getValue(Entry entry) {
    // shift back result to potentially negative value
    return static_cast<Value>((entry & TT_VALUE_MASK) >> TT_VALUE_SHIFT) + VALUE_NONE;
  }

  static inline Depth getDepth(Entry data) {
    return static_cast<Depth>((data & TT_DEPTH_MASK) >> TT_DEPTH_SHIFT);
  }

  static inline uint8_t getAge(Entry entry) {
    return static_cast<uint8_t>((entry & TT_AGE_MASK) >> TT_AGE_SHIFT);
  }

  static inline EntryType getType(Entry entry) {
    return static_cast<EntryType>((entry & TT_TYPE_MASK) >> TT_TYPE_SHIFT);
  }

  static inline bool hasMateThreat(Entry entry) {
    return ((entry & TT_MATE_MASK) >> TT_MATE_SHIFT) == 1;
  }

  /**
   * Prints a bit representation of an entry
   * @param entry
   * @return
   */
  static std::string printBitString(Entry entry);

  static inline std::string str(EntryType type) {
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
  }

  static inline std::string str(Key key, Entry entry) {
    if (!entry) {
      return fmt::format("Key: {} Entry: NONE", key);
    }
    else {
      return fmt::format("Key: {} Entry: Value {} Type {} Depth {} Move {} Age {} Mate threat {}",
                         key, TT::getValue(entry), TT::str(TT::getType(entry)), TT::getDepth(entry),
                         printMove(TT::getBestMove(entry)), TT::getAge(entry),
                         TT::hasMateThreat(entry));
    }
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

};


#endif //FRANKYCPP_TT_H
