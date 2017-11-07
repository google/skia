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
    int numClipCoverageFragmentProcessors() const { return fClipCoverageFPs.count(); }
    const GrFragmentProcessor* clipCoverageFragmentProcessor(int i) const {
        SkASSERT(fClipCoverageFPs[i]);
        return fClipCoverageFPs[i].get();
    }
    std::unique_ptr<const GrFragmentProcessor> detachClipCoverageFragmentProcessor(int i) {
        SkASSERT(fClipCoverageFPs[i]);
        return std::move(fClipCoverageFPs[i]);
    }
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

    void addCoverageFP(std::unique_ptr<GrFragmentProcessor> fp) {
        SkASSERT(fp);
        fClipCoverageFPs.push_back(std::move(fp));
    }

    void addStencilClip(uint32_t clipStackID) {
        SkASSERT(SkClipStack::kInvalidGenID == fClipStackID);
        fClipStackID = clipStackID;
    }

    bool doesClip() const {
        return fScissorState.enabled() || !fClipCoverageFPs.empty() || this->hasStencilClip() ||
               fWindowRectsState.enabled();
    }

    bool operator==(const GrAppliedClip& that) const {
        if (fScissorState != that.fScissorState ||
            fWindowRectsState != that.fWindowRectsState ||
            fClipCoverageFPs.count() != that.fClipCoverageFPs.count() ||
            fClipStackID != that.fClipStackID) {
            return false;
        }
        for (int i = 0; i < fClipCoverageFPs.count(); ++i) {
            if (!fClipCoverageFPs[i] || !that.fClipCoverageFPs[i]) {
                if (fClipCoverageFPs[i] == that.fClipCoverageFPs[i]) {
                    continue; // Both are null.
                }
                return false;
            }
            if (!fClipCoverageFPs[i]->isEqual(*that.fClipCoverageFPs[i])) {
                return false;
            }
        }
        return true;
    }
    bool operator!=(const GrAppliedClip& that) const { return !(*this == that); }

    void visitProxies(const std::function<void(GrSurfaceProxy*)>& func) const {
        for (const std::unique_ptr<GrFragmentProcessor>& fp : fClipCoverageFPs) {
            if (fp) { // This might be called after detach.
                fp->visitProxies(func);
            }
        }
    }

private:
    GrScissorState             fScissorState;
    GrWindowRectsState         fWindowRectsState;
    SkSTArray<4, std::unique_ptr<GrFragmentProcessor>> fClipCoverageFPs;
    uint32_t                   fClipStackID = SkClipStack::kInvalidGenID;
};

#endif
