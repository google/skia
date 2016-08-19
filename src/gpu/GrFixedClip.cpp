/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrFixedClip.h"

#include "GrAppliedClip.h"
#include "GrDrawContext.h"

bool GrFixedClip::quickContains(const SkRect& rect) const {
    if (fHasStencilClip) {
        return false;
    }
    return !fScissorState.enabled() || GrClip::IsInsideClip(fScissorState.rect(), rect);
}

void GrFixedClip::getConservativeBounds(int width, int height, SkIRect* devResult,
                                        bool* isIntersectionOfRects) const {
    devResult->setXYWH(0, 0, width, height);
    if (fScissorState.enabled()) {
        if (!devResult->intersect(fScissorState.rect())) {
            devResult->setEmpty();
        }
    }
    if (isIntersectionOfRects) {
        *isIntersectionOfRects = true;
    }
}

bool GrFixedClip::apply(GrContext*, GrDrawContext* drawContext, bool isHWAntiAlias,
                        bool hasUserStencilSettings, GrAppliedClip* out) const {
    if (fScissorState.enabled()) {
        SkIRect tightScissor;
        if (!tightScissor.intersect(fScissorState.rect(),
                                    SkIRect::MakeWH(drawContext->width(), drawContext->height()))) {
            return false;
        }
        if (IsOutsideClip(tightScissor, out->clippedDrawBounds())) {
            return false;
        }
        if (!IsInsideClip(fScissorState.rect(), out->clippedDrawBounds())) {
            out->addScissor(tightScissor);
        }
    }

    if (fHasStencilClip) {
        out->addStencilClip();
    }

    return true;
}
