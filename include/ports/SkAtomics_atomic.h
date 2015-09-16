/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkAtomics_atomic_DEFINED
#define SkAtomics_atomic_DEFINED

template <typename T>
T sk_atomic_load(const T* ptr, sk_memory_order mo) {
    SkASSERT(mo == sk_memory_order_relaxed ||
             mo == sk_memory_order_seq_cst ||
             mo == sk_memory_order_acquire ||
             mo == sk_memory_order_consume);
    return __atomic_load_n(ptr, mo);
}

template <typename T>
void sk_atomic_store(T* ptr, T val, sk_memory_order mo) {
    SkASSERT(mo == sk_memory_order_relaxed ||
             mo == sk_memory_order_seq_cst ||
             mo == sk_memory_order_release);
    __atomic_store_n(ptr, val, mo);
}

template <typename T>
T sk_atomic_fetch_add(T* ptr, T val, sk_memory_order mo) {
    // All values of mo are valid.
    return __atomic_fetch_add(ptr, val, mo);
}

template <typename T>
T sk_atomic_fetch_sub(T* ptr, T val, sk_memory_order mo) {
    // All values of mo are valid.
    return __atomic_fetch_sub(ptr, val, mo);
}

template <typename T>
bool sk_atomic_compare_exchange(T* ptr, T* expected, T desired,
                                sk_memory_order success,
                                sk_memory_order failure) {
    // All values of success are valid.
    SkASSERT(failure == sk_memory_order_relaxed ||
             failure == sk_memory_order_seq_cst ||
             failure == sk_memory_order_acquire ||
             failure == sk_memory_order_consume);
    SkASSERT(failure <= success);
    return __atomic_compare_exchange_n(ptr, expected, desired, false/*weak?*/, success, failure);
}

template <typename T>
T sk_atomic_exchange(T* ptr, T val, sk_memory_order mo) {
    // All values of mo are valid.
    return __atomic_exchange_n(ptr, val, mo);
}

#endif//SkAtomics_atomic_DEFINED
