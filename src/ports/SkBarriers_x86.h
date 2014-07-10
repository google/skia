/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkBarriers_x86_DEFINED
#define SkBarriers_x86_DEFINED

#ifdef SK_BUILD_FOR_WIN
#  include <intrin.h>
static inline void sk_compiler_barrier() { _ReadWriteBarrier(); }
#else
static inline void sk_compiler_barrier() { asm volatile("" : : : "memory"); }
#endif

template <typename T>
T sk_acquire_load(T* ptr) {
    T val = *ptr;
    // On x86, all loads are acquire loads, so we only need a compiler barrier.
    sk_compiler_barrier();
    return val;
}

template <typename T>
T sk_consume_load(T* ptr) {
    // On x86, consume is the same as acquire, i.e. a normal load.
    return sk_acquire_load(ptr);
}

template <typename T>
void sk_release_store(T* ptr, T val) {
    // On x86, all stores are release stores, so we only need a compiler barrier.
    sk_compiler_barrier();
    *ptr = val;
}

#endif//SkBarriers_x86_DEFINED
