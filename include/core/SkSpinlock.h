/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// This file is not part of the public Skia API.

#ifndef SkSpinlock_DEFINED
#define SkSpinlock_DEFINED

#include "SkAtomics.h"

#define SK_DECLARE_STATIC_SPINLOCK(name) namespace {} static SkPODSpinlock name

// This class has no constructor and must be zero-initialized (the macro above does this).
class SK_API SkPODSpinlock {
public:
    void acquire() {
        // To act as a mutex, we need an acquire barrier if we take the lock.
        if (sk_atomic_exchange(&fLocked, true, sk_memory_order_acquire)) {
            // Lock was contended.  Fall back to an out-of-line spin loop.
            this->contendedAcquire();
        }
    }

    void release() {
        // To act as a mutex, we need a release barrier.
        sk_atomic_store(&fLocked, false, sk_memory_order_release);
    }

private:
    void contendedAcquire();
    bool fLocked;
};

// For non-global-static use cases, this is normally what you want.
class SkSpinlock : public SkPODSpinlock {
public:
    SkSpinlock() { this->release(); }
};

#endif//SkSpinlock_DEFINED
