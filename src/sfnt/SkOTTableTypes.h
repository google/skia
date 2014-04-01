/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkOTTableTypes_DEFINED
#define SkOTTableTypes_DEFINED

#include "SkTemplates.h"
#include "SkTypes.h"
#include "SkEndian.h"

//All SK_OT_ prefixed types should be considered as big endian.
typedef uint8_t SK_OT_BYTE;
#if CHAR_BIT == 8
typedef signed char SK_OT_CHAR; //easier to debug
#else
typedef int8_t SK_OT_CHAR;
#endif
typedef uint16_t SK_OT_SHORT;
typedef uint16_t SK_OT_USHORT;
typedef uint32_t SK_OT_ULONG;
typedef uint32_t SK_OT_LONG;
//16.16 Signed fixed point representation.
typedef int32_t SK_OT_Fixed;
//2.14 Signed fixed point representation.
typedef uint16_t SK_OT_F2DOT14;
//F units are the units of measurement in em space.
typedef uint16_t SK_OT_FWORD;
typedef uint16_t SK_OT_UFWORD;
//Number of seconds since 12:00 midnight, January 1, 1904.
typedef uint64_t SK_OT_LONGDATETIME;

#define SK_OT_BYTE_BITFIELD SK_UINT8_BITFIELD

template<typename T> class SkOTTableTAG {
public:
    /**
     * SkOTTableTAG<T>::value is the big endian value of an OpenType table tag.
     * It may be directly compared with raw big endian table data.
     */
    static const SK_OT_ULONG value = SkTEndian_SwapBE32(
        SkSetFourByteTag(T::TAG0, T::TAG1, T::TAG2, T::TAG3)
    );
};

/** SkOTSetUSHORTBit<N>::value is an SK_OT_USHORT with the Nth BE bit set. */
template <unsigned N> struct SkOTSetUSHORTBit {
    SK_COMPILE_ASSERT(N < 16, NTooBig);
    static const uint16_t bit = 1u << N;
    static const SK_OT_USHORT value = SkTEndian_SwapBE16(bit);
};

/** SkOTSetULONGBit<N>::value is an SK_OT_ULONG with the Nth BE bit set. */
template <unsigned N> struct SkOTSetULONGBit {
    SK_COMPILE_ASSERT(N < 32, NTooBig);
    static const uint32_t bit = 1u << N;
    static const SK_OT_ULONG value = SkTEndian_SwapBE32(bit);
};

#endif
