/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkUtils_DEFINED
#define SkUtils_DEFINED

#include "include/core/SkTypes.h"
#include "src/core/SkOpts.h"

/** Similar to memset(), but it assigns a 16, 32, or 64-bit value into the buffer.
    @param buffer   The memory to have value copied into it
    @param value    The value to be copied into buffer
    @param count    The number of times value should be copied into the buffer.
*/
static inline void sk_memset16(uint16_t buffer[], uint16_t value, int count) {
    SkOpts::memset16(buffer, value, count);
}
static inline void sk_memset32(uint32_t buffer[], uint32_t value, int count) {
    SkOpts::memset32(buffer, value, count);
}
static inline void sk_memset64(uint64_t buffer[], uint64_t value, int count) {
    SkOpts::memset64(buffer, value, count);
}

///////////////////////////////////////////////////////////////////////////////

namespace SkHexadecimalDigits {
    extern const char gUpper[16];  // 0-9A-F
    extern const char gLower[16];  // 0-9a-f
}  // namespace SkHexadecimalDigits

///////////////////////////////////////////////////////////////////////////////

// If T is an 8-byte GCC or Clang vector extension type, it would naturally
// pass or return in the MMX mm0 register on 32-bit x86 builds.  This has the
// fun side effect of clobbering any state in the x87 st0 register.  (There is
// no ABI governing who should preserve mm?/st? registers, so no one does!)
//
// We force-inline sk_unaligned_load() and sk_unaligned_store() to avoid that,
// making them safe to use for all types on all platforms, thus solving the
// problem once and for all!

template <typename T, typename P>
static SK_ALWAYS_INLINE T sk_unaligned_load(const P* ptr) {
    static_assert(std::is_trivially_copyable<T>::value);
    static_assert(std::is_trivially_copyable<P>::value);
    T val;
    memcpy(&val, ptr, sizeof(val));
    return val;
}

template <typename T, typename P>
static SK_ALWAYS_INLINE void sk_unaligned_store(P* ptr, T val) {
    static_assert(std::is_trivially_copyable<T>::value);
    static_assert(std::is_trivially_copyable<P>::value);
    memcpy(ptr, &val, sizeof(val));
}

template <typename Dst, typename Src>
static SK_ALWAYS_INLINE Dst sk_bit_cast(const Src& src) {
    static_assert(sizeof(Dst) == sizeof(Src));
    return sk_unaligned_load<Dst>(&src);
}

#endif
