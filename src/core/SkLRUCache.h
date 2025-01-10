/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkLRUCache_DEFINED
#define SkLRUCache_DEFINED

#include "src/base/SkTInternalLList.h"
#include "src/core/SkChecksum.h"
#include "src/core/SkTHash.h"

struct SkNoOpPurge {
    template <typename K, typename V>
    void operator()(const K& /* k */, const V* /* v */) const {}
};

/**
 * A generic LRU cache.
 */
template <typename K, typename V, typename HashK = SkGoodHash, typename PurgeCB = SkNoOpPurge>
class SkLRUCache {
private:
    struct Entry {
        Entry(const K& key, V&& value)
            : fKey(key)
            , fValue(std::move(value)) {}

        K fKey;
        V fValue;

        SK_DECLARE_INTERNAL_LLIST_INTERFACE(Entry);
    };

public:
    explicit SkLRUCache(int maxCount) : fMaxCount(maxCount) {}
    SkLRUCache() = delete;

    ~SkLRUCache() {
        Entry* node = fLRU.head();
        while (node) {
            fLRU.remove(node);
            delete node;
            node = fLRU.head();
        }
    }

    // Make noncopyable
    SkLRUCache(const SkLRUCache&) = delete;
    SkLRUCache& operator=(const SkLRUCache&) = delete;

    V* find(const K& key) {
        Entry** value = fMap.find(key);
        if (!value) {
            return nullptr;
        }
        Entry* entry = *value;
        if (entry != fLRU.head()) {
            fLRU.remove(entry);
            fLRU.addToHead(entry);
        } // else it's already at head position, don't need to do anything
        return &entry->fValue;
    }

    V* insert(const K& key, V value) {
        SkASSERT(!this->find(key));

        Entry* entry = new Entry(key, std::move(value));
        fMap.set(entry);
        fLRU.addToHead(entry);
        while (fMap.count() > fMaxCount) {
            this->remove(fLRU.tail()->fKey);
        }
        return &entry->fValue;
    }

    V* insert_or_update(const K& key, V value) {
        if (V* found = this->find(key)) {
            *found = std::move(value);
            return found;
        } else {
            return this->insert(key, std::move(value));
        }
    }

    int count() const {
        return fMap.count();
    }

    template <typename Fn>  // f(K*, V*)
    void foreach(Fn&& fn) {
        typename SkTInternalLList<Entry>::Iter iter;
        for (Entry* e = iter.init(fLRU, SkTInternalLList<Entry>::Iter::kHead_IterStart); e;
             e = iter.next()) {
            fn(&e->fKey, &e->fValue);
        }
    }

    void reset() {
        fMap.reset();
        for (Entry* e = fLRU.head(); e; e = fLRU.head()) {
            fLRU.remove(e);
            delete e;
        }
    }

    void remove(const K& key) {
        Entry** value = fMap.find(key);
        SkASSERT(value);
        Entry* entry = *value;
        SkASSERT(key == entry->fKey);
        PurgeCB()(key, &entry->fValue);
        fMap.remove(key);
        fLRU.remove(entry);
        delete entry;
    }

private:
    struct Traits {
        static const K& GetKey(Entry* e) {
            return e->fKey;
        }

        static uint32_t Hash(const K& k) {
            return HashK()(k);
        }
    };

    int                                         fMaxCount;
    skia_private::THashTable<Entry*, K, Traits> fMap;
    SkTInternalLList<Entry>                     fLRU;
};

#endif
