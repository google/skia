/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrClip.h"

#include "GrClipMaskManager.h"
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

bool GrFixedClip::apply(GrContext*, const GrPipelineBuilder& pipelineBuilder,
                        GrDrawContext* drawContext,
                        const SkRect* devBounds, GrAppliedClip* out) const {
    if (fScissorState.enabled()) {
        SkIRect tightScissor;
        if (!tightScissor.intersect(fScissorState.rect(),
                                    SkIRect::MakeWH(drawContext->width(), drawContext->height()))) {
            return false;
        }
        if (devBounds && !devBounds->intersects(SkRect::Make(tightScissor))) {
            return false;
        }
        out->makeScissoredStencil(fHasStencilClip, tightScissor);
        return true;
    }

    out->makeStencil(fHasStencilClip);
    return true;
}

bool GrClipStackClip::quickContains(const SkRect& rect) const {
    if (!fStack) {
        return true;
    }
    return fStack->quickContains(rect.makeOffset(SkIntToScalar(fOrigin.x()),
                                                 SkIntToScalar(fOrigin.y())));
}

void GrClipStackClip::getConservativeBounds(int width, int height, SkIRect* devResult,
                                            bool* isIntersectionOfRects) const {
    if (!fStack) {
        devResult->setXYWH(0, 0, width, height);
        if (isIntersectionOfRects) {
            *isIntersectionOfRects = true;
        }
        return;
    }
    SkRect devBounds;
    fStack->getConservativeBounds(-fOrigin.x(), -fOrigin.y(), width, height, &devBounds,
                                  isIntersectionOfRects);
    devBounds.roundOut(devResult);
}

bool GrClipStackClip::apply(GrContext* context,
                            const GrPipelineBuilder& pipelineBuilder, GrDrawContext* drawContext,
                            const SkRect* devBounds, GrAppliedClip* out) const {
    return GrClipMaskManager::SetupClipping(context, pipelineBuilder, drawContext,
                                            *this, devBounds, out);
}
