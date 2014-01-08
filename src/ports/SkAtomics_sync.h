/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkAtomics_sync_DEFINED
#define SkAtomics_sync_DEFINED

/** GCC/Clang __sync based atomics. */

#include <stdint.h>

static inline __attribute__((always_inline)) int32_t sk_atomic_inc(int32_t* addr) {
    return __sync_fetch_and_add(addr, 1);
}

static inline __attribute__((always_inline)) int32_t sk_atomic_add(int32_t* addr, int32_t inc) {
    return __sync_fetch_and_add(addr, inc);
}

static inline __attribute__((always_inline)) int32_t sk_atomic_dec(int32_t* addr) {
    return __sync_fetch_and_add(addr, -1);
}

static inline __attribute__((always_inline)) void sk_membar_acquire__after_atomic_dec() { }

static inline __attribute__((always_inline)) int32_t sk_atomic_conditional_inc(int32_t* addr) {
    int32_t value = *addr;

    while (true) {
        if (value == 0) {
            return 0;
        }

        int32_t before = __sync_val_compare_and_swap(addr, value, value + 1);

        if (before == value) {
            return value;
        } else {
            value = before;
        }
    }
}

static inline __attribute__((always_inline)) bool sk_atomic_cas(int32_t* addr,
                                                                int32_t before,
                                                                int32_t after) {
    return __sync_bool_compare_and_swap(addr, before, after);
}

static inline __attribute__((always_inline)) void sk_membar_acquire__after_atomic_conditional_inc() { }

#endif
