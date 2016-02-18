/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPM4f_DEFINED
#define SkPM4f_DEFINED

#include "SkColorPriv.h"

/*
 *  The float values are 0...1 premultiplied
 */
struct SkPM4f {
    enum {
        A = SK_A32_SHIFT/8,
        R = SK_R32_SHIFT/8,
        G = SK_G32_SHIFT/8,
        B = SK_B32_SHIFT/8,
    };
    float fVec[4];

    float a() const { return fVec[A]; }

    SkColor4f unpremul() const;

    static SkPM4f FromPMColor(SkPMColor);

    // half-float routines
    void toF16(uint16_t[4]) const;
    uint64_t toF16() const; // 4 float16 values packed into uint64_t
    static SkPM4f FromF16(const uint16_t[4]);

#ifdef SK_DEBUG
    void assertIsUnit() const;
#else
    void assertIsUnit() const {}
#endif
};

typedef SkPM4f (*SkXfermodeProc4f)(const SkPM4f& src, const SkPM4f& dst);


#endif
