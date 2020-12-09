/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrFixedClip.h"

#include "src/gpu/GrAppliedClip.h"
#include "src/gpu/GrSurfaceDrawContext.h"

SkIRect GrFixedClip::getConservativeBounds() const {
    return fScissorState.rect();
}

GrClip::PreClipResult GrFixedClip::preApply(const SkRect& drawBounds, GrAA aa) const {
    SkIRect pixelBounds = GetPixelIBounds(drawBounds, aa);
    if (!SkIRect::Intersects(fScissorState.rect(), pixelBounds)) {
        return Effect::kClippedOut;
    }

    if (fWindowRectsState.enabled()) {
        return Effect::kClipped;
    }

    if (!fScissorState.enabled() || fScissorState.rect().contains(pixelBounds)) {
        // Either no scissor or the scissor doesn't clip the draw
        return Effect::kUnclipped;
    }
    // Report the scissor as a degenerate round rect
    return {SkRect::Make(fScissorState.rect()), GrAA::kNo};
}

GrClip::Effect GrFixedClip::apply(GrAppliedHardClip* out, SkIRect* bounds) const {
    if (!SkIRect::Intersects(fScissorState.rect(), *bounds)) {
        return Effect::kClippedOut;
    }

    Effect effect = Effect::kUnclipped;
    if (fScissorState.enabled() && !fScissorState.rect().contains(*bounds)) {
        SkAssertResult(bounds->intersect(fScissorState.rect()));
        out->setScissor(*bounds);
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
