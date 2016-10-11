/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrColorSpaceXform.h"
#include "SkColorSpace.h"
#include "SkColorSpace_Base.h"
#include "SkMatrix44.h"

static inline bool sk_float_almost_equals(float x, float y, float tol) {
    return sk_float_abs(x - y) <= tol;
}

static inline bool matrix_is_almost_identity(const SkMatrix44& m,
                                             SkMScalar tol = SK_MScalar1 / (1 << 12)) {
    return
        sk_float_almost_equals(m.getFloat(0, 0), 1.0f, tol) &&
        sk_float_almost_equals(m.getFloat(0, 1), 0.0f, tol) &&
        sk_float_almost_equals(m.getFloat(0, 2), 0.0f, tol) &&
        sk_float_almost_equals(m.getFloat(0, 3), 0.0f, tol) &&
        sk_float_almost_equals(m.getFloat(1, 0), 0.0f, tol) &&
        sk_float_almost_equals(m.getFloat(1, 1), 1.0f, tol) &&
        sk_float_almost_equals(m.getFloat(1, 2), 0.0f, tol) &&
        sk_float_almost_equals(m.getFloat(1, 3), 0.0f, tol) &&
        sk_float_almost_equals(m.getFloat(2, 0), 0.0f, tol) &&
        sk_float_almost_equals(m.getFloat(2, 1), 0.0f, tol) &&
        sk_float_almost_equals(m.getFloat(2, 2), 1.0f, tol) &&
        sk_float_almost_equals(m.getFloat(2, 3), 0.0f, tol) &&
        sk_float_almost_equals(m.getFloat(3, 0), 0.0f, tol) &&
        sk_float_almost_equals(m.getFloat(3, 1), 0.0f, tol) &&
        sk_float_almost_equals(m.getFloat(3, 2), 0.0f, tol) &&
        sk_float_almost_equals(m.getFloat(3, 3), 1.0f, tol);
}

GrColorSpaceXform::GrColorSpaceXform(const SkMatrix44& srcToDst) 
    : fSrcToDst(srcToDst) {}

sk_sp<GrColorSpaceXform> GrColorSpaceXform::Make(SkColorSpace* src, SkColorSpace* dst) {
    if (!src || !dst) {
        // Invalid
        return nullptr;
    }

    if (src == dst) {
        // Quick equality check - no conversion needed in this case
        return nullptr;
    }

    SkMatrix44 srcToDst(SkMatrix44::kUninitialized_Constructor);
    srcToDst.setConcat(as_CSB(dst)->fromXYZD50(), as_CSB(src)->toXYZD50());

    if (matrix_is_almost_identity(srcToDst)) {
        return nullptr;
    }

    return sk_make_sp<GrColorSpaceXform>(srcToDst);
}

bool GrColorSpaceXform::Equals(const GrColorSpaceXform* a, const GrColorSpaceXform* b) {
    if (a == b) {
        return true;
    }

    if (!a || !b) {
        return false;
    }

    return a->fSrcToDst == b->fSrcToDst;
}

GrColor4f GrColorSpaceXform::apply(const GrColor4f& srcColor) {
    GrColor4f result;
    fSrcToDst.mapScalars(srcColor.fRGBA, result.fRGBA);
    return result;
}
