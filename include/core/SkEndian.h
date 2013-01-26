
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkEndian_DEFINED
#define SkEndian_DEFINED

#include "SkTypes.h"

/** \file SkEndian.h

    Macros and helper functions for handling 16 and 32 bit values in
    big and little endian formats.
*/

#if defined(SK_CPU_LENDIAN) && defined(SK_CPU_BENDIAN)
    #error "can't have both LENDIAN and BENDIAN defined"
#endif

#if !defined(SK_CPU_LENDIAN) && !defined(SK_CPU_BENDIAN)
    #error "need either LENDIAN or BENDIAN defined"
#endif

/** Swap the two bytes in the low 16bits of the parameters.
    e.g. 0x1234 -> 0x3412
*/
static inline uint16_t SkEndianSwap16(U16CPU value) {
    SkASSERT(value == (uint16_t)value);
    return static_cast<uint16_t>((value >> 8) | (value << 8));
}
template<uint16_t N> struct SkTEndianSwap16 {
    static const uint16_t value = static_cast<uint16_t>((N >> 8) | ((N & 0xFF) << 8));
};

/** Vector version of SkEndianSwap16(), which swaps the
    low two bytes of each value in the array.
*/
static inline void SkEndianSwap16s(uint16_t array[], int count) {
    SkASSERT(count == 0 || array != NULL);

    while (--count >= 0) {
        *array = SkEndianSwap16(*array);
        array += 1;
    }
}

/** Reverse all 4 bytes in a 32bit value.
    e.g. 0x12345678 -> 0x78563412
*/
static inline uint32_t SkEndianSwap32(uint32_t value) {
    return  ((value & 0xFF) << 24) |
            ((value & 0xFF00) << 8) |
            ((value & 0xFF0000) >> 8) |
            (value >> 24);
}
template<uint32_t N> struct SkTEndianSwap32 {
    static const uint32_t value = ((N & 0xFF) << 24) |
                                  ((N & 0xFF00) << 8) |
                                  ((N & 0xFF0000) >> 8) |
                                  (N >> 24);
};

/** Vector version of SkEndianSwap16(), which swaps the
    bytes of each value in the array.
*/
static inline void SkEndianSwap32s(uint32_t array[], int count) {
    SkASSERT(count == 0 || array != NULL);

    while (--count >= 0) {
        *array = SkEndianSwap32(*array);
        array += 1;
    }
}

#ifdef SK_CPU_LENDIAN
    #define SkEndian_SwapBE16(n)    SkEndianSwap16(n)
    #define SkEndian_SwapBE32(n)    SkEndianSwap32(n)
    #define SkEndian_SwapLE16(n)    (n)
    #define SkEndian_SwapLE32(n)    (n)

    #define SkTEndian_SwapBE16(n)    SkTEndianSwap16<n>::value
    #define SkTEndian_SwapBE32(n)    SkTEndianSwap32<n>::value
    #define SkTEndian_SwapLE16(n)    (n)
    #define SkTEndian_SwapLE32(n)    (n)
#else   // SK_CPU_BENDIAN
    #define SkEndian_SwapBE16(n)    (n)
    #define SkEndian_SwapBE32(n)    (n)
    #define SkEndian_SwapLE16(n)    SkEndianSwap16(n)
    #define SkEndian_SwapLE32(n)    SkEndianSwap32(n)

    #define SkTEndian_SwapBE16(n)    (n)
    #define SkTEndian_SwapBE32(n)    (n)
    #define SkTEndian_SwapLE16(n)    SkTEndianSwap16<n>::value
    #define SkTEndian_SwapLE32(n)    SkTEndianSwap32<n>::value
#endif

// When a bytestream is embedded in a 32-bit word, how far we need to
// shift the word to extract each byte from the low 8 bits by anding with 0xff.
#ifdef SK_CPU_LENDIAN
    #define SkEndian_Byte0Shift 0
    #define SkEndian_Byte1Shift 8
    #define SkEndian_Byte2Shift 16
    #define SkEndian_Byte3Shift 24
#else   // SK_CPU_BENDIAN
    #define SkEndian_Byte0Shift 24
    #define SkEndian_Byte1Shift 16
    #define SkEndian_Byte2Shift 8
    #define SkEndian_Byte3Shift 0
#endif


#if defined(SK_UINT8_BITFIELD_LENDIAN) && defined(SK_UINT8_BITFIELD_BENDIAN)
    #error "can't have both bitfield LENDIAN and BENDIAN defined"
#endif

#if !defined(SK_UINT8_BITFIELD_LENDIAN) && !defined(SK_UINT8_BITFIELD_BENDIAN)
    #ifdef SK_CPU_LENDIAN
        #define SK_UINT8_BITFIELD_LENDIAN
    #else
        #define SK_UINT8_BITFIELD_BENDIAN
    #endif
#endif

#ifdef SK_UINT8_BITFIELD_LENDIAN
    #define SK_UINT8_BITFIELD(f0, f1, f2, f3, f4, f5, f6, f7) \
        SK_OT_BYTE f0 : 1; \
        SK_OT_BYTE f1 : 1; \
        SK_OT_BYTE f2 : 1; \
        SK_OT_BYTE f3 : 1; \
        SK_OT_BYTE f4 : 1; \
        SK_OT_BYTE f5 : 1; \
        SK_OT_BYTE f6 : 1; \
        SK_OT_BYTE f7 : 1;
#else
    #define SK_UINT8_BITFIELD(f0, f1, f2, f3, f4, f5, f6, f7) \
        SK_OT_BYTE f7 : 1; \
        SK_OT_BYTE f6 : 1; \
        SK_OT_BYTE f5 : 1; \
        SK_OT_BYTE f4 : 1; \
        SK_OT_BYTE f3 : 1; \
        SK_OT_BYTE f2 : 1; \
        SK_OT_BYTE f1 : 1; \
        SK_OT_BYTE f0 : 1;
#endif

#endif
