/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_GraphiteTypes_DEFINED
#define skgpu_GraphiteTypes_DEFINED

#include "include/core/SkTypes.h"

namespace skgpu {

/**
 * Defines overloaded bitwise operators to make it easier to use an enum as a
 * bitfield.
 */
#define SKGPU_MAKE_BITFIELD_OPS(X) \
    inline X operator |(X a, X b) { \
        return (X) (+a | +b); \
    } \
    inline X& operator |=(X& a, X b) { \
        return (a = a | b); \
    } \
    inline X operator &(X a, X b) { \
        return (X) (+a & +b); \
    } \
    inline X& operator &=(X& a, X b) { \
        return (a = a & b); \
    } \
    template <typename T> \
    inline X operator &(T a, X b) { \
        return (X) (+a & +b); \
    } \
    template <typename T> \
    inline X operator &(X a, T b) { \
        return (X) (+a & +b); \
    } \

#define SKGPU_DECL_BITFIELD_OPS_FRIENDS(X) \
    friend X operator |(X a, X b); \
    friend X& operator |=(X& a, X b); \
    \
    friend X operator &(X a, X b); \
    friend X& operator &=(X& a, X b); \
    \
    template <typename T> \
    friend X operator &(T a, X b); \
    \
    template <typename T> \
    friend X operator &(X a, T b); \

/**
 * Wraps a C++11 enum that we use as a bitfield, and enables a limited amount of
 * masking with type safety. Instantiated with the ~ operator.
 */
template<typename TFlags> class TFlagsMask {
public:
    constexpr explicit TFlagsMask(TFlags value) : TFlagsMask(static_cast<int>(value)) {}
    constexpr explicit TFlagsMask(int value) : fValue(value) {}
    constexpr int value() const { return fValue; }
private:
    const int fValue;
};

/**
 * Defines bitwise operators that make it possible to use an enum class as a
 * basic bitfield.
 */
#define SKGPU_MAKE_BITFIELD_CLASS_OPS(X) \
    SK_MAYBE_UNUSED constexpr TFlagsMask<X> operator~(X a) { \
        return TFlagsMask<X>(~static_cast<int>(a)); \
    } \
    SK_MAYBE_UNUSED constexpr X operator|(X a, X b) { \
        return static_cast<X>(static_cast<int>(a) | static_cast<int>(b)); \
    } \
    SK_MAYBE_UNUSED inline X& operator|=(X& a, X b) { \
        return (a = a | b); \
    } \
    SK_MAYBE_UNUSED constexpr bool operator&(X a, X b) { \
        return SkToBool(static_cast<int>(a) & static_cast<int>(b)); \
    } \
    SK_MAYBE_UNUSED constexpr TFlagsMask<X> operator|(TFlagsMask<X> a, TFlagsMask<X> b) { \
        return TFlagsMask<X>(a.value() | b.value()); \
    } \
    SK_MAYBE_UNUSED constexpr TFlagsMask<X> operator|(TFlagsMask<X> a, X b) { \
        return TFlagsMask<X>(a.value() | static_cast<int>(b)); \
    } \
    SK_MAYBE_UNUSED constexpr TFlagsMask<X> operator|(X a, TFlagsMask<X> b) { \
        return TFlagsMask<X>(static_cast<int>(a) | b.value()); \
    } \
    SK_MAYBE_UNUSED constexpr X operator&(TFlagsMask<X> a, TFlagsMask<X> b) { \
        return static_cast<X>(a.value() & b.value()); \
    } \
    SK_MAYBE_UNUSED constexpr X operator&(TFlagsMask<X> a, X b) { \
        return static_cast<X>(a.value() & static_cast<int>(b)); \
    } \
    SK_MAYBE_UNUSED constexpr X operator&(X a, TFlagsMask<X> b) { \
        return static_cast<X>(static_cast<int>(a) & b.value()); \
    } \
    SK_MAYBE_UNUSED inline X& operator&=(X& a, TFlagsMask<X> b) { \
        return (a = a & b); \
    } \

#define SKGPU_DECL_BITFIELD_CLASS_OPS_FRIENDS(X) \
    friend constexpr TFlagsMask<X> operator ~(X); \
    friend constexpr X operator |(X, X); \
    friend X& operator |=(X&, X); \
    friend constexpr bool operator &(X, X); \
    friend constexpr TFlagsMask<X> operator|(TFlagsMask<X>, TFlagsMask<X>); \
    friend constexpr TFlagsMask<X> operator|(TFlagsMask<X>, X); \
    friend constexpr TFlagsMask<X> operator|(X, TFlagsMask<X>); \
    friend constexpr X operator&(TFlagsMask<X>, TFlagsMask<X>); \
    friend constexpr X operator&(TFlagsMask<X>, X); \
    friend constexpr X operator&(X, TFlagsMask<X>); \
    friend X& operator &=(X&, TFlagsMask<X>)

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
