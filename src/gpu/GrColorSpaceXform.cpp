/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrColorSpaceXform.h"
#include "SkColorSpace.h"
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

GrColorSpaceXform::GrColorSpaceXform(const SkMatrix44& srcToDst, SkAlphaType srcAlphaType)
    : fSrcAlphaType(srcAlphaType) {
    srcToDst.asColMajorf(fSrcToDst);
}

sk_sp<GrColorSpaceXform> GrColorSpaceXform::Make(SkColorSpace* src, SkColorSpace* dst,
                                                 SkAlphaType srcAlphaType) {
    if (!src || !dst) {
        // Invalid
        return nullptr;
    }

    if (src == dst) {
        // Quick equality check - no conversion needed in this case
        return nullptr;
    }

    SkMatrix44 srcToDst(SkMatrix44::kUninitialized_Constructor);
    if (!dst->xyz().invert(&srcToDst)) {
        return nullptr;
    }
    srcToDst.postConcat(src->xyz());

    if (matrix_is_almost_identity(srcToDst)) {
        return nullptr;
    }

    return sk_make_sp<GrColorSpaceXform>(srcToDst, srcAlphaType);
}
