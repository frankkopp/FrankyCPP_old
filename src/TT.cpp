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

TT::TT(uint64_t sizeInByte) {
  resize(sizeInByte);
}

TT::~TT() {
  LOG->trace("Dtor: Delete previous memory allocation");
  delete[] _data;
}

void TT::resize(const uint64_t newSizeInByte) {
  LOG->trace("Resizing TT from {:n} to {:n}", sizeInByte, newSizeInByte);
  delete[] _data;
  // number of entries
  sizeInByte = newSizeInByte;
  // find the highest power of 2 smaller than maxPossibleEntries
  maxNumberOfEntries = (1ULL
    << static_cast<uint64_t>(std::floor(std::log2(sizeInByte / ENTRY_SIZE))));
  hashKeyMask = maxNumberOfEntries - 1;
  // if TT is resized to 0 we cant have any entries.
  if (sizeInByte == 0) maxNumberOfEntries = 0;
  _data = new Entry[maxNumberOfEntries];
  sizeInByte = maxNumberOfEntries * ENTRY_SIZE;
  clear();
  LOG->info("TT Size {:n} Byte, Capacity {:n} entries (size={}Byte) (Requested were {:n} Bytes)",
            sizeInByte, maxNumberOfEntries, sizeof(Entry), newSizeInByte);
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
        _data[i].key = 0;
        _data[i].move = MOVE_NONE;
        _data[i].depth = DEPTH_NONE;
        _data[i].value = VALUE_NONE;
        _data[i].type = TYPE_NONE;
        _data[i].age = 1;
        _data[i].mateThreat = false;
      }
    });
  }
  for (std::thread &th: threads) th.join();
  auto finish = std::chrono::high_resolution_clock::now();
  auto time = std::chrono::duration_cast<std::chrono::milliseconds>(finish - start).count();
  LOG->info("TT cleared {:n} entries in {:n} ms ({} threads)", maxNumberOfEntries, time,
            noOfThreads);
}

inline std::size_t TT::getHash(const Key key) const {
  return key & hashKeyMask;
}

inline TT::Entry* TT::getEntryPtr(const Key key) const {
  return &_data[getHash(key)];
}

const TT::Entry* TT::getMatch(const Key key) const {
  const Entry* const entryPtr = getEntryPtr(key);
  return entryPtr->key == key ? entryPtr : nullptr;
}

void TT::put(const Key key, const Depth depth, const Move move, const Value value,
             const Value_Type type, const bool mateThreat, const bool forced) {
  assert (value > VALUE_NONE);

  // if the size of the TT = 0 we
  // do not store anything
  if (!maxNumberOfEntries) return;

  // read the entries for this hash
  Entry* entryDataPtr = getEntryPtr(key);
  numberOfPuts++;

  // cleanup move
  const Move pureMove = moveOf(move);

  // New entry
  if (entryDataPtr->key == 0) {
    numberOfEntries++;
    writeEntry(entryDataPtr, key, depth, pureMove, value, type, mateThreat, 1);
    return;
  }

  // Same hash but different position
  if (entryDataPtr->key != key) {
    numberOfCollisions++;
    // overwrite if
    // - the new entry's depth is higher
    // - the new entry's depth is higher and the previous entry has not been used (is aged)
    if (depth > entryDataPtr->depth ||
        (depth == entryDataPtr->depth && (forced || entryDataPtr->age > 0))) {
      numberOfOverwrites++;
      writeEntry(entryDataPtr, key, depth, pureMove, value, type, mateThreat, 1);
    }
    return;
  }

  // Same hash and same position -> update entry?
  if (entryDataPtr->key == key) {
    numberOfUpdates++;
    // we always update as the stored moved can't be any good otherwise
    // we would have found this during the search in a previous probe
    // and we would not have come to store it again
    writeEntry(entryDataPtr, key, depth, pureMove ? pureMove : entryDataPtr->move, value, type, mateThreat, 1);
    return;
  }
  
  assert (numberOfPuts == (numberOfEntries + numberOfCollisions + numberOfUpdates));
}

const TT::Entry* TT::probe(const Key &key) {
  numberOfProbes++;
  Entry* ttEntryPtr = getEntryPtr(key);
  if (ttEntryPtr->key == key) {
    numberOfHits++; // entries with identical keys found
    ttEntryPtr->age = std::max(0, ttEntryPtr->age - 1);
    return ttEntryPtr;
  }
  numberOfMisses++; // keys not found (not equal to TT misses)
  return nullptr;
}

inline void
TT::writeEntry(Entry* const entryPtr, const Key key, const Depth depth, const Move move,
               const Value value, const Value_Type type, bool mateThreat, uint8_t age) {
  entryPtr->key = key;
  entryPtr->move = move;
  entryPtr->depth = depth;
  entryPtr->value = value;
  entryPtr->type = type;
  entryPtr->age = age;
  entryPtr->mateThreat = mateThreat;
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
        if (_data[i].key == 0) continue;
        _data[i].age = std::min(7, _data[i].age + 1);
      }
    });
  }
  for (std::thread &th: threads) th.join();
  auto finish = std::chrono::high_resolution_clock::now();
  auto time = std::chrono::duration_cast<std::chrono::milliseconds>(finish - start).count();
  LOG->info("TT aged {:n} entries in {:n} ms ({} threads)", maxNumberOfEntries, time, noOfThreads);
}

std::ostream &operator<<(std::ostream &os, const TT::Entry &entry) {
  os << "key: " << entry.key << " depth: " << entry.depth << " move: " << entry.move << " value: "
     << entry.value << " type: " << entry.type << " mateThreat: " << entry.mateThreat
     << " age: " << entry.age;
  return os;

}

