/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrClip_DEFINED
#define GrClip_DEFINED

#include "include/core/SkRRect.h"
#include "include/core/SkRect.h"
#include "src/gpu/GrAppliedClip.h"
#include "src/gpu/GrRenderTargetContext.h"

class GrContext;

/**
 * GrClip is an abstract base class for applying a clip. It constructs a clip mask if necessary, and
 * fills out a GrAppliedClip instructing the caller on how to set up the draw state.
 */
class GrClip {
public:
    virtual bool quickContains(const SkRect&) const = 0;
    virtual bool quickContains(const SkRRect& rrect) const {
        return this->quickContains(rrect.getBounds());
    }
    virtual void getConservativeBounds(int width, int height, SkIRect* devResult,
                                       bool* isIntersectionOfRects = nullptr) const = 0;
    /**
     * This computes a GrAppliedClip from the clip which in turn can be used to build a GrPipeline.
     * To determine the appropriate clipping implementation the GrClip subclass must know whether
     * the draw will enable HW AA or uses the stencil buffer. On input 'bounds' is a conservative
     * bounds of the draw that is to be clipped. After return 'bounds' has been intersected with a
     * conservative bounds of the clip. A return value of false indicates that the draw can be
     * skipped as it is fully clipped out.
     */
    virtual bool apply(GrRecordingContext*, GrRenderTargetContext*, bool useHWAA,
                       bool hasUserStencilSettings, GrAppliedClip*, SkRect* bounds) const = 0;

    virtual ~GrClip() {}

    /**
     * This method quickly and conservatively determines whether the entire clip is equivalent to
     * intersection with a rrect. This will only return true if the rrect does not fully contain
     * the render target bounds. Moreover, the returned rrect need not be contained by the render
     * target bounds. We assume all draws will be implicitly clipped by the render target bounds.
     *
     * @param rtBounds The bounds of the render target that the clip will be applied to.
     * @param rrect    If return is true rrect will contain the rrect equivalent to the clip within
     *                 rtBounds.
     * @param aa       If return is true aa will indicate whether the rrect clip is antialiased.
     * @return true if the clip is equivalent to a single rrect, false otherwise.
     *
     */
    virtual bool isRRect(const SkRect& rtBounds, SkRRect* rrect, GrAA* aa) const = 0;

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
    template <typename TRect>
    constexpr static bool IsInsideClip(const TRect& innerClipBounds, const SkRect& queryBounds) {
        return innerClipBounds.fRight > innerClipBounds.fLeft + kBoundsTolerance &&
               innerClipBounds.fBottom > innerClipBounds.fTop + kBoundsTolerance &&
               innerClipBounds.fLeft < queryBounds.fLeft + kBoundsTolerance &&
               innerClipBounds.fTop < queryBounds.fTop + kBoundsTolerance &&
               innerClipBounds.fRight > queryBounds.fRight - kBoundsTolerance &&
               innerClipBounds.fBottom > queryBounds.fBottom - kBoundsTolerance;
    }

    /**
     * Returns true if the given query bounds count as entirely outside the clip.
     *
     * @param outerClipBounds   device-space rect that contains the clip (SkRect or SkIRect).
     * @param queryBounds       device-space bounds of the query region.
     */
    template <typename TRect>
    constexpr static bool IsOutsideClip(const TRect& outerClipBounds, const SkRect& queryBounds) {
        return
            // Is the clip so small that it is effectively empty?
            outerClipBounds.fRight - outerClipBounds.fLeft <= kBoundsTolerance ||
            outerClipBounds.fBottom - outerClipBounds.fTop <= kBoundsTolerance ||

            // Are the query bounds effectively outside the clip?
            outerClipBounds.fLeft >= queryBounds.fRight - kBoundsTolerance ||
            outerClipBounds.fTop >= queryBounds.fBottom - kBoundsTolerance ||
            outerClipBounds.fRight <= queryBounds.fLeft + kBoundsTolerance ||
            outerClipBounds.fBottom <= queryBounds.fTop + kBoundsTolerance;
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
 * GrHardClip never uses coverage FPs. It can only enforce the clip using the already-existing
 * stencil buffer contents and/or fixed-function state like scissor. Always aliased if MSAA is off.
 */
class GrHardClip : public GrClip {
public:
    /**
     * Sets the appropriate hardware state modifications on GrAppliedHardClip that will implement
     * the clip. On input 'bounds' is a conservative bounds of the draw that is to be clipped. After
     * return 'bounds' has been intersected with a conservative bounds of the clip. A return value
     * of false indicates that the draw can be skipped as it is fully clipped out.
     */
    virtual bool apply(int rtWidth, int rtHeight, GrAppliedHardClip* out, SkRect* bounds) const = 0;

private:
    bool apply(GrRecordingContext*, GrRenderTargetContext* rtc, bool useHWAA,
               bool hasUserStencilSettings, GrAppliedClip* out, SkRect* bounds) const final {
        return this->apply(rtc->width(), rtc->height(), &out->hardClip(), bounds);
    }
};

/**
 * Specialized implementation for no clip.
 */
class GrNoClip final : public GrHardClip {
private:
    bool quickContains(const SkRect&) const final { return true; }
    bool quickContains(const SkRRect&) const final { return true; }
    void getConservativeBounds(int width, int height, SkIRect* devResult,
                               bool* isIntersectionOfRects) const final {
        devResult->setXYWH(0, 0, width, height);
        if (isIntersectionOfRects) {
            *isIntersectionOfRects = true;
        }
    }
    bool apply(int rtWidth, int rtHeight, GrAppliedHardClip*, SkRect*) const final { return true; }
    bool isRRect(const SkRect&, SkRRect*, GrAA*) const override { return false; }
};

#endif
