/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkTDynamicHash_DEFINED
#define SkTDynamicHash_DEFINED

#include "SkTypes.h"

template <typename T,
          typename KEY,
          const KEY& (KEY_FROM_T)(const T&),
          uint32_t (HASH_FROM_KEY)(const KEY&),
          bool (EQ_T_KEY)(const T&, const KEY&)>
class SkTDynamicHash {
private:
    T* fStorage[4];     // cheap storage for small arrays
    T** fArray;
    int fCapacity;      // number of slots in fArray. Must be pow2
    int fCountUsed;     // number of valid entries in fArray
    int fCountDeleted;  // number of deletedValue() entries in fArray

    // Need an illegal ptr value different from NULL (which we use to
    // signal empty/unused.
    const T* deletedValue() const { return reinterpret_cast<const T*>(-1); }

    // fCapacity is a pow2, so that minus one is a clean mask to grab
    // the low bits of hash to use as an index.
    uint32_t hashMask() const { return fCapacity - 1; }

    int hashToIndex(uint32_t hash) const {
        // this 16bit fold may be overkill, if we trust that hash is good
        return ((hash >> 16) ^ hash) & this->hashMask();
    }

public:
    SkTDynamicHash() {
        sk_bzero(fStorage, sizeof(fStorage));
        fArray = fStorage;
        fCapacity = SK_ARRAY_COUNT(fStorage);
        fCountUsed = fCountDeleted = 0;
    }
    ~SkTDynamicHash() {
        if (fArray != fStorage) {
            sk_free(fArray);
        }
    }

    T* find(const KEY& key) {
        const T* const deleted = this->deletedValue();
        const unsigned mask = this->hashMask();
        int index = this->hashToIndex(HASH_FROM_KEY(key));
        int delta = 1;

        do {
            T* candidate = fArray[index];
            if (NULL == candidate) {
                return NULL;
            }
            if (deleted != candidate && EQ_T_KEY(*candidate, key)) {
                return candidate;
            }
            index = (index + delta) & mask;
            delta <<= 1;
        } while (delta <= fCapacity);
        return NULL;
    }

    bool add(const KEY& key, T* newElement, bool autoGrow = true) {
        const T* const deleted = this->deletedValue();
        for (;;) {
            const unsigned mask = this->hashMask();
            int index = this->hashToIndex(HASH_FROM_KEY(key));
            int delta = 1;

            do {
                const T* candidate = fArray[index];
                if (NULL == candidate || deleted == candidate) {
                    fArray[index] = newElement;
                    fCountUsed += 1;
                    if (deleted == candidate) {
                        SkASSERT(fCountDeleted > 0);
                        fCountDeleted -= 1;
                    }
                    return true;
                }
                index = (index + delta) & mask;
                delta <<= 1;
            } while (delta <= fCapacity);
            if (autoGrow) {
                this->grow();
            } else {
                return false;
            }
        }
        SkASSERT(!"never get here");
        return false;
    }

    void remove(const KEY& key) {
        const T* const deleted = this->deletedValue();
        const unsigned mask = this->hashMask();
        int index = this->hashToIndex(HASH_FROM_KEY(key));
        int delta = 1;

        for (;;) {
            const T* candidate = fArray[index];
            SkASSERT(candidate);
            if (deleted != candidate && EQ_T_KEY(*candidate, key)) {
                fArray[index] = const_cast<T*>(deleted);
                fCountUsed -= 1;
                fCountDeleted += 1;
                break;
            }
            index = (index + delta) & mask;
            delta <<= 1;
            SkASSERT(delta <= fCapacity);
        }
        this->checkStrink();
    }

private:
    int countCollisions(const KEY& key) const {
        const T* const deleted = this->deletedValue();
        const unsigned mask = this->hashMask();
        int index = this->hashToIndex(HASH_FROM_KEY(key));
        int delta = 1;
        int collisionCount = 0;

        for (;;) {
            const T* candidate = fArray[index];
            SkASSERT(candidate);
            if (deleted != candidate && EQ_T_KEY(*candidate, key)) {
                break;
            }
            index = (index + delta) & mask;
            delta <<= 1;
            collisionCount += 1;
            SkASSERT(delta <= fCapacity);
        }
        return collisionCount;
    }

    void grow() {
        const T* const deleted = this->deletedValue();
#if 0
        SkDebugf("growing from %d: used=%d\n", fCapacity, fCountUsed);
        for (int i = 0; i < fCapacity; ++i) {
            T* elem = fArray[i];
            if (NULL == elem || deleted == elem) {
                continue;
            }
            SkDebugf("    entry[%d] had %d collisions\n", i, countCollisions(KEY_FROM_T(*elem)));
        }
#endif
        int oldCapacity = fCapacity;
        T** oldArray = fArray;

        int newCapacity = oldCapacity << 1;
        T** newArray = (T**)sk_malloc_throw(sizeof(T*) * newCapacity);
        sk_bzero(newArray, sizeof(T*) * newCapacity);

        SkDEBUGCODE(int oldCountUsed = fCountUsed;)
        fArray = newArray;
        fCapacity = newCapacity;
        fCountUsed = 0;
        fCountDeleted = 0;

        for (int i = 0; i < oldCapacity; ++i) {
            T* elem = oldArray[i];
            if (NULL == elem || deleted == elem) {
                continue;
            }
            SkDEBUGCODE(bool success =) this->add(KEY_FROM_T(*elem), elem, false);
            SkASSERT(success);
        }
        SkASSERT(oldCountUsed == fCountUsed);

        if (oldArray != fStorage) {
            sk_free(oldArray);
        }
    }

    void checkStrink() {
        // todo: based on density and deadspace (fCountDeleted), consider
        // shrinking fArray and repopulating it.
    }
};

#endif
