/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/surface_factory/SurfaceFactory.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkColorType.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkSurface.h"
#include "include/private/base/SkAssert.h"

#include <string>

// Returns a raster SkSurface for the given config.
sk_sp<SkSurface> make_surface(std::string config, int width, int height) {
    // These configs are based on the RasterSink configs here:
    // https://skia.googlesource.com/skia/+/faaa8393a68b518ec1f204a60c7c3393e1da2fa2/dm/DM.cpp#1046.
    if (config == "8888") {
        sk_sp<SkSurface> surface = SkSurfaces::Raster(SkImageInfo::Make(
                width, height, kN32_SkColorType, kPremul_SkAlphaType, SkColorSpace::MakeSRGB()));
        SkASSERT_RELEASE(surface);
        return surface;
    }
    if (config == "565") {
        sk_sp<SkSurface> surface = SkSurfaces::Raster(SkImageInfo::Make(width,
                                                                        height,
                                                                        kRGB_565_SkColorType,
                                                                        kPremul_SkAlphaType,
                                                                        SkColorSpace::MakeSRGB()));
        SkASSERT_RELEASE(surface);
        return surface;
    }
    return nullptr;
}
