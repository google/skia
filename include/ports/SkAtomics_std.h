/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkAtomics_std_DEFINED
#define SkAtomics_std_DEFINED

// We try not to depend on the C++ standard library,
// but these uses of <atomic> should all inline, so we don't feel to bad here.
#include <atomic>

template <typename T>
T sk_atomic_load(const T* ptr, sk_memory_order mo) {
    SkASSERT(mo == sk_memory_order_relaxed ||
             mo == sk_memory_order_seq_cst ||
             mo == sk_memory_order_acquire ||
             mo == sk_memory_order_consume);
    const std::atomic<T>* ap = reinterpret_cast<const std::atomic<T>*>(ptr);
    return std::atomic_load_explicit(ap, (std::memory_order)mo);
}

template <typename T>
void sk_atomic_store(T* ptr, T val, sk_memory_order mo) {
    SkASSERT(mo == sk_memory_order_relaxed ||
             mo == sk_memory_order_seq_cst ||
             mo == sk_memory_order_release);
    std::atomic<T>* ap = reinterpret_cast<std::atomic<T>*>(ptr);
    return std::atomic_store_explicit(ap, val, (std::memory_order)mo);
}

template <typename T>
T sk_atomic_fetch_add(T* ptr, T val, sk_memory_order mo) {
    // All values of mo are valid.
    std::atomic<T>* ap = reinterpret_cast<std::atomic<T>*>(ptr);
    return std::atomic_fetch_add_explicit(ap, val, (std::memory_order)mo);
}

template <typename T>
T sk_atomic_fetch_sub(T* ptr, T val, sk_memory_order mo) {
    // All values of mo are valid.
    std::atomic<T>* ap = reinterpret_cast<std::atomic<T>*>(ptr);
    return std::atomic_fetch_sub_explicit(ap, val, (std::memory_order)mo);
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
    std::atomic<T>* ap = reinterpret_cast<std::atomic<T>*>(ptr);
    return std::atomic_compare_exchange_strong_explicit(ap, expected, desired,
                                                        (std::memory_order)success,
                                                        (std::memory_order)failure);
}

template <typename T>
T sk_atomic_exchange(T* ptr, T val, sk_memory_order mo) {
    // All values of mo are valid.
    std::atomic<T>* ap = reinterpret_cast<std::atomic<T>*>(ptr);
    return std::atomic_exchange_explicit(ap, val, (std::memory_order)mo);
}

#endif//SkAtomics_std_DEFINED
