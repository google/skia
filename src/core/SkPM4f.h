/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPM4f_DEFINED
#define SkPM4f_DEFINED

#include "SkColorPriv.h"
#include "SkNx.h"

static inline Sk4f swizzle_rb(const Sk4f& x) {
    return SkNx_shuffle<2, 1, 0, 3>(x);
}

static inline Sk4f swizzle_rb_if_bgra(const Sk4f& x) {
#ifdef SK_PMCOLOR_IS_BGRA
    return swizzle_rb(x);
#else
    return x;
#endif
}

/*
 *  The float values are 0...1 premultiplied in RGBA order (regardless of SkPMColor order)
 */
struct SkPM4f {
    enum {
        R, G, B, A,
    };
    float fVec[4];

    float r() const { return fVec[R]; }
    float g() const { return fVec[G]; }
    float b() const { return fVec[B]; }
    float a() const { return fVec[A]; }

    static SkPM4f FromPremulRGBA(float r, float g, float b, float a) {
        SkPM4f p;
        p.fVec[R] = r;
        p.fVec[G] = g;
        p.fVec[B] = b;
        p.fVec[A] = a;
        return p;
    }

    static SkPM4f From4f(const Sk4f& x) {
        SkPM4f pm;
        x.store(pm.fVec);
        return pm;
    }
    static SkPM4f FromF16(const uint16_t[4]);
    static SkPM4f FromPMColor(SkPMColor);

    Sk4f to4f() const { return Sk4f::Load(fVec); }
    Sk4f to4f_rgba() const { return this->to4f(); }
    Sk4f to4f_bgra() const { return swizzle_rb(this->to4f()); }
    Sk4f to4f_pmorder() const { return swizzle_rb_if_bgra(this->to4f()); }

    SkPMColor toPMColor() const {
        Sk4f value = swizzle_rb_if_bgra(this->to4f());
        SkPMColor result;
        SkNx_cast<uint8_t>(value * Sk4f(255) + Sk4f(0.5f)).store(&result);
        return result;
    }

    void toF16(uint16_t[4]) const;
    uint64_t toF16() const; // 4 float16 values packed into uint64_t

    SkColor4f unpremul() const;

#ifdef SK_DEBUG
    void assertIsUnit() const;
#else
    void assertIsUnit() const {}
#endif
};

#endif
