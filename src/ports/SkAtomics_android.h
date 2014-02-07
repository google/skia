/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkAtomics_android_DEFINED
#define SkAtomics_android_DEFINED

/** Android framework atomics. */

#include <cutils/atomic.h>
#include <stdint.h>

static inline __attribute__((always_inline)) int32_t sk_atomic_inc(int32_t* addr) {
    return android_atomic_inc(addr);
}

static inline __attribute__((always_inline)) int32_t sk_atomic_add(int32_t* addr, int32_t inc) {
    return android_atomic_add(inc, addr);
}

static inline __attribute__((always_inline)) int32_t sk_atomic_dec(int32_t* addr) {
    return android_atomic_dec(addr);
}

static inline __attribute__((always_inline)) void sk_membar_acquire__after_atomic_dec() {
    //HACK: Android is actually using full memory barriers.
    //      Should this change, uncomment below.
    //int dummy;
    //android_atomic_acquire_store(0, &dummy);
}

static inline __attribute__((always_inline)) int32_t sk_atomic_conditional_inc(int32_t* addr) {
    while (true) {
        int32_t value = *addr;
        if (value == 0) {
            return 0;
        }
        if (0 == android_atomic_release_cas(value, value + 1, addr)) {
            return value;
        }
    }
}

static inline __attribute__((always_inline)) bool sk_atomic_cas(int32_t* addr,
                                                                 int32_t before,
                                                                 int32_t after) {
    // android_atomic_release_cas returns 0 for success (if *addr == before and it wrote after).
    return android_atomic_release_cas(before, after, addr) == 0;
}

static inline __attribute__((always_inline)) void sk_membar_acquire__after_atomic_conditional_inc() {
    //HACK: Android is actually using full memory barriers.
    //      Should this change, uncomment below.
    //int dummy;
    //android_atomic_acquire_store(0, &dummy);
}

#endif
