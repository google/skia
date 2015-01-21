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

static inline __attribute__((always_inline)) int64_t sk_atomic_inc(int64_t* addr) {
#if defined(__mips__) && !defined(__LP64__) && !defined(__GCC_HAVE_SYNC_COMPARE_AND_SWAP_8)
    /** Some versions of the GCC 32-bit MIPS toolchains (e.g. 4.8) for android are missing
     * support for the __sync* functions that operate on 64-bit values. The workaround
     * is to use __atomic* functions until we can move everything to <stdatomic.h>.
     */
    return __atomic_fetch_add(addr, 1, __ATOMIC_SEQ_CST);
#else
    return __sync_fetch_and_add(addr, 1);
#endif
}

static inline __attribute__((always_inline)) int32_t sk_atomic_add(int32_t* addr, int32_t inc) {
    return __sync_fetch_and_add(addr, inc);
}

static inline __attribute__((always_inline)) int32_t sk_atomic_dec(int32_t* addr) {
    return __sync_fetch_and_add(addr, -1);
}

static inline __attribute__((always_inline)) void sk_membar_acquire__after_atomic_dec() { }

static inline __attribute__((always_inline)) bool sk_atomic_cas(int32_t* addr,
                                                                int32_t before,
                                                                int32_t after) {
    return __sync_bool_compare_and_swap(addr, before, after);
}

static inline __attribute__((always_inline)) void* sk_atomic_cas(void** addr,
                                                                 void* before,
                                                                 void* after) {
    return __sync_val_compare_and_swap(addr, before, after);
}

static inline __attribute__((always_inline)) void sk_membar_acquire__after_atomic_conditional_inc() { }

#endif
