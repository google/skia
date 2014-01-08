/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkAtomics_none_DEFINED
#define SkAtomics_none_DEFINED

/** Non-atomic atomics for uniprocessor systems. */

#include <stdint.h>

static inline int32_t sk_atomic_inc(int32_t* addr) {
    int32_t value = *addr;
    *addr = value + 1;
    return value;
}

static inline int32_t sk_atomic_add(int32_t* addr, int32_t inc) {
    int32_t value = *addr;
    *addr = value + inc;
    return value;
}

static inline int32_t sk_atomic_dec(int32_t* addr) {
    int32_t value = *addr;
    *addr = value - 1;
    return value;
}

static inline void sk_membar_acquire__after_atomic_dec() { }

static inline int32_t sk_atomic_conditional_inc(int32_t* addr) {
    int32_t value = *addr;
    if (value != 0) ++*addr;
    return value;
}

static inline bool sk_atomic_cas(int32_t* addr, int32_t before, int32_t after) {
    if (*addr != before) return false;
    *addr = after;
    return true;
}

static inline void sk_membar_acquire__after_atomic_conditional_inc() { }

#endif
