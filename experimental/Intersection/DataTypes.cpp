/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "DataTypes.h"

#include <sys/types.h>
#include <stdlib.h>

#if USE_EPSILON
const double PointEpsilon = 0.000001;
const double SquaredEpsilon = PointEpsilon * PointEpsilon;
#endif

const int UlpsEpsilon = 16;

_Vector operator-(const _Point& a, const _Point& b) {
    _Vector v = {a.x - b.x, a.y - b.y};
    return v;
}

_Point operator+(const _Point& a, const _Vector& b) {
    _Point v = {a.x + b.x, a.y + b.y};
    return v;
}

// from http://randomascii.wordpress.com/2012/02/25/comparing-floating-point-numbers-2012-edition/
union Float_t
{
    Float_t(float num = 0.0f) : f(num) {}
    // Portable extraction of components.
    bool negative() const { return (i >> 31) != 0; }
#if 0 // unused
    int32_t RawMantissa() const { return i & ((1 << 23) - 1); }
    int32_t RawExponent() const { return (i >> 23) & 0xFF; }
#endif
    int32_t i;
    float f;
#ifdef SK_DEBUG
    struct
    {   // Bitfields for exploration. Do not use in production code.
        uint32_t mantissa : 23;
        uint32_t exponent : 8;
        uint32_t sign : 1;
    } parts;
#endif
};

bool AlmostEqualUlps(float A, float B)
{
    Float_t uA(A);
    Float_t uB(B);

    // Different signs means they do not match.
    if (uA.negative() != uB.negative())
    {
        // Check for equality to make sure +0==-0
        return A == B;
    }

    // Find the difference in ULPs.
    int ulpsDiff = abs(uA.i - uB.i);
    return ulpsDiff <= UlpsEpsilon;
}

// FIXME: obsolete, delete
#if 1
int UlpsDiff(float A, float B)
{
    Float_t uA(A);
    Float_t uB(B);

    return abs(uA.i - uB.i);
}
#endif

#ifdef SK_DEBUG
void mathematica_ize(char* str, size_t bufferLen) {
    size_t len = strlen(str);
    bool num = false;
    for (size_t idx = 0; idx < len; ++idx) {
        if (num && str[idx] == 'e') {
            if (len + 2 >= bufferLen) {
                return;
            }
            memmove(&str[idx + 2], &str[idx + 1], len - idx);
            str[idx] = '*';
            str[idx + 1] = '^';
            ++len;
        }
        num = str[idx] >= '0' && str[idx] <= '9';
    }
}

bool valid_wind(int wind) {
    return wind > SK_MinS32 + 0xFFFF && wind < SK_MaxS32 - 0xFFFF;
}

void winding_printf(int wind) {
    if (wind == SK_MinS32) {
        SkDebugf("?");
    } else {
        SkDebugf("%d", wind);
    }
}
#endif
