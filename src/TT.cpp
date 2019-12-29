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
#include "TT.h"

TT::TT(uint64_t size) {
  resize(size);
}

TT::~TT() {
  LOG->debug("Dtor: Delete previous memory allocation");
  delete[] _keys;
  delete[] _data;
}

void TT::resize(uint64_t newsize) {

  LOG->info("Resizing TT from {:n} to {:n}", sizeInByte, newsize);

  LOG->debug("Delete previous memory allocation");
  delete[] _keys;
  delete[] _data;

  // number of entries
  sizeInByte = newsize;
  maxNumberOfEntries = sizeInByte / ENTRY_SIZE;
  LOG->debug("Max number of entries: {:n}", maxNumberOfEntries);

  LOG->debug("Allocating new memory");
  _keys = new Key[maxNumberOfEntries];
  _data = new Entry[maxNumberOfEntries];
#ifdef TT_DEBUG
  _fens = new std::string[maxNumberOfEntries];
#endif

  clear();
}

void TT::clear() {
  // This clears the TT by overwriting each entry with 0.
  // It uses multiple threads if noOfThreads is > 1.
  LOG->debug("Clearing TT ({} threads)...", noOfThreads);
  auto start = std::chrono::high_resolution_clock::now();
  std::vector<std::thread> threads;
  threads.reserve(noOfThreads);
  for (int t = 0; t < noOfThreads; ++t) {
    threads.emplace_back([this, t]() {
      auto range = maxNumberOfEntries / noOfThreads;
      auto start = t * range;
      auto end = start + range;
      if (t == noOfThreads - 1) end = maxNumberOfEntries;
      for (std::size_t i = start; i < end; ++i) {
        _keys[i] = 0L;
        _data[i] = 0L;
      }
    });
  }
  for (std::thread &th: threads) th.join();
  auto finish = std::chrono::high_resolution_clock::now();
  auto time = std::chrono::duration_cast<std::chrono::milliseconds>(finish - start).count();
  LOG->debug("{:n} entries cleared in {:n} ms", maxNumberOfEntries, time);
}

#ifdef TT_DEBUG
void TT::put(bool forced, Key key, Value value, TT::EntryType type, Depth depth, Move bestMove,
             bool mateThreat, std::string fen) {
#else
  void TT::put(bool forced, Key key, Value value, TT::EntryType type, Depth depth, Move bestMove,
             bool mateThreat) {
#endif

  assert (depth >= 0);
  assert (type == EntryType::TYPE_EXACT || type == EntryType::TYPE_ALPHA || type == EntryType::TYPE_BETA);
  assert (value > VALUE_NONE);

  // get hash key
  std::size_t hash = getHash(key);

  // read the entries fpr this hash
  Key entryKey = _keys[hash];
  Entry entryData = _data[hash];

  numberOfPuts++;

  // New hash
  if (entryKey == 0) {
    numberOfEntries++;
    _keys[hash] = key;
#ifdef TT_DEBUG
    _fens[hash] = fen;
#endif
    entryData = resetAge(entryData);
    entryData = setMateThreat(entryData, mateThreat);
    entryData = setValue(entryData, value);
    entryData = setType(entryData, type);
    entryData = setDepth(entryData, depth);
    entryData = setBestMove(entryData, bestMove);
    _data[hash] = entryData;
  }
    // Same hash but different position
    // overwrite if
    // - the new entry's depth is higher
    // - the new entry's depth is higher and the previous entry has not been used (is aged)
  else if (entryKey != key) {
    numberOfCollisions++;
    if (depth > getDepth(entryData)
        || (depth == getDepth(entryData) && (forced || getAge(entryData) > 0))) {
      numberOfOverwrites++;
      _keys[hash] = key;
#ifdef TT_DEBUG
      _fens[hash] = fen;
#endif
      entryData = resetAge(entryData);
      entryData = setMateThreat(entryData, mateThreat);
      entryData = setValue(entryData, value);
      entryData = setType(entryData, type);
      entryData = setDepth(entryData, depth);
      entryData = setBestMove(entryData, bestMove);
      _data[hash] = entryData;
    }
  }
    // Same hash and same position -> update entry?
  else if (entryKey == key) {
    numberOfUpdates++;

    // if from same depth only update when quality of new entry is better
    // e.g. don't replace EXACT with ALPHA or BETA
    if (depth == getDepth(entryData)) {

      entryData = resetAge(entryData);
      entryData = setMateThreat(entryData, mateThreat);

      // old was not EXACT - update
      if (getType(entryData) != EntryType::TYPE_EXACT) {
        entryData = setValue(entryData, value);
        entryData = setType(entryData, type);
        entryData = setDepth(entryData, depth);
      }
      else {
        // old entry was exact, the new entry is also EXACT -> assert that they are identical
        if (type == TYPE_EXACT) {
#ifdef TT_DEBUG
          if (getValue(entryData) != value) {
            LOG->error("ENTRY VAL {} FEN {} ENTRY {}", getValue(entryData),_fens[hash], str(entryKey, entryData));
            LOG->error("NEW   VAL {} FEN {} ENTRY Key: {} Entry: Value {} Type {} Depth {} Move {}", value,_fens[hash], key, value, str(type), depth, printMove(bestMove));
            assert (1);
          }
#else
          assert (getValue(entryData) == value);
#endif
        }
      }

      // overwrite bestMove only with a valid move
      if (bestMove != MOVE_NONE) {
        entryData = setBestMove(entryData, bestMove);
      }

      _data[hash] = entryData;
    }
      // if depth is greater then update in any case
    else if (depth > getDepth(entryData)) {
      entryData = resetAge(entryData);
      entryData = setMateThreat(entryData, mateThreat);
      entryData = setValue(entryData, value);
      entryData = setType(entryData, type);
      entryData = setDepth(entryData, depth);

      // overwrite bestMove only with a valid move
      if (bestMove != MOVE_NONE) entryData = setBestMove(entryData, bestMove);

      _data[hash] = entryData;
    }
      // overwrite bestMove if there wasn't any before
    else if (getBestMove(entryData) == MOVE_NONE) {
      entryData = setBestMove(entryData, bestMove);
      _data[hash] = entryData;
    }
  }
  assert (numberOfPuts == (numberOfEntries + numberOfCollisions + numberOfUpdates));
}

TT::Entry TT::get(Key key) {
  numberOfProbes++;
  uint64_t hashKey = getHash(key);

  if (_keys[hashKey] == key) { // hash hit
    numberOfHits++;
    // to return unchanged data store them in tmp and return tmp
    Entry tmp = _data[hashKey];
    // decrease age of entry until 0
    _data[hashKey] = decreaseAge(_data[hashKey]);
    return tmp;
  }
  else { numberOfMisses++; }
  // cache miss or collision
  return 0;
}

void TT::ageEntries() {
  LOG->debug("Aging TT ({} threads)...", noOfThreads);
  auto start = std::chrono::high_resolution_clock::now();
  std::vector<std::thread> threads;
  threads.reserve(noOfThreads);
  for (int idx = 0; idx < noOfThreads; ++idx) {
    threads.emplace_back([this, idx]() {
      auto range = maxNumberOfEntries / noOfThreads;
      auto start = idx * range;
      auto end = start + range;
      if (idx == noOfThreads - 1) end = maxNumberOfEntries;
      for (std::size_t i = start; i < end; ++i) {
        _data[i] = increaseAge(_data[i]);
      }
    });
  }
  for (std::thread &th: threads) th.join();
  auto finish = std::chrono::high_resolution_clock::now();
  auto time = std::chrono::duration_cast<std::chrono::milliseconds>(finish - start).count();
  LOG->debug("{:n} entries aged in {:n} ms", maxNumberOfEntries, time);
}

inline std::size_t TT::getHash(Key key) {
  return key % maxNumberOfEntries;
}

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
constexpr TT::Entry MOVE_bitMASK = 0b11111111111111111111111111111111;
constexpr TT::Entry VALUE_bitMASK = 0b1111111111111111;
constexpr TT::Entry DEPTH_bitMASK = 0b11111111;
constexpr TT::Entry AGE_bitMASK = 0b111;
constexpr TT::Entry TYPE_bitMASK = 0b11;
constexpr TT::Entry MATE_bitMASK = 0b1;

// Bit operation values
constexpr TT::Entry TT_MOVE_MASK = MOVE_bitMASK;
constexpr TT::Entry TT_VALUE_SHIFT = 32;
constexpr TT::Entry TT_VALUE_MASK = VALUE_bitMASK << TT_VALUE_SHIFT;
constexpr TT::Entry TT_DEPTH_SHIFT = 48;
constexpr TT::Entry TT_DEPTH_MASK = DEPTH_bitMASK << TT_DEPTH_SHIFT;
constexpr TT::Entry TT_AGE_SHIFT = 56;
constexpr TT::Entry TT_AGE_MASK = AGE_bitMASK << TT_AGE_SHIFT;
constexpr TT::Entry TT_TYPE_SHIFT = 59;
constexpr TT::Entry TT_TYPE_MASK = TYPE_bitMASK << TT_TYPE_SHIFT;
constexpr TT::Entry TT_MATE_SHIFT = 61;
constexpr TT::Entry TT_MATE_MASK = MATE_bitMASK << TT_MATE_SHIFT;

TT::Entry TT::setBestMove(Entry eData, Move bestMove) {
  eData &= ~TT_MOVE_MASK; // reset old move
  return eData | bestMove;
}

Move TT::getBestMove(Entry entry) {
  return static_cast<Move>(entry & TT_MOVE_MASK);
}

TT::Entry TT::setValue(Entry entry, Value value) {
  if (value < VALUE_NONE) { value = VALUE_NONE; }
  else if (value > VALUE_INF) value = VALUE_INF;
  // shift to positive short to avoid signed values setting the negative bits
  value += -VALUE_NONE;
  assert (value >= 0);
  entry &= ~TT_VALUE_MASK;
  return entry | static_cast<Entry>(value) << TT_VALUE_SHIFT;
}

Value TT::getValue(Entry entry) {
  // shift back result to potentially negative value
  return static_cast<Value>((entry & TT_VALUE_MASK) >> TT_VALUE_SHIFT) + VALUE_NONE;
}

TT::Entry TT::setDepth(Entry entry, Depth depth) {
  entry &= ~TT_DEPTH_MASK;
  return entry | static_cast<Entry>(depth) << TT_DEPTH_SHIFT;
}

Depth TT::getDepth(Entry data) {
  return static_cast<Depth>((data & TT_DEPTH_MASK) >> TT_DEPTH_SHIFT);
}

TT::Entry TT::setAge(Entry entry, uint8_t age) {
  if (age > 7) age = 7;
  entry &= ~TT_AGE_MASK;
  return entry | static_cast<Entry>(age) << TT_AGE_SHIFT;
}

uint8_t TT::getAge(Entry entry) {
  return static_cast<uint8_t>((entry & TT_AGE_MASK) >> TT_AGE_SHIFT);
}

TT::Entry TT::resetAge(Entry entry) {
  return setAge(entry, (uint8_t) 1);
}

TT::Entry TT::increaseAge(Entry entry) {
  return setAge(entry, std::min(7, getAge(entry) + 1));
}

TT::Entry TT::decreaseAge(Entry entry) {
  return setAge(entry, std::max(0, getAge(entry) - 1));
}

TT::Entry TT::setType(Entry entry, EntryType type) {
  if (type > 3) type = EntryType::TYPE_NONE;
  entry &= ~TT_TYPE_MASK;
  return entry | static_cast<Entry>(type) << TT_TYPE_SHIFT;
}

TT::EntryType TT::getType(Entry entry) {
  return static_cast<TT::EntryType>((entry & TT_TYPE_MASK) >> TT_TYPE_SHIFT);
}

TT::Entry TT::setMateThreat(Entry entry, bool mateThreat) {
  if (mateThreat) { return entry | static_cast<Entry>(1) << TT_MATE_SHIFT; }
  else { return entry & ~TT_MATE_MASK; }
}

bool TT::hasMateThreat(Entry entry) {
  return ((entry & TT_MATE_MASK) >> TT_MATE_SHIFT) == 1;
}

std::string TT::printBitString(Entry entry) {
  std::stringstream s;
  s << std::bitset<64>(entry);
  return s.str();
}

