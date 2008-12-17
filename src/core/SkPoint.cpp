/*
 * Copyright (C) 2006-2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "SkPoint.h"

void SkIPoint::rotateCW(SkIPoint* dst) const {
    SkASSERT(dst);

    // use a tmp in case this == dst
    int32_t tmp = fX;
    dst->fX = -fY;
    dst->fY = tmp;
}

void SkIPoint::rotateCCW(SkIPoint* dst) const {
    SkASSERT(dst);

    // use a tmp in case this == dst
    int32_t tmp = fX;
    dst->fX = fY;
    dst->fY = -tmp;
}

///////////////////////////////////////////////////////////////////////////////

void SkPoint::rotateCW(SkPoint* dst) const {
    SkASSERT(dst);

    // use a tmp in case this == dst
    SkScalar tmp = fX;
    dst->fX = -fY;
    dst->fY = tmp;
}

void SkPoint::rotateCCW(SkPoint* dst) const {
    SkASSERT(dst);

    // use a tmp in case this == dst
    SkScalar tmp = fX;
    dst->fX = fY;
    dst->fY = -tmp;
}

void SkPoint::scale(SkScalar scale, SkPoint* dst) const {
    SkASSERT(dst);
    dst->set(SkScalarMul(fX, scale), SkScalarMul(fY, scale));
}

#define kNearlyZero     (SK_Scalar1 / 8092)

bool SkPoint::normalize() {
    return this->setLength(fX, fY, SK_Scalar1);
}

bool SkPoint::setNormalize(SkScalar x, SkScalar y) {
    return this->setLength(x, y, SK_Scalar1);
}

bool SkPoint::setLength(SkScalar length) {
    return this->setLength(fX, fY, length);
}

#ifdef SK_SCALAR_IS_FLOAT

SkScalar SkPoint::Length(SkScalar dx, SkScalar dy) {
    return sk_float_sqrt(dx * dx + dy * dy);
}

bool SkPoint::setLength(float x, float y, float length) {
    float mag = sk_float_sqrt(x * x + y * y);
    if (mag > kNearlyZero) {
        length /= mag;
        fX = x * length;
        fY = y * length;
        return true;
    }
    return false;
}

#else

#include "Sk64.h"

SkScalar SkPoint::Length(SkScalar dx, SkScalar dy) {
    Sk64    tmp1, tmp2;

    tmp1.setMul(dx, dx);
    tmp2.setMul(dy, dy);
    tmp1.add(tmp2);

    return tmp1.getSqrt();
}

#ifdef SK_DEBUGx
static SkFixed fixlen(SkFixed x, SkFixed y) {
    float fx = (float)x;
    float fy = (float)y;

    return (int)floorf(sqrtf(fx*fx + fy*fy) + 0.5f);
}
#endif

static inline uint32_t squarefixed(unsigned x) {
    x >>= 16;
    return x*x;
}

#if 1   // Newton iter for setLength

static inline unsigned invsqrt_iter(unsigned V, unsigned U) {
    unsigned x = V * U >> 14;
    x = x * U >> 14;
    x = (3 << 14) - x;
    x = (U >> 1) * x >> 14;
    return x;
}

static const uint16_t gInvSqrt14GuessTable[] = {
    0x4000, 0x3c57, 0x393e, 0x3695, 0x3441, 0x3235, 0x3061,
    0x2ebd, 0x2d41, 0x2be7, 0x2aaa, 0x2987, 0x287a, 0x2780,
    0x2698, 0x25be, 0x24f3, 0x2434, 0x2380, 0x22d6, 0x2235,
    0x219d, 0x210c, 0x2083, 0x2000, 0x1f82, 0x1f0b, 0x1e99,
    0x1e2b, 0x1dc2, 0x1d5d, 0x1cfc, 0x1c9f, 0x1c45, 0x1bee,
    0x1b9b, 0x1b4a, 0x1afc, 0x1ab0, 0x1a67, 0x1a20, 0x19dc,
    0x1999, 0x1959, 0x191a, 0x18dd, 0x18a2, 0x1868, 0x1830,
    0x17fa, 0x17c4, 0x1791, 0x175e, 0x172d, 0x16fd, 0x16ce
};

#define BUILD_INVSQRT_TABLEx
#ifdef BUILD_INVSQRT_TABLE
static void build_invsqrt14_guess_table() {
    for (int i = 8; i <= 63; i++) {
        unsigned x = SkToU16((1 << 28) / SkSqrt32(i << 25));
        printf("0x%x, ", x);
    }
    printf("\n");
}
#endif

static unsigned fast_invsqrt(uint32_t x) {
#ifdef BUILD_INVSQRT_TABLE
    unsigned top2 = x >> 25;
    SkASSERT(top2 >= 8 && top2 <= 63);

    static bool gOnce;
    if (!gOnce) {
        build_invsqrt14_guess_table();
        gOnce = true;
    }
#endif

    unsigned V = x >> 14;   // make V .14

    unsigned top = x >> 25;
    SkASSERT(top >= 8 && top <= 63);
    SkASSERT(top - 8 < SK_ARRAY_COUNT(gInvSqrt14GuessTable));
    unsigned U = gInvSqrt14GuessTable[top - 8];
    
    U = invsqrt_iter(V, U);
    return invsqrt_iter(V, U);
}

/*  We "normalize" x,y to be .14 values (so we can square them and stay 32bits.
    Then we Newton-iterate this in .14 space to compute the invser-sqrt, and
    scale by it at the end. The .14 space means we can execute our iterations
    and stay in 32bits as well, making the multiplies much cheaper than calling
    SkFixedMul.
*/
bool SkPoint::setLength(SkFixed ox, SkFixed oy, SkFixed length) {
    if (ox == 0) {
        if (oy == 0) {
            return false;
        }
        this->set(0, SkApplySign(length, SkExtractSign(oy)));
        return true;
    }
    if (oy == 0) {
        this->set(SkApplySign(length, SkExtractSign(ox)), 0);
        return true;
    }

    unsigned x = SkAbs32(ox);
    unsigned y = SkAbs32(oy);
    int zeros = SkCLZ(x | y);

    // make x,y 1.14 values so our fast sqr won't overflow
    if (zeros > 17) {
        x <<= zeros - 17;
        y <<= zeros - 17; 
    } else {
        x >>= 17 - zeros;
        y >>= 17 - zeros;
    }
    SkASSERT((x | y) <= 0x7FFF);

    unsigned invrt = fast_invsqrt(x*x + y*y);

    x = x * invrt >> 12;
    y = y * invrt >> 12;

    if (length != SK_Fixed1) {
        x = SkFixedMul(x, length);
        y = SkFixedMul(y, length);
    }
    this->set(SkApplySign(x, SkExtractSign(ox)),
              SkApplySign(y, SkExtractSign(oy)));
    return true;
}
#else
/*
    Normalize x,y, and then scale them by length.

    The obvious way to do this would be the following:
        S64 tmp1, tmp2;
        tmp1.setMul(x,x);
        tmp2.setMul(y,y);
        tmp1.add(tmp2);
        len = tmp1.getSqrt();
        x' = SkFixedDiv(x, len);
        y' = SkFixedDiv(y, len);
    This is fine, but slower than what we do below.

    The present technique does not compute the starting length, but
    rather fiddles with x,y iteratively, all the while checking its
    magnitude^2 (avoiding a sqrt).

    We normalize by first shifting x,y so that at least one of them
    has bit 31 set (after taking the abs of them).
    Then we loop, refining x,y by squaring them and comparing
    against a very large 1.0 (1 << 28), and then adding or subtracting
    a delta (which itself is reduced by half each time through the loop).
    For speed we want the squaring to be with a simple integer mul. To keep
    that from overflowing we shift our coordinates down until we are dealing
    with at most 15 bits (2^15-1)^2 * 2 says withing 32 bits)
    When our square is close to 1.0, we shift x,y down into fixed range.
*/
bool SkPoint::setLength(SkFixed ox, SkFixed oy, SkFixed length) {
    if (ox == 0) {
        if (oy == 0)
            return false;
        this->set(0, SkApplySign(length, SkExtractSign(oy)));
        return true;
    }
    if (oy == 0) {
        this->set(SkApplySign(length, SkExtractSign(ox)), 0);
        return true;
    }

    SkFixed x = SkAbs32(ox);
    SkFixed y = SkAbs32(oy);

    // shift x,y so that the greater of them is 15bits (1.14 fixed point)
    {
        int shift = SkCLZ(x | y);
        // make them .30
        x <<= shift - 1;
        y <<= shift - 1;
    }

    SkFixed dx = x;
    SkFixed dy = y;

    for (int i = 0; i < 17; i++) {
        dx >>= 1;
        dy >>= 1;

        U32 len2 = squarefixed(x) + squarefixed(y);
        if (len2 >> 28) {
            x -= dx;
            y -= dy;
        } else {
            x += dx;
            y += dy;
        }
    }
    x >>= 14;
    y >>= 14;

#ifdef SK_DEBUGx    // measure how far we are from unit-length
    {
        static int gMaxError;
        static int gMaxDiff;

        SkFixed len = fixlen(x, y);
        int err = len - SK_Fixed1;
        err = SkAbs32(err);

        if (err > gMaxError) {
            gMaxError = err;
            SkDebugf("gMaxError %d\n", err);
        }

        float fx = SkAbs32(ox)/65536.0f;
        float fy = SkAbs32(oy)/65536.0f;
        float mag = sqrtf(fx*fx + fy*fy);
        fx /= mag;
        fy /= mag;
        SkFixed xx = (int)floorf(fx * 65536 + 0.5f);
        SkFixed yy = (int)floorf(fy * 65536 + 0.5f);
        err = SkMax32(SkAbs32(xx-x), SkAbs32(yy-y));
        if (err > gMaxDiff) {
            gMaxDiff = err;
            SkDebugf("gMaxDiff %d\n", err);
        }
    }
#endif

    x = SkApplySign(x, SkExtractSign(ox));
    y = SkApplySign(y, SkExtractSign(oy));
    if (length != SK_Fixed1) {
        x = SkFixedMul(x, length);
        y = SkFixedMul(y, length);
    }
    
    this->set(x, y);
    return true;
}
#endif

#endif

