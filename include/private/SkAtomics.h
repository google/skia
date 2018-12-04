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

// ~~~~~~~~ Very Legacy APIs ~~~~~~~~~
//
// Here are shims for our very old atomics API, to be weaned off of.  They use
// sequentially-consistent memory order to match historical behavior, but most
// of the callers could perform better with explicit, weaker memory ordering.

inline int32_t sk_atomic_inc(int32_t* ptr) {
    return reinterpret_cast<std::atomic<int32_t>*>(ptr)->fetch_add(+1);
}
inline int32_t sk_atomic_dec(int32_t* ptr) {
    return reinterpret_cast<std::atomic<int32_t>*>(ptr)->fetch_add(-1);
}

#endif//SkAtomics_DEFINED
