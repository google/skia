/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrClearOp.h"

#include "GrGpuCommandBuffer.h"
#include "GrOpFlushState.h"
#include "GrResourceProvider.h"

GrClearOp::GrClearOp(const GrFixedClip& clip, GrColor color, GrSurfaceProxy* proxy)
        : INHERITED(ClassID())
        , fClip(clip)
        , fColor(color) {
    const SkIRect rtRect = SkIRect::MakeWH(proxy->width(), proxy->height());
    if (fClip.scissorEnabled()) {
        // Don't let scissors extend outside the RT. This may improve op combining.
        if (!fClip.intersect(rtRect)) {
            SkASSERT(0);  // should be caught upstream
            fClip = GrFixedClip(SkIRect::MakeEmpty());
        }

        if (GrResourceProvider::IsFunctionallyExact(proxy) && fClip.scissorRect() == rtRect) {
            fClip.disableScissor();
        }
    }
    this->setBounds(SkRect::Make(fClip.scissorEnabled() ? fClip.scissorRect() : rtRect),
                    HasAABloat::kNo, IsZeroArea::kNo);
}

void GrClearOp::onExecute(GrOpFlushState* state) {
    SkASSERT(state->drawOpArgs().fRenderTarget);

    state->commandBuffer()->clear(state->drawOpArgs().fRenderTarget, fClip, fColor);
}
