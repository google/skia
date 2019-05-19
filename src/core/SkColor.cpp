/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkColor.h"
#include "include/private/SkColorData.h"
#include "include/private/SkFixed.h"

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

template <>
SkColor4f SkColor4f::FromColor(SkColor bgra) {
    SkColor4f rgba;
    swizzle_rb(Sk4f_fromL32(bgra)).store(rgba.vec());
    return rgba;
}

template <>
SkColor SkColor4f::toSkColor() const {
    return Sk4f_toL32(swizzle_rb(Sk4f::Load(this->vec())));
}

template <>
uint32_t SkColor4f::toBytes_RGBA() const {
    return Sk4f_toL32(Sk4f::Load(this->vec()));
}

template <>
SkColor4f SkColor4f::FromBytes_RGBA(uint32_t c) {
    SkColor4f color;
    Sk4f_fromL32(c).store(&color);
    return color;
}

template <>
SkPMColor4f SkPMColor4f::FromPMColor(SkPMColor c) {
    SkPMColor4f color;
    swizzle_rb_if_bgra(Sk4f_fromL32(c)).store(&color);
    return color;
}

template <>
uint32_t SkPMColor4f::toBytes_RGBA() const {
    return Sk4f_toL32(Sk4f::Load(this->vec()));
}

template <>
SkPMColor4f SkPMColor4f::FromBytes_RGBA(uint32_t c) {
    SkPMColor4f color;
    Sk4f_fromL32(c).store(&color);
    return color;
}
