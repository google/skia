/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrFixedClip.h"

#include "src/gpu/GrAppliedClip.h"
#include "src/gpu/GrRenderTargetContext.h"

bool GrFixedClip::quickContains(const SkRect& rect) const {
    if (fWindowRectsState.enabled()) {
        return false;
    }

    return !fScissorState.enabled() || GrClip::IsInsideClip(fScissorState.rect(), rect);
}

SkIRect GrFixedClip::getConservativeBounds() const {
    return fScissorState.rect();
}

bool GrFixedClip::isRRect(SkRRect* rr, GrAA* aa) const {
    if (fWindowRectsState.enabled()) {
        return false;
    }
    // Whether or not the scissor test is enabled, the remaining clip is a rectangle described
    // by scissorState.rect() (either the scissor or the rt bounds).
    rr->setRect(SkRect::Make(fScissorState.rect()));
    *aa = GrAA::kNo;
    return true;
};

bool GrFixedClip::apply(GrAppliedHardClip* out, SkRect* bounds) const {
    if (IsOutsideClip(fScissorState.rect(), *bounds)) {
        return false;
    }
    if (!IsInsideClip(fScissorState.rect(), *bounds)) {
        SkIRect tightScissor = bounds->roundOut();
        SkAssertResult(tightScissor.intersect(fScissorState.rect()));
        out->addScissor(tightScissor, bounds);
    }

    if (fWindowRectsState.enabled()) {
        out->addWindowRectangles(fWindowRectsState);
    }

    return true;
}
