/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrAppliedClip_DEFINED
#define GrAppliedClip_DEFINED

#include "GrFragmentProcessor.h"
#include "GrScissorState.h"
#include "GrWindowRectsState.h"

/**
 * Produced by GrClip. It provides a set of modifications to the drawing state that are used to
 * create the final GrPipeline for a GrOp.
 */
class GrAppliedClip : public SkNoncopyable {
public:
    GrAppliedClip(const SkRect& drawBounds)
        : fHasStencilClip(false)
        , fClippedDrawBounds(drawBounds) {
    }

    const GrScissorState& scissorState() const { return fScissorState; }
    const GrWindowRectsState& windowRectsState() const { return fWindowRectsState; }
    GrFragmentProcessor* clipCoverageFragmentProcessor() const { return fClipCoverageFP.get(); }
    bool hasStencilClip() const { return fHasStencilClip; }

    /**
     * Intersects the applied clip with the provided rect. Returns false if the draw became empty.
     */
    bool addScissor(const SkIRect& irect) {
        return fScissorState.intersect(irect) && fClippedDrawBounds.intersect(SkRect::Make(irect));
    }

    void addWindowRectangles(const GrWindowRectsState& windowState) {
        SkASSERT(!fWindowRectsState.enabled());
        fWindowRectsState = windowState;
    }

    void addWindowRectangles(const GrWindowRectangles& windows, const SkIPoint& origin,
                             GrWindowRectsState::Mode mode) {
        SkASSERT(!fWindowRectsState.enabled());
        fWindowRectsState.set(windows, origin, mode);
    }

    void addCoverageFP(sk_sp<GrFragmentProcessor> fp) {
        SkASSERT(!fClipCoverageFP);
        fClipCoverageFP = fp;
    }

    void addStencilClip() {
        SkASSERT(!fHasStencilClip);
        fHasStencilClip = true;
    }

    /**
     * Returns the device bounds of the draw after clip has been applied. TODO: Ideally this would
     * consider the combined effect of all clipping techniques in play (scissor, stencil, fp, etc.).
     */
    const SkRect& clippedDrawBounds() const { return fClippedDrawBounds; }

private:
    GrScissorState             fScissorState;
    GrWindowRectsState         fWindowRectsState;
    sk_sp<GrFragmentProcessor> fClipCoverageFP;
    bool                       fHasStencilClip;
    SkRect                     fClippedDrawBounds;
    typedef SkNoncopyable INHERITED;
};

#endif
