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
#include "TT.h"

/*
 * TODO:
 *  Buckets for less replacements
 */

TT::TT(uint64_t size) {
  resize(size);
}

TT::~TT() {
  LOG->trace("Dtor: Delete previous memory allocation");
  delete[] _keys;
  delete[] _data;
}

void TT::resize(uint64_t newSize) {
  LOG->trace("Resizing TT from {:n} to {:n}", sizeInByte, newSize);
  delete[] _keys;
  delete[] _data;

  // number of entries
  sizeInByte = newSize;
  uint64_t maxPossibleEntries = sizeInByte / ENTRY_SIZE;

  // find the highest power of 2 smaller than maxPossibleEntries
  maxNumberOfEntries = (1ULL << static_cast<uint64_t>(std::floor(std::log2(maxPossibleEntries)))) - 1;

  _keys = new Key[maxNumberOfEntries];
  _data = new Entry[maxNumberOfEntries];

  sizeInByte = maxNumberOfEntries * ENTRY_SIZE;

  LOG->info("TT Size {:n} Byte, Capacity {:n} entries (Requested were {:n} Bytes ",
    sizeInByte, maxNumberOfEntries, newSize);
  
  clear();
}

void TT::clear() {
  // This clears the TT by overwriting each entry with 0.
  // It uses multiple threads if noOfThreads is > 1.
  LOG->trace("Clearing TT ({} threads)...", noOfThreads);
  auto start = std::chrono::high_resolution_clock::now();
  std::vector<std::thread> threads;
  threads.reserve(noOfThreads);
  for (int t = 0; t < noOfThreads; ++t) {
    threads.emplace_back([&, this, t]() {
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
  LOG->info("TT cleared {:n} entries in {:n} ms ({} threads)", maxNumberOfEntries, time,
            noOfThreads);
}

void TT::put(bool forced, Key key, Value value, TT::EntryType type, Depth depth, Move bestMove,
             bool mateThreat) {

  assert (value > VALUE_NONE);

  // if the size of the TT = 0 we
  // do not store anything
  if (!maxNumberOfEntries) return;

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
          assert (getValue(entryData) == value);
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

TT::Result
TT::probe(const Key &key, const Depth &depth, const Value &alpha, const Value &beta, Value &ttValue,
          Move &ttMove, bool isPVNode) {
  Entry ttEntry = get(key);
  if (ttEntry != 0) { // HIT
    // get best move independent from tt entry depth
    ttMove = TT::getBestMove(ttEntry);
    // TODO: Implement Mate Threat
    // mateThreat[ply] = tt.hasMateThreat(ttEntry);
    // use value only if tt depth was equal or deeper
    if (TT::getDepth(ttEntry) >= depth) {
      ttValue = TT::getValue(ttEntry);
      assert (ttValue != VALUE_NONE);
      // In a PV node use the value only for a cut off if it is exact.
      // In non PV nodes we use it only if it is outside our current bounds.
      const EntryType entryType = getType(ttEntry);
      if ((isPVNode && entryType == TT::TYPE_EXACT) ||
          (!isPVNode && (entryType == TT::TYPE_EXACT ||
                         (entryType == TT::TYPE_ALPHA && ttValue <= alpha) ||
                         (entryType == TT::TYPE_BETA && ttValue >= beta)))) {
        return TT_HIT;
      }
    }
  }
  // MISS
  return TT_MISS;
}

TT::Entry TT::get(Key key) {
  numberOfProbes++;
  uint64_t hashKey = getHash(key);
  if (_keys[hashKey] == key) { // hash hit
    numberOfHits++;
    Entry tmp = _data[hashKey];
    _data[hashKey] = decreaseAge(_data[hashKey]);
    return tmp;
  }
  numberOfMisses++;
  return 0;
}

void TT::ageEntries() {
  LOG->trace("Aging TT ({} threads)...", noOfThreads);
  auto start = std::chrono::high_resolution_clock::now();
  std::vector<std::thread> threads;
  threads.reserve(noOfThreads);
  for (int idx = 0; idx < noOfThreads; ++idx) {
    threads.emplace_back([&, this, idx]() {
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
  LOG->info("TT aged {:n} entries in {:n} ms ({} threads)", maxNumberOfEntries, time, noOfThreads);
}

inline std::size_t TT::getHash(Key key) {
  return key & maxNumberOfEntries;
}

std::string TT::printBitString(Entry entry) {
  std::stringstream s;
  s << std::bitset<64>(entry);
  return s.str();
}


