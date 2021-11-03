/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkUtils_DEFINED
#define SkUtils_DEFINED

#include "include/core/SkTypes.h"

#include <type_traits> // is_trivially_copyable

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
