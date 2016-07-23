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
#include <atomic>

// ~~~~~~~~ APIs ~~~~~~~~~

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
T sk_atomic_fetch_sub(T*, T, sk_memory_order = sk_memory_order_seq_cst);

template <typename T>
bool sk_atomic_compare_exchange(T*, T* expected, T desired,
                                sk_memory_order success = sk_memory_order_seq_cst,
                                sk_memory_order failure = sk_memory_order_seq_cst);

template <typename T>
T sk_atomic_exchange(T*, T, sk_memory_order = sk_memory_order_seq_cst);

// A little wrapper class for small T (think, builtins: int, float, void*) to
// ensure they're always used atomically.  This is our stand-in for std::atomic<T>.
// !!! Please _really_ know what you're doing if you change default_memory_order. !!!
template <typename T, sk_memory_order default_memory_order = sk_memory_order_seq_cst>
class SkAtomic : SkNoncopyable {
public:
    SkAtomic() {}
    explicit SkAtomic(const T& val) : fVal(val) {}

    // It is essential we return by value rather than by const&.  fVal may change at any time.
    T load(sk_memory_order mo = default_memory_order) const {
        return sk_atomic_load(&fVal, mo);
    }

    void store(const T& val, sk_memory_order mo = default_memory_order) {
        sk_atomic_store(&fVal, val, mo);
    }

    // Alias for .load(default_memory_order).
    operator T() const {
        return this->load();
    }

    // Alias for .store(v, default_memory_order).
    T operator=(const T& v) {
        this->store(v);
        return v;
    }

    T fetch_add(const T& val, sk_memory_order mo = default_memory_order) {
        return sk_atomic_fetch_add(&fVal, val, mo);
    }

    T fetch_sub(const T& val, sk_memory_order mo = default_memory_order) {
        return sk_atomic_fetch_sub(&fVal, val, mo);
    }

    bool compare_exchange(T* expected, const T& desired,
                          sk_memory_order success = default_memory_order,
                          sk_memory_order failure = default_memory_order) {
        return sk_atomic_compare_exchange(&fVal, expected, desired, success, failure);
    }
private:
    T fVal;
};

// ~~~~~~~~ Implementations ~~~~~~~~~

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

// ~~~~~~~~ Legacy APIs ~~~~~~~~~

// From here down we have shims for our old atomics API, to be weaned off of.
// We use the default sequentially-consistent memory order to make things simple
// and to match the practical reality of our old _sync and _win implementations.

inline int32_t sk_atomic_inc(int32_t* ptr)            { return sk_atomic_fetch_add(ptr, +1); }
inline int32_t sk_atomic_dec(int32_t* ptr)            { return sk_atomic_fetch_add(ptr, -1); }
inline int32_t sk_atomic_add(int32_t* ptr, int32_t v) { return sk_atomic_fetch_add(ptr,  v); }

inline int64_t sk_atomic_inc(int64_t* ptr) { return sk_atomic_fetch_add<int64_t>(ptr, +1); }

#endif//SkAtomics_DEFINED
