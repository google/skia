/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkLRUCache_DEFINED
#define SkLRUCache_DEFINED

#include "SkChecksum.h"
#include "SkTDynamicHash.h"
#include "SkTInternalLList.h"

/**
 * A generic LRU cache.
 */
template <typename K, typename V, typename HashK = SkGoodHash>
class SkLRUCache : public SkNoncopyable {
public:
    SkLRUCache(size_t maxSize)
    : fMaxSize(maxSize) {}

    ~SkLRUCache() {
        while (fMap.count()) {
            remove(fLRU.tail()->fKey);
        }
    }

    V* find(K key) {
        Entry* result = fMap.find(key);
        if (!result) {
            return nullptr;
        }
        if (result != fLRU.head()) {
            fLRU.remove(result);
            fLRU.addToHead(result);
        }
        return result->fValue.get();
    }

    void insert(K key, std::unique_ptr<V> value) {
        Entry* entry = new Entry(key, std::move(value));
        fMap.add(entry);
        fLRU.addToHead(entry);
        while ((size_t) fMap.count() > fMaxSize) {
            remove(fLRU.tail()->fKey);
        }
    }

private:
    struct Entry {
        Entry(K key, std::unique_ptr<V> value)
        : fKey(key)
        , fValue(std::move(value)) {}

        static const K& GetKey(const Entry& e) {
            return e.fKey;
        }

        static uint32_t Hash(const K& key) {
            return HashK()(key);
        }

        const K fKey;
        std::unique_ptr<V> fValue;
        SK_DECLARE_INTERNAL_LLIST_INTERFACE(Entry);
    };

    void remove(K key) {
        Entry* value = fMap.find(key);
        if (value) {
            fMap.remove(value->fKey);
            fLRU.remove(value);
            delete value;
        }
    }

    size_t                   fMaxSize;
    SkTDynamicHash<Entry, K> fMap;
    SkTInternalLList<Entry>  fLRU;
};

#endif
