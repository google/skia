/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkASAN_DEFINED
#define SkASAN_DEFINED

#include <cstddef>

#ifdef __SANITIZE_ADDRESS__
    #define SK_SANITIZE_ADDRESS 1
#endif
#if !defined(SK_SANITIZE_ADDRESS) && defined(__has_feature)
    #if __has_feature(address_sanitizer)
        #define SK_SANITIZE_ADDRESS 1
    #endif
#endif

// Typically declared in LLVM's asan_interface.h.
#ifdef SK_SANITIZE_ADDRESS
extern "C" {
    void __asan_poison_memory_region(void const volatile *addr, size_t size);
    void __asan_unpoison_memory_region(void const volatile *addr, size_t size);
    int __asan_address_is_poisoned(void const volatile *addr);
}
#endif

// Code that implements bespoke allocation arenas can poison the entire arena on creation, then
// unpoison chunks of arena memory as they are parceled out. Consider leaving gaps between blocks
// to detect buffer overrun.
static inline void sk_asan_poison_memory_region([[maybe_unused]] void const volatile* addr,
                                                [[maybe_unused]] size_t size) {
#ifdef SK_SANITIZE_ADDRESS
    __asan_poison_memory_region(addr, size);
#endif
}

static inline void sk_asan_unpoison_memory_region([[maybe_unused]] void const volatile* addr,
                                                  [[maybe_unused]] size_t size) {
#ifdef SK_SANITIZE_ADDRESS
    __asan_unpoison_memory_region(addr, size);
#endif
}

static inline int sk_asan_address_is_poisoned([[maybe_unused]] void const volatile* addr) {
#ifdef SK_SANITIZE_ADDRESS
    return __asan_address_is_poisoned(addr);
#else
    return 0;
#endif
}

#endif  // SkASAN_DEFINED
