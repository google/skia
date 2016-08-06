/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrClip.h"

#include "GrDrawContext.h"

void GrNoClip::getConservativeBounds(int width, int height, SkIRect* devResult,
                                     bool* isIntersectionOfRects) const {
    devResult->setXYWH(0, 0, width, height);
    if (isIntersectionOfRects) {
        *isIntersectionOfRects = true;
    }
}

bool GrFixedClip::quickContains(const SkRect& rect) const {
    if (fHasStencilClip) {
        return false;
    }
    if (!fScissorState.enabled()) {
        return true;
    }
    return fScissorState.rect().contains(rect);
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

bool GrFixedClip::apply(GrContext*,
                        GrDrawContext* drawContext,
                        const SkRect* devBounds,
                        bool isHWAntiAlias,
                        bool hasUserStencilSettings,
                        GrAppliedClip* out) const {
    SkASSERT(!fDeviceBounds.isLargest());
    if (fScissorState.enabled()) {
        SkIRect tightScissor;
        if (!tightScissor.intersect(fScissorState.rect(),
                                    SkIRect::MakeWH(drawContext->width(), drawContext->height()))) {
            return false;
        }
        if (devBounds && IsOutsideClip(tightScissor, *devBounds)) {
            return false;
        }
        if (!devBounds || !IsInsideClip(fScissorState.rect(), *devBounds)) {
            if (fHasStencilClip) {
                out->makeScissoredStencil(tightScissor, &fDeviceBounds);
            } else {
                out->makeScissored(tightScissor);
            }
            return true;
        }
    }

    out->makeStencil(fHasStencilClip, fDeviceBounds);
    return true;
}
