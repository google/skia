/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkBarriers_tsan_DEFINED
#define SkBarriers_tsan_DEFINED

#include <sanitizer/tsan_interface_atomic.h>

static inline void sk_compiler_barrier() { asm volatile("" : : : "memory"); }

// We'd do this as separate functions, but you can't partially specialize functions...
template <typename T, size_t bits>
struct SkBarriers {
    static T AcquireLoad(T*);
    static void ReleaseStore(T*, T);
};

#define SK_BARRIERS(BITS)                                                          \
    template <typename T>                                                          \
    struct SkBarriers<T, BITS> {                                                   \
        static T AcquireLoad(T* ptr) {                                             \
            return (T)__tsan_atomic ## BITS ## _load((__tsan_atomic ## BITS*)ptr,  \
                                                     __tsan_memory_order_acquire); \
        }                                                                          \
        static void ReleaseStore(T* ptr, T val) {                                  \
            __tsan_atomic ## BITS ## _store((__tsan_atomic ## BITS*)ptr,           \
                                            val,                                   \
                                            __tsan_memory_order_release);          \
        }                                                                          \
    }
SK_BARRIERS(8);
SK_BARRIERS(16);
SK_BARRIERS(32);
SK_BARRIERS(64);
#undef SK_BARRIERS

template <typename T>
T sk_acquire_load(T* ptr) { return SkBarriers<T, 8*sizeof(T)>::AcquireLoad(ptr); }

template <typename T>
void sk_release_store(T* ptr, T val) { SkBarriers<T, 8*sizeof(T)>::ReleaseStore(ptr, val); }


#endif//SkBarriers_tsan_DEFINED
