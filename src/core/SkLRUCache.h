/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkLRUCache_DEFINED
#define SkLRUCache_DEFINED

#include "SkChecksum.h"
#include "SkTHash.h"
#include "SkTInternalLList.h"

/**
 * A generic LRU cache.
 */
template <typename K, typename V, typename HashK = SkGoodHash>
class SkLRUCache : public SkNoncopyable {
public:
    SkLRUCache(size_t maxCount)
    : fMaxCount(maxCount) {}

    ~SkLRUCache() {
        while (fMap.count()) {
            remove(fLRU.tail()->fKey);
        }
    }

    V* find(K key) {
        Entry** value = fMap.find(key);
        if (!value) {
            return nullptr;
        }
        Entry* entry = *value;
        if (entry != fLRU.head()) {
            fLRU.remove(entry);
            fLRU.addToHead(entry);
        }
        return entry->fValue.get();
    }

    void insert(K key, std::unique_ptr<V> value) {
        Entry* entry = new Entry(key, std::move(value));
        fMap.set(key, entry);
        fLRU.addToHead(entry);
        while ((size_t) fMap.count() > fMaxCount) {
            remove(fLRU.tail()->fKey);
        }
    }

    size_t count() {
        return fMap.count();
    }

private:
    struct Entry {
        Entry(K key, std::unique_ptr<V> value)
        : fKey(key)
        , fValue(std::move(value)) {}

        K                  fKey;
        std::unique_ptr<V> fValue;

        SK_DECLARE_INTERNAL_LLIST_INTERFACE(Entry);
    };

    void remove(K key) {
        Entry** value = fMap.find(key);
        SkASSERT(value);
        Entry* entry = *value;
        SkASSERT(key == entry->fKey);
        fMap.remove(key);
        fLRU.remove(entry);
        delete entry;
    }

    size_t                       fMaxCount;
    SkTHashMap<K, Entry*, HashK> fMap;
    SkTInternalLList<Entry>      fLRU;
};

#endif
