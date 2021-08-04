/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SurfaceFillContext_v2_DEFINED
#define SurfaceFillContext_v2_DEFINED

#include "src/gpu/SurfaceFillContext.h"

namespace skgpu::v2 {

class SurfaceFillContext : public skgpu::SurfaceFillContext {
public:
    SurfaceFillContext(GrRecordingContext*,
                       GrSurfaceProxyView readView,
                       GrSurfaceProxyView writeView,
                       const GrColorInfo&,
                       bool flushTimeOpsTask = false);

    void discard() override;

    void fillRectWithFP(const SkIRect& dstRect, std::unique_ptr<GrFragmentProcessor>) override;

    bool blitTexture(GrSurfaceProxyView,
                     const SkIRect& srcRect,
                     const SkIPoint& dstPoint) override;

    sk_sp<GrRenderTask> refRenderTask() override { return nullptr; }

private:
    void internalClear(const SkIRect* scissor,
                       std::array<float, 4> color,
                       bool upgradePartialToFull = false) override;

    using INHERITED = skgpu::SurfaceFillContext;
};

} // namespace skgpu::v2

#endif // SurfaceFillContext_v2_DEFINED
