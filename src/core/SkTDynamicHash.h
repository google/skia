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
          bool (Equal)(const T&, const Key&),
          int kGrowPercent   = 75,  // Larger -> more memory efficient, but slower.
          int kShrinkPercent = 25>
class SkTDynamicHash {
    static const int kMinCapacity = 4;  // Smallest capacity we allow.
public:
    SkTDynamicHash(int initialCapacity=64/sizeof(T*)) {
        this->reset(SkNextPow2(initialCapacity > kMinCapacity ? initialCapacity : kMinCapacity));
        SkASSERT(this->validate());
    }

    ~SkTDynamicHash() {
        sk_free(fArray);
    }

    int count() const { return fCount; }

    // Return the entry with this key if we have it, otherwise NULL.
    T* find(const Key& key) const {
        int index = this->firstIndex(key);
        for (int round = 0; round < fCapacity; round++) {
            T* candidate = fArray[index];
            if (Empty() == candidate) {
                return NULL;
            }
            if (Deleted() != candidate && Equal(*candidate, key)) {
                return candidate;
            }
            index = this->nextIndex(index, round);
        }
        SkASSERT(0); //  find: should be unreachable
        return NULL;
    }

    // Add an entry with this key.  We require that no entry with newEntry's key is already present.
    void add(T* newEntry) {
        SkASSERT(NULL == this->find(GetKey(*newEntry)));
        this->maybeGrow();
        SkASSERT(this->validate());
        this->innerAdd(newEntry);
        SkASSERT(this->validate());
    }

    // Remove the entry with this key.  We reqire that an entry with this key is present.
    void remove(const Key& key) {
        SkASSERT(NULL != this->find(key));
        this->innerRemove(key);
        SkASSERT(this->validate());
        this->maybeShrink();
        SkASSERT(this->validate());
    }

protected:
    // These methods are used by tests only.

    int capacity() const { return fCapacity; }

    // How many collisions do we go through before finding where this entry should be inserted?
    int countCollisions(const Key& key) const {
        int index = this->firstIndex(key);
        for (int round = 0; round < fCapacity; round++) {
            const T* candidate = fArray[index];
            if (Empty() == candidate || Deleted() == candidate || Equal(*candidate, key)) {
                return round;
            }
            index = this->nextIndex(index, round);
        }
        SkASSERT(0); // countCollisions: should be unreachable
        return -1;
    }

private:
    // We have two special values to indicate an empty or deleted entry.
    static T* Empty()   { return reinterpret_cast<T*>(0); }  // i.e. NULL
    static T* Deleted() { return reinterpret_cast<T*>(1); }  // Also an invalid pointer.

    static T** AllocArray(int capacity) {
        return (T**)sk_calloc_throw(sizeof(T*) * capacity);  // All cells == Empty().
    }

    void reset(int capacity) {
        fCount = 0;
        fDeleted = 0;
        fCapacity = capacity;
        fArray = AllocArray(fCapacity);
    }

    bool validate() const {
        #define SKTDYNAMICHASH_CHECK(x) SkASSERT((x)); if (!(x)) return false

        // Is capacity sane?
        SKTDYNAMICHASH_CHECK(SkIsPow2(fCapacity));
        SKTDYNAMICHASH_CHECK(fCapacity >= kMinCapacity);

        // Is fCount correct?
        int count = 0;
        for (int i = 0; i < fCapacity; i++) {
            if (Empty() != fArray[i] && Deleted() != fArray[i]) {
                count++;
            }
        }
        SKTDYNAMICHASH_CHECK(count == fCount);

        // Is fDeleted correct?
        int deleted = 0;
        for (int i = 0; i < fCapacity; i++) {
            if (Deleted() == fArray[i]) {
                deleted++;
            }
        }
        SKTDYNAMICHASH_CHECK(deleted == fDeleted);

        // Are all entries findable?
        for (int i = 0; i < fCapacity; i++) {
            if (Empty() == fArray[i] || Deleted() == fArray[i]) {
                continue;
            }
            SKTDYNAMICHASH_CHECK(NULL != this->find(GetKey(*fArray[i])));
        }

        // Are all entries unique?
        for (int i = 0; i < fCapacity; i++) {
            if (Empty() == fArray[i] || Deleted() == fArray[i]) {
                continue;
            }
            for (int j = i+1; j < fCapacity; j++) {
                if (Empty() == fArray[j] || Deleted() == fArray[j]) {
                    continue;
                }
                SKTDYNAMICHASH_CHECK(fArray[i] != fArray[j]);
                SKTDYNAMICHASH_CHECK(!Equal(*fArray[i], GetKey(*fArray[j])));
                SKTDYNAMICHASH_CHECK(!Equal(*fArray[j], GetKey(*fArray[i])));
            }
        }
        #undef SKTDYNAMICHASH_CHECK
        return true;
    }

    void innerAdd(T* newEntry) {
        const Key& key = GetKey(*newEntry);
        int index = this->firstIndex(key);
        for (int round = 0; round < fCapacity; round++) {
            const T* candidate = fArray[index];
            if (Empty() == candidate || Deleted() == candidate) {
                if (Deleted() == candidate) {
                    fDeleted--;
                }
                fCount++;
                fArray[index] = newEntry;
                return;
            }
            index = this->nextIndex(index, round);
        }
        SkASSERT(0); // add: should be unreachable
    }

    void innerRemove(const Key& key) {
        const int firstIndex = this->firstIndex(key);
        int index = firstIndex;
        for (int round = 0; round < fCapacity; round++) {
            const T* candidate = fArray[index];
            if (Deleted() != candidate && Equal(*candidate, key)) {
                fDeleted++;
                fCount--;
                fArray[index] = Deleted();
                return;
            }
            index = this->nextIndex(index, round);
        }
        SkASSERT(0); // innerRemove: should be unreachable
    }

    void maybeGrow() {
        if (fCount + fDeleted + 1 > (fCapacity * kGrowPercent) / 100) {
            resize(fCapacity * 2);
        }
    }

    void maybeShrink() {
        if (fCount < (fCapacity * kShrinkPercent) / 100 && fCapacity / 2 > kMinCapacity) {
            resize(fCapacity / 2);
        }
    }

    void resize(int newCapacity) {
        SkDEBUGCODE(int oldCount = fCount;)
        int oldCapacity = fCapacity;
        T** oldArray = fArray;

        reset(newCapacity);

        for (int i = 0; i < oldCapacity; i++) {
            T* entry = oldArray[i];
            if (Empty() != entry && Deleted() != entry) {
                this->add(entry);
            }
        }
        SkASSERT(oldCount == fCount);

        sk_free(oldArray);
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

    int fCount;     // Number of non Empty(), non Deleted() entries in fArray.
    int fDeleted;   // Number of Deleted() entries in fArray.
    int fCapacity;  // Number of entries in fArray.  Always a power of 2.
    T** fArray;
};

#endif
