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

    /**
     * This is the maximum distance that a draw may extend beyond a clip's boundary and still count
     * count as "on the other side". We leave some slack because floating point rounding error is
     * likely to blame. The rationale for 1e-3 is that in the coverage case (and barring unexpected
     * rounding), as long as coverage stays within 0.5 * 1/256 of its intended value it shouldn't
     * have any effect on the final pixel values.
     */
    constexpr static SkScalar kBoundsTolerance = 1e-3f;

    /**
     * Returns true if the given query bounds count as entirely inside the clip.
     *
     * @param innerClipBounds   device-space rect contained by the clip (SkRect or SkIRect).
     * @param queryBounds       device-space bounds of the query region.
     */
    template<typename TRect> constexpr static bool IsInsideClip(const TRect& innerClipBounds,
                                                                const SkRect& queryBounds) {
        return innerClipBounds.fRight - innerClipBounds.fLeft >= kBoundsTolerance &&
               innerClipBounds.fBottom - innerClipBounds.fTop >= kBoundsTolerance &&
               innerClipBounds.fLeft <= queryBounds.fLeft + kBoundsTolerance &&
               innerClipBounds.fTop <= queryBounds.fTop + kBoundsTolerance &&
               innerClipBounds.fRight >= queryBounds.fRight - kBoundsTolerance &&
               innerClipBounds.fBottom >= queryBounds.fBottom - kBoundsTolerance;
    }

    /**
     * Returns true if the given query bounds count as entirely outside the clip.
     *
     * @param outerClipBounds   device-space rect that contains the clip (SkRect or SkIRect).
     * @param queryBounds       device-space bounds of the query region.
     */
    template<typename TRect> constexpr static bool IsOutsideClip(const TRect& outerClipBounds,
                                                                 const SkRect& queryBounds) {
        return outerClipBounds.fRight - outerClipBounds.fLeft < kBoundsTolerance ||
               outerClipBounds.fBottom - outerClipBounds.fTop < kBoundsTolerance ||
               outerClipBounds.fLeft > queryBounds.fRight - kBoundsTolerance ||
               outerClipBounds.fTop > queryBounds.fBottom - kBoundsTolerance ||
               outerClipBounds.fRight < queryBounds.fLeft + kBoundsTolerance ||
               outerClipBounds.fBottom < queryBounds.fTop + kBoundsTolerance;
    }

    /**
     * Returns the minimal integer rect that counts as containing a given set of bounds.
     */
    static SkIRect GetPixelIBounds(const SkRect& bounds) {
        return SkIRect::MakeLTRB(SkScalarFloorToInt(bounds.fLeft + kBoundsTolerance),
                                 SkScalarFloorToInt(bounds.fTop + kBoundsTolerance),
                                 SkScalarCeilToInt(bounds.fRight - kBoundsTolerance),
                                 SkScalarCeilToInt(bounds.fBottom - kBoundsTolerance));
    }

    /**
     * Returns the minimal pixel-aligned rect that counts as containing a given set of bounds.
     */
    static SkRect GetPixelBounds(const SkRect& bounds) {
        return SkRect::MakeLTRB(SkScalarFloorToScalar(bounds.fLeft + kBoundsTolerance),
                                SkScalarFloorToScalar(bounds.fTop + kBoundsTolerance),
                                SkScalarCeilToScalar(bounds.fRight - kBoundsTolerance),
                                SkScalarCeilToScalar(bounds.fBottom - kBoundsTolerance));
    }

    /**
     * Returns true if the given rect counts as aligned with pixel boundaries.
     */
    static bool IsPixelAligned(const SkRect& rect) {
        return SkScalarAbs(SkScalarRoundToScalar(rect.fLeft) - rect.fLeft) <= kBoundsTolerance &&
               SkScalarAbs(SkScalarRoundToScalar(rect.fTop) - rect.fTop) <= kBoundsTolerance &&
               SkScalarAbs(SkScalarRoundToScalar(rect.fRight) - rect.fRight) <= kBoundsTolerance &&
               SkScalarAbs(SkScalarRoundToScalar(rect.fBottom) - rect.fBottom) <= kBoundsTolerance;
    }
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
