/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/surface_manager/SurfaceManager.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkColorType.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkSurface.h"
#include "include/private/base/SkAssert.h"

#include <string>

class RasterSurfaceManager : public SurfaceManager {
public:
    RasterSurfaceManager(sk_sp<SkSurface> surface, std::string config, SkColorInfo colorInfo)
            : SurfaceManager(config, colorInfo), fSurface(surface) {}

    sk_sp<SkSurface> getSurface() override { return fSurface; }

    void flush() override {}  // Nothing to do.

private:
    sk_sp<SkSurface> fSurface;
};

std::unique_ptr<SurfaceManager> SurfaceManager::FromConfig(std::string config,
                                                           int width,
                                                           int height) {
    // These configs are based on the RasterSink configs here:
    // https://skia.googlesource.com/skia/+/faaa8393a68b518ec1f204a60c7c3393e1da2fa2/dm/DM.cpp#1046.
    if (config == "8888") {
        SkColorInfo colorInfo(kN32_SkColorType, kPremul_SkAlphaType, SkColorSpace::MakeSRGB());
        sk_sp<SkSurface> surface =
                SkSurfaces::Raster(SkImageInfo::Make({width, height}, colorInfo));
        SkASSERT_RELEASE(surface);
        return std::make_unique<RasterSurfaceManager>(surface, config, colorInfo);
    }
    if (config == "565") {
        SkColorInfo colorInfo(kRGB_565_SkColorType, kPremul_SkAlphaType, SkColorSpace::MakeSRGB());
        sk_sp<SkSurface> surface =
                SkSurfaces::Raster(SkImageInfo::Make({width, height}, colorInfo));
        SkASSERT_RELEASE(surface);
        return std::make_unique<RasterSurfaceManager>(surface, config, colorInfo);
    }
    return nullptr;
}
