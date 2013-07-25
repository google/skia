/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkTDynamicHash_DEFINED
#define SkTDynamicHash_DEFINED

#include "SkTypes.h"

template <typename T, typename KEY, const KEY& (KEY_FROM_T)(const T&),
uint32_t (HASH_FROM_KEY)(const Key&), bool (EQ_T_KEY)(const T&, const Key&)>
class SkTDynamicHash {
private:
    T* fStorage[1];
    T** fArray;
    int fCapacity;
    int fCountUsed;
    int fCountDeleted;
    
    unsigned hashMask() const { return fCapacity - 1; }
    const T* deletedValue() const { return reinterpret_cast<const T*>(-1); }
    
public:
    SkTDynamicHash() {
        fArray = fStorage;
        fCapacity = 1;
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
        int index = HASH_FROM_KEY(key) & mask;
        const int origIndex = index;
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
        } while (index != origIndex);
        return NULL;
    }
    
    bool add(const KEY& key, T* newElement, bool autoGrow = true) {
        const T* const deleted = this->deletedValue();
        for (;;) {
            const unsigned mask = this->hashMask();
            int index = HASH_FROM_KEY(key) & mask;
            const int origIndex = index;
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
            } while (index != origIndex);
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
        const uint32_t hash = HASH_FROM_KEY(key);
        int index = hash & mask;
        SkDEBUGCODE(const int origIndex = index;)
        int delta = 1;
        
        for (;;) {
            const T* candidate = fArray[index];
            SkASSERT(candidate);
            if (deleted != candidate && EQ_T_KEY(*candidate, key)) {
                fArray[index] = const_cast<T*>(deleted);
                fCountDeleted += 1;
                break;
            }
            index = (index + delta) & mask;
            delta <<= 1;
            SkASSERT(index != origIndex);
        }
        this->checkStrink();
    }
    
private:
    void grow() {
        const T* const deleted = this->deletedValue();
        
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
        sk_free(oldArray);
    }
    
    void checkStrink() {}
};

#endif
