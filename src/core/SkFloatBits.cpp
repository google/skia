/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkFloatBits.h"
#include "SkMathPriv.h"

/******************************************************************************
    SkFloatBits_toInt[Floor, Round, Ceil] are identical except for what they
    do right before they return ... >> exp;
    Floor - adds nothing
    Round - adds 1 << (exp - 1)
    Ceil - adds (1 << exp) - 1

    Floor and Cast are very similar, but Cast applies its sign after all other
    computations on value. Also, Cast does not need to check for negative zero,
    as that value (0x80000000) "does the right thing" for Ceil. Note that it
    doesn't for Floor/Round/Ceil, hence the explicit check.
******************************************************************************/

#define EXP_BIAS            (127+23)
#define MATISSA_MAGIC_BIG   (1 << 23)

static inline int unpack_exp(uint32_t packed) {
    return (packed << 1 >> 24);
}

#if 0
// the ARM compiler generates an extra BIC, so I use the dirty version instead
static inline int unpack_matissa(uint32_t packed) {
    // we could mask with 0x7FFFFF, but that is harder for ARM to encode
    return (packed & ~0xFF000000) | MATISSA_MAGIC_BIG;
}
#endif

// returns the low 24-bits, so we need to OR in the magic_bit afterwards
static inline int unpack_matissa_dirty(uint32_t packed) {
    return packed & ~0xFF000000;
}

// same as (int)float
int32_t SkFloatBits_toIntCast(int32_t packed) {
    int exp = unpack_exp(packed) - EXP_BIAS;
    int value = unpack_matissa_dirty(packed) | MATISSA_MAGIC_BIG;

    if (exp >= 0) {
        if (exp > 7) {    // overflow
            value = SK_MaxS32;
        } else {
            value <<= exp;
        }
    } else {
        exp = -exp;
        if (exp > 25) {   // underflow
            exp = 25;
        }
        value >>= exp;
    }
    return SkApplySign(value, SkExtractSign(packed));
}

// same as (int)floor(float)
int32_t SkFloatBits_toIntFloor(int32_t packed) {
    // curse you negative 0
    if (SkLeftShift(packed, 1) == 0) {
        return 0;
    }

    int exp = unpack_exp(packed) - EXP_BIAS;
    int value = unpack_matissa_dirty(packed) | MATISSA_MAGIC_BIG;

    if (exp >= 0) {
        if (exp > 7) {    // overflow
            value = SK_MaxS32;
        } else {
            value <<= exp;
        }
        // apply the sign after we check for overflow
        return SkApplySign(value, SkExtractSign(packed));
    } else {
        // apply the sign before we right-shift
        value = SkApplySign(value, SkExtractSign(packed));
        exp = -exp;
        if (exp > 25) {   // underflow
#ifdef SK_CPU_FLUSH_TO_ZERO
        // The iOS ARM processor discards small denormalized numbers to go faster.
        // The comparision below empirically causes the result to agree with the
        // tests in MathTest test_float_floor
            if (exp > 149) {
                return 0;
            }
#else
            exp = 25;
#endif
        }
        // int add = 0;
        return value >> exp;
    }
}

// same as (int)floor(float + 0.5)
int32_t SkFloatBits_toIntRound(int32_t packed) {
    // curse you negative 0
    if (SkLeftShift(packed, 1) == 0) {
        return 0;
    }

    int exp = unpack_exp(packed) - EXP_BIAS;
    int value = unpack_matissa_dirty(packed) | MATISSA_MAGIC_BIG;

    if (exp >= 0) {
        if (exp > 7) {    // overflow
            value = SK_MaxS32;
        } else {
            value <<= exp;
        }
        // apply the sign after we check for overflow
        return SkApplySign(value, SkExtractSign(packed));
    } else {
        // apply the sign before we right-shift
        value = SkApplySign(value, SkExtractSign(packed));
        exp = -exp;
        if (exp > 25) {   // underflow
            exp = 25;
        }
        int add = 1 << (exp - 1);
        return (value + add) >> exp;
    }
}

// same as (int)ceil(float)
int32_t SkFloatBits_toIntCeil(int32_t packed) {
    // curse you negative 0
    if (SkLeftShift(packed, 1) == 0) {
        return 0;
    }

    int exp = unpack_exp(packed) - EXP_BIAS;
    int value = unpack_matissa_dirty(packed) | MATISSA_MAGIC_BIG;

    if (exp >= 0) {
        if (exp > 7) {    // overflow
            value = SK_MaxS32;
        } else {
            value <<= exp;
        }
        // apply the sign after we check for overflow
        return SkApplySign(value, SkExtractSign(packed));
    } else {
        // apply the sign before we right-shift
        value = SkApplySign(value, SkExtractSign(packed));
        exp = -exp;
        if (exp > 25) {   // underflow
#ifdef SK_CPU_FLUSH_TO_ZERO
        // The iOS ARM processor discards small denormalized numbers to go faster.
        // The comparision below empirically causes the result to agree with the
        // tests in MathTest test_float_ceil
            if (exp > 149) {
                return 0;
            }
            return 0 < value;
#else
            exp = 25;
#endif
        }
        int add = (1 << exp) - 1;
        return (value + add) >> exp;
    }
}

float SkIntToFloatCast(int32_t value) {
    if (0 == value) {
        return 0;
    }

    int shift = EXP_BIAS;

    // record the sign and make value positive
    int sign = SkExtractSign(value);
    value = SkApplySign(value, sign);

    if (value >> 24) {    // value is too big (has more than 24 bits set)
        int bias = 8 - SkCLZ(value);
        SkDebugf("value = %d, bias = %d\n", value, bias);
        SkASSERT(bias > 0 && bias < 8);
        value >>= bias; // need to round?
        shift += bias;
    } else {
        int zeros = SkCLZ(value << 8);
        SkASSERT(zeros >= 0 && zeros <= 23);
        value <<= zeros;
        shift -= zeros;
    }

    // now value is left-aligned to 24 bits
    SkASSERT((value >> 23) == 1);
    SkASSERT(shift >= 0 && shift <= 255);

    SkFloatIntUnion data;
    data.fSignBitInt = SkLeftShift(sign, 31) | SkLeftShift(shift, 23) | (value & ~MATISSA_MAGIC_BIG);
    return data.fFloat;
}
