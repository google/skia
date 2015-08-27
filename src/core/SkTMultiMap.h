
/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkTMultiMap_DEFINED
#define SkTMultiMap_DEFINED

#include "GrTypes.h"
#include "SkTDynamicHash.h"

/** A set that contains pointers to instances of T. Instances can be looked up with key Key.
 * Multiple (possibly same) values can have the same key.
 */
template <typename T,
          typename Key,
          typename HashTraits=T>
class SkTMultiMap {
    struct ValueList {
        explicit ValueList(T* value) : fValue(value), fNext(nullptr) {}

        static const Key& GetKey(const ValueList& e) { return HashTraits::GetKey(*e.fValue); }
        static uint32_t Hash(const Key& key) { return HashTraits::Hash(key); }
        T* fValue;
        ValueList* fNext;
    };
public:
    SkTMultiMap() : fCount(0) {}

    ~SkTMultiMap() {
        SkASSERT(fCount == 0);
        SkASSERT(fHash.count() == 0);
    }

    void insert(const Key& key, T* value) {
        ValueList* list = fHash.find(key);
        if (list) {
            // The new ValueList entry is inserted as the second element in the
            // linked list, and it will contain the value of the first element.
            ValueList* newEntry = new ValueList(list->fValue);
            newEntry->fNext = list->fNext;
            // The existing first ValueList entry is updated to contain the
            // inserted value.
            list->fNext = newEntry;
            list->fValue = value;
        } else {
            fHash.add(new ValueList(value));
        }

        ++fCount;
    }

    void remove(const Key& key, const T* value) {
        ValueList* list = fHash.find(key);
        // Since we expect the caller to be fully aware of what is stored, just
        // assert that the caller removes an existing value.
        SkASSERT(list);
        ValueList* prev = nullptr;
        while (list->fValue != value) {
            prev = list;
            list = list->fNext;
        }

        if (list->fNext) {
            ValueList* next = list->fNext;
            list->fValue = next->fValue;
            list->fNext = next->fNext;
            delete next;
        } else if (prev) {
            prev->fNext = nullptr;
            delete list;
        } else {
            fHash.remove(key);
            delete list;
        }

        --fCount;
    }

    T* find(const Key& key) const {
        ValueList* list = fHash.find(key);
        if (list) {
            return list->fValue;
        }
        return nullptr;
    }

    template<class FindPredicate>
    T* find(const Key& key, const FindPredicate f) {
        ValueList* list = fHash.find(key);
        while (list) {
            if (f(list->fValue)){
                return list->fValue;
            }
            list = list->fNext;
        }
        return nullptr;
    }

    int count() const { return fCount; }

#ifdef SK_DEBUG
    // This is not particularly fast and only used for validation, so debug only.
    int countForKey(const Key& key) const {
        int count = 0;
        ValueList* list = fHash.find(key);
        while (list) {
            list = list->fNext;
            ++count;
        }
        return count;
    }
#endif

private:
    SkTDynamicHash<ValueList, Key> fHash;
    int fCount;
};

#endif
