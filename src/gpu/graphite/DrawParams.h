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
#include "src/gpu/graphite/DrawOrder.h"
#include "src/gpu/graphite/geom/Geometry.h"
#include "src/gpu/graphite/geom/Rect.h"
#include "src/gpu/graphite/geom/Transform_graphite.h"

#include <optional>

namespace skgpu::graphite {

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
    Clip() = default;
    Clip(const Rect& drawBounds,
         const Rect& shapeBounds,
         const SkIRect& scissor,
         const SkShader* shader)
            : fDrawBounds(drawBounds)
            , fTransformedShapeBounds(shapeBounds)
            , fScissor(scissor)
            , fShader(shader) {}

    // Tight bounds of the draw, including any padding/outset for stroking and expansion due to
    // inverse fill and intersected with the scissor.
    const Rect& drawBounds() const { return fDrawBounds; }

    // The scissor rectangle obtained by restricting the bounds of the clip stack that affects the
    // draw to the device bounds. The scissor must contain drawBounds() and must already be
    // intersected with the device bounds.
    const SkIRect& scissor() const { return fScissor; }

    // Clipped bounds of the shape in device space, including any padding/outset for stroking,
    // intersected with the scissor and ignoring the fill rule. For a regular fill this is identical
    // to drawBounds(). For an inverse fill, this is a subset of drawBounds().
    const Rect& transformedShapeBounds() const { return fTransformedShapeBounds; }

    // If set, the clip shader's output alpha is further used to clip the draw.
    const SkShader* shader() const { return fShader; }

    bool isClippedOut() const { return fDrawBounds.isEmptyNegativeOrNaN(); }

private:
    // DrawList assumes the DrawBounds are correct for a given shape, transform, and style. They
    // are provided to the DrawList to avoid re-calculating the same bounds.
    Rect            fDrawBounds;
    Rect            fTransformedShapeBounds;
    SkIRect         fScissor;
    const SkShader* fShader;

    // TODO: If we add more complex analytic shapes for clipping, e.g. coverage rrect, it should
    // go here.
};

// Encapsulates all geometric state for a single high-level draw call. RenderSteps are responsible
// for transforming this state into actual rendering; shading from PaintParams is handled separately
class DrawParams {
public:
    DrawParams(const Transform& transform,
               const Geometry& geometry,
               const Clip& clip,
               DrawOrder drawOrder,
               const StrokeStyle* stroke)
            : fTransform(transform)
            , fGeometry(geometry)
            , fClip(clip)
            , fOrder(drawOrder)
            , fStroke(stroke ? std::optional<StrokeStyle>(*stroke) : std::nullopt) {}

    const Transform& transform() const { return fTransform; }
    const Geometry&  geometry()  const { return fGeometry;  }
    const Clip&      clip()      const { return fClip;      }
    DrawOrder        order()     const { return fOrder;     }

    // Optional stroke parameters if the geometry is stroked instead of filled
    bool isStroke() const { return fStroke.has_value(); }
    const StrokeStyle& strokeStyle() const {
        SkASSERT(this->isStroke());
        return *fStroke;
    }

private:
    const Transform& fTransform; // Lifetime of the transform must be held longer than the geometry

    Geometry  fGeometry;
    Clip      fClip;
    DrawOrder fOrder;

    std::optional<StrokeStyle> fStroke; // Not present implies fill
};

}  // namespace skgpu::graphite

#endif // skgpu_graphite_DrawParams_DEFINED
