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
  maxNumberOfEntries = (1ULL << static_cast<uint64_t>(std::floor(std::log2(maxPossibleEntries))));
//  fprintln("maxNumberOfEntries {} {}", maxNumberOfEntries, TT::printBitString(maxNumberOfEntries));

  // calculate how many buckets we can map
  numberOfBuckets = maxNumberOfEntries >> 2; // 2 bits for buckets = 4 buckets
//  fprintln("numberOfBuckets    {} {}", numberOfBuckets, TT::printBitString(numberOfBuckets));

  bucketMask = numberOfBuckets - 1;
//  fprintln("bucketMask         {} {}", bucketMask, TT::printBitString(bucketMask));

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
        _keys[i] = 0;
        _data[i] = 0;
      }
    });
  }
  for (std::thread &th: threads) th.join();
  auto finish = std::chrono::high_resolution_clock::now();
  auto time = std::chrono::duration_cast<std::chrono::milliseconds>(finish - start).count();
  LOG->info("TT cleared {:n} entries in {:n} ms ({} threads)", maxNumberOfEntries, time,
            noOfThreads);
}

inline std::size_t TT::getBucket(const Key key) const {
  //  fprintln("key         = {}", TT::printBitString(key));
  //  fprintln("bucket mask = {}", TT::printBitString(bucketMask));
  //  fprintln("bucket      = {}", TT::printBitString((key & bucketMask) << 2));
  return (key & bucketMask) << 2;
}

void
TT::put(Key key, Value value, TT::EntryType type, Depth depth, Move bestMove, bool mateThreat) {
  assert (value > VALUE_NONE);

  // if the size of the TT = 0 we do not store anything
  if (!maxNumberOfEntries) return;

  numberOfPuts++;

  // get hash key
  std::size_t bucket = getBucket(key);

//  fprintln("bucket    = {} {}", bucket, TT::printBitString(bucket));

  // check if we already have this entry in the bucket
  int bucketEntry = 4; // not found
  for (int i = 0; i < 4; ++i) {
    if (_keys[bucket | i] == key) {
      bucketEntry = i;
      break;
    }
  } // found if bucketEntry < 4

  if (bucketEntry < 4) { // found - update!
    // if we have found the entry already we always replace it
    // as it would not have been calculated again if a useful
    // entry have been in the TT.
    numberOfUpdates++;
    std::size_t entryAddr = bucket | bucketEntry;
//    fprintln("entryAddr = {} {} UPDATE (bucketEntry={})", entryAddr, TT::printBitString(entryAddr), bucketEntry);
    _data[entryAddr] = createEntry(value, type, depth, bestMove, mateThreat);
  }
  else { // not found - new or replace other?

    int oldest = 0;
    int minDepth = DEPTH_MAX; // per age
    int replaceEntry = 4; // no replace
    // Find a place to store either new or replace
    // We use one loop to find empty slots or replacable slots
    for (int pos = 0; pos < 4; ++pos) {
      const size_t entryAddr = bucket | pos;
      if (_data[entryAddr] == 0) {
        // empty entry - use this pos and stop looking
        replaceEntry = pos;
        break;
      }
      // if we iterate to the last pos we didn't find an empty pos so this is
      // a real collision where we have to decide which pos to replace
      if (pos == 3) numberOfCollisions++;
      // we replace the oldest entry with the lowest depth
      const uint8_t age = getAge(_data[entryAddr]);
      const uint8_t draft = getDepth(_data[entryAddr]);
      // we found an older entry - use this as replace pos
      if (age > oldest) { // older
        oldest = age;
        minDepth = DEPTH_MAX; // reset for age
        replaceEntry = pos;
      }
        // same age but less or equal draft - use this slot
      else if (age == oldest && draft <= minDepth) {
        minDepth = draft;
        replaceEntry = pos;
      }
    }
    // here we know the pos to use (new or replace)
    if (replaceEntry < 4) { // create or replace the entry
      std::size_t entryAddr = bucket | replaceEntry;
      if (_data[entryAddr] == 0) { // new entry
        numberOfEntries++;
//        fprintln("entryAddr = {} {} NEW (bucketEntry={})", entryAddr, TT::printBitString(entryAddr), replaceEntry);
      }
      else { // replace old entry
        numberOfReplaces++;
//        fprintln("entryAddr = {} {} REPLACE (bucketEntry={})", entryAddr, TT::printBitString(entryAddr), replaceEntry);
      }
      _keys[entryAddr] = key;
      _data[entryAddr] = createEntry(value, type, depth, bestMove, mateThreat);;
    }
  }
  assert (numberOfPuts == (numberOfEntries + numberOfCollisions + numberOfUpdates));
}

inline TT::Entry TT::createEntry(const Value &value, const TT::EntryType &type, const Depth &depth,
                                 const Move &bestMove, bool mateThreat) {
  Entry entryData = 0;
  entryData = setAge(entryData, 1);
  entryData = setMateThreat(entryData, mateThreat);
  entryData = setValue(entryData, value);
  entryData = setType(entryData, type);
  entryData = setDepth(entryData, depth);
  entryData = setBestMove(entryData, bestMove);
  return entryData;
}

TT::Entry TT::getEntry(Key key) const {
  Entry* const ptr = getEntryPtr(key);
  return ptr == nullptr ? 0 : *ptr;
}

inline TT::Entry* TT::getEntryPtr(Key key) const {
  std::size_t bucket = getBucket(key);
  for (int pos = 0; pos < 4; ++pos) {
    if (_keys[bucket | pos] == key)
      return &_data[bucket | pos];
  }
  return nullptr;
}

TT::Result
TT::probe(const Key &key, const Depth &depth, const Value &alpha, const Value &beta, Value &ttValue,
          Move &ttMove, bool isPVNode) {

  Entry* ttEntryPtr = getEntryPtr(key);

  if (ttEntryPtr != nullptr) { // HIT
    // we found an entry - decrease age for this entry
    *ttEntryPtr = increaseAge(*ttEntryPtr);
    
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
  return TT_MISS;
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

std::string TT::printBitString(Entry entry) {
  std::stringstream s;
  s << std::bitset<64>(entry);
  return s.str();
}


