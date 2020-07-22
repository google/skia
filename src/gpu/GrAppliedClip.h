/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrAppliedClip_DEFINED
#define GrAppliedClip_DEFINED

#include "src/gpu/GrFragmentProcessor.h"
#include "src/gpu/GrScissorState.h"
#include "src/gpu/GrWindowRectsState.h"
#include "src/gpu/effects/GrXfermodeFragmentProcessor.h"

#include "src/core/SkClipStack.h"


/**
 * Produced by GrHardClip. It provides a set of modifications to the hardware drawing state that
 * implement the clip.
 */
class GrAppliedHardClip {
public:
    static const GrAppliedHardClip& Disabled() {
        // The size doesn't really matter here since it's returned as const& so an actual scissor
        // will never be set on it, and applied clips are not used to query or bounds test like
        // the GrClip is.
        static const GrAppliedHardClip kDisabled({1 << 29, 1 << 29});
        return kDisabled;
    }

    GrAppliedHardClip(const SkISize& rtDims) : fScissorState(rtDims) {}
    GrAppliedHardClip(const SkISize& logicalRTDims, const SkISize& backingStoreDims)
            : fScissorState(backingStoreDims) {
        fScissorState.set(SkIRect::MakeSize(logicalRTDims));
    }

    GrAppliedHardClip(GrAppliedHardClip&& that) = default;
    GrAppliedHardClip(const GrAppliedHardClip&) = delete;

    const GrScissorState& scissorState() const { return fScissorState; }
    const GrWindowRectsState& windowRectsState() const { return fWindowRectsState; }
    uint32_t stencilStackID() const { return fStencilStackID; }
    bool hasStencilClip() const { return SkClipStack::kInvalidGenID != fStencilStackID; }

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

    void addStencilClip(uint32_t stencilStackID) {
        SkASSERT(SkClipStack::kInvalidGenID == fStencilStackID);
        fStencilStackID = stencilStackID;
    }

    bool doesClip() const {
        return fScissorState.enabled() || this->hasStencilClip() || fWindowRectsState.enabled();
    }

    bool operator==(const GrAppliedHardClip& that) const {
        return fScissorState == that.fScissorState &&
               fWindowRectsState == that.fWindowRectsState &&
               fStencilStackID == that.fStencilStackID;
    }
    bool operator!=(const GrAppliedHardClip& that) const { return !(*this == that); }

private:
    GrScissorState             fScissorState;
    GrWindowRectsState         fWindowRectsState;
    uint32_t                   fStencilStackID = SkClipStack::kInvalidGenID;
};

/**
 * Produced by GrClip. It provides a set of modifications to GrPipeline that implement the clip.
 */
class GrAppliedClip {
public:
    static GrAppliedClip Disabled() {
        return GrAppliedClip({1 << 29, 1 << 29});
    }

    GrAppliedClip(const SkISize& rtDims) : fHardClip(rtDims) {}
    GrAppliedClip(const SkISize& logicalRTDims, const SkISize& backingStoreDims)
            : fHardClip(logicalRTDims, backingStoreDims) {}

    GrAppliedClip(GrAppliedClip&& that) = default;
    GrAppliedClip(const GrAppliedClip&) = delete;

    const GrScissorState& scissorState() const { return fHardClip.scissorState(); }
    const GrWindowRectsState& windowRectsState() const { return fHardClip.windowRectsState(); }
    uint32_t stencilStackID() const { return fHardClip.stencilStackID(); }
    bool hasStencilClip() const { return fHardClip.hasStencilClip(); }
    int hasClipCoverageFragmentProcessor() const { return fClipCoverageFP != nullptr; }
    const GrFragmentProcessor* clipCoverageFragmentProcessor() const {
        SkASSERT(fClipCoverageFP != nullptr);
        return fClipCoverageFP.get();
    }
    std::unique_ptr<const GrFragmentProcessor> detachClipCoverageFragmentProcessor() {
        SkASSERT(fClipCoverageFP != nullptr);
        return std::move(fClipCoverageFP);
    }

    const GrAppliedHardClip& hardClip() const { return fHardClip; }
    GrAppliedHardClip& hardClip() { return fHardClip; }

    void addCoverageFP(std::unique_ptr<GrFragmentProcessor> fp) {
        if (fClipCoverageFP == nullptr) {
            fClipCoverageFP = std::move(fp);
        } else {
            // Run this coverage FP in series with the previously-added clip coverage.
            std::unique_ptr<GrFragmentProcessor> series[] = {
                std::move(fClipCoverageFP),
                std::move(fp),
            };
            fClipCoverageFP = GrFragmentProcessor::RunInSeries(series, SK_ARRAY_COUNT(series));
        }
    }

    bool doesClip() const {
        return fHardClip.doesClip() || fClipCoverageFP != nullptr;
    }

    bool operator==(const GrAppliedClip& that) const {
        if (fHardClip != that.fHardClip ||
            this->hasClipCoverageFragmentProcessor() != that.hasClipCoverageFragmentProcessor()) {
            return false;
        }
        if (fClipCoverageFP != nullptr) {
            if (!fClipCoverageFP->isEqual(*that.fClipCoverageFP)) {
                return false;
            }
        }
        return true;
    }
    bool operator!=(const GrAppliedClip& that) const { return !(*this == that); }

    void visitProxies(const GrOp::VisitProxyFunc& func) const {
        if (fClipCoverageFP != nullptr) {
            fClipCoverageFP->visitProxies(func);
        }
    }

private:
    GrAppliedHardClip fHardClip;
    std::unique_ptr<GrFragmentProcessor> fClipCoverageFP;
};

#endif
