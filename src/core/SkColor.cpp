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
#include "SkNx.h"

SkPM4f SkPM4f::FromPMColor(SkPMColor c) {
    Sk4f value = SkNx_cast<float>(Sk4b::Load((const uint8_t*)&c));
    SkPM4f c4;
    (value * Sk4f(1.0f / 255)).store(c4.fVec);
    return c4;
}

SkColor4f SkColor4f::FromColor(SkColor c) {
    Sk4f value = SkNx_shuffle<3,2,1,0>(SkNx_cast<float>(Sk4b::Load((const uint8_t*)&c)));
    SkColor4f c4;
    (value * Sk4f(1.0f / 255)).store(c4.vec());
    return c4;
}

SkColor4f SkColor4f::Pin(float a, float r, float g, float b) {
    SkColor4f c4;
    Sk4f::Min(Sk4f::Max(Sk4f(a, r, g, b), Sk4f(0)), Sk4f(1)).store(c4.vec());
    return c4;
}

SkPM4f SkColor4f::premul() const {
    auto src = Sk4f::Load(this->pin().vec());
    float srcAlpha = src.kth<0>();  // need the pinned version of our alpha
    src = src * Sk4f(1, srcAlpha, srcAlpha, srcAlpha);

#ifdef SK_PMCOLOR_IS_BGRA
    // ARGB -> BGRA
    Sk4f dst = SkNx_shuffle<3,2,1,0>(src);
#else
    // ARGB -> RGBA
    Sk4f dst = SkNx_shuffle<1,2,3,0>(src);
#endif

    SkPM4f pm4;
    dst.store(pm4.fVec);
    return pm4;
}

bool SkPM4f::isUnit() const {
    auto c4 = Sk4f::Load(fVec);
    return (c4 >= Sk4f(0)).allTrue() && (c4 <= Sk4f(1)).allTrue();
}
