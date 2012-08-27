/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "DataTypes.h"

#include <sys/types.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

void    *memcpy(void *, const void *, size_t);

#ifdef __cplusplus
}
#endif


#if USE_EPSILON
const double PointEpsilon = 0.000001;
const double SquaredEpsilon = PointEpsilon * PointEpsilon;
#endif

const int UlpsEpsilon = 16;

// from http://randomascii.wordpress.com/2012/02/25/comparing-floating-point-numbers-2012-edition/
union Float_t
{
    Float_t(float num = 0.0f) : f(num) {}
    // Portable extraction of components.
    bool Negative() const { return (i >> 31) != 0; }
    int32_t RawMantissa() const { return i & ((1 << 23) - 1); }
    int32_t RawExponent() const { return (i >> 23) & 0xFF; }

    int32_t i;
    float f;
#ifdef _DEBUG
    struct
    {   // Bitfields for exploration. Do not use in production code.
        uint32_t mantissa : 23;
        uint32_t exponent : 8;
        uint32_t sign : 1;
    } parts;
#endif
};

bool AlmostEqualUlps(float A, float B, int maxUlpsDiff)
{
    Float_t uA(A);
    Float_t uB(B);

    // Different signs means they do not match.
    if (uA.Negative() != uB.Negative())
    {
        // Check for equality to make sure +0==-0
        return A == B;
    }

    // Find the difference in ULPs.
    int ulpsDiff = abs(uA.i - uB.i);
    return ulpsDiff <= maxUlpsDiff;
}

int UlpsDiff(float A, float B)
{
    Float_t uA(A);
    Float_t uB(B);

    return abs(uA.i - uB.i);
}

int FloatAsInt(float A)
{
    Float_t uA(A);
    return uA.i;
}


