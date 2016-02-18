/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkAtomics.h"
#include "SkColorSpace.h"

static inline bool SkFloatIsFinite(float x) { return 0 == x * 0; }

//
// SkFloat3x3
//
// In memory order, values are a, b, c, d, e, f, g, h, i
//
// When applied to a color component vector (e.g. [ r, r, r ] or [ g, g, g ] we do
//
// [ r r r ] * [ a b c ] + [ g g g ] * [ d e f ] + [ b b b ] * [ g h i ]
//
// Thus in our point-on-the-right notation, the matrix looks like
//
// [ a d g ]   [ r ]
// [ b e h ] * [ g ]
// [ c f i ]   [ b ]
//
static SkFloat3x3 concat(const SkFloat3x3& left, const SkFloat3x3& rite) {
    SkFloat3x3 result;
    for (int row = 0; row < 3; ++row) {
        for (int col = 0; col < 3; ++col) {
            double tmp = 0;
            for (int i = 0; i < 3; ++i) {
                tmp += (double)left.fMat[row + i * 3] * rite.fMat[i + col * 3];
            }
            result.fMat[row + col * 3] = (double)tmp;
        }
    }
    return result;
}

static double det(const SkFloat3x3& m) {
    return (double)m.fMat[0] * m.fMat[4] * m.fMat[8] +
           (double)m.fMat[3] * m.fMat[7] * m.fMat[2] +
           (double)m.fMat[6] * m.fMat[1] * m.fMat[5] -
           (double)m.fMat[0] * m.fMat[7] * m.fMat[5] -
           (double)m.fMat[3] * m.fMat[1] * m.fMat[8] -
           (double)m.fMat[6] * m.fMat[4] * m.fMat[2];
}

static double det2x2(const SkFloat3x3& m, int a, int b, int c, int d) {
    return (double)m.fMat[a] * m.fMat[b] - (double)m.fMat[c] * m.fMat[d];
}

static SkFloat3x3 invert(const SkFloat3x3& m) {
    double d = det(m);
    SkASSERT(SkFloatIsFinite((float)d));
    double scale = 1 / d;
    SkASSERT(SkFloatIsFinite((float)scale));

    return {{
        (float)(scale * det2x2(m, 4, 8, 5, 7)),
        (float)(scale * det2x2(m, 7, 2, 8, 1)),
        (float)(scale * det2x2(m, 1, 5, 2, 4)),

        (float)(scale * det2x2(m, 6, 5, 8, 3)),
        (float)(scale * det2x2(m, 0, 8, 2, 6)),
        (float)(scale * det2x2(m, 3, 2, 5, 0)),

        (float)(scale * det2x2(m, 3, 7, 4, 6)),
        (float)(scale * det2x2(m, 6, 1, 7, 0)),
        (float)(scale * det2x2(m, 0, 4, 1, 3)),
    }};
}

void SkFloat3::dump() const {
    SkDebugf("[%7.4f %7.4f %7.4f]\n", fVec[0], fVec[1], fVec[2]);
}

void SkFloat3x3::dump() const {
    SkDebugf("[%7.4f %7.4f %7.4f] [%7.4f %7.4f %7.4f] [%7.4f %7.4f %7.4f]\n",
             fMat[0], fMat[1], fMat[2],
             fMat[3], fMat[4], fMat[5],
             fMat[6], fMat[7], fMat[8]);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

static int32_t gUniqueColorSpaceID;

SkColorSpace::SkColorSpace(const SkFloat3x3& toXYZD50, const SkFloat3& gamma, Named named)
    : fToXYZD50(toXYZD50)
    , fGamma(gamma)
    , fUniqueID(sk_atomic_inc(&gUniqueColorSpaceID))
    , fNamed(named)
{
    for (int i = 0; i < 3; ++i) {
        SkASSERT(SkFloatIsFinite(gamma.fVec[i]));
        for (int j = 0; j < 3; ++j) {
            SkASSERT(SkFloatIsFinite(toXYZD50.fMat[3*i + j]));
        }
    }
}

SkColorSpace* SkColorSpace::NewRGB(const SkFloat3x3& toXYZD50, const SkFloat3& gamma) {
    for (int i = 0; i < 3; ++i) {
        if (!SkFloatIsFinite(gamma.fVec[i]) || gamma.fVec[i] < 0) {
            return nullptr;
        }
        for (int j = 0; j < 3; ++j) {
            if (!SkFloatIsFinite(toXYZD50.fMat[3*i + j])) {
                return nullptr;
            }
        }
    }

    // check the matrix for invertibility
    float d = det(toXYZD50);
    if (!SkFloatIsFinite(d) || !SkFloatIsFinite(1 / d)) {
        return nullptr;
    }

    return new SkColorSpace(toXYZD50, gamma, kUnknown_Named);
}

void SkColorSpace::dump() const {
    fToXYZD50.dump();
    fGamma.dump();
}

//////////////////////////////////////////////////////////////////////////////////////////////////

const SkFloat3   gDevice_gamma {{ 0, 0, 0 }};
const SkFloat3x3 gDevice_toXYZD50 {{
    1, 0, 0,
    0, 1, 0,
    0, 0, 1
}};

const SkFloat3 gSRGB_gamma {{ 2.2f, 2.2f, 2.2f }};
const SkFloat3x3 gSRGB_toXYZD50 {{
    0.4358f, 0.2224f, 0.0139f,    // * R
    0.3853f, 0.7170f, 0.0971f,    // * G
    0.1430f, 0.0606f, 0.7139f,    // * B
}};

SkColorSpace* SkColorSpace::NewNamed(Named named) {
    switch (named) {
        case kDevice_Named:
            return new SkColorSpace(gDevice_toXYZD50, gDevice_gamma, kDevice_Named);
        case kSRGB_Named:
            return new SkColorSpace(gSRGB_toXYZD50, gSRGB_gamma, kSRGB_Named);
        default:
            break;
    }
    return nullptr;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

SkColorSpace::Result SkColorSpace::Concat(const SkColorSpace* src, const SkColorSpace* dst,
                                          SkFloat3x3* result) {
    if (!src || !dst || (src->named() == kDevice_Named) || (src->named() == dst->named())) {
        if (result) {
            *result = {{ 1, 0, 0, 0, 1, 0, 0, 0, 1 }};
        }
        return kIdentity_Result;
    }
    if (result) {
        *result = concat(src->fToXYZD50, invert(dst->fToXYZD50));
    }
    return kNormal_Result;
}

#include "SkColor.h"
#include "SkNx.h"
#include "SkPM4f.h"

void SkApply3x3ToPM4f(const SkFloat3x3& m, const SkPM4f src[], SkPM4f dst[], int count) {
    SkASSERT(1 == SkPM4f::G);
    SkASSERT(3 == SkPM4f::A);

    Sk4f cr, cg, cb;
    cg = Sk4f::Load(m.fMat + 3);
    if (0 == SkPM4f::R) {
        SkASSERT(2 == SkPM4f::B);
        cr = Sk4f::Load(m.fMat + 0);
        cb = Sk4f(m.fMat[6], m.fMat[7], m.fMat[8], 0);
    } else {
        SkASSERT(0 == SkPM4f::B);
        SkASSERT(2 == SkPM4f::R);
        cb = Sk4f::Load(m.fMat + 0);
        cr = Sk4f(m.fMat[6], m.fMat[7], m.fMat[8], 0);
    }
    cr = cr * Sk4f(1, 1, 1, 0);
    cg = cg * Sk4f(1, 1, 1, 0);
    cb = cb * Sk4f(1, 1, 1, 0);

    for (int i = 0; i < count; ++i) {
        Sk4f r = Sk4f(src[i].fVec[SkPM4f::R]);
        Sk4f g = Sk4f(src[i].fVec[SkPM4f::G]);
        Sk4f b = Sk4f(src[i].fVec[SkPM4f::B]);
        Sk4f a = Sk4f(0, 0, 0, src[i].fVec[SkPM4f::A]);
        (cr * r + cg * g + cb * b + a).store(&dst[i]);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void SkColorSpace::Test() {
    SkFloat3x3 mat {{ 2, 0, 0, 0, 3, 0, 0, 0, 4 }};
    SkFloat3x3 inv = invert(mat);
    mat.dump();
    inv.dump();
    concat(mat, inv).dump();
    concat(inv, mat).dump();
    SkDebugf("\n");

    mat = gSRGB_toXYZD50;
    inv = invert(mat);
    mat.dump();
    inv.dump();
    concat(mat, inv).dump();
    concat(inv, mat).dump();
    SkDebugf("\n");

    SkAutoTUnref<SkColorSpace> cs0(SkColorSpace::NewNamed(SkColorSpace::kSRGB_Named));
    SkAutoTUnref<SkColorSpace> cs1(SkColorSpace::NewNamed(SkColorSpace::kSRGB_Named));

    cs0->dump();
    cs1->dump();
    SkFloat3x3 xform;
    (void)SkColorSpace::Concat(cs0, cs1, &xform);
    xform.dump();
    SkDebugf("\n");
}

// D65 white point of Rec.  709 [8] are:
//
// D65 white-point in unit luminance XYZ = 0.9505, 1.0000, 1.0890
//
//          R           G           B           white
//   x      0.640       0.300       0.150       0.3127
//   y      0.330       0.600       0.060       0.3290
//   z      0.030       0.100       0.790       0.3582
