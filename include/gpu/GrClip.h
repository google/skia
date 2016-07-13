/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrClip_DEFINED
#define GrClip_DEFINED

#include "GrFragmentProcessor.h"
#include "GrTypesPriv.h"
#include "SkClipStack.h"

class GrDrawContext;

/**
 * Produced by GrClip. It provides a set of modifications to the drawing state that are used to
 * create the final GrPipeline for a GrBatch.
 */
class GrAppliedClip : public SkNoncopyable {
public:
    GrAppliedClip() : fHasStencilClip(false), fDeviceBounds(SkRect::MakeLargest()) {}
    GrFragmentProcessor* getClipCoverageFragmentProcessor() const {
        return fClipCoverageFP.get();
    }
    const GrScissorState& scissorState() const { return fScissorState; }
    bool hasStencilClip() const { return fHasStencilClip; }

    void makeStencil(bool hasStencil, const SkRect& deviceBounds) {
        fClipCoverageFP = nullptr;
        fScissorState.setDisabled();
        fHasStencilClip = hasStencil;
        fDeviceBounds = deviceBounds;
    }

    /**
     * The device bounds of the clip defaults to the scissor rect, but a tighter bounds (based
     * on the known effect of the stencil values) can be provided.
     */
    void makeScissoredStencil(const SkIRect& scissor, const SkRect* deviceBounds = nullptr) {
        fClipCoverageFP = nullptr;
        fScissorState.set(scissor);
        fHasStencilClip = true;
        if (deviceBounds) {
            fDeviceBounds = *deviceBounds;
            SkASSERT(scissor.contains(*deviceBounds))
        } else {
            fDeviceBounds = SkRect::Make(scissor);
        }
    }

    void makeFPBased(sk_sp<GrFragmentProcessor> fp, const SkRect& deviceBounds) {
        fClipCoverageFP = fp;
        fScissorState.setDisabled();
        fHasStencilClip = false;
        fDeviceBounds = deviceBounds;
    }

    void makeScissored(SkIRect& scissor) {
        fClipCoverageFP.reset();
        fScissorState.set(scissor);
        fHasStencilClip = false;
        fDeviceBounds = SkRect::Make(scissor);
    }

    /**
     * The device bounds of the clip defaults to the scissor rect, but a tighter bounds (based
     * on the known effect of the fragment processor) can be provided.
     */
    void makeScissoredFPBased(sk_sp<GrFragmentProcessor> fp, const SkIRect& scissor,
                              const SkRect* deviceBounds = nullptr) {
        fClipCoverageFP = fp;
        fScissorState.set(scissor);
        fHasStencilClip = false;
        if (deviceBounds) {
            fDeviceBounds = *deviceBounds;
            SkASSERT(scissor.contains(*deviceBounds))
        } else {
            fDeviceBounds = SkRect::Make(scissor);
        }
    }

    /**
     * Returns the device bounds of the applied clip. Ideally this considers the combined effect of
     * all clipping techniques in play (scissor, stencil, and/or coverage fp).
     */
    const SkRect& deviceBounds() const { return fDeviceBounds; }

private:
    sk_sp<GrFragmentProcessor> fClipCoverageFP;
    GrScissorState             fScissorState;
    bool                       fHasStencilClip;
    SkRect                     fDeviceBounds;
    typedef SkNoncopyable INHERITED;
};

/**
 * GrClip is an abstract base class for applying a clip. It constructs a clip mask if necessary, and
 * fills out a GrAppliedClip instructing the caller on how to set up the draw state.
 */
class GrClip {
public:
    virtual bool quickContains(const SkRect&) const = 0;
    virtual void getConservativeBounds(int width, int height, SkIRect* devResult,
                                       bool* isIntersectionOfRects = nullptr) const = 0;
    virtual bool apply(GrContext*,
                       GrDrawContext*,
                       const SkRect* devBounds,
                       bool useHWAA,
                       bool hasUserStencilSettings,
                       GrAppliedClip* out) const = 0;

    virtual ~GrClip() {}

protected:
    /**
     * Returns true if a clip can safely disable its scissor test for a particular draw.
     */
    static bool CanIgnoreScissor(const SkIRect& scissorRect, const SkRect& drawBounds) {
        // This is the maximum distance that a draw may extend beyond a clip's scissor and still
        // count as inside. We use a sloppy compare because the draw may have chosen its bounds in a
        // different coord system. The rationale for 1e-3 is that in the coverage case (and barring
        // unexpected rounding), as long as coverage stays below 0.5 * 1/256 we ought to be OK.
        constexpr SkScalar fuzz = 1e-3f;
        SkASSERT(!scissorRect.isEmpty());
        SkASSERT(!drawBounds.isEmpty());
        return scissorRect.fLeft <= drawBounds.fLeft + fuzz &&
               scissorRect.fTop <= drawBounds.fTop + fuzz &&
               scissorRect.fRight >= drawBounds.fRight - fuzz &&
               scissorRect.fBottom >= drawBounds.fBottom - fuzz;
    }

    friend class GrClipMaskManager;
};

/**
 * Specialized implementation for no clip.
 */
class GrNoClip final : public GrClip {
private:
    bool quickContains(const SkRect&) const final { return true; }
    void getConservativeBounds(int width, int height, SkIRect* devResult,
                               bool* isIntersectionOfRects) const final;
    bool apply(GrContext*,
               GrDrawContext*,
               const SkRect* /* devBounds */,
               bool /* useHWAA */,
               bool /* hasUserStencilSettings */,
               GrAppliedClip* /* out */) const final { return true; }
};

/**
 * GrFixedClip is a clip that can be represented by fixed-function hardware. It never modifies the
 * stencil buffer itself, but can be configured to use whatever clip is already there.
 */
class GrFixedClip final : public GrClip {
public:
    GrFixedClip() : fDeviceBounds(SkRect::MakeLargest()), fHasStencilClip(false) {}
    GrFixedClip(const SkIRect& scissorRect)
        : fScissorState(scissorRect)
        , fDeviceBounds(SkRect::Make(scissorRect))
        , fHasStencilClip(false) {}

    void reset() {
        fScissorState.setDisabled();
        fDeviceBounds.setLargest();
        fHasStencilClip = false;
    }

    void reset(const SkIRect& scissorRect) {
        fScissorState.set(scissorRect);
        fDeviceBounds = SkRect::Make(scissorRect);
        fHasStencilClip = false;
    }

    /**
     * Enables stenciling. The stencil bounds is the device space bounds where the stencil test
     * may pass.
     */
    void enableStencilClip(const SkRect& stencilBounds) {
        fHasStencilClip = true;
        fDeviceBounds = stencilBounds;
        if (fScissorState.enabled()) {
            const SkIRect& s = fScissorState.rect();
            fDeviceBounds.fLeft   = SkTMax(fDeviceBounds.fLeft,   SkIntToScalar(s.fLeft));
            fDeviceBounds.fTop    = SkTMax(fDeviceBounds.fTop,    SkIntToScalar(s.fTop));
            fDeviceBounds.fRight  = SkTMin(fDeviceBounds.fRight,  SkIntToScalar(s.fRight));
            fDeviceBounds.fBottom = SkTMin(fDeviceBounds.fBottom, SkIntToScalar(s.fBottom));
        }
    }

    void disableStencilClip() {
        fHasStencilClip = false;
        if (fScissorState.enabled()) {
            fDeviceBounds = SkRect::Make(fScissorState.rect());
        } else {
            fDeviceBounds.setLargest();
        }
    }

    const GrScissorState& scissorState() const { return fScissorState; }
    bool hasStencilClip() const { return fHasStencilClip; }

    bool quickContains(const SkRect&) const final;
    void getConservativeBounds(int width, int height, SkIRect* devResult,
                               bool* isIntersectionOfRects) const final;

private:
    bool apply(GrContext*,
               GrDrawContext*,
               const SkRect* devBounds,
               bool useHWAA,
               bool hasUserStencilSettings,
               GrAppliedClip* out) const final;

    GrScissorState   fScissorState;
    SkRect           fDeviceBounds;
    bool             fHasStencilClip;
};

/**
 * GrClipStackClip can apply a generic SkClipStack to the draw state. It may generate clip masks or
 * write to the stencil buffer during apply().
 */
class GrClipStackClip final : public GrClip {
public:
    GrClipStackClip(const SkClipStack* stack = nullptr, const SkIPoint* origin = nullptr) {
        this->reset(stack, origin);
    }

    void reset(const SkClipStack* stack = nullptr, const SkIPoint* origin = nullptr) {
        fOrigin = origin ? *origin : SkIPoint::Make(0, 0);
        fStack.reset(SkSafeRef(stack));
    }

    const SkIPoint& origin() const { return fOrigin; }
    const SkClipStack* clipStack() const { return fStack; }

    bool quickContains(const SkRect&) const final;
    void getConservativeBounds(int width, int height, SkIRect* devResult,
                               bool* isIntersectionOfRects) const final;
    bool apply(GrContext*,
               GrDrawContext*,
               const SkRect* devBounds,
               bool useHWAA,
               bool hasUserStencilSettings,
               GrAppliedClip* out) const final;

private:
    SkIPoint                          fOrigin;
    SkAutoTUnref<const SkClipStack>   fStack;
};

#endif
