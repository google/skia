/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/src/SurfaceDrawContext.h"

#include "experimental/graphite/src/DrawList.h"
#include "experimental/graphite/src/RenderPassTask.h"

namespace skgpu {

sk_sp<SurfaceDrawContext> SurfaceDrawContext::Make(const SkImageInfo& ii) {
    return sk_sp<SurfaceDrawContext>(new SurfaceDrawContext(ii));
}

SurfaceDrawContext::SurfaceDrawContext(const SkImageInfo& ii)
        : fImageInfo(ii)
        , fPendingDraws(std::make_unique<DrawList>())
        , fTail(nullptr) {
    // TBD - Will probably want DrawLists (and its internal commands) to come from an arena
    // that the SDC manages.
}

SurfaceDrawContext::~SurfaceDrawContext() {
    // If the SDC is destroyed and there are pending commands, they won't be drawn. Maybe that's ok
    // but for now consider it a bug for not calling snapDrawTask().
    SkASSERT(fPendingDraws->count() == 0);
}

void SurfaceDrawContext::fillPath(const SkM44& localToDevice,
                                  const SkPath& path,
                                  const SkIRect& scissor,
                                  uint16_t sortZ,
                                  uint16_t testZ,
                                  const PaintParams* paint) {
    fPendingDraws->fillPath(localToDevice, path, scissor, sortZ, testZ, paint);
}

void SurfaceDrawContext::strokePath(const SkM44& localToDevice,
                                    const SkPath& path,
                                    const StrokeParams& stroke,
                                    const SkIRect& scissor,
                                    uint16_t sortZ,
                                    uint16_t testZ,
                                    const PaintParams* paint) {
    fPendingDraws->strokePath(localToDevice, path, stroke, scissor, sortZ, testZ, paint);
}

void SurfaceDrawContext::snapDrawPass(const BoundsManager* occlusionCuller) {
    // TODO: actually sort, cull, and merge the DL for the DrawPass
    (void) occlusionCuller;
    // TODO: eventually the RenderPassTask will be handled later since it might take over
    // multiple DrawPasses from the SDC.
    auto task = RenderPassTask::Make(std::move(fTail), std::move(fPendingDraws));
    fTail = task;
    fPendingDraws = std::make_unique<DrawList>();
}

} // namespace skgpu
