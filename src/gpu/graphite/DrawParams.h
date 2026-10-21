/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_DrawParams_DEFINED
#define skgpu_graphite_DrawParams_DEFINED

#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"
#include "include/private/SkAssert.h"
#include "include/private/SkOnlyStackAllocatable.h"
#include "include/private/SkTo.h"
#include "src/gpu/graphite/DrawOrder.h"
#include "src/gpu/graphite/DrawTypes.h"
#include "src/gpu/graphite/geom/Geometry.h"
#include "src/gpu/graphite/geom/NonMSAAClip.h"
#include "src/gpu/graphite/geom/Rect.h"
#include "src/gpu/graphite/geom/Transform.h"

#if defined(SK_ENABLE_SPARSE_STRIPS)
#include "src/gpu/graphite/sparse_strips/SparseStripsConfig.h"
#endif

#include <algorithm>
#include <optional>

class SkShader;

namespace skgpu::graphite {
class Transform;

// NOTE: Only represents the stroke or hairline styles; stroke-and-fill must be handled higher up.
class StrokeStyle {
public:
    StrokeStyle() : fHalfWidth(0.f), fJoinLimit(0.f), fCap(SkPaint::kButt_Cap) {}
    StrokeStyle(float width,
                float miterLimit,
                SkPaint::Join join,
                SkPaint::Cap cap)
            : fHalfWidth(std::max(0.f, 0.5f * width))
            , fJoinLimit(join == SkPaint::kMiter_Join ? std::max(0.f, miterLimit) :
                         (join == SkPaint::kBevel_Join ? 0.f : -1.f))
            , fCap(cap) {}

    StrokeStyle(const StrokeStyle&) = default;

    StrokeStyle& operator=(const StrokeStyle&) = default;

    bool isMiterJoin() const { return fJoinLimit > 0.f;  }
    bool isBevelJoin() const { return fJoinLimit == 0.f; }
    bool isRoundJoin() const { return fJoinLimit < 0.f;  }

    float         halfWidth()  const { return fHalfWidth;                }
    float         width()      const { return 2.f * fHalfWidth;          }
    float         miterLimit() const { return std::max(0.f, fJoinLimit); }
    SkPaint::Cap  cap()        const { return fCap;                      }
    SkPaint::Join join()       const {
        return fJoinLimit > 0.f ? SkPaint::kMiter_Join :
               (fJoinLimit == 0.f ? SkPaint::kBevel_Join : SkPaint::kRound_Join);
    }

    // Raw join limit, compatible with tess::StrokeParams
    float joinLimit() const { return fJoinLimit; }

private:
    float        fHalfWidth; // >0: relative to transform; ==0: hairline, 1px in device space
    float        fJoinLimit; // >0: miter join; ==0: bevel join; <0: round join
    SkPaint::Cap fCap;
};

// TBD: Separate DashParams extracted from an SkDashPathEffect? Or folded into StrokeStyle?

class Clip {
public:
    SK_ONLY_STACK_ALLOCATABLE;

    Clip() = default;
    Clip(const Rect& drawBounds,
         const Rect& shapeBounds,
         const SkIRect& scissor,
         const NonMSAAClip& nonMSAAClip,
         const SkShader* shader)
            : fDrawBounds(drawBounds)
            , fTransformedShapeBounds(shapeBounds)
            , fScissor(scissor)
            , fNonMSAAClip(nonMSAAClip)
            , fShader(shader) {}

    // Tight bounds of the draw, including any padding/outset for stroking and expansion due to
    // inverse fill and intersected with the scissor.
    const Rect& drawBounds() const { return fDrawBounds; }

    // The scissor rectangle obtained by restricting the bounds of the clip stack that affects the
    // draw to the device bounds. The scissor must contain drawBounds() and must already be
    // intersected with the device bounds.
    const SkIRect& scissor() const { return fScissor; }

    // Unclipped bounds of the shape in device space, including any padding/outset for stroking but
    // ignoring the fill rule. This is not restricted by the scissor (or the target device's
    // physical bounds).
    //
    // For a regular fill, drawBounds() is the intersection of this rectangle and scissor().
    //
    // For an inverse fill, this is the bounding box of the interesting portion of any coverage
    // mask. If it doesn't intersect the scissor, the draw fully covers the scissor; regardless the
    // drawBounds() are equal to the scissor.
    const Rect& transformedShapeBounds() const { return fTransformedShapeBounds; }

    // If set, the shape's bounds and/or an atlas mask are further used to clip the draw.
    // NOTE: This cannot impact `drawBounds()` as pixels outside of the non-msaa clip may still be
    // shaded and blended with a coverage value of 0, which could lead to undefined behavior on the
    // GPU if operations were ordered assuming tighter bounds.
    const NonMSAAClip& nonMSAAClip() const { return fNonMSAAClip; }

    // If set, the clip shader's output alpha is further used to clip the draw.
    const SkShader* shader() const { return fShader; }

    bool isClippedOut() const { return fDrawBounds.isEmptyNegativeOrNaN(); }

    bool needsCoverage() const { return SkToBool(fShader) || !fNonMSAAClip.isEmpty(); }

    void outsetBoundsForAA() {
        // We use 1px to handle both subpixel/hairline approaches and the standard 1/2px outset
        // for shapes that cover multiple pixels.
        fTransformedShapeBounds.outset(1.f);
        // This is a no-op for inverse fills (where fDrawBounds was already equal to fScissor),
        // and equivalent to fDrawBounds = fTransformedShapeBounds.makeIntersect(fScissor) with
        // the outset shape bounds.
        fDrawBounds.outset(1.f).intersect(fScissor);
    }

#if defined(SK_ENABLE_SPARSE_STRIPS)
    // Outsets the bounds to the SparseStrips tile grid in device space, since EndCaps rasterize
    // whole tiles regardless of the shape's coverage. The draw bounds are integral afterwards.
    void outsetBoundsToGrid() {
        // Rect stores [left, top, -right, -bot], so flooring every value rounds the rect out. The
        // tile size is a compile-time power of two, so the divide folds into a multiply.
        auto snap = [](const Rect& r) {
            constexpr float kW = SparseStripConfig::kTileWidth;
            constexpr float kH = SparseStripConfig::kTileHeight;
            const skvx::float4 tile(kW, kH, kW, kH);
            return Rect::FromVals(skvx::floor(r.vals() / tile) * tile);
        };
        fTransformedShapeBounds = snap(fTransformedShapeBounds);
        // Like outsetBoundsForAA(), this is a no-op for inverse fills, and otherwise keeps
        // fDrawBounds equal to fTransformedShapeBounds.makeIntersect(fScissor).
        fDrawBounds = snap(fDrawBounds).makeIntersect(fScissor);
    }
#endif

private:
    // DrawList assumes the DrawBounds are correct for a given shape, transform, and style. They
    // are provided to the DrawList to avoid re-calculating the same bounds.
    Rect              fDrawBounds;
    Rect              fTransformedShapeBounds;
    SkIRect           fScissor;
    NonMSAAClip       fNonMSAAClip;
    const SkShader*   fShader;
};

// Encapsulates all geometric state for a single high-level draw call. RenderSteps are responsible
// for transforming this state into actual rendering; shading from PaintParams is handled separately
class DrawParams {
public:
    DrawParams(const Transform& transform,
               const Geometry& geometry,
               const Clip& clip,
               DrawOrder drawOrder,
               const StrokeStyle* stroke,
               BarrierType barrierBeforeDraws)
            : fTransform(transform)
            , fGeometry(geometry)
            , fDrawBounds(clip.drawBounds())
            , fTransformedShapeBounds(clip.transformedShapeBounds())
            , fScissor(clip.scissor())
            , fOrder(drawOrder)
            , fBarrierBeforeDraws(barrierBeforeDraws)
            , fStroke(stroke ? std::optional<StrokeStyle>(*stroke) : std::nullopt) {}

    const Transform& transform() const { return fTransform; }
    const Geometry&  geometry()  const { return fGeometry;  }
    DrawOrder        order()     const { return fOrder;     }

    // The subset of a Clip's state that is preserved in a DrawList, whereas the other properties
    // of a clip get consumed into the paint's key and uniform data.
    Rect drawBounds() const { return fDrawBounds; }
    Rect transformedShapeBounds() const { return fTransformedShapeBounds; }
    BarrierType barrierBeforeDraws() const {return fBarrierBeforeDraws; }
    const SkIRect& scissor() const { return fScissor; }

    // Optional stroke parameters if the geometry is stroked instead of filled
    bool isStroke() const { return fStroke.has_value(); }
    const StrokeStyle& strokeStyle() const {
        SkASSERT(this->isStroke());
        return *fStroke;
    }

    Rect styledBounds() const {
        Rect localBounds = fGeometry.bounds();
        if (this->isStroke() && fGeometry.isShape()) {
            float localOutset = fGeometry.shape().getInflationRadius(fStroke->join(),
                                                                     fStroke->miterLimit(),
                                                                     fStroke->cap(),
                                                                     fStroke->width());
            localBounds.outset(localOutset);
        }
        return localBounds;
    }

private:
    friend class ClipStack; // For updating clip and order components on depth only draws

    const Transform& fTransform; // Lifetime of the transform must be held longer than the geometry
    Geometry         fGeometry;
    Rect             fDrawBounds;
    Rect             fTransformedShapeBounds;
    SkIRect          fScissor;
    DrawOrder        fOrder;
    BarrierType      fBarrierBeforeDraws;

    std::optional<StrokeStyle> fStroke; // Not present implies fill
};

}  // namespace skgpu::graphite

#endif // skgpu_graphite_DrawParams_DEFINED
