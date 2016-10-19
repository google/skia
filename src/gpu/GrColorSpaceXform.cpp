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
    
    const SkMatrix44* toXYZD50   = as_CSB(src)->toXYZD50();
    const SkMatrix44* fromXYZD50 = as_CSB(dst)->fromXYZD50();
    if (!toXYZD50 || !fromXYZD50) {
        // unsupported colour spaces -- cannot specify gamut as a matrix
        return nullptr;
    }

    if (as_CSB(src)->toXYZD50Hash() == as_CSB(dst)->toXYZD50Hash()) {
        // Identical gamut - no conversion needed in this case
        SkASSERT(*toXYZD50 == *as_CSB(dst)->toXYZD50() && "Hash collision");
        return nullptr;
    }

    SkMatrix44 srcToDst(SkMatrix44::kUninitialized_Constructor);
    srcToDst.setConcat(*fromXYZD50, *toXYZD50);

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
