/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkColor.h"
#include "SkColorPriv.h"
#include "SkFixed.h"

SkPMColor SkPreMultiplyARGB(U8CPU a, U8CPU r, U8CPU g, U8CPU b) {
    return SkPremultiplyARGBInline(a, r, g, b);
}

SkPMColor SkPreMultiplyColor(SkColor c) {
    return SkPremultiplyARGBInline(SkColorGetA(c), SkColorGetR(c),
                                   SkColorGetG(c), SkColorGetB(c));
}

///////////////////////////////////////////////////////////////////////////////

static inline SkScalar ByteToScalar(U8CPU x) {
    SkASSERT(x <= 255);
    return SkIntToScalar(x) / 255;
}

static inline SkScalar ByteDivToScalar(int numer, U8CPU denom) {
    // cast to keep the answer signed
    return SkIntToScalar(numer) / (int)denom;
}

void SkRGBToHSV(U8CPU r, U8CPU g, U8CPU b, SkScalar hsv[3]) {
    SkASSERT(hsv);

    unsigned min = SkMin32(r, SkMin32(g, b));
    unsigned max = SkMax32(r, SkMax32(g, b));
    unsigned delta = max - min;

    SkScalar v = ByteToScalar(max);
    SkASSERT(v >= 0 && v <= SK_Scalar1);

    if (0 == delta) { // we're a shade of gray
        hsv[0] = 0;
        hsv[1] = 0;
        hsv[2] = v;
        return;
    }

    SkScalar s = ByteDivToScalar(delta, max);
    SkASSERT(s >= 0 && s <= SK_Scalar1);

    SkScalar h;
    if (r == max) {
        h = ByteDivToScalar(g - b, delta);
    } else if (g == max) {
        h = SkIntToScalar(2) + ByteDivToScalar(b - r, delta);
    } else { // b == max
        h = SkIntToScalar(4) + ByteDivToScalar(r - g, delta);
    }

    h *= 60;
    if (h < 0) {
        h += SkIntToScalar(360);
    }
    SkASSERT(h >= 0 && h < SkIntToScalar(360));

    hsv[0] = h;
    hsv[1] = s;
    hsv[2] = v;
}

SkColor SkHSVToColor(U8CPU a, const SkScalar hsv[3]) {
    SkASSERT(hsv);

    U8CPU s = SkUnitScalarClampToByte(hsv[1]);
    U8CPU v = SkUnitScalarClampToByte(hsv[2]);

    if (0 == s) { // shade of gray
        return SkColorSetARGB(a, v, v, v);
    }
    SkFixed hx = (hsv[0] < 0 || hsv[0] >= SkIntToScalar(360)) ? 0 : SkScalarToFixed(hsv[0]/60);
    SkFixed f = hx & 0xFFFF;

    unsigned v_scale = SkAlpha255To256(v);
    unsigned p = SkAlphaMul(255 - s, v_scale);
    unsigned q = SkAlphaMul(255 - (s * f >> 16), v_scale);
    unsigned t = SkAlphaMul(255 - (s * (SK_Fixed1 - f) >> 16), v_scale);

    unsigned r, g, b;

    SkASSERT((unsigned)(hx >> 16) < 6);
    switch (hx >> 16) {
        case 0: r = v; g = t; b = p; break;
        case 1: r = q; g = v; b = p; break;
        case 2: r = p; g = v; b = t; break;
        case 3: r = p; g = q; b = v; break;
        case 4: r = t;  g = p; b = v; break;
        default: r = v; g = p; b = q; break;
    }
    return SkColorSetARGB(a, r, g, b);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
#include "SkPM4fPriv.h"
#include "SkHalf.h"

SkPM4f SkPM4f::FromPMColor(SkPMColor c) {
    Sk4f value = to_4f_rgba(c);
    SkPM4f c4;
    (value * Sk4f(1.0f / 255)).store(&c4);
    return c4;
}

SkColor4f SkPM4f::unpremul() const {
    float alpha = fVec[A];
    if (0 == alpha) {
        return { 0, 0, 0, 0 };
    } else {
        float invAlpha = 1 / alpha;
        return { fVec[R] * invAlpha, fVec[G] * invAlpha, fVec[B] * invAlpha, alpha };
    }
}

void SkPM4f::toF16(uint16_t half[4]) const {
    for (int i = 0; i < 4; ++i) {
        half[i] = SkFloatToHalf(fVec[i]);
    }
}

uint64_t SkPM4f::toF16() const {
    uint64_t value;
    this->toF16(reinterpret_cast<uint16_t*>(&value));
    return value;
}

SkPM4f SkPM4f::FromF16(const uint16_t half[4]) {
    return {{
        SkHalfToFloat(half[0]),
        SkHalfToFloat(half[1]),
        SkHalfToFloat(half[2]),
        SkHalfToFloat(half[3])
    }};
}

#ifdef SK_DEBUG
void SkPM4f::assertIsUnit() const {
    auto c4 = Sk4f::Load(fVec);
    SkASSERT((c4 >= Sk4f(0)).allTrue() && (c4 <= Sk4f(1)).allTrue());
}
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////

SkColor4f SkColor4f::FromColor(SkColor c) {
    Sk4f value = SkNx_shuffle<2,1,0,3>(SkNx_cast<float>(Sk4b::Load(&c)));
    SkColor4f c4;
    (value * Sk4f(1.0f / 255)).store(&c4);
    c4.fR = srgb_to_linear(c4.fR);
    c4.fG = srgb_to_linear(c4.fG);
    c4.fB = srgb_to_linear(c4.fB);
    return c4;
}

SkColor SkColor4f::toSkColor() const {
    SkColor result;
    Sk4f value = Sk4f(linear_to_srgb(fB), linear_to_srgb(fG), linear_to_srgb(fR), fA);
    SkNx_cast<uint8_t>(value * Sk4f(255) + Sk4f(0.5f)).store(&result);
    return result;
}

SkColor4f SkColor4f::Pin(float r, float g, float b, float a) {
    SkColor4f c4;
    Sk4f::Min(Sk4f::Max(Sk4f(r, g, b, a), Sk4f(0)), Sk4f(1)).store(c4.vec());
    return c4;
}

SkPM4f SkColor4f::premul() const {
    auto src = Sk4f::Load(this->pin().vec());
    float srcAlpha = src[3];  // need the pinned version of our alpha
    src = src * Sk4f(srcAlpha, srcAlpha, srcAlpha, 1);

    return SkPM4f::From4f(src);
}
