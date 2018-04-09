/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkColorData.h"
#include "SkColorSpacePriv.h"
#include "SkColorSpaceXformPriv.h"
#include "SkColorSpaceXform_Base.h"

///////////////////////////////////////////////////////////////////////////////////////////////////

std::unique_ptr<SkColorSpaceXform> SkColorSpaceXform::New(SkColorSpace* src,
                                                          SkColorSpace* dst) {
    return SkColorSpaceXform_Base::New(src, dst, SkTransferFunctionBehavior::kRespect);
}

std::unique_ptr<SkColorSpaceXform> SkColorSpaceXform_Base::New(
        SkColorSpace* src,
        SkColorSpace* dst,
        SkTransferFunctionBehavior premulBehavior) {
#if !defined(SK_USE_SKCMS)
    return nullptr;
#else
    if (!src || !dst) {
        // Invalid input
        return nullptr;
    }

    if (!dst->toXYZD50()) {
        return nullptr;
    }

    return MakeSkcmsXform(src, dst, premulBehavior);
#endif
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
    return New(srcCS, dstCS)->apply(dstFormat, dst, srcFormat, src, len, at);
}
