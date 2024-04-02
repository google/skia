/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkUtils_DEFINED
#define SkUtils_DEFINED

#include "include/private/base/SkAttributes.h"

#include <cstring>
#include <type_traits> // is_trivially_copyable

namespace SkHexadecimalDigits {
    extern const char gUpper[16];  // 0-9A-F
    extern const char gLower[16];  // 0-9a-f
}  // namespace SkHexadecimalDigits

///////////////////////////////////////////////////////////////////////////////

// If T is an 8-byte GCC or Clang vector extension type, it would naturally pass or return in the
// MMX mm0 register on 32-bit x86 builds.  This has the fun side effect of clobbering any state in
// the x87 st0 register.  (There is no ABI governing who should preserve mm?/st? registers, so no
// one does!)
//
// We force-inline sk_unaligned_load() and sk_unaligned_store() to avoid that, making them safe to
// use for all types on all platforms, thus solving the problem once and for all!

// A separate problem exists with 32-bit x86. The default calling convention returns values in
// ST0 (the x87 FPU). Unfortunately, doing so can mutate some bit patterns (signaling NaNs
// become quiet). If you're using these functions to pass data around as floats, but it's actually
// integers, that can be bad -- raster pipeline does this.
//
// With GCC and Clang, the always_inline attribute ensures we don't have a problem. MSVC, though,
// ignores __forceinline in debug builds, so the return-via-ST0 is always present. Switching to
// __vectorcall changes the functions to return in xmm0.
#if defined(_MSC_VER) && defined(_M_IX86)
    #define SK_FP_SAFE_ABI __vectorcall
#else
    #define SK_FP_SAFE_ABI
#endif

template <typename T, typename P>
static SK_ALWAYS_INLINE T SK_FP_SAFE_ABI sk_unaligned_load(const P* ptr) {
    static_assert(std::is_trivially_copyable_v<P> || std::is_void_v<P>);
    static_assert(std::is_trivially_copyable_v<T>);
    T val;
    // gcc's class-memaccess sometimes triggers when:
    // - `T` is trivially copyable but
    // - `T` is non-trivial (e.g. at least one eligible default constructor is
    //    non-trivial).
    // Use `reinterpret_cast<const void*>` to explicit suppress this warning; a
    // trivially copyable type is safe to memcpy from/to.
    memcpy(&val, static_cast<const void*>(ptr), sizeof(val));
    return val;
}

template <typename T, typename P>
static SK_ALWAYS_INLINE void SK_FP_SAFE_ABI sk_unaligned_store(P* ptr, T val) {
    static_assert(std::is_trivially_copyable<T>::value);
    memcpy(ptr, &val, sizeof(val));
}

// Copy the bytes from src into an instance of type Dst and return it.
template <typename Dst, typename Src>
static SK_ALWAYS_INLINE Dst SK_FP_SAFE_ABI sk_bit_cast(const Src& src) {
    static_assert(sizeof(Dst) == sizeof(Src));
    static_assert(std::is_trivially_copyable<Dst>::value);
    static_assert(std::is_trivially_copyable<Src>::value);
    return sk_unaligned_load<Dst>(&src);
}

#undef SK_FP_SAFE_ABI

#endif
