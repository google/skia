/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/base/SkSpinlock.h"

#include "include/private/base/SkFeatures.h"
#include "include/private/base/SkThreadAnnotations.h"

#if 0
    #include "include/private/base/SkMutex.h"
    #include <execinfo.h>
    #include <stdio.h>

    static void debug_trace() {
        void* stack[64];
        int len = backtrace(stack, std::size(stack));

        // As you might imagine, we can't use an SkSpinlock here...
        static SkMutex lock;
        {
            SkAutoMutexExclusive locked(lock);
            fprintf(stderr, "\n");
            backtrace_symbols_fd(stack, len, 2/*stderr*/);
            fprintf(stderr, "\n");
        }
    }
#else
    static void debug_trace() {}
#endif

// Renamed from "pause" to avoid conflict with function defined in unistd.h
#if SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSE2
    #include <emmintrin.h>
    static void do_pause() { _mm_pause(); }
#else
    static void do_pause() { /*spin*/ }
#endif

void SkSpinlock::contendedAcquire() {
    debug_trace();

    // To act as a mutex, we need an acquire barrier when we acquire the lock.
    SK_POTENTIALLY_BLOCKING_REGION_BEGIN;
    while (fLocked.exchange(true, std::memory_order_acquire)) {
        do_pause();
    }
    SK_POTENTIALLY_BLOCKING_REGION_END;
}
