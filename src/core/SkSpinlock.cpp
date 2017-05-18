/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkSpinlock.h"

void SkSpinlock::contendedAcquire() {
    // To act as a mutex, we need an acquire barrier when we acquire the lock.
    while (fLocked.exchange(true, std::memory_order_acquire)) {
        /*spin*/
    }
}
