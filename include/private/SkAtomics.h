/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkAtomics_DEFINED
#define SkAtomics_DEFINED

#include "SkTypes.h"
#include <atomic>

// ~~~~~~~~ Legacy APIs ~~~~~~~~~
//
// Please use types from <atomic> for any new code.
// That's all this file ends up doing under the hood.

template <typename T>
T sk_atomic_fetch_add(T* ptr, T val, std::memory_order mo = std::memory_order_seq_cst) {
    // All values of mo are valid.
    std::atomic<T>* ap = reinterpret_cast<std::atomic<T>*>(ptr);
    return std::atomic_fetch_add_explicit(ap, val, mo);
}

// ~~~~~~~~ Very Legacy APIs ~~~~~~~~~
//
// Here are shims for our very old atomics API, to be weaned off of.  They use
// sequentially-consistent memory order to match historical behavior, but most
// of the callers could perform better with explicit, weaker memory ordering.

inline int32_t sk_atomic_inc(int32_t* ptr) { return sk_atomic_fetch_add(ptr, +1); }
inline int32_t sk_atomic_dec(int32_t* ptr) { return sk_atomic_fetch_add(ptr, -1); }

#endif//SkAtomics_DEFINED
