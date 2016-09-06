/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrAppliedClip_DEFINED
#define GrAppliedClip_DEFINED

#include "GrTypesPriv.h"
#include "GrWindowRectangles.h"

class GrFragmentProcessor;

/**
 * Produced by GrClip. It provides a set of modifications to the drawing state that are used to
 * create the final GrPipeline for a GrBatch.
 */
class GrAppliedClip : public SkNoncopyable {
public:
    GrAppliedClip(const SkRect& drawBounds)
        : fHasStencilClip(false)
        , fClippedDrawBounds(drawBounds) {
    }

    const GrScissorState& scissorState() const { return fScissorState; }
    const GrWindowRectangles& windowRects() const { return fWindowRects; }
    GrFragmentProcessor* clipCoverageFragmentProcessor() const { return fClipCoverageFP.get(); }
    bool hasStencilClip() const { return fHasStencilClip; }

    /**
     * Intersects the applied clip with the provided rect. Returns false if the draw became empty.
     */
    bool addScissor(const SkIRect& irect) {
        return fScissorState.intersect(irect) && fClippedDrawBounds.intersect(SkRect::Make(irect));
    }

    /**
     * Adds an exclusive window rectangle to the clip. It is not currently supported to switch the
     * windows to inclusive mode.
     */
    void addWindowRectangle(const SkIRect& window) {
        fWindowRects.addWindow(window);
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
    GrWindowRectangles         fWindowRects;
    sk_sp<GrFragmentProcessor> fClipCoverageFP;
    bool                       fHasStencilClip;
    SkRect                     fClippedDrawBounds;
    typedef SkNoncopyable INHERITED;
};

#endif
