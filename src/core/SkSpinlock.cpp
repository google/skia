/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/SkSpinlock.h"

#if SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSE2
    #include <emmintrin.h>
    static void pause() { _mm_pause(); }
#else
    static void pause() { /*spin*/ }
#endif

void SkSpinlock::contendedAcquire() {
    // To act as a mutex, we need an acquire barrier when we acquire the lock.
    while (fLocked.exchange(true, std::memory_order_acquire)) {
        pause();
    }
}
