/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkBarriers_tsan_DEFINED
#define SkBarriers_tsan_DEFINED

static inline void sk_compiler_barrier() { asm volatile("" : : : "memory"); }

template <typename T>
T sk_acquire_load(T* ptr) {
    SkASSERT(__atomic_always_lock_free(sizeof(T), ptr));
    return __atomic_load_n(ptr, __ATOMIC_ACQUIRE);
}

template <typename T>
T sk_consume_load(T* ptr) {
    SkASSERT(__atomic_always_lock_free(sizeof(T), ptr));
    return __atomic_load_n(ptr, __ATOMIC_CONSUME);
}

template <typename T>
void sk_release_store(T* ptr, T val) {
    SkASSERT(__atomic_always_lock_free(sizeof(T), ptr));
    return __atomic_store_n(ptr, val, __ATOMIC_RELEASE);
}

#endif//SkBarriers_tsan_DEFINED
