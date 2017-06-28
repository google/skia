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

#include "SkClipStack.h"

/**
 * Produced by GrClip. It provides a set of modifications to the drawing state that are used to
 * create the final GrPipeline for a GrOp.
 */
class GrAppliedClip {
public:
    GrAppliedClip() = default;
    GrAppliedClip(GrAppliedClip&& that) = default;
    GrAppliedClip(const GrAppliedClip&) = delete;

    const GrScissorState& scissorState() const { return fScissorState; }
    const GrWindowRectsState& windowRectsState() const { return fWindowRectsState; }
    GrFragmentProcessor* clipCoverageFragmentProcessor() const { return fClipCoverageFP.get(); }
    bool hasStencilClip() const { return SkClipStack::kInvalidGenID != fClipStackID; }

    /**
     * Intersects the applied clip with the provided rect. Returns false if the draw became empty.
     * 'clippedDrawBounds' will be intersected with 'irect'. This returns false if the clip becomes
     * empty or the draw no longer intersects the clip. In either case the draw can be skipped.
     */
    bool addScissor(const SkIRect& irect, SkRect* clippedDrawBounds) {
        return fScissorState.intersect(irect) && clippedDrawBounds->intersect(SkRect::Make(irect));
    }

    void addWindowRectangles(const GrWindowRectsState& windowState) {
        SkASSERT(!fWindowRectsState.enabled());
        fWindowRectsState = windowState;
    }

    void addWindowRectangles(const GrWindowRectangles& windows, GrWindowRectsState::Mode mode) {
        SkASSERT(!fWindowRectsState.enabled());
        fWindowRectsState.set(windows, mode);
    }

    void addCoverageFP(sk_sp<GrFragmentProcessor> fp) {
        SkASSERT(!fClipCoverageFP);
        fClipCoverageFP = fp;
    }

    void addStencilClip(uint32_t clipStackID) {
        SkASSERT(SkClipStack::kInvalidGenID == fClipStackID);
        fClipStackID = clipStackID;
    }

    bool doesClip() const {
        return fScissorState.enabled() || fClipCoverageFP || this->hasStencilClip() ||
               fWindowRectsState.enabled();
    }

    bool operator==(const GrAppliedClip& that) const {
        if (fScissorState != that.fScissorState || fClipStackID != that.fClipStackID) {
            return false;
        }
        if (SkToBool(fClipCoverageFP)) {
            if (!SkToBool(that.fClipCoverageFP) ||
                !that.fClipCoverageFP->isEqual(*fClipCoverageFP)) {
                return false;
            }
        } else if (SkToBool(that.fClipCoverageFP)) {
            return false;
        }
        return fWindowRectsState == that.fWindowRectsState;
    }
    bool operator!=(const GrAppliedClip& that) const { return !(*this == that); }

private:
    GrScissorState             fScissorState;
    GrWindowRectsState         fWindowRectsState;
    sk_sp<GrFragmentProcessor> fClipCoverageFP;
    uint32_t                   fClipStackID = SkClipStack::kInvalidGenID;
};

#endif
