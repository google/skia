/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_GraphiteTypes_DEFINED
#define skgpu_GraphiteTypes_DEFINED

#include "include/core/SkTypes.h"
#include "include/private/SkVx.h"

namespace skgpu {

// Use familiar type names from SkSL.
template<int N> using vec = skvx::Vec<N, float>;
using float2 = vec<2>;
using float4 = vec<4>;

template<int N> using ivec = skvx::Vec<N, int32_t>;
using int2 = ivec<2>;
using int4 = ivec<4>;

template<int N> using uvec = skvx::Vec<N, uint32_t>;
using uint2 = uvec<2>;
using uint4 = uvec<4>;

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

/**
 * Possible 3D APIs that may be used by Graphite.
 */
enum class BackendApi : unsigned {
    kMetal,
    kMock,
};

/**
 * Is the texture mipmapped or not
 */
enum class Mipmapped: bool {
    kNo = false,
    kYes = true,
};

/**
 * Is the data protected on the GPU or not.
 */
enum class Protected : bool {
    kNo = false,
    kYes = true,
};

/**
 * An ordinal number that allows draw commands to be re-ordered so long as when they are executed,
 * the read/writes to the color|depth|stencil attachments respect the original painter's order.
 */
using CompressedPaintersOrder = uint16_t;

} // namespace skgpu

#endif // skgpu_GraphiteTypes_DEFINED
