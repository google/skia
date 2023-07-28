/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkEnumBitMask_DEFINED
#define SkEnumBitMask_DEFINED

#include "include/private/base/SkAttributes.h"

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
 *   SK_MAKE_BITMASK_OPS(MyFlags)
 *
 *   ...
 *
 *       SkEnumBitMask<MyFlags> flags = MyFlags::kA | MyFlags::kB;
 *
 *       if (flags & MyFlags::kB) {}
 *
 *   ...
 */
template<typename E>
class SkEnumBitMask {
public:
    SK_ALWAYS_INLINE constexpr SkEnumBitMask(E e) : SkEnumBitMask((int)e) {}

    SK_ALWAYS_INLINE constexpr explicit operator bool() const { return fValue; }
    SK_ALWAYS_INLINE constexpr int value() const              { return fValue; }

    SK_ALWAYS_INLINE constexpr bool operator==(SkEnumBitMask m) const { return fValue == m.fValue; }
    SK_ALWAYS_INLINE constexpr bool operator!=(SkEnumBitMask m) const { return fValue != m.fValue; }

    SK_ALWAYS_INLINE constexpr SkEnumBitMask operator|(SkEnumBitMask m) const {
        return SkEnumBitMask(fValue | m.fValue);
    }
    SK_ALWAYS_INLINE constexpr SkEnumBitMask operator&(SkEnumBitMask m) const {
        return SkEnumBitMask(fValue & m.fValue);
    }
    SK_ALWAYS_INLINE constexpr SkEnumBitMask operator^(SkEnumBitMask m) const {
        return SkEnumBitMask(fValue ^ m.fValue);
    }
    SK_ALWAYS_INLINE constexpr SkEnumBitMask operator~() const { return SkEnumBitMask(~fValue); }

    SK_ALWAYS_INLINE SkEnumBitMask& operator|=(SkEnumBitMask m) { return *this = *this | m; }
    SK_ALWAYS_INLINE SkEnumBitMask& operator&=(SkEnumBitMask m) { return *this = *this & m; }
    SK_ALWAYS_INLINE SkEnumBitMask& operator^=(SkEnumBitMask m) { return *this = *this ^ m; }

private:
    SK_ALWAYS_INLINE constexpr explicit SkEnumBitMask(int value) : fValue(value) {}

    int fValue;
};

/**
 * Defines functions that make it possible to use bitwise operators on an enum.
 */
#define SK_MAKE_BITMASK_OPS(E)                                        \
    [[maybe_unused]] constexpr SkEnumBitMask<E> operator|(E a, E b) { \
        return SkEnumBitMask<E>(a) | b;                               \
    }                                                                 \
    [[maybe_unused]] constexpr SkEnumBitMask<E> operator&(E a, E b) { \
        return SkEnumBitMask<E>(a) & b;                               \
    }                                                                 \
    [[maybe_unused]] constexpr SkEnumBitMask<E> operator^(E a, E b) { \
        return SkEnumBitMask<E>(a) ^ b;                               \
    }                                                                 \
    [[maybe_unused]] constexpr SkEnumBitMask<E> operator~(E e) {      \
        return ~SkEnumBitMask<E>(e);                                  \
    }

#define SK_DECL_BITMASK_OPS_FRIENDS(E)                 \
    friend constexpr SkEnumBitMask<E> operator|(E, E); \
    friend constexpr SkEnumBitMask<E> operator&(E, E); \
    friend constexpr SkEnumBitMask<E> operator^(E, E); \
    friend constexpr SkEnumBitMask<E> operator~(E);

#endif  // SkEnumBitMask_DEFINED
