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

void TT::resize(uint64_t newSize) {

  LOG->info("Resizing TT from {:n} to {:n}", sizeInByte, newSize);

  LOG->debug("Delete previous memory allocation");
  delete[] _keys;
  delete[] _data;

  // number of entries
  sizeInByte = newSize;
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
  assert (
    type == EntryType::TYPE_EXACT || type == EntryType::TYPE_ALPHA || type == EntryType::TYPE_BETA);
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
    if (depth > getDepth(entryData) ||
        (depth == getDepth(entryData) && (forced || getAge(entryData) > 0))) {
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
  else {
    numberOfMisses++;
  }
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

std::string TT::printBitString(Entry entry) {
  std::stringstream s;
  s << std::bitset<64>(entry);
  return s.str();
}

