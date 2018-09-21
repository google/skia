/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPM4fPriv_DEFINED
#define SkPM4fPriv_DEFINED

#include "SkColorSpacePriv.h"
#include "SkColorSpaceXformSteps.h"
#include "SkPM4f.h"

// This file is mostly helper routines for doing color space management.
// It probably wants a new name, and they likely don't need to be inline.

static inline SkColor4f premul_in_dst_colorspace(SkColor4f color4f,
                                                 SkColorSpace* srcCS, SkColorSpace* dstCS) {
    // TODO: In the very common case of srcCS being sRGB,
    // can we precompute an sRGB -> dstCS SkColorSpaceXformSteps for each device and use it here?
    SkColorSpaceXformSteps(srcCS, kUnpremul_SkAlphaType,
                           dstCS, kPremul_SkAlphaType)
        .apply(color4f.vec());

    return color4f;
}

static inline SkColor4f premul_in_dst_colorspace(SkColor c, SkColorSpace* dstCS) {
    SkColor4f color4f;
    swizzle_rb(Sk4f_fromL32(c)).store(color4f.vec());

    // SkColors are always sRGB.
    return premul_in_dst_colorspace(color4f, sk_srgb_singleton(), dstCS);
}

#endif
