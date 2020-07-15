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

/**
 * GrClip is an abstract base class for applying a clip. It constructs a clip mask if necessary, and
 * fills out a GrAppliedClip instructing the caller on how to set up the draw state.
 */
class GrClip {
public:
    enum class Effect {
        // The clip conservatively modifies the draw's coverage but doesn't eliminate the draw
        kClipped,
        // The clip definitely does not modify the draw's coverage and the draw can be performed
        // without clipping (beyond the automatic device bounds clip).
        kUnclipped,
        // The clip definitely eliminates all of the draw's coverage and the draw can be skipped
        kClippedOut
    };

    struct PreClipResult {
        Effect  fEffect;
        SkRRect fRRect; // Ignore if 'isRRect' is false
        GrAA    fAA;    // Ignore if 'isRRect' is false
        bool    fIsRRect;

        PreClipResult(Effect effect) : fEffect(effect), fIsRRect(false) {}
        PreClipResult(SkRect rect, GrAA aa) : PreClipResult(SkRRect::MakeRect(rect), aa) {}
        PreClipResult(SkRRect rrect, GrAA aa)
                : fEffect(Effect::kClipped)
                , fRRect(rrect)
                , fAA(aa)
                , fIsRRect(true) {}
    };

    virtual ~GrClip() {}

    /**
     * Compute a conservative pixel bounds restricted to the given render target dimensions.
     * The returned bounds represent the limits of pixels that can be drawn; anything outside of the
     * bounds will be entirely clipped out.
     */
    virtual SkIRect getConservativeBounds() const = 0;

    /**
     * This computes a GrAppliedClip from the clip which in turn can be used to build a GrPipeline.
     * To determine the appropriate clipping implementation the GrClip subclass must know whether
     * the draw will enable HW AA or uses the stencil buffer. On input 'bounds' is a conservative
     * bounds of the draw that is to be clipped. If kClipped or kUnclipped is returned, the 'bounds'
     * will have been updated to be contained within the clip bounds (or the device's, for wide-open
     * clips). If kNoDraw is returned, 'bounds' and the applied clip are in an undetermined state
     * and should be ignored (and the draw should be skipped).
     */
    virtual Effect apply(GrRecordingContext*, GrRenderTargetContext*, bool useHWAA,
                         bool hasUserStencilSettings, GrAppliedClip*, SkRect* bounds) const = 0;

    /**
     * Perform preliminary, conservative analysis on the draw bounds as if it were provided to
     * apply(). The results of this are returned the PreClipResults struct, where 'result.fEffect'
     * corresponds to what 'apply' would return. If this value is kUnclipped or kNoDraw, then it
     * can be assumed that apply() would also always result in the same Effect.
     *
     * If kClipped is returned, apply() may further refine the effect to kUnclipped or kNoDraw,
     * with one exception. When 'result.fIsRRect' is true, preApply() reports the single round rect
     * and anti-aliased state that would act as an intersection on the draw geometry. If no further
     * action is taken to modify the draw, apply() will represent this round rect in the applied
     * clip.
     *
     * When set, 'result.fRRect' will intersect with the render target bounds but may extend
     * beyond it. If the render target bounds are the only clip effect on the draw, this is reported
     * as kUnclipped and not as a degenerate rrect that matches the bounds.
     */
    virtual PreClipResult preApply(const SkRect& drawBounds) const {
        bool outside = !drawBounds.intersects(SkRect::Make(this->getConservativeBounds()));
        return outside ? Effect::kClippedOut : Effect::kClipped;
    }

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
    virtual Effect apply(GrAppliedHardClip* out, SkRect* bounds) const = 0;

private:
    Effect apply(GrRecordingContext*, GrRenderTargetContext* rtc, bool useHWAA,
                     bool hasUserStencilSettings, GrAppliedClip* out, SkRect* bounds) const final {
        return this->apply(&out->hardClip(), bounds);
    }
};

#endif
