/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSpinlock_DEFINED
#define SkSpinlock_DEFINED

#include "SkTypes.h"
#include <atomic>

class SkSpinlock {
public:
    constexpr SkSpinlock() = default;

    void acquire() {
        // To act as a mutex, we need an acquire barrier when we acquire the lock.
        if (fLocked.exchange(true, std::memory_order_acquire)) {
            // Lock was contended.  Fall back to an out-of-line spin loop.
            this->contendedAcquire();
        }
    }

    // Acquire the lock or fail (quickly). Lets the caller decide to do something other than wait.
    bool tryAcquire() {
        // To act as a mutex, we need an acquire barrier when we acquire the lock.
        if (fLocked.exchange(true, std::memory_order_acquire)) {
            // Lock was contended. Let the caller decide what to do.
            return false;
        }
        return true;
    }

    void release() {
        // To act as a mutex, we need a release barrier when we release the lock.
        fLocked.store(false, std::memory_order_release);
    }

private:
    SK_API void contendedAcquire();

    std::atomic<bool> fLocked{false};
};

#endif//SkSpinlock_DEFINED
