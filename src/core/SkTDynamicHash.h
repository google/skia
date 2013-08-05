/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkTDynamicHash_DEFINED
#define SkTDynamicHash_DEFINED

#include "SkTypes.h"
#include "SkMath.h"

template <typename T,
          typename Key,
          const Key& (GetKey)(const T&),
          uint32_t (Hash)(const Key&),
          bool (Equal)(const T&, const Key&)>
class SkTDynamicHash {
public:
    SkTDynamicHash(int initialCapacity=64/sizeof(T*))
    : fCount(0)
    , fCapacity(SkNextPow2(initialCapacity > 0 ? initialCapacity : 1))
    , fArray(AllocArray(fCapacity)) {}

    ~SkTDynamicHash() {
        sk_free(fArray);
    }

    int count() const { return fCount; }

    // Return the entry with this key if we have it, otherwise NULL.
    T* find(const Key& key) const {
        int index = this->firstIndex(key);
        for (int round = 0; round < fCapacity; round++) {
            T* candidate = fArray[index];
            if (candidate == Empty()) {
                return NULL;
            }
            if (candidate != Deleted() && Equal(*candidate, key)) {
                return candidate;
            }
            index = this->nextIndex(index, round);
        }
        SkASSERT(!"find: should be unreachable");
        return NULL;
    }

    // Add an entry with this key.
    void add(T* newEntry) {
        this->maybeGrow();

        const Key& key = GetKey(*newEntry);
        int index = this->firstIndex(key);
        for (int round = 0; round < fCapacity; round++) {
            T* candidate = fArray[index];
            if (candidate == Empty() || candidate == Deleted()) {
                fArray[index] = newEntry;
                fCount++;
                return;
            }
            if (Equal(*candidate, key)) {
                fArray[index] = newEntry;
                return;
            }
            index = this->nextIndex(index, round);
        }
        SkASSERT(!"add: should be unreachable");
    }

    // Remove entry with this key, if we have it.
    void remove(const Key& key) {
        this->innerRemove(key);
        this->maybeShrink();
    }

protected:
    // These methods are used by tests only.

    int capacity() const { return fCapacity; }

    // How many collisions do we go through before finding where this entry should be inserted?
    int countCollisions(const Key& key) const {
        int index = this->firstIndex(key);
        for (int round = 0; round < fCapacity; round++) {
            const T* candidate = fArray[index];
            if (candidate == Empty() || candidate == Deleted() || Equal(*candidate, key)) {
                return round;
            }
            index = this->nextIndex(index, round);
        }
        SkASSERT(!"countCollisions: should be unreachable");
        return -1;
    }

private:
    // We have two special values to indicate an empty or deleted entry.
    static const T* Empty()   { return reinterpret_cast<const T*>(0); }  // i.e. NULL
    static const T* Deleted() { return reinterpret_cast<const T*>(1); }  // Also an invalid pointer.

    static T** AllocArray(int capacity) {
        T** array = (T**)sk_malloc_throw(sizeof(T*) * capacity);
        sk_bzero(array, sizeof(T*) * capacity);  // All cells == Empty().
        return array;
    }

    void innerRemove(const Key& key) {
        int index = this->firstIndex(key);
        for (int round = 0; round < fCapacity; round++) {
            const T* candidate = fArray[index];
            if (candidate == Empty()) {
                return;
            }
            if (candidate != Deleted() && Equal(*candidate, key)) {
                fArray[index] = const_cast<T*>(Deleted());
                fCount--;
                return;
            }
            index = this->nextIndex(index, round);
        }
        SkASSERT(!"innerRemove: should be unreachable");
    }

    void maybeGrow() {
        if (fCount < fCapacity / 2) {
            return;
        }

        SkDEBUGCODE(int oldCount = fCount;)
        int oldCapacity = fCapacity;
        T** oldArray = fArray;

        fCount = 0;
        fCapacity *= 2;
        fArray = AllocArray(fCapacity);

        for (int i = 0; i < oldCapacity; i++) {
            T* entry = oldArray[i];
            if (entry != Empty() && entry != Deleted()) {
                this->add(entry);
            }
        }
        SkASSERT(oldCount == fCount);

        sk_free(oldArray);
    }

    void maybeShrink() {
        // TODO
    }

    // fCapacity is always a power of 2, so this masks the correct low bits to index into our hash.
    uint32_t hashMask() const { return fCapacity - 1; }

    int firstIndex(const Key& key) const {
        return Hash(key) & this->hashMask();
    }

    // Given index at round N, what is the index to check at N+1?  round should start at 0.
    int nextIndex(int index, int round) const {
        // This will search a power-of-two array fully without repeating an index.
        return (index + round + 1) & this->hashMask();
    }

    int fCount;     // Number of non-empty, non-deleted entries in fArray.
    int fCapacity;  // Number of entries in fArray.  Always a power of 2.
    T** fArray;
};

#endif
