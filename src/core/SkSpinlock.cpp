/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/SkSpinlock.h"

// Renamed from "pause" to avoid conflict with function defined in unistd.h
#if SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSE2
    #include <emmintrin.h>
    static void do_pause() { _mm_pause(); }
#else
    static void do_pause() { /*spin*/ }
#endif

void SkSpinlock::contendedAcquire() {
    // To act as a mutex, we need an acquire barrier when we acquire the lock.
    while (fLocked.exchange(true, std::memory_order_acquire)) {
        do_pause();
    }
}
