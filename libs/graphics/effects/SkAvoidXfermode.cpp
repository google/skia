/* libs/graphics/effects/SkAvoidXfermode.cpp
**
** Copyright 2006, Google Inc.
**
** Licensed under the Apache License, Version 2.0 (the "License"); 
** you may not use this file except in compliance with the License. 
** You may obtain a copy of the License at 
**
**     http://www.apache.org/licenses/LICENSE-2.0 
**
** Unless required by applicable law or agreed to in writing, software 
** distributed under the License is distributed on an "AS IS" BASIS, 
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
** See the License for the specific language governing permissions and 
** limitations under the License.
*/

#include "SkAvoidXfermode.h"
#include "SkColorPriv.h"

SkAvoidXfermode::SkAvoidXfermode(SkColor opColor, U8CPU tolerance, bool reverse)
{    
    if (tolerance > 255)
        tolerance = 255;

    fOpColor = opColor;
    fDistMul = (256 << 14) / (tolerance + 1);
    fReverse = reverse;
}

static unsigned color_dist16(uint16_t c, unsigned r, unsigned g, unsigned b)
{
    unsigned dr = SkAbs32(SkPacked16ToR32(c) - r);
    unsigned dg = SkAbs32(SkPacked16ToG32(c) - g);
    unsigned db = SkAbs32(SkPacked16ToB32(c) - b);
    
    return SkMax32(dr, SkMax32(dg, db));
}

static unsigned color_dist32(SkPMColor c, U8CPU r, U8CPU g, U8CPU b)
{
    unsigned dr = SkAbs32(SkGetPackedR32(c) - r);
    unsigned dg = SkAbs32(SkGetPackedG32(c) - g);
    unsigned db = SkAbs32(SkGetPackedB32(c) - b);
    
    return SkMax32(dr, SkMax32(dg, db));
}

static float dot14tofloat(int x)
{
    return (float)x / (1 << 14);
}

static int scale_dist_14(int dist, uint32_t mul, uint32_t sub)
{
    int tmp = dist * mul - sub;
    int result = (tmp + (1 << 13)) >> 14;

    return result;
}

static SkPMColor SkFourByteInterp(SkPMColor src, SkPMColor dst, U8CPU alpha)
{
    unsigned scale = SkAlpha255To256(alpha);

    unsigned a = SkAlphaBlend(SkGetPackedA32(src), SkGetPackedA32(dst), scale);
    unsigned r = SkAlphaBlend(SkGetPackedR32(src), SkGetPackedR32(dst), scale);
    unsigned g = SkAlphaBlend(SkGetPackedG32(src), SkGetPackedG32(dst), scale);
    unsigned b = SkAlphaBlend(SkGetPackedB32(src), SkGetPackedB32(dst), scale);

    return SkPackARGB32(a, r, g, b);
}

void SkAvoidXfermode::xfer32(SkPMColor dst[], const SkPMColor src[], int count, const SkAlpha aa[])
{
    // override in subclass
}

static inline U16CPU SkBlend3216(SkPMColor src, U16CPU dst, unsigned scale)
{
    SkASSERT(scale <= 256);

    return SkPackRGB16( SkAlphaBlend(SkPacked32ToR16(src), SkGetPackedR16(dst), scale),
                        SkAlphaBlend(SkPacked32ToG16(src), SkGetPackedG16(dst), scale),
                        SkAlphaBlend(SkPacked32ToB16(src), SkGetPackedB16(dst), scale));
}

void SkAvoidXfermode::xfer16(uint16_t dst[], const SkPMColor src[], int count, const SkAlpha aa[])
{
    SkColor opColor = fOpColor;
    unsigned    opR = SkPacked16ToR32(opColor);
    unsigned    opG = SkPacked16ToG32(opColor);
    unsigned    opB = SkPacked16ToB32(opColor);
    uint32_t    mul = fDistMul;
    uint32_t    sub = (fDistMul - (1 << 14)) << 8;

    int MAX, mask;
    
    if (fReverse)
    {
        mask = -1;
        MAX = 256;
    }
    else
    {
        mask = 0;
        MAX = 0;
    }

    if (NULL == aa)
        for (int i = 0; i < count; i++) {
            int d = color_dist16(dst[i], opR, opG, opB);
            d = SkAlpha255To256(d);
            // now reverse d if we need to
            d = MAX + (d ^ mask) - mask;
            SkASSERT((unsigned)d <= 256);

            d = scale_dist_14(d, mul, sub);
            SkASSERT(d <= 256);

            if (d > 0)
                dst[i] = SkBlend3216(src[i], dst[i], d);
        }
    else
        for (int i = 0; i < count; i++) {
            unsigned antialias = aa[i];
            if (0 == antialias)
                continue;

            int d = color_dist16(dst[i], opR, opG, opB);
            d = SkAlpha255To256(d);
            // now reverse d if we need to
            d = MAX + (d ^ mask) - mask;
            SkASSERT((unsigned)d <= 256);

            d = scale_dist_14(d, mul, sub);
            SkASSERT(d <= 256);

            if (d > 0)
                dst[i] = SkBlend3216(src[i], dst[i], SkAlphaMul(antialias, d));
        }
}

void SkAvoidXfermode::xferA8(SkAlpha dst[], const SkPMColor src[], int count, const SkAlpha aa[])
{
    // override in subclass
}

