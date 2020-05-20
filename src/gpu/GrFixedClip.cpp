/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrFixedClip.h"

#include "src/gpu/GrAppliedClip.h"
#include "src/gpu/GrRenderTargetContext.h"

SkIRect GrFixedClip::getConservativeBounds() const {
    return fScissorState.rect();
}

bool GrFixedClip::preApply(const SkRect& drawBounds, ClipEffect* effect,
                           SkRRect* rr, GrAA* aa) const {
    if (IsOutsideClip(fScissorState.rect(), drawBounds)) {
        *effect = ClipEffect::kNoDraw;
        return false;
    }

    if (fWindowRectsState.enabled()) {
        *effect = ClipEffect::kClipped;
        return false;
    }

    if (!fScissorState.enabled() || IsInsideClip(fScissorState.rect(), drawBounds)) {
        // Either no scissor or the scissor doesn't clip the draw
        *effect = ClipEffect::kUnclipped;
        return false;
    } else {
        // Report the scissor as a degenerate round rect
        *effect = ClipEffect::kClipped;
        rr->setRect(SkRect::Make(fScissorState.rect()));
        *aa = GrAA::kNo;
        return true;
    }
};

GrClip::ClipEffect GrFixedClip::apply(GrAppliedHardClip* out, SkRect* bounds) const {
    if (IsOutsideClip(fScissorState.rect(), *bounds)) {
        return ClipEffect::kNoDraw;
    }

    ClipEffect effect = ClipEffect::kUnclipped;
    if (fScissorState.enabled() && !IsInsideClip(fScissorState.rect(), *bounds)) {
        SkIRect tightScissor = bounds->roundOut();
        SkAssertResult(tightScissor.intersect(fScissorState.rect()));
        out->addScissor(tightScissor, bounds);
        effect = ClipEffect::kClipped;
    }

    if (fWindowRectsState.enabled()) {
        out->addWindowRectangles(fWindowRectsState);
        // We could iterate each window rectangle to check for intersection, but be conservative
        // and report that it's clipped
        effect = ClipEffect::kClipped;
    }

    return effect;
}
