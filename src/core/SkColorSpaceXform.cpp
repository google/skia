/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkColorSpaceXform.h"
#include "SkColorSpaceXformPriv.h"

std::unique_ptr<SkColorSpaceXform> SkColorSpaceXform::New(SkColorSpace* src, SkColorSpace* dst) {
    return SkMakeColorSpaceXform(src, dst, SkTransferFunctionBehavior::kRespect);
}

std::unique_ptr<SkColorSpaceXform> SkMakeColorSpaceXform(
        SkColorSpace* src,
        SkColorSpace* dst,
        SkTransferFunctionBehavior premulBehavior) {
#if defined(SK_USE_SKCMS)
    if (src && dst && dst->toXYZD50()) {
        return SkMakeColorSpaceXform_skcms(src, dst, premulBehavior);
    }
#endif
    return nullptr;
}

bool SkColorSpaceXform::Apply(SkColorSpace* dstCS, ColorFormat dstFormat, void* dst,
                              SkColorSpace* srcCS, ColorFormat srcFormat, const void* src,
                              int len, AlphaOp op) {
    SkAlphaType at;
    switch (op) {
        case kPreserve_AlphaOp:    at = kUnpremul_SkAlphaType; break;
        case kPremul_AlphaOp:      at = kPremul_SkAlphaType;   break;
        case kSrcIsOpaque_AlphaOp: at = kOpaque_SkAlphaType;   break;
    }
    return SkColorSpaceXform::New(srcCS, dstCS)->apply(dstFormat, dst, srcFormat, src, len, at);
}
