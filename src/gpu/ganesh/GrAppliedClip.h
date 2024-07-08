/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrAppliedClip_DEFINED
#define GrAppliedClip_DEFINED

#include "include/core/SkRect.h"
#include "include/private/base/SkAssert.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/core/SkClipStack.h"
#include "src/gpu/ganesh/GrFragmentProcessor.h"
#include "src/gpu/ganesh/GrScissorState.h"
#include "src/gpu/ganesh/GrWindowRectsState.h"

#include <cstdint>
#include <memory>
#include <utility>

class GrWindowRectangles;
struct SkISize;

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
    explicit GrAppliedHardClip(const GrAppliedHardClip&) = default;

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

    void setScissor(const SkIRect& irect) {
        fScissorState.set(irect);
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
    int hasCoverageFragmentProcessor() const { return fCoverageFP != nullptr; }
    const GrFragmentProcessor* coverageFragmentProcessor() const {
        SkASSERT(fCoverageFP != nullptr);
        return fCoverageFP.get();
    }
    std::unique_ptr<GrFragmentProcessor> detachCoverageFragmentProcessor() {
        SkASSERT(fCoverageFP != nullptr);
        return std::move(fCoverageFP);
    }

    const GrAppliedHardClip& hardClip() const { return fHardClip; }
    GrAppliedHardClip& hardClip() { return fHardClip; }

    void addCoverageFP(std::unique_ptr<GrFragmentProcessor> fp) {
        if (fCoverageFP == nullptr) {
            fCoverageFP = std::move(fp);
        } else {
            // Compose this coverage FP with the previously-added coverage.
            fCoverageFP = GrFragmentProcessor::Compose(std::move(fp), std::move(fCoverageFP));
        }
    }

    bool doesClip() const {
        return fHardClip.doesClip() || fCoverageFP != nullptr;
    }

    bool operator==(const GrAppliedClip& that) const {
        if (fHardClip != that.fHardClip ||
            this->hasCoverageFragmentProcessor() != that.hasCoverageFragmentProcessor()) {
            return false;
        }
        if (fCoverageFP != nullptr && !fCoverageFP->isEqual(*that.fCoverageFP)) {
            return false;
        }
        return true;
    }
    bool operator!=(const GrAppliedClip& that) const { return !(*this == that); }

    void visitProxies(const GrVisitProxyFunc& func) const {
        if (fCoverageFP != nullptr) {
            fCoverageFP->visitProxies(func);
        }
    }

private:
    GrAppliedHardClip fHardClip;
    std::unique_ptr<GrFragmentProcessor> fCoverageFP;
};

#endif
