/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkFixed_DEFINED
#define SkFixed_DEFINED

#include "SkTypes.h"

/** \file SkFixed.h

    Types and macros for 16.16 fixed point
*/

/** 32 bit signed integer used to represent fractions values with 16 bits to the right of the decimal point
*/
typedef int32_t             SkFixed;
#define SK_Fixed1           (1 << 16)
#define SK_FixedHalf        (1 << 15)
#define SK_FixedMax         (0x7FFFFFFF)
#define SK_FixedMin         (-SK_FixedMax)
#define SK_FixedNaN         ((int) 0x80000000)
#define SK_FixedPI          (0x3243F)
#define SK_FixedSqrt2       (92682)
#define SK_FixedTanPIOver8  (0x6A0A)
#define SK_FixedRoot2Over2  (0xB505)

#define SkFixedToFloat(x)   ((x) * 1.52587890625e-5f)
#if 1
    #define SkFloatToFixed(x)   ((SkFixed)((x) * SK_Fixed1))
#else
    // pins over/under flows to max/min int32 (slower than just a cast)
    static inline SkFixed SkFloatToFixed(float x) {
        int64_t n = x * SK_Fixed1;
        return (SkFixed)n;
    }
#endif

#ifdef SK_DEBUG
    static inline SkFixed SkFloatToFixed_Check(float x) {
        int64_t n64 = (int64_t)(x * SK_Fixed1);
        SkFixed n32 = (SkFixed)n64;
        SkASSERT(n64 == n32);
        return n32;
    }
#else
    #define SkFloatToFixed_Check(x) SkFloatToFixed(x)
#endif

#define SkFixedToDouble(x)  ((x) * 1.52587890625e-5)
#define SkDoubleToFixed(x)  ((SkFixed)((x) * SK_Fixed1))

/** Converts an integer to a SkFixed, asserting that the result does not overflow
    a 32 bit signed integer
*/
#ifdef SK_DEBUG
    inline SkFixed SkIntToFixed(int n)
    {
        SkASSERT(n >= -32768 && n <= 32767);
        // Left shifting a negative value has undefined behavior in C, so we cast to unsigned before
        // shifting.
        return (unsigned)n << 16;
    }
#else
    // Left shifting a negative value has undefined behavior in C, so we cast to unsigned before
    // shifting. Then we force the cast to SkFixed to ensure that the answer is signed (like the
    // debug version).
    #define SkIntToFixed(n)     (SkFixed)((unsigned)(n) << 16)
#endif

#define SkFixedRoundToInt(x)    (((x) + SK_FixedHalf) >> 16)
#define SkFixedCeilToInt(x)     (((x) + SK_Fixed1 - 1) >> 16)
#define SkFixedFloorToInt(x)    ((x) >> 16)

#define SkFixedRoundToFixed(x)  (((x) + SK_FixedHalf) & 0xFFFF0000)
#define SkFixedCeilToFixed(x)   (((x) + SK_Fixed1 - 1) & 0xFFFF0000)
#define SkFixedFloorToFixed(x)  ((x) & 0xFFFF0000)

#define SkFixedAbs(x)       SkAbs32(x)
#define SkFixedAve(a, b)    (((a) + (b)) >> 1)

// Blink layout tests are baselined to Clang optimizing through undefined behavior in SkDivBits.
#if defined(SK_SUPPORT_LEGACY_DIVBITS_UB)
    #define SkFixedDiv(numer, denom) SkDivBits(numer, denom, 16)
#else
    // TODO(reed): this clamp shouldn't be needed.  Use SkToS32().
    #define SkFixedDiv(numer, denom) \
        SkTPin<int32_t>(((int64_t)numer << 16) / denom, SK_MinS32, SK_MaxS32)
#endif

//////////////////////////////////////////////////////////////////////////////////////////////////////
// Now look for ASM overrides for our portable versions (should consider putting this in its own file)

inline SkFixed SkFixedMul_longlong(SkFixed a, SkFixed b) {
    return (SkFixed)((int64_t)a * b >> 16);
}
#define SkFixedMul(a,b)     SkFixedMul_longlong(a,b)


#if defined(SK_CPU_ARM32)
    /* This guy does not handle NaN or other obscurities, but is faster than
       than (int)(x*65536).  When built on Android with -Os, needs forcing
       to inline or we lose the speed benefit.
    */
    SK_ALWAYS_INLINE SkFixed SkFloatToFixed_arm(float x)
    {
        int32_t y, z;
        asm("movs    %1, %3, lsl #1         \n"
            "mov     %2, #0x8E              \n"
            "sub     %1, %2, %1, lsr #24    \n"
            "mov     %2, %3, lsl #8         \n"
            "orr     %2, %2, #0x80000000    \n"
            "mov     %1, %2, lsr %1         \n"
            "it cs                          \n"
            "rsbcs   %1, %1, #0             \n"
            : "=r"(x), "=&r"(y), "=&r"(z)
            : "r"(x)
            : "cc"
            );
        return y;
    }
    inline SkFixed SkFixedMul_arm(SkFixed x, SkFixed y)
    {
        int32_t t;
        asm("smull  %0, %2, %1, %3          \n"
            "mov    %0, %0, lsr #16         \n"
            "orr    %0, %0, %2, lsl #16     \n"
            : "=r"(x), "=&r"(y), "=r"(t)
            : "r"(x), "1"(y)
            :
            );
        return x;
    }
    #undef SkFixedMul
    #define SkFixedMul(x, y)        SkFixedMul_arm(x, y)

    #undef SkFloatToFixed
    #define SkFloatToFixed(x)  SkFloatToFixed_arm(x)
#endif

///////////////////////////////////////////////////////////////////////////////

typedef int64_t SkFixed3232;   // 32.32

#define SkIntToFixed3232(x)       ((SkFixed3232)(x) << 32)
#define SkFixed3232ToInt(x)       ((int)((x) >> 32))
#define SkFixedToFixed3232(x)     ((SkFixed3232)(x) << 16)
#define SkFixed3232ToFixed(x)     ((SkFixed)((x) >> 16))
#define SkFloatToFixed3232(x)     ((SkFixed3232)((x) * (65536.0f * 65536.0f)))

#define SkScalarToFixed3232(x)    SkFloatToFixed3232(x)

///////////////////////////////////////////////////////////////////////////////

// 64bits wide, with a 16bit bias. Useful when accumulating lots of 16.16 so
// we don't overflow along the way
typedef int64_t Sk48Dot16;

#define Sk48Dot16FloorToInt(x)    static_cast<int>((x) >> 16)

static inline float Sk48Dot16ToScalar(Sk48Dot16 x) {
    return static_cast<float>(x * 1.5258789e-5);  // x * (1.0f / (1 << 16))
}
#define SkFloatTo48Dot16(x)       (static_cast<Sk48Dot16>((x) * (1 << 16)))

#define SkScalarTo48Dot16(x)      SkFloatTo48Dot16(x)

#endif
