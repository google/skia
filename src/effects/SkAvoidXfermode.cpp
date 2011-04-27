/* libs/graphics/effects/SkAvoidXfermode.cpp
**
** Copyright 2006, The Android Open Source Project
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

SkAvoidXfermode::SkAvoidXfermode(SkColor opColor, U8CPU tolerance, Mode mode)
{
    if (tolerance > 255) {
        tolerance = 255;
    }

    fOpColor = opColor;
    fDistMul = (256 << 14) / (tolerance + 1);
    fMode = mode;
}

SkAvoidXfermode::SkAvoidXfermode(SkFlattenableReadBuffer& buffer)
    : INHERITED(buffer)
{
    fOpColor = buffer.readU32();
    fDistMul = buffer.readU32();
    fMode = (Mode)buffer.readU8();
}

void SkAvoidXfermode::flatten(SkFlattenableWriteBuffer& buffer)
{
    this->INHERITED::flatten(buffer);

    buffer.write32(fOpColor);
    buffer.write32(fDistMul);
    buffer.write8(fMode);
}

SkFlattenable* SkAvoidXfermode::Create(SkFlattenableReadBuffer& rb)
{
    return SkNEW_ARGS(SkAvoidXfermode, (rb));
}

SkFlattenable::Factory SkAvoidXfermode::getFactory()
{
    return Create;
}

// returns 0..31
static unsigned color_dist16(uint16_t c, unsigned r, unsigned g, unsigned b)
{
    SkASSERT(r <= SK_R16_MASK);
    SkASSERT(g <= SK_G16_MASK);
    SkASSERT(b <= SK_B16_MASK);

    unsigned dr = SkAbs32(SkGetPackedR16(c) - r);
    unsigned dg = SkAbs32(SkGetPackedG16(c) - g) >> (SK_G16_BITS - SK_R16_BITS);
    unsigned db = SkAbs32(SkGetPackedB16(c) - b);

    return SkMax32(dr, SkMax32(dg, db));
}

// returns 0..15
static unsigned color_dist4444(uint16_t c, unsigned r, unsigned g, unsigned b)
{
    SkASSERT(r <= 0xF);
    SkASSERT(g <= 0xF);
    SkASSERT(b <= 0xF);

    unsigned dr = SkAbs32(SkGetPackedR4444(c) - r);
    unsigned dg = SkAbs32(SkGetPackedG4444(c) - g);
    unsigned db = SkAbs32(SkGetPackedB4444(c) - b);

    return SkMax32(dr, SkMax32(dg, db));
}

// returns 0..255
static unsigned color_dist32(SkPMColor c, U8CPU r, U8CPU g, U8CPU b)
{
    SkASSERT(r <= 0xFF);
    SkASSERT(g <= 0xFF);
    SkASSERT(b <= 0xFF);

    unsigned dr = SkAbs32(SkGetPackedR32(c) - r);
    unsigned dg = SkAbs32(SkGetPackedG32(c) - g);
    unsigned db = SkAbs32(SkGetPackedB32(c) - b);

    return SkMax32(dr, SkMax32(dg, db));
}

static int scale_dist_14(int dist, uint32_t mul, uint32_t sub)
{
    int tmp = dist * mul - sub;
    int result = (tmp + (1 << 13)) >> 14;

    return result;
}

static SkPMColor SkFourByteInterp(SkPMColor src, SkPMColor dst, unsigned scale)
{
    unsigned a = SkAlphaBlend(SkGetPackedA32(src), SkGetPackedA32(dst), scale);
    unsigned r = SkAlphaBlend(SkGetPackedR32(src), SkGetPackedR32(dst), scale);
    unsigned g = SkAlphaBlend(SkGetPackedG32(src), SkGetPackedG32(dst), scale);
    unsigned b = SkAlphaBlend(SkGetPackedB32(src), SkGetPackedB32(dst), scale);

    return SkPackARGB32(a, r, g, b);
}

static inline unsigned Accurate255To256(unsigned x) {
    return x + (x >> 7);
}

void SkAvoidXfermode::xfer32(SkPMColor dst[], const SkPMColor src[], int count,
                             const SkAlpha aa[])
{
    unsigned    opR = SkColorGetR(fOpColor);
    unsigned    opG = SkColorGetG(fOpColor);
    unsigned    opB = SkColorGetB(fOpColor);
    uint32_t    mul = fDistMul;
    uint32_t    sub = (fDistMul - (1 << 14)) << 8;

    int MAX, mask;

    if (kTargetColor_Mode == fMode) {
        mask = -1;
        MAX = 255;
    } else {
        mask = 0;
        MAX = 0;
    }

    for (int i = 0; i < count; i++) {
        int d = color_dist32(dst[i], opR, opG, opB);
        // now reverse d if we need to
        d = MAX + (d ^ mask) - mask;
        SkASSERT((unsigned)d <= 255);
        d = Accurate255To256(d);

        d = scale_dist_14(d, mul, sub);
        SkASSERT(d <= 256);

        if (d > 0) {
            if (NULL != aa) {
                d = SkAlphaMul(d, Accurate255To256(*aa++));
                if (0 == d) {
                    continue;
                }
            }
            dst[i] = SkFourByteInterp(src[i], dst[i], d);
        }
    }
}

static inline U16CPU SkBlend3216(SkPMColor src, U16CPU dst, unsigned scale)
{
    SkASSERT(scale <= 32);
    scale <<= 3;

    return SkPackRGB16( SkAlphaBlend(SkPacked32ToR16(src), SkGetPackedR16(dst), scale),
                        SkAlphaBlend(SkPacked32ToG16(src), SkGetPackedG16(dst), scale),
                        SkAlphaBlend(SkPacked32ToB16(src), SkGetPackedB16(dst), scale));
}

void SkAvoidXfermode::xfer16(uint16_t dst[], const SkPMColor src[], int count,
                             const SkAlpha aa[])
{
    unsigned    opR = SkColorGetR(fOpColor) >> (8 - SK_R16_BITS);
    unsigned    opG = SkColorGetG(fOpColor) >> (8 - SK_G16_BITS);
    unsigned    opB = SkColorGetB(fOpColor) >> (8 - SK_R16_BITS);
    uint32_t    mul = fDistMul;
    uint32_t    sub = (fDistMul - (1 << 14)) << SK_R16_BITS;

    int MAX, mask;

    if (kTargetColor_Mode == fMode) {
        mask = -1;
        MAX = 31;
    } else {
        mask = 0;
        MAX = 0;
    }

    for (int i = 0; i < count; i++) {
        int d = color_dist16(dst[i], opR, opG, opB);
        // now reverse d if we need to
        d = MAX + (d ^ mask) - mask;
        SkASSERT((unsigned)d <= 31);
        // convert from 0..31 to 0..32
        d += d >> 4;
        d = scale_dist_14(d, mul, sub);
        SkASSERT(d <= 32);

        if (d > 0) {
            if (NULL != aa) {
                d = SkAlphaMul(d, Accurate255To256(*aa++));
                if (0 == d) {
                    continue;
                }
            }
            dst[i] = SkBlend3216(src[i], dst[i], d);
        }
    }
}

void SkAvoidXfermode::xfer4444(uint16_t dst[], const SkPMColor src[], int count,
                               const SkAlpha aa[])
{
    unsigned    opR = SkColorGetR(fOpColor) >> 4;
    unsigned    opG = SkColorGetG(fOpColor) >> 4;
    unsigned    opB = SkColorGetB(fOpColor) >> 4;
    uint32_t    mul = fDistMul;
    uint32_t    sub = (fDistMul - (1 << 14)) << 4;

    int MAX, mask;

    if (kTargetColor_Mode == fMode) {
        mask = -1;
        MAX = 15;
    } else {
        mask = 0;
        MAX = 0;
    }

    for (int i = 0; i < count; i++) {
        int d = color_dist4444(dst[i], opR, opG, opB);
        // now reverse d if we need to
        d = MAX + (d ^ mask) - mask;
        SkASSERT((unsigned)d <= 15);
        // convert from 0..15 to 0..16
        d += d >> 3;
        d = scale_dist_14(d, mul, sub);
        SkASSERT(d <= 16);

        if (d > 0) {
            if (NULL != aa) {
                d = SkAlphaMul(d, Accurate255To256(*aa++));
                if (0 == d) {
                    continue;
                }
            }
            dst[i] = SkBlend4444(SkPixel32ToPixel4444(src[i]), dst[i], d);
        }
    }
}

void SkAvoidXfermode::xferA8(SkAlpha dst[], const SkPMColor src[], int count, const SkAlpha aa[])
{
    // override in subclass
}

static SkFlattenable::Registrar
    gSkAvoidXfermodeReg("SkAvoidXfermode", SkAvoidXfermode::CreateProc);
