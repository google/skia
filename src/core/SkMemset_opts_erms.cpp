/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/base/SkMSAN.h"
#include "src/core/SkMemset.h"
#include <cstddef>
#include <cstdint>

// memset16 and memset32 could work on 32-bit x86 too, but for simplicity just use this on x64
#if (defined(__x86_64__) || defined(_M_X64)) && !defined(SK_ENABLE_OPTIMIZE_SIZE)

static const char* note = "MSAN can't see that repsto initializes memory.";

#if defined(_MSC_VER)
#include <intrin.h>
static inline void repsto(uint16_t* dst, uint16_t v, size_t n) {
    sk_msan_mark_initialized(dst, dst + n, note);
    __stosw(dst, v, n);
}
static inline void repsto(uint32_t* dst, uint32_t v, size_t n) {
    sk_msan_mark_initialized(dst, dst + n, note);
    static_assert(sizeof(uint32_t) == sizeof(unsigned long));
    __stosd(reinterpret_cast<unsigned long*>(dst), v, n);
}
static inline void repsto(uint64_t* dst, uint64_t v, size_t n) {
    sk_msan_mark_initialized(dst, dst + n, note);
    __stosq(dst, v, n);
}
#else
static inline void repsto(uint16_t* dst, uint16_t v, size_t n) {
    sk_msan_mark_initialized(dst, dst + n, note);
    asm volatile("rep stosw" : "+D"(dst), "+c"(n) : "a"(v) : "memory");
}
static inline void repsto(uint32_t* dst, uint32_t v, size_t n) {
    sk_msan_mark_initialized(dst, dst + n, note);
    asm volatile("rep stosl" : "+D"(dst), "+c"(n) : "a"(v) : "memory");
}
static inline void repsto(uint64_t* dst, uint64_t v, size_t n) {
    sk_msan_mark_initialized(dst, dst + n, note);
    asm volatile("rep stosq" : "+D"(dst), "+c"(n) : "a"(v) : "memory");
}
#endif

// ERMS is ideal for large copies but has a relatively high setup cost,
// so we use the previous best routine for small inputs.  FSRM would make this moot.
static void (*g_memset16_prev)(uint16_t*, uint16_t, int);
static void (*g_memset32_prev)(uint32_t*, uint32_t, int);
static void (*g_memset64_prev)(uint64_t*, uint64_t, int);
static void (*g_rect_memset16_prev)(uint16_t*, uint16_t, int, size_t, int);
static void (*g_rect_memset32_prev)(uint32_t*, uint32_t, int, size_t, int);
static void (*g_rect_memset64_prev)(uint64_t*, uint64_t, int, size_t, int);

// Empirically determined with `nanobench -m memset`.
static bool small(size_t bytes) { return bytes < 1024; }

namespace erms {

static inline void memset16(uint16_t* dst, uint16_t v, int n) {
    return small(sizeof(v) * n) ? g_memset16_prev(dst, v, n) : repsto(dst, v, n);
}
static inline void memset32(uint32_t* dst, uint32_t v, int n) {
    return small(sizeof(v) * n) ? g_memset32_prev(dst, v, n) : repsto(dst, v, n);
}
static inline void memset64(uint64_t* dst, uint64_t v, int n) {
    return small(sizeof(v) * n) ? g_memset64_prev(dst, v, n) : repsto(dst, v, n);
}

static inline void rect_memset16(uint16_t* dst, uint16_t v, int n, size_t rowBytes, int height) {
    if (small(sizeof(v) * n)) {
        return g_rect_memset16_prev(dst, v, n, rowBytes, height);
    }
    for (int stride = rowBytes / sizeof(v); height-- > 0; dst += stride) {
        repsto(dst, v, n);
    }
}
static inline void rect_memset32(uint32_t* dst, uint32_t v, int n, size_t rowBytes, int height) {
    if (small(sizeof(v) * n)) {
        return g_rect_memset32_prev(dst, v, n, rowBytes, height);
    }
    for (int stride = rowBytes / sizeof(v); height-- > 0; dst += stride) {
        repsto(dst, v, n);
    }
}
static inline void rect_memset64(uint64_t* dst, uint64_t v, int n, size_t rowBytes, int height) {
    if (small(sizeof(v) * n)) {
        return g_rect_memset64_prev(dst, v, n, rowBytes, height);
    }
    for (int stride = rowBytes / sizeof(v); height-- > 0; dst += stride) {
        repsto(dst, v, n);
    }
}

}  // namespace erms

#endif // X86_64 && !SK_ENABLE_OPTIMIZE_SIZE

namespace SkOpts {
    void Init_Memset_erms() {
        #if (defined(__x86_64__) || defined(_M_X64)) && !defined(SK_ENABLE_OPTIMIZE_SIZE)
            g_memset16_prev      = memset16;
            g_memset32_prev      = memset32;
            g_memset64_prev      = memset64;
            g_rect_memset16_prev = rect_memset16;
            g_rect_memset32_prev = rect_memset32;
            g_rect_memset64_prev = rect_memset64;

            memset16      = erms::memset16;
            memset32      = erms::memset32;
            memset64      = erms::memset64;
            rect_memset16 = erms::rect_memset16;
            rect_memset32 = erms::rect_memset32;
            rect_memset64 = erms::rect_memset64;
        #endif  // X86_64 && !SK_ENABLE_OPTIMIZE_SIZE
    }
}  // namespace SkOpts
