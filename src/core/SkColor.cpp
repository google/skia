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

    SkScalar s = SkScalarPin(hsv[1], 0, 1);
    SkScalar v = SkScalarPin(hsv[2], 0, 1);

    U8CPU v_byte = SkScalarRoundToInt(v * 255);

    if (SkScalarNearlyZero(s)) { // shade of gray
        return SkColorSetARGB(a, v_byte, v_byte, v_byte);
    }
    SkScalar hx = (hsv[0] < 0 || hsv[0] >= SkIntToScalar(360)) ? 0 : hsv[0]/60;
    SkScalar w = SkScalarFloorToScalar(hx);
    SkScalar f = hx - w;

    unsigned p = SkScalarRoundToInt((SK_Scalar1 - s) * v * 255);
    unsigned q = SkScalarRoundToInt((SK_Scalar1 - (s * f)) * v * 255);
    unsigned t = SkScalarRoundToInt((SK_Scalar1 - (s * (SK_Scalar1 - f))) * v * 255);

    unsigned r, g, b;

    SkASSERT((unsigned)(w) < 6);
    switch ((unsigned)(w)) {
        case 0: r = v_byte;  g = t;      b = p; break;
        case 1: r = q;       g = v_byte; b = p; break;
        case 2: r = p;       g = v_byte; b = t; break;
        case 3: r = p;       g = q;      b = v_byte; break;
        case 4: r = t;       g = p;      b = v_byte; break;
        default: r = v_byte; g = p;      b = q; break;
    }
    return SkColorSetARGB(a, r, g, b);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
#include "SkPM4fPriv.h"
#include "SkHalf.h"

SkPM4f SkPM4f::FromPMColor(SkPMColor c) {
    return From4f(swizzle_rb_if_bgra(Sk4f_fromL32(c)));
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

SkColor4f SkColor4f::FromColor(SkColor bgra) {
    SkColor4f rgba;
    swizzle_rb(Sk4f_fromS32(bgra)).store(rgba.vec());
    return rgba;
}

SkColor4f SkColor4f::FromColor3f(SkColor3f color3f, float a) {
    SkColor4f rgba;
    rgba.fR = color3f.fX;
    rgba.fG = color3f.fY;
    rgba.fB = color3f.fZ;
    rgba.fA = a;
    return rgba;
}

SkColor SkColor4f::toSkColor() const {
    return Sk4f_toS32(swizzle_rb(Sk4f::Load(this->vec())));
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
