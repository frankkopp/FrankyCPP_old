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
  maxNumberOfEntries = (1ULL << static_cast<uint64_t>(std::floor(std::log2(maxPossibleEntries))));
  hashKeyMask = maxNumberOfEntries - 1;

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

inline std::size_t TT::getHash(Key key) const {
  return key & hashKeyMask;
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

  // New entry
  if (entryKey == 0) {
    numberOfEntries++;
    _keys[hash] = key;
    _data[hash] = createEntry(value, type, depth, bestMove, mateThreat);
    return;
  }

  // Same hash but different position
  if (entryKey != key) {
    numberOfCollisions++;
    // overwrite if
    // - the new entry's depth is higher
    // - the new entry's depth is higher and the previous entry has not been used (is aged)
    if (depth > getDepth(entryData) ||
        (depth == getDepth(entryData) && (forced || getAge(entryData) > 0))) {
      numberOfOverwrites++;
      _keys[hash] = key;
      _data[hash] = createEntry(value, type, depth, bestMove, mateThreat);
    }
    return;
  }

  // Same hash and same position -> update entry?
  if (entryKey == key) {
    numberOfUpdates++;
    // we always update as the stored moved can't be any good otherwise
    // we would have found this in the previous probe.
    if (depth == getDepth(entryData)) {
      entryData = resetAge(entryData);
      entryData = setMateThreat(entryData, mateThreat);
      entryData = setValue(entryData, value);
      entryData = setType(entryData, type);
      entryData = setDepth(entryData, depth);
      // overwrite bestMove only with a valid move
      if (bestMove) entryData = setBestMove(entryData, bestMove);
      _data[hash] = entryData;
      return;
    }
  }
  assert (numberOfPuts == (numberOfEntries + numberOfCollisions + numberOfUpdates));
}

TT::Entry TT::createEntry(const Value &value, const TT::EntryType &type, const Depth &depth,
                          const Move &bestMove, bool mateThreat) {
  Entry entryData = 0;
  entryData = resetAge(entryData);
  entryData = setMateThreat(entryData, mateThreat);
  entryData = setValue(entryData, value);
  entryData = setType(entryData, type);
  entryData = setDepth(entryData, depth);
  entryData = setBestMove(entryData, bestMove);
  return entryData;
}

TT::Result
TT::probe(const Key &key, const Depth &depth, const Value &alpha, const Value &beta, Value &ttValue,
          Move &ttMove, bool isPVNode) {
  numberOfProbes++;
  Entry* ttEntryPtr = getEntryPtr(key);
  if (ttEntryPtr) { // HIT
    numberOfHits++;
    *ttEntryPtr = decreaseAge(*ttEntryPtr);
    // get best move independent from tt entry depth
    ttMove = TT::getBestMove(*ttEntryPtr);
    // TODO: Implement Mate Threat
    // mateThreat[ply] = tt.hasMateThreat(ttEntry);
    // use value only if tt depth was equal or deeper
    if (TT::getDepth(*ttEntryPtr) >= depth) {
      ttValue = TT::getValue(*ttEntryPtr);
      assert (ttValue != VALUE_NONE);
      // In a PV node use the value only for a cut off if it is exact.
      // In non PV nodes we use it only if it is outside our current bounds.
      const EntryType entryType = getType(*ttEntryPtr);
      if ((isPVNode && entryType == TT::TYPE_EXACT) ||
          (!isPVNode && (entryType == TT::TYPE_EXACT ||
                         (entryType == TT::TYPE_ALPHA && ttValue <= alpha) ||
                         (entryType == TT::TYPE_BETA && ttValue >= beta)))) {
        return TT_HIT;
      }
    }
  }
  // MISS
  numberOfMisses++;
  return TT_MISS;
}

TT::Entry TT::getEntry(Key key) const {
  const Entry* const entryPtr = getEntryPtr(key);
  return entryPtr ? *entryPtr : 0;
}

inline TT::Entry* TT::getEntryPtr(Key key) const {
  const uint64_t hashKey = getHash(key);
  return  _keys[hashKey] == key ? &_data[hashKey] : nullptr;
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
        if (!_data[i]) continue;
        _data[i] = increaseAge(_data[i]);
      }
    });
  }
  for (std::thread &th: threads) th.join();
  auto finish = std::chrono::high_resolution_clock::now();
  auto time = std::chrono::duration_cast<std::chrono::milliseconds>(finish - start).count();
  LOG->info("TT aged {:n} entries in {:n} ms ({} threads)", maxNumberOfEntries, time, noOfThreads);
}


std::string TT::printBitString(Entry entry) {
  std::stringstream s;
  s << std::bitset<64>(entry);
  return s.str();
}


