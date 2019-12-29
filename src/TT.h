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

// #define TT_DEBUG

/**
 * Simple TT implementation using heap memory and simple hash for entries.
 * It is not yet thread safe as it has no synchronization. 
 */
class TT {
public:

  typedef uint64_t Entry;

  static constexpr uint64_t KB = 1024;
  static constexpr uint64_t MB = KB * KB;
  static constexpr uint64_t ENTRY_SIZE = sizeof(Key) + sizeof(Entry); // 16 byte
  static constexpr uint64_t DEFAULT_TT_SIZE = 2 * MB; // byte

  enum EntryType : u_int8_t {
    TYPE_NONE = 0, TYPE_EXACT = 1, TYPE_ALPHA = 2, TYPE_BETA = 3,
  };

private:

  std::shared_ptr<spdlog::logger> const LOG = spdlog::get("TT_Logger");

  // threads for clearing hash
  int noOfThreads = 1;

private:

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
#ifdef TT_DEBUG
  std::string* _fens = new std::string[0]; // default initialization
#endif

public:

  TT() : TT(DEFAULT_TT_SIZE) {};
  TT(uint64_t size);
  ~TT();

  // disallow copies
  TT(TT const &tt) = delete;
  TT &operator=(const TT &) = delete;

  /**
   * Changes the soze of the transposition table and clears all entries. 
   * @param newsize in Byte
   */
  void resize(uint64_t newsize);

  /**
   * Clears the transposition table be resetting all entries to 0.
   */
  void clear();


#ifdef TT_DEBUG
  void put(bool forced, Key key, Value value, EntryType type, Depth depth, Move bestMove,
           bool mateThreat, std::string fen);
#endif

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

#ifdef TT_DEBUG
  void put(bool forced, Key key, Value value, EntryType type, Depth depth, Move bestMove,
           bool mateThreat) {
    put(forced, key, value, type, depth, bestMove, mateThreat, "");
  }
#else
  void put(bool forced, Key key, Value value, EntryType type, Depth depth, Move bestMove,
           bool mateThreat);
#endif

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
   * This retrieves the cached value of this node from cache if the
   * cached value has been calculated at a depth equal or deeper as the
   * depth value provided.
   * Decreases the age of the entry found.
   *
   * @param key
   * @return value for key or <tt>VALUE_NONE</tt> if not found
   */
  Entry get(Key key);

  /**
   * Age all entries by 1
   */
  void ageEntries();

  /**
   * Returns how full the transposition table is in permill as per UCI
   */
  inline int hashFull() const {
    return static_cast<int>(1000 *
                            (static_cast<double>(numberOfEntries) / (maxNumberOfEntries + 1)));
  };

  /**
   * Encodes the given move into the bit representation of the Entry.
   *
   * @param data
   * @param bestMove
   * @return long with value encoded
   */
  static Entry setBestMove(Entry eData, Move bestMove);

  /**
   * Decodes the move from the long. Does not have to be a valid move as it
   * will ont be checked during storing or retrieving.
   *
   * @param entry
   * @return decoded value
   */
  static Move getBestMove(Entry entry);

  /**
   * Encodes the given value into the bit representation of the long.
   * @param entry
   * @param value
   * @return entry with value encoded
   */
  static Entry setValue(Entry entry, Value value);

  /**
   * Decodes the value from the entry. Does not have to be a valid value
   * as it will not be checked during storing or retrieving.
   * @param data
   * @return decoded value
   */
  static Value getValue(Entry entry);

  /**
   * Encodes the given depth into the bit representation of the long.
   *
   * @param entry
   * @param depth
   * @return entry with value encoded
   */
  static Entry setDepth(Entry entry, Depth depth);

  /**
   * Decodes the depth from the long. Does not have to be a valid depth
   * as it will ont be checked during storing or retrieving.
   *
   * @param data
   * @return decoded value
   */
  static Depth getDepth(Entry data);

  /**
   * Encodes the given age into the bit representation of the long.
   * Age value greater 7 will be set to 7.
   *
   * @param entry
   * @param age
   * @return long with value encoded
   */
  static Entry setAge(Entry entry, uint8_t age);

  /**
   * Decodes the age from the long.
   *
   * @param data
   * @return decoded value
   */
  static uint8_t getAge(Entry entry);

  /**
   * Encodes default age (1) into the bit representation of the long.
   *
   * @param data
   * @return long with age encoded
   */
  static Entry resetAge(Entry entry);

  /**
   * Encodes age+1 into the bit representation of the long.
   * Maximum age of 7 is ensured.
   *
   * @param entry
   * @return long with value encoded
   */
  static Entry increaseAge(Entry entry);

  /**
   * Encodes age-1 into the bit representation of the long.
   * Minimum age of 0 is ensured.
   *
   * @param entry
   * @return long with value encoded
   */
  static Entry decreaseAge(Entry entry);

  /**
  * Encodes the given type into the bit representation of the long.
  * Accepts values 0 =< value <= 3. Other values will be set to 0.
  *
  * @param entry
  * @param type
  * @return long with value encoded
  */
  static Entry setType(Entry entry, EntryType type);

  /**
   * Decodes the type from the entry.
   *
   * @param entry
   * @return decoded value
   */
  static EntryType getType(Entry entry);

  /**
   * Encodes the given mateThreat into the bit representation of the long.
   *
   * @param entry
   * @param mateThreat
   * @return long with value encoded
   */
  static Entry setMateThreat(Entry entry, bool mateThreat);

  /**
   * Decodes the mateThreat from the long.
   *
   * @param entry
   * @return decoded value
   */
  static bool hasMateThreat(Entry entry);

  /**
   * Prints a bit representation of an entry
   * @param entry
   * @return
   */
  static std::string printBitString(Entry entry);

private:

  /**
   * @param key
   * @return returns a hash key
   */
  std::size_t getHash(Key key);

public:

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

  std::string str() {
    return fmt::format(
      "TT: size {:n} MB max entries {:n} hushfull {} entries {:n} puts {:n} updates {:n} collisions {:n} overwrites {:n} ",
      sizeInByte / MB, maxNumberOfEntries, hashFull(), numberOfEntries,
      numberOfPuts, numberOfUpdates, numberOfCollisions, numberOfOverwrites);
  }

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
