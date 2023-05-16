/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrHashMapWithCache_DEFINED
#define GrHashMapWithCache_DEFINED

#include "include/private/base/SkNoncopyable.h"
#include "src/core/SkChecksum.h"
#include "src/core/SkTHash.h"

// Cheaper than SkGoodHash and good enough for UniqueID tables.
struct GrCheapHash {
    uint32_t operator()(uint32_t val) {
        return SkChecksum::CheapMix(val);
    }
};

/** A hash map that caches the most recently accessed entry.
    The API is a subset of SkHashMap, and you must provide a
    sentinel key that will never be present, such as SK_InvalidUniqueID.

    KeyTraits must have:
      - static K GetInvalidKey()
*/
template <typename K, typename V, typename KeyTraits, typename HashT = SkGoodHash>
class GrHashMapWithCache : public SkNoncopyable {
public:
    // How many key/value pairs are in the table?
    int count() const { return fMap.count(); }

    // Approximately how many bytes of memory do we use beyond sizeof(*this)?
    size_t approxBytesUsed() const { return fMap.approxBytesUsed(); }

    // N.B. The pointers returned by set() and find() are valid only until the next call to set().

    // If there is key/value entry in the table with this key, return a pointer to the value.
    // If not, return null.
    const V* find(const K& key) const {
        if (key != fLastKey) {
            fLastKey = key;
            fLastValue = fMap.find(key);
        }
        return fLastValue;
    }

    // Set key to val in the map, replacing any previous value with the same key.
    // We copy both key and val, and return a pointer to the value copy now in the map.
    const V* set(K key, V val) {
        if (fLastValue && key == fLastKey) {
            *fLastValue = std::move(val);
        } else {
            fLastKey = key;
            fLastValue = fMap.set(std::move(key), std::move(val));
        }
        return fLastValue;
    }

    // Remove the key/value entry in the table with this key.
    void remove(K key) {
        // Match THashMap requirement. The caller can find() if they're unsure.
        SkASSERT(fMap.find(fLastKey));
        fLastKey = std::move(key);
        fLastValue = nullptr;
        fMap.remove(fLastKey);
    }

    // Clear the map.
    void reset() {
        fLastKey = KeyTraits::GetInvalidKey();
        fLastValue = nullptr;
        fMap.reset();
    }

private:
    skia_private::THashMap<K, V, HashT> fMap;
    mutable K                           fLastKey   = KeyTraits::GetInvalidKey();
    mutable V*                          fLastValue = nullptr;
};

#endif
