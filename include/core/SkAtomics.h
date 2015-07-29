/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkAtomics_DEFINED
#define SkAtomics_DEFINED

// This file is not part of the public Skia API.
#include "SkTypes.h"

enum sk_memory_order {
    sk_memory_order_relaxed,
    sk_memory_order_consume,
    sk_memory_order_acquire,
    sk_memory_order_release,
    sk_memory_order_acq_rel,
    sk_memory_order_seq_cst,
};

template <typename T>
T sk_atomic_load(const T*, sk_memory_order = sk_memory_order_seq_cst);

template <typename T>
void sk_atomic_store(T*, T, sk_memory_order = sk_memory_order_seq_cst);

template <typename T>
T sk_atomic_fetch_add(T*, T, sk_memory_order = sk_memory_order_seq_cst);

template <typename T>
bool sk_atomic_compare_exchange(T*, T* expected, T desired,
                                sk_memory_order success = sk_memory_order_seq_cst,
                                sk_memory_order failure = sk_memory_order_seq_cst);

template <typename T>
T sk_atomic_exchange(T*, T, sk_memory_order = sk_memory_order_seq_cst);

// A little wrapper class for small T (think, builtins: int, float, void*) to
// ensure they're always used atomically.  This is our stand-in for std::atomic<T>.
template <typename T>
class SkAtomic : SkNoncopyable {
public:
    SkAtomic() {}
    explicit SkAtomic(const T& val) : fVal(val) {}

    // It is essential we return by value rather than by const&.  fVal may change at any time.
    T load(sk_memory_order mo = sk_memory_order_seq_cst) const {
        return sk_atomic_load(&fVal, mo);
    }

    void store(const T& val, sk_memory_order mo = sk_memory_order_seq_cst) {
        sk_atomic_store(&fVal, val, mo);
    }

    T fetch_add(const T& val, sk_memory_order mo = sk_memory_order_seq_cst) {
        return sk_atomic_fetch_add(&fVal, val, mo);
    }

    bool compare_exchange(T* expected, const T& desired,
                          sk_memory_order success = sk_memory_order_seq_cst,
                          sk_memory_order failure = sk_memory_order_seq_cst) {
        return sk_atomic_compare_exchange(&fVal, expected, desired, success, failure);
    }
private:
    T fVal;
};

// IWYU pragma: begin_exports
#if defined(_MSC_VER)
    #include "../ports/SkAtomics_std.h"
#elif !defined(SK_BUILD_FOR_IOS) && defined(__ATOMIC_RELAXED)
    #include "../ports/SkAtomics_atomic.h"
#else
    #include "../ports/SkAtomics_sync.h"
#endif
// IWYU pragma: end_exports

// From here down we have shims for our old atomics API, to be weaned off of.
// We use the default sequentially-consistent memory order to make things simple
// and to match the practical reality of our old _sync and _win implementations.

inline int32_t sk_atomic_inc(int32_t* ptr)            { return sk_atomic_fetch_add(ptr, +1); }
inline int32_t sk_atomic_dec(int32_t* ptr)            { return sk_atomic_fetch_add(ptr, -1); }
inline int32_t sk_atomic_add(int32_t* ptr, int32_t v) { return sk_atomic_fetch_add(ptr,  v); }

inline int64_t sk_atomic_inc(int64_t* ptr) { return sk_atomic_fetch_add<int64_t>(ptr, +1); }

inline bool sk_atomic_cas(int32_t* ptr, int32_t expected, int32_t desired) {
    return sk_atomic_compare_exchange(ptr, &expected, desired);
}

inline void* sk_atomic_cas(void** ptr, void* expected, void* desired) {
    (void)sk_atomic_compare_exchange(ptr, &expected, desired);
    return expected;
}

inline int32_t sk_atomic_conditional_inc(int32_t* ptr) {
    int32_t prev = sk_atomic_load(ptr);
    do {
        if (0 == prev) {
            break;
        }
    } while(!sk_atomic_compare_exchange(ptr, &prev, prev+1));
    return prev;
}

template <typename T>
T sk_acquire_load(T* ptr) { return sk_atomic_load(ptr, sk_memory_order_acquire); }

template <typename T>
void sk_release_store(T* ptr, T val) { sk_atomic_store(ptr, val, sk_memory_order_release); }

inline void sk_membar_acquire__after_atomic_dec() {}
inline void sk_membar_acquire__after_atomic_conditional_inc() {}

#endif//SkAtomics_DEFINED
