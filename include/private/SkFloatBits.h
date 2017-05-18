/*
 * Copyright 2008 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkFloatBits_DEFINED
#define SkFloatBits_DEFINED

#include "SkTypes.h"
#include "SkSafe_math.h"

/** Convert a sign-bit int (i.e. float interpreted as int) into a 2s compliement
    int. This also converts -0 (0x80000000) to 0. Doing this to a float allows
    it to be compared using normal C operators (<, <=, etc.)
*/
static inline int32_t SkSignBitTo2sCompliment(int32_t x) {
    if (x < 0) {
        x &= 0x7FFFFFFF;
        x = -x;
    }
    return x;
}

/** Convert a 2s compliment int to a sign-bit (i.e. int interpreted as float).
    This undoes the result of SkSignBitTo2sCompliment().
 */
static inline int32_t Sk2sComplimentToSignBit(int32_t x) {
    int sign = x >> 31;
    // make x positive
    x = (x ^ sign) - sign;
    // set the sign bit as needed
    x |= SkLeftShift(sign, 31);
    return x;
}

union SkFloatIntUnion {
    float   fFloat;
    int32_t fSignBitInt;
};

// Helper to see a float as its bit pattern (w/o aliasing warnings)
static inline int32_t SkFloat2Bits(float x) {
    SkFloatIntUnion data;
    data.fFloat = x;
    return data.fSignBitInt;
}

// Helper to see a bit pattern as a float (w/o aliasing warnings)
static inline float SkBits2Float(int32_t floatAsBits) {
    SkFloatIntUnion data;
    data.fSignBitInt = floatAsBits;
    return data.fFloat;
}

/** Return the float as a 2s compliment int. Just to be used to compare floats
    to each other or against positive float-bit-constants (like 0). This does
    not return the int equivalent of the float, just something cheaper for
    compares-only.
 */
static inline int32_t SkFloatAs2sCompliment(float x) {
    return SkSignBitTo2sCompliment(SkFloat2Bits(x));
}

/** Return the 2s compliment int as a float. This undos the result of
    SkFloatAs2sCompliment
 */
static inline float Sk2sComplimentAsFloat(int32_t x) {
    return SkBits2Float(Sk2sComplimentToSignBit(x));
}

static inline int32_t pin_double_to_int(double x) {
    return (int32_t)SkTPin<double>(x, SK_MinS32, SK_MaxS32);
}

/** Return the floor of the float as an int.
    If the value is out of range, or NaN, return +/- SK_MaxS32
*/
static inline int32_t SkFloatToIntFloor(float x) {
    return pin_double_to_int(floor(x));
}

/** Return the float rounded to an int.
    If the value is out of range, or NaN, return +/- SK_MaxS32
*/
static inline int32_t SkFloatToIntRound(float x) {
    return pin_double_to_int(floor((double)x + 0.5));
}

/** Return the ceiling of the float as an int.
    If the value is out of range, or NaN, return +/- SK_MaxS32
*/
static inline int32_t SkFloatToIntCeil(float x) {
    return pin_double_to_int(ceil(x));
}

//  Scalar wrappers for float-bit routines

#define SkScalarAs2sCompliment(x)    SkFloatAs2sCompliment(x)
#define Sk2sComplimentAsScalar(x)    Sk2sComplimentAsFloat(x)

#endif
