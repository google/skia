/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkColorSpace.h"
#include "include/core/SkColorType.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkSurface.h"
#include "include/private/base/SkAssert.h"
#include "tools/testrunners/common/surface_manager/SurfaceManager.h"

#include <string>

class RasterSurfaceManager : public SurfaceManager {
public:
    RasterSurfaceManager(sk_sp<SkSurface> surface, std::string config, SkColorInfo colorInfo)
            : SurfaceManager(config, colorInfo, CpuOrGpu::kCPU), fSurface(surface) {}

    sk_sp<SkSurface> getSurface() override { return fSurface; }

private:
    sk_sp<SkSurface> fSurface;
};

std::unique_ptr<SurfaceManager> SurfaceManager::FromConfig(std::string config,
                                                           SurfaceOptions surfaceOptions) {
    // This config is based on nanobench's "nonrendering" config:
    // https://skia.googlesource.com/skia/+/a063eaeaf1e09e4d6f42e0f44a5723622a46d21c/bench/nanobench.cpp#663.
    // It is placed here because RasterSurfaceManager is the SurfaceManager implementation used
    // when no GPU backend is specified via Bazel's --config flag, which should be the case for all
    // nonrendering benchmarks.
    if (config == "nonrendering") {
        // The surface and color info are never used by nonrendering benchmarks, so their values do
        // not matter.
        return std::make_unique<RasterSurfaceManager>(nullptr, config, SkColorInfo());
    }

    // These configs are based on the RasterSink configs here:
    // https://skia.googlesource.com/skia/+/faaa8393a68b518ec1f204a60c7c3393e1da2fa2/dm/DM.cpp#1046.
    if (config == "8888") {
        SkColorInfo colorInfo(kN32_SkColorType, kPremul_SkAlphaType, SkColorSpace::MakeSRGB());
        sk_sp<SkSurface> surface = SkSurfaces::Raster(
                SkImageInfo::Make({surfaceOptions.width, surfaceOptions.height}, colorInfo));
        SkASSERT_RELEASE(surface);
        return std::make_unique<RasterSurfaceManager>(surface, config, colorInfo);
    }
    if (config == "565") {
        SkColorInfo colorInfo(kRGB_565_SkColorType, kPremul_SkAlphaType, SkColorSpace::MakeSRGB());
        sk_sp<SkSurface> surface = SkSurfaces::Raster(
                SkImageInfo::Make({surfaceOptions.width, surfaceOptions.height}, colorInfo));
        SkASSERT_RELEASE(surface);
        return std::make_unique<RasterSurfaceManager>(surface, config, colorInfo);
    }
    return nullptr;
}
