/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrAffinedStorage_DEFINED
#define GrAffinedStorage_DEFINED

#include "include/private/SkNoncopyable.h"

#include <atomic>

/** A container that can be owned by one thread-affined object at a time and used
    for a while before being released then potentially picked up by another owner.
    For example, if you need temporary storage that may be shared by many threads in some
    use cases, but not shared at all in other use cases, you can use this class for one
    immediately-available storage and fall back to slower storage when it's occupied.
*/
template <typename O, typename V>
class GrAffinedStorage : public SkNoncopyable {
public:
    GrAffinedStorage(V val) : fValue(std::move(val)) {}

    // Attempt to take ownership and set the value.
    // Returns whether the operation was successful.
    bool set(O* owner, V value) {
        const O* oldOwner = nullptr;
        bool took = fOwner.compare_exchange_strong(oldOwner, owner, std::memory_order_acquire);
        if (took || oldOwner == owner) {
            fValue = std::move(value);
            return true;
        }
        return false;
    }

    // Attempt to access the internal storage for reading.
    // Returns nullptr if not owned by the given owner.
    const V* get(const O* owner) const {
        if (owner == fOwner.load(std::memory_order_relaxed)) {
            return &fValue;
        }
        return nullptr;
    }

    // Attempt to release ownership. Does nothing if not owned by the given owner.
    bool release(const O* owner) {
        return fOwner.compare_exchange_strong(owner, nullptr, std::memory_order_acquire);
    }

private:
    std::atomic<const O*> fOwner{nullptr};
    V                     fValue;
};

#endif
