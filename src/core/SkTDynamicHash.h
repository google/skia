/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkTDynamicHash_DEFINED
#define SkTDynamicHash_DEFINED

// This is now a simple API wrapper around SkTHashTable<T*>;
// please just use SkTHash{Map,Set,Table} directly for new code.
#include "include/private/SkTHash.h"

// Traits requires:
//   static const Key& GetKey(const T&) { ... }
//   static uint32_t Hash(const Key&) { ... }
// We'll look on T for these by default, or you can pass a custom Traits type.
template <typename T,
          typename Key,
          typename Traits = T>
class SkTDynamicHash {
public:
    SkTDynamicHash() {}

    // It is not safe to call set() or remove() while iterating with either foreach().
    // If you mutate the entries be very careful not to change the Key.

    template <typename Fn>  // f(T*)
    void foreach(Fn&& fn) {
        fTable.foreach([&](T** entry) { fn(*entry); });
    }
    template <typename Fn>  // f(T) or f(const T&)
    void foreach(Fn&& fn) const {
        fTable.foreach([&](T* entry) { fn(*entry); });
    }

    int count() const { return fTable.count(); }

    T* find(const Key& key) const { return fTable.findOrNull(key); }

    void add(T* entry) { fTable.set(entry); }
    void remove(const Key& key) { fTable.remove(key); }

    void rewind() { fTable.reset(); }
    void reset () { fTable.reset(); }

private:
    struct AdaptedTraits {
        static const Key& GetKey(T* entry) { return Traits::GetKey(*entry); }
        static uint32_t Hash(const Key& key) { return Traits::Hash(key); }
    };
    SkTHashTable<T*, Key, AdaptedTraits> fTable;
};

#endif
