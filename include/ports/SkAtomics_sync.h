/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkAtomics_sync_DEFINED
#define SkAtomics_sync_DEFINED

// This file is mostly a shim.  We'd like to delete it.  Please don't put much
// effort into maintaining it, and if you find bugs in it, the right fix is to
// delete this file and upgrade your compiler to something that supports
// __atomic builtins or std::atomic.

static inline void barrier(sk_memory_order mo) {
    asm volatile("" : : : "memory");  // Prevents the compiler from reordering code.
    #if SK_CPU_X86
        // On x86, we generally don't need an extra memory barrier for loads or stores.
        if (sk_memory_order_seq_cst == mo) { __sync_synchronize(); }
    #else
        // On other platforms (e.g. ARM) we do unless the memory order is relaxed.
        if (sk_memory_order_relaxed != mo) { __sync_synchronize(); }
    #endif
}

// These barriers only support our majority use cases: acquire and relaxed loads, release stores.
// For anything more complicated, please consider deleting this file and upgrading your compiler.

template <typename T>
T sk_atomic_load(const T* ptr, sk_memory_order mo) {
    T val = *ptr;
    barrier(mo);
    return val;
}

template <typename T>
void sk_atomic_store(T* ptr, T val, sk_memory_order mo) {
    barrier(mo);
    *ptr = val;
}

template <typename T>
T sk_atomic_fetch_add(T* ptr, T val, sk_memory_order) {
    return __sync_fetch_and_add(ptr, val);
}

template <typename T>
bool sk_atomic_compare_exchange(T* ptr, T* expected, T desired, sk_memory_order, sk_memory_order) {
    T prev = __sync_val_compare_and_swap(ptr, *expected, desired);
    if (prev == *expected) {
        return true;
    }
    *expected = prev;
    return false;
}

template <typename T>
T sk_atomic_exchange(T* ptr, T val, sk_memory_order) {
    // There is no __sync exchange.  Emulate it with a CAS loop.
    T prev;
    do {
        prev = sk_atomic_load(ptr);
    } while(!sk_atomic_compare_exchange(ptr, &prev, val));
    return prev;
}

#endif//SkAtomics_sync_DEFINED
