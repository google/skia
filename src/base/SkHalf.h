/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkHalf_DEFINED
#define SkHalf_DEFINED

#include <cstdint>

// 16-bit floating point value
// format is 1 bit sign, 5 bits exponent, 10 bits mantissa
// only used for storage
using SkHalf = uint16_t;

static constexpr uint16_t SK_HalfNaN      = 0x7c01; // a NaN value, not all possible NaN values
static constexpr uint16_t SK_HalfInfinity = 0x7c00;
static constexpr uint16_t SK_HalfMin      = 0x0400; // 2^-14  (minimum positive normal value)
static constexpr uint16_t SK_HalfMax      = 0x7bff; // 65504  (maximum positive normal value)
static constexpr uint16_t SK_HalfEpsilon  = 0x1400; // 2^-10
static constexpr uint16_t SK_Half1        = 0x3C00; // 1

// Convert between half and single precision floating point. Vectorized functions
// skvx::from_half and skvx::to_half are also available. Unlike skvx::to_half, this will
// correctly handle float NaN -> half NaN.
float SkHalfToFloat(SkHalf h);
SkHalf SkFloatToHalf(float f);

#endif
