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
    void operator()(void* /* context */, const K& /* k */, const V* /* v */) const {}
};

/**
 * A generic LRU cache.
 */
template <typename K, typename V, typename HashK = SkGoodHash, typename PurgeCB = SkNoOpPurge>
class SkLRUCache {
private:
    struct Entry {
        template<typename K1, typename V1>
        Entry(K1&& key, V1&& value)
            : fKey(std::forward<K1>(key))
            , fValue(std::forward<V1>(value)) {}

        const K fKey;
        V fValue;

        SK_DECLARE_INTERNAL_LLIST_INTERFACE(Entry);
    };

public:
    explicit SkLRUCache(int maxCount, void* context = nullptr)
            : fMaxCount(maxCount)
            , fContext(context) {}
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

    V* insert(Entry* entry) {
        fMap.set(entry);
        fLRU.addToHead(entry);
        while (fMap.count() > fMaxCount) {
            this->remove(fLRU.tail()->fKey);
        }
        return &entry->fValue;
    }

    template<typename K1, typename V1>
    V* insert(K1&& key, V1&& value) {
        SkASSERT(!this->find(key));
        return this->insert(new Entry(std::forward<K1>(key), std::forward<V1>(value)));
    }

    template<typename K1, typename V1>
    V* insert_or_update(K1&& key, V1&& value) {
        if (V* found = this->find(key)) {
            *found = std::forward<V1>(value);
            return found;
        }
        return this->insert(new Entry(std::forward<K1>(key), std::forward<V1>(value)));
    }

    int count() const {
        return fMap.count();
    }

    template <typename Fn>  // f(const K*, V*)
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
        PurgeCB()(fContext, key, &entry->fValue);
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
    void*                                       fContext;
};

#endif
