/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/v2/SurfaceFillContext_v2.h"

namespace skgpu::v2 {

SurfaceFillContext::SurfaceFillContext(GrRecordingContext* rContext,
                                       GrSurfaceProxyView readView,
                                       GrSurfaceProxyView writeView,
                                       const GrColorInfo& colorInfo)
    : skgpu::SurfaceFillContext(rContext,
                                std::move(readView),
                                std::move(writeView),
                                colorInfo) {
}

void SurfaceFillContext::discard() {
}

void SurfaceFillContext::fillRectWithFP(const SkIRect& dstRect,
                                        std::unique_ptr<GrFragmentProcessor>) {
}

bool SurfaceFillContext::blitTexture(GrSurfaceProxyView,
                                     const SkIRect& srcRect,
                                     const SkIPoint& dstPoint) {
    return false;
}

void SurfaceFillContext::internalClear(const SkIRect* scissor,
                                       std::array<float, 4> color,
                                       bool upgradePartialToFull) {

}

} // namespace skgpu::v2
