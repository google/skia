/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_EnumBitMask_DEFINED
#define skgpu_EnumBitMask_DEFINED

#include "experimental/graphite/include/GraphiteTypes.h"

namespace skgpu {

/**
 * Wraps an enum that is used for flags, and enables masking with type safety. Example:
 *
 *   enum class MyFlags {
 *       kNone = 0,
 *       kA = 1,
 *       kB = 2,
 *       kC = 4,
 *   };
 *
 *   SKGPU_MAKE_MASK_OPS(MyFlags)
 *
 *   ...
 *
 *       Mask<MyFlags> flags = MyFlags::kA | MyFlags::kB;
 *
 *       if (flags & MyFlags::kB) {}
 *
 *   ...
 */
template<typename E>
class Mask {
public:
    SK_ALWAYS_INLINE constexpr Mask(E e) : Mask((int)e) {}

    SK_ALWAYS_INLINE constexpr operator bool() const { return fValue; }

    SK_ALWAYS_INLINE bool operator==(Mask m) const { return fValue == m.fValue; }
    SK_ALWAYS_INLINE bool operator!=(Mask m) const { return fValue != m.fValue; }

    SK_ALWAYS_INLINE constexpr Mask operator|(Mask m) const { return Mask(fValue | m.fValue); }
    SK_ALWAYS_INLINE constexpr Mask operator&(Mask m) const { return Mask(fValue & m.fValue); }
    SK_ALWAYS_INLINE constexpr Mask operator^(Mask m) const { return Mask(fValue ^ m.fValue); }
    SK_ALWAYS_INLINE constexpr Mask operator~() const { return Mask(~fValue); }

    SK_ALWAYS_INLINE Mask& operator|=(Mask m) { return *this = *this | m; }
    SK_ALWAYS_INLINE Mask& operator&=(Mask m) { return *this = *this & m; }
    SK_ALWAYS_INLINE Mask& operator^=(Mask m) { return *this = *this ^ m; }

private:
    SK_ALWAYS_INLINE constexpr explicit Mask(int value) : fValue(value) {}

    int fValue;
};

/**
 * Defines functions that make it possible to use bitwise operators on an enum.
 */
#define SKGPU_MAKE_MASK_OPS(E) \
    SK_MAYBE_UNUSED constexpr skgpu::Mask<E> operator|(E a, E b) { return skgpu::Mask<E>(a) | b; } \
    SK_MAYBE_UNUSED constexpr skgpu::Mask<E> operator&(E a, E b) { return skgpu::Mask<E>(a) & b; } \
    SK_MAYBE_UNUSED constexpr skgpu::Mask<E> operator^(E a, E b) { return skgpu::Mask<E>(a) ^ b; } \
    SK_MAYBE_UNUSED constexpr skgpu::Mask<E> operator~(E e) { return ~skgpu::Mask<E>(e); } \

#define SKGPU_DECL_MASK_OPS_FRIENDS(E) \
    friend constexpr skgpu::Mask<E> operator|(E, E); \
    friend constexpr skgpu::Mask<E> operator&(E, E); \
    friend constexpr skgpu::Mask<E> operator^(E, E); \
    friend constexpr skgpu::Mask<E> operator~(E); \

};  // namespace skgpu

#endif // skgpu_EnumBitMask_DEFINED


