/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrColorInfo.h"

#include "src/core/SkColorSpacePriv.h"

GrColorInfo::GrColorInfo(
        GrColorType colorType, SkAlphaType alphaType, sk_sp<SkColorSpace> colorSpace)
        : fColorSpace(std::move(colorSpace)), fColorType(colorType), fAlphaType(alphaType) {}

GrColorInfo::GrColorInfo(const SkColorInfo& ci)
        : GrColorInfo(SkColorTypeToGrColorType(ci.colorType()),
                      ci.alphaType(),
                      ci.refColorSpace()) {}

GrColorSpaceXform* GrColorInfo::colorSpaceXformFromSRGB() const {
    // TODO: Make this atomic if we start accessing this on multiple threads.
    if (!fInitializedColorSpaceXformFromSRGB) {
        // sRGB sources are very common (SkColor, etc...), so we cache that transformation
        fColorXformFromSRGB = GrColorSpaceXform::Make(sk_srgb_singleton(), kUnpremul_SkAlphaType,
                                                      fColorSpace.get(),   kUnpremul_SkAlphaType);
        fInitializedColorSpaceXformFromSRGB = true;
    }
    // You can't be color-space aware in legacy mode
    SkASSERT(fColorSpace || !fColorXformFromSRGB);
    return fColorXformFromSRGB.get();
}
