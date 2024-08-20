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
#include "include/core/SkScalar.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/gpu/ganesh/GrAppliedClip.h"

class GrDrawOp;
class GrRecordingContext;
namespace skgpu {
namespace ganesh {
class SurfaceDrawContext;
}
}  // namespace skgpu

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
    virtual Effect apply(GrRecordingContext*,
                         skgpu::ganesh::SurfaceDrawContext*,
                         GrDrawOp*,
                         GrAAType,
                         GrAppliedClip*,
                         SkRect* bounds) const = 0;

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
    virtual PreClipResult preApply(const SkRect& drawBounds, GrAA aa) const {
        SkIRect pixelBounds = GetPixelIBounds(drawBounds, aa);
        bool outside = !SkIRect::Intersects(pixelBounds, this->getConservativeBounds());
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
     * This is the slack around a half-pixel vertex coordinate where we don't trust the GPU's
     * rasterizer to round consistently. The rounding method is not defined in GPU specs, and
     * rasterizer precision frequently introduces errors where a fraction < 1/2 still rounds up.
     *
     * For non-AA bounds edges, an edge value between 0.45 and 0.55 will round in or round out
     * depending on what side its on. Outside of this range, the non-AA edge will snap using round()
     */
    constexpr static SkScalar kHalfPixelRoundingTolerance = 5e-2f;

    /**
     * Returns true if the given draw bounds count as entirely inside the clip.

     * @param innerClipBounds   device-space rect fully contained by the clip
     * @param drawBounds        device-space bounds of the query region.
     */
    static bool IsInsideClip(const SkIRect& innerClipBounds, const SkRect& drawBounds, GrAA aa) {
        return innerClipBounds.contains(GetPixelIBounds(drawBounds, aa));
    }

    /**
     * Returns true if the given draw bounds count as entirely outside the clip.

     * @param outerClipBounds   device-space rect that contains the clip
     * @param drawBounds        device-space bounds of the query region.
     * @param aa                whether or not the draw will use anti-aliasing
     */
    static bool IsOutsideClip(const SkIRect& outerClipBounds, const SkRect& drawBounds, GrAA aa) {
        return !SkIRect::Intersects(outerClipBounds, GetPixelIBounds(drawBounds, aa));
    }

    // Modifies the behavior of GetPixelIBounds
    enum class BoundsType {
        /**
         * Returns the tightest integer pixel bounding box such that the rasterization of a shape
         * contained in the analytic 'bounds', using the 'aa' method, will only have non-zero
         * coverage for pixels inside the returned bounds. Pixels outside the bounds will either
         * not be touched, or will have 0 coverage that creates no visual change.
         */
        kExterior,
        /**
         * Returns the largest integer pixel bounding box such that were 'bounds' to be rendered as
         * a solid fill using 'aa', every pixel in the returned bounds will have full coverage.
         *
         * This effectively determines the pixels that are definitely covered by a draw or clip. It
         * effectively performs the opposite operations as GetOuterPixelBounds. It rounds in instead
         * of out for coverage AA and non-AA near pixel centers.
         */
        kInterior
    };

    /**
     * Convert the analytic bounds of a shape into an integer pixel bounds, where the given aa type
     * is used when the shape is rendered. The bounds mode can be used to query exterior or interior
     * pixel boundaries. Interior bounds only make sense when its know that the analytic bounds
     * are filled completely.
     *
     * NOTE: When using kExterior_Bounds, some coverage-AA rendering methods may still touch a pixel
     * center outside of these bounds but will evaluate to 0 coverage. This is visually acceptable,
     * but an additional outset of 1px should be used for dst proxy access.
     */
    static SkIRect GetPixelIBounds(const SkRect& bounds, GrAA aa,
                                   BoundsType mode = BoundsType::kExterior) {
        auto roundLow = [aa](float v) {
            v += kBoundsTolerance;
            return aa == GrAA::kNo ? SkScalarRoundToInt(v - kHalfPixelRoundingTolerance)
                                   : SkScalarFloorToInt(v);
        };
        auto roundHigh = [aa](float v) {
            v -= kBoundsTolerance;
            return aa == GrAA::kNo ? SkScalarRoundToInt(v + kHalfPixelRoundingTolerance)
                                   : SkScalarCeilToInt(v);
        };

        if (bounds.isEmpty()) {
            return SkIRect::MakeEmpty();
        }

        if (mode == BoundsType::kExterior) {
            return SkIRect::MakeLTRB(roundLow(bounds.fLeft),   roundLow(bounds.fTop),
                                     roundHigh(bounds.fRight), roundHigh(bounds.fBottom));
        } else {
            return SkIRect::MakeLTRB(roundHigh(bounds.fLeft), roundHigh(bounds.fTop),
                                     roundLow(bounds.fRight), roundLow(bounds.fBottom));
        }
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
     * return 'bounds' has been intersected with a conservative bounds of the clip.
     */
    virtual Effect apply(GrAppliedHardClip* out, SkIRect* bounds) const = 0;

private:
    Effect apply(GrRecordingContext*,
                 skgpu::ganesh::SurfaceDrawContext*,
                 GrDrawOp*,
                 GrAAType aa,
                 GrAppliedClip* out,
                 SkRect* bounds) const final {
        SkIRect pixelBounds = GetPixelIBounds(*bounds, GrAA(aa != GrAAType::kNone));
        Effect effect = this->apply(&out->hardClip(), &pixelBounds);
        bounds->intersect(SkRect::Make(pixelBounds));
        return effect;
    }
};

#endif
