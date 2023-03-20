/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_DrawList_DEFINED
#define skgpu_graphite_DrawList_DEFINED

#include "include/core/SkPaint.h"
#include "src/base/SkTBlockList.h"

#include "src/gpu/graphite/DrawOrder.h"
#include "src/gpu/graphite/DrawParams.h"
#include "src/gpu/graphite/PaintParams.h"
#include "src/gpu/graphite/geom/Geometry.h"
#include "src/gpu/graphite/geom/Transform_graphite.h"

#include <limits>
#include <optional>

namespace skgpu::graphite {

class Renderer;

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

    // DrawList requires that all Transforms be valid and asserts as much; invalid transforms should
    // be detected at the Device level or similar. The provided Renderer must be compatible with the
    // 'shape' and 'stroke' parameters. If the renderer uses coverage AA, 'ordering' must have a
    // compressed painters order that reflects that. If the renderer uses stencil, the 'ordering'
    // must have a valid stencil index as well.
    void recordDraw(const Renderer* renderer,
                    const Transform& localToDevice,
                    const Geometry& geometry,
                    const Clip& clip,
                    DrawOrder ordering,
                    const PaintParams* paint,
                    const StrokeStyle* stroke);

    int drawCount() const { return fDraws.count(); }
    int renderStepCount() const { return fRenderStepCount; }

private:
    friend class DrawPass;

    struct Draw {
        const Renderer* fRenderer; // Owned by SharedContext of Recorder that recorded the draw
        DrawParams fDrawParams; // The DrawParam's transform is owned by fTransforms of the DrawList
        std::optional<PaintParams> fPaintParams; // Not present implies depth-only draw

        Draw(const Renderer* renderer, const Transform& transform, const Geometry& geometry,
             const Clip& clip, DrawOrder order, const PaintParams* paint,
             const StrokeStyle* stroke)
                : fRenderer(renderer)
                , fDrawParams(transform, geometry, clip, order, stroke)
                , fPaintParams(paint ? std::optional<PaintParams>(*paint) : std::nullopt) {}
    };

    // The returned Transform reference remains valid for the lifetime of the DrawList.
    const Transform& deduplicateTransform(const Transform&);

    SkTBlockList<Transform, 16> fTransforms;
    SkTBlockList<Draw, 16>      fDraws;

    // Running total of RenderSteps for all draws, assuming nothing is culled
    int fRenderStepCount;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_DrawList_DEFINED
