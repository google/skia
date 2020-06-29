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

GrClip::PreClipResult GrFixedClip::preApply(const SkRect& drawBounds) const {
    if (IsOutsideClip(fScissorState.rect(), drawBounds)) {
        return Effect::kClippedOut;
    }

    if (fWindowRectsState.enabled()) {
        return Effect::kClipped;
    }

    if (!fScissorState.enabled() || IsInsideClip(fScissorState.rect(), drawBounds)) {
        // Either no scissor or the scissor doesn't clip the draw
        return Effect::kUnclipped;
    }
    // Report the scissor as a degenerate round rect
    return {SkRect::Make(fScissorState.rect()), GrAA::kNo};
}

GrClip::Effect GrFixedClip::apply(GrAppliedHardClip* out, SkRect* bounds) const {
    if (IsOutsideClip(fScissorState.rect(), *bounds)) {
        return Effect::kClippedOut;
    }

    Effect effect = Effect::kUnclipped;
    if (fScissorState.enabled() && !IsInsideClip(fScissorState.rect(), *bounds)) {
        SkIRect tightScissor = bounds->roundOut();
        SkAssertResult(tightScissor.intersect(fScissorState.rect()));
        out->addScissor(tightScissor, bounds);
        effect = Effect::kClipped;
    }

    if (fWindowRectsState.enabled()) {
        out->addWindowRectangles(fWindowRectsState);
        // We could iterate each window rectangle to check for intersection, but be conservative
        // and report that it's clipped
        effect = Effect::kClipped;
    }

    return effect;
}
