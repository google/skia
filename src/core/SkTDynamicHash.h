/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkTDynamicHash_DEFINED
#define SkTDynamicHash_DEFINED

#include "SkTypes.h"
#include <unordered_map>

// This is just a thin wrapper over std::unordered_map now.
// Please feel free to use std::unordered_map directly.

// Traits requires:
//   static const Key& GetKey(const T&) { ... }
//   static uint32_t Hash(const Key&) { ... }
// We'll look on T for these by default, or you can pass a custom Traits type.
template <typename T, typename Key, typename Traits = T>
class SkTDynamicHash {
private:
    struct Hasher {
        size_t operator()(const Key* k) const { return Traits::Hash(*k); }
    };
    struct Equal {
        bool operator()(const Key* a, const Key* b) const { return *a == *b; }
    };
    using Map = std::unordered_map<const Key*, T*, Hasher, Equal>;

public:
    SkTDynamicHash() {}
    ~SkTDynamicHash() {}

    class Iter {
    public:
        explicit Iter(SkTDynamicHash* hash) : fHash(hash) {
            SkASSERT(hash);
            fCurrent = fHash->fMap.begin();
        }
        bool done() const {
            return fCurrent == fHash->fMap.end();
        }
        T& operator*() const {
            return *fCurrent->second;
        }
        void operator++() {
            fCurrent++;
        }

    private:
        SkTDynamicHash* fHash;
        typename Map::iterator fCurrent;
    };

    class ConstIter {
    public:
        explicit ConstIter(const SkTDynamicHash* hash) : fHash(hash) {
            SkASSERT(hash);
            fCurrent = fHash->fMap.begin();
        }
        bool done() const {
            return fCurrent == fHash->fMap.end();
        }
        const T& operator*() const {
            return *fCurrent->second;
        }
        void operator++() {
            fCurrent++;
        }

    private:
        const SkTDynamicHash* fHash;
        typename Map::const_iterator fCurrent;
    };

    int count() const { return SkToInt(fMap.size()); }

    // Return the entry with this key if we have it, otherwise nullptr.
    T* find(const Key& key) const {
        for (auto it = fMap.find(&key); it != fMap.end();) {
            return it->second;
        }
        return nullptr;
    }

    // Add an entry with this key.  We require that no entry with newEntry's key is already present.
    void add(T* newEntry) {
        const Key& key = Traits::GetKey(*newEntry);
        SkAssertResult(fMap.insert({&key, newEntry}).second);
    }

    // Remove the entry with this key.  We require that an entry with this key is present.
    void remove(const Key& key) {
        SkDEBUGCODE(size_t removed =) fMap.erase(&key);
        SkASSERT(removed == 1);
    }

    void rewind() { fMap.clear(); }
    void  reset() {
        fMap.~Map();
        new (&fMap) Map;
    }

private:
    Map fMap;
};

#endif
