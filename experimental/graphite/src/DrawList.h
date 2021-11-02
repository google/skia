/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_DrawList_DEFINED
#define skgpu_DrawList_DEFINED

#include "include/core/SkColor.h"
#include "include/core/SkPaint.h"
#include "include/private/SkTOptional.h"
#include "src/core/SkTBlockList.h"

#include "experimental/graphite/src/DrawOrder.h"
#include "experimental/graphite/src/geom/Shape.h"
#include "experimental/graphite/src/geom/Transform_graphite.h"

#include <limits>

class SkPath;
class SkShader;
struct SkIRect;

namespace skgpu {

struct IndexWriter;
class Renderer;
struct VertexWriter;

// TBD: If occlusion culling is eliminated as a phase, we can easily move the paint conversion
// back to Device when the command is recorded (similar to SkPaint -> GrPaint), and then
// PaintParams is not required as an intermediate representation.
// NOTE: Only represents the shading state of an SkPaint. Style and complex effects (mask filters,
// image filters, path effects) must be handled higher up. AA is not tracked since everything is
// assumed to be anti-aliased.
class PaintParams {
public:
    PaintParams(const SkColor4f& color, SkBlendMode, sk_sp<SkShader>);
    PaintParams(const PaintParams&);
    ~PaintParams();

    PaintParams& operator=(const PaintParams&);

    SkColor4f color() const { return fColor; }
    SkBlendMode blendMode() const { return fBlendMode; }
    SkShader* shader() const { return fShader.get(); }
    sk_sp<SkShader> refShader() const;

private:
    SkColor4f       fColor;
    SkBlendMode     fBlendMode;
    sk_sp<SkShader> fShader; // For now only use SkShader::asAGradient() when converting to GPU
    // TODO: Will also store ColorFilter, custom Blender, dither, and any extra shader from an
    // active clipShader().
};

// NOTE: Only represents the stroke or hairline styles; stroke-and-fill must be handled higher up.
class StrokeParams {
public:
    StrokeParams() : fHalfWidth(0.f), fJoinLimit(0.f), fCap(SkPaint::kButt_Cap) {}
    StrokeParams(float width,
                 float miterLimit,
                 SkPaint::Join join,
                 SkPaint::Cap cap)
            : fHalfWidth(std::max(0.f, 0.5f * width))
            , fJoinLimit(join == SkPaint::kMiter_Join ? std::max(0.f, miterLimit) :
                         (join == SkPaint::kBevel_Join ? 0.f : -1.f))
            , fCap(cap) {}

    StrokeParams(const StrokeParams&) = default;

    StrokeParams& operator=(const StrokeParams&) = default;

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

private:
    float        fHalfWidth; // >0: relative to transform; ==0: hairline, 1px in device space
    float        fJoinLimit; // >0: miter join; ==0: bevel join; <0: round join
    SkPaint::Cap fCap;
};

// TBD: Separate DashParams extracted from an SkDashPathEffect? Or folded into StrokeParams?

class Clip {
public:
    Clip(const Rect& drawBounds, const SkIRect& scissor)
            : fDrawBounds(drawBounds)
            , fScissor(scissor) {}

    const Rect&    drawBounds() const { return fDrawBounds; }
    const SkIRect& scissor()    const { return fScissor;    }

private:
    // Draw bounds represent the tight bounds of the draw, including any padding/outset for stroking
    // and intersected with the scissor.
    // - DrawList assumes the DrawBounds are correct for a given shape, transform, and style. They
    //   are provided to the DrawList to avoid re-calculating the same bounds.
    Rect    fDrawBounds;
    // The scissor must contain fDrawBounds, and must already be intersected with the device bounds.
    SkIRect fScissor;
    // TODO: If we add more complex analytic shapes for clipping, e.g. coverage rrect, it should
    // go here.
};

/**
 * A DrawList represents a collection of drawing commands (and related clip/shading state) in
 * a form that closely mirrors what can be rendered efficiently and directly by the GPU backend
 * (while balancing how much pre-processing to do for draws that might get eliminated later due to
 * occlusion culling).
 *
 * A draw command combines:
 *   - a shape
 *   - a transform
 *   - a primitive clip (not affected by the transform)
 *   - optional shading description (shader, color filter, blend mode, etc)
 *   - a draw ordering (compressed painters index, stencil set, and write/test depth)
 *
 * Commands are accumulated in an arbitrary order and then sorted by increasing sort z when the list
 * is prepared into an actual command buffer. The result of a draw command is the rasterization of
 * the transformed shape, restricted by its primitive clip (e.g. a scissor rect) and a depth test
 * of "GREATER" vs. its write/test z. (A test of GREATER, as opposed to GEQUAL, avoids double hits
 * for draws that may have overlapping geometry, e.g. stroking.) If the command has a shading
 * description, the color buffer will be modified; if not, it will be a depth-only draw.
 *
 * In addition to sorting the collected commands, the command list can be optimized during
 * preparation. Commands that are fully occluded by later operations can be skipped entirely without
 * affecting the final results. Adjacent commands (post sort) that would use equivalent GPU
 * pipelines are merged to produce fewer (but larger) operations on the GPU.
 *
 * Other than flush-time optimizations (sort, cull, and merge), the command list does what you tell
 * it to. Draw-specific simplification, style application, and advanced clipping should be handled
 * at a higher layer.
 */
class DrawList {
public:
    // The maximum number of draw calls that can be recorded into a DrawList before it must be
    // converted to a DrawPass. The true fundamental limit is imposed by the limits of the depth
    // attachment and precision of CompressedPaintersOrder and PaintDepth. These values can be
    // shared by multiple draw calls so it's more difficult to reason about how much room is left
    // in a DrawList. Limiting it to this keeps tracking simple and ensures that the sequences in
    // DrawOrder cannot overflow since they are always less than or equal to the number of draws.
    static constexpr int kMaxDraws = std::numeric_limits<uint16_t>::max();

    // NOTE: All path rendering functions, e.g. [fill|stroke|...]Path() that take a Shape
    // draw using the same underlying techniques regardless of the shape's type. If a Shape has
    // a type matching a simpler primitive technique or coverage AA, the caller must explicitly
    // invoke it to use that rendering algorithms.
    //
    // Additionally, DrawList requires that all Transforms passed to its draw calls be valid and
    // assert as much; invalid transforms should be detected at the Device level or similar.

    void stencilAndFillPath(const Transform& localToDevice,
                            const Shape& shape,
                            const Clip& clip,
                            DrawOrder ordering,
                            const PaintParams* paint);

    void fillConvexPath(const Transform& localToDevice,
                        const Shape& shape,
                        const Clip& clip,
                        DrawOrder ordering,
                        const PaintParams* paint);

    void strokePath(const Transform& localToDevice,
                    const Shape& shape,
                    const StrokeParams& stroke,
                    const Clip& clip,
                    DrawOrder ordering,
                    const PaintParams* paint);

    // TODO: fill[R]Rect, stroke[R]Rect (will need to support per-edge aa and arbitrary quads)
    //       fillImage (per-edge aa and arbitrary quad, only if this fast path is required)
    //       dashPath(feasible for general paths?)
    //       dash[R]Rect(only if general dashPath isn't viable)
    //       dashLine(only if general or rrect version aren't viable)

    int drawCount() const { return fDraws.count(); }
    int renderStepCount() const { return fRenderStepCount; }

private:
    friend class DrawPass;

    struct Draw {
        const Renderer&  fRenderer;  // Statically defined by function that recorded the Draw
        const Transform& fTransform; // Points to a transform in fTransforms

        Shape     fShape;
        Clip      fClip;
        DrawOrder fOrder;

        skstd::optional<PaintParams>  fPaintParams; // Not present implies depth-only draw
        skstd::optional<StrokeParams> fStrokeParams; // Not present implies fill

        Draw(const Renderer& renderer, const Transform& transform, const Shape& shape,
             const Clip& clip, DrawOrder order, const PaintParams* paint,
             const StrokeParams* stroke)
                : fRenderer(renderer)
                , fTransform(transform)
                , fShape(shape)
                , fClip(clip)
                , fOrder(order)
                , fPaintParams(paint ? skstd::optional<PaintParams>(*paint) : skstd::nullopt)
                , fStrokeParams(stroke ? skstd::optional<StrokeParams>(*stroke) : skstd::nullopt) {}

        size_t requiredVertexSpace(int renderStep) const;
        size_t requiredIndexSpace(int renderStep) const;

        void writeVertices(VertexWriter, IndexWriter, int renderStep) const;
    };

    // The returned Transform reference remains valid for the lifetime of the DrawList.
    const Transform& deduplicateTransform(const Transform&);

    SkTBlockList<Transform, 16> fTransforms;
    SkTBlockList<Draw, 16>      fDraws;

    // Running total of RenderSteps for all draws, assuming nothing is culled
    int fRenderStepCount;
};

} // namespace skgpu

#endif // skgpu_DrawList_DEFINED
