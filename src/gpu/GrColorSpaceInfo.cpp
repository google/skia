/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkColorSpacePriv.h"
#include "src/gpu/GrColorSpaceInfo.h"

GrColorSpaceInfo::GrColorSpaceInfo(sk_sp<SkColorSpace> colorSpace, GrPixelConfig config)
        : fColorSpace(std::move(colorSpace))
        , fConfig(config)
        , fInitializedColorSpaceXformFromSRGB(false) {}

GrColorSpaceXform* GrColorSpaceInfo::colorSpaceXformFromSRGB() const {
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
