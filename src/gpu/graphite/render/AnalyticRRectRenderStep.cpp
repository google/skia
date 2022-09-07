/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/render/AnalyticRRectRenderStep.h"

#include "src/core/SkRRectPriv.h"
#include "src/gpu/graphite/DrawParams.h"
#include "src/gpu/graphite/DrawWriter.h"
#include "src/gpu/graphite/render/CommonDepthStencilSettings.h"

// This RenderStep is flexible and can draw filled rectangles, filled quadrilaterals with per-edge
// AA, filled rounded rectangles with arbitrary corner radii, stroked rectangles with any join,
// and stroked rounded rectangles with circular corners (each corner can be different or square).
// We combine all of these together to maximize batching across simple geometric draws and reduce
// the number pipeline specializations. Additionally, these primitives are the most common
// operations and help us avoid triggering MSAA.
//
// Each of these "primitives" is represented by a single instance. The instance attributes are
// flexible enough to describe any of the above shapes without relying on uniforms to define its
// operation. The attributes encode shape as follows:

// float4 xRadiiOrFlags - if any components is > 0, the instance represents a filled round rect
//    with elliptical corners and these values specify the X radii in top-left CW order.
//    Otherwise, if .x < -1, the instance represents a stroked [round] rect and .y holds the
//    stroke radius and .z holds the join limit (matching StrokeStyle's conventions).
//    Else it's a filled quadrilateral with per-edge AA defined by each component: aa != 0.
// float4 radiiOrQuadXs - if in filled round rect mode, these values provide the Y radii in
//    top-left CW order. If in stroked [round] rect mode, these values provide the circular
//    corner radii (same order). Otherwise, when in per-edge quad mode, these values provide
//    the X coordinates of the quadrilateral (same order).
// float4 ltrbOrQuadYs - if in filled round rect mode or stroked [round] rect mode, these values
//    define the LTRB edge coordinates of the rectangle surrounding the round rect (or the
//    rect itself when the radii are 0s). Otherwise, in per-edge quad mode, these values provide
//    the Y coordinates of the quadrilateral.
//
// From the other direction, shapes produce instance values like:
//  - filled rect:   [-1 -1 -1 -1]              [L R R L]             [T T B B]
//  - stroked rect:  [-2 stroke join 0]         [0 0 0 0]             [L T R B]
//  - filled rrect:  [xRadii(tl,tr,br,bl)]      [yRadii(tl,tr,br,bl)] [L T R B]
//  - stroked rrect: [-2 stroke join 0]         [radii(tl,tr,br,bl)]  [L T R B]
//  - per-edge quad: [aa(tl,tr,br,bl) ? -1 : 0] [xs(tl,tr,br,bl)]     [ys(tl,tr,br,bl)]
//
// This encoding relies on the fact that a valid SkRRect with all x radii equal to 0 must have
// y radii equal to 0 (so it's a rectangle and we can treat it as a quadrilateral with
// all edges AA'ed). This avoids other encodings' inability to represent a quad with all edges
// anti-aliased (e.g. checking for negatives in xRadiiOrFlags to turn on per-edge mode).
//
// From this encoding, data can be unpacked for each corner, which are equivalent under
// rotational symmetry. A corner can have an outer curve, be mitered, or be beveled. It can
// have an inner curve, an inner miter, or fill the interior. Per-edge quads are always mitered
// and fill the interior, but the vertices are placed such that the edge coverage ramps can
// collapse to 0 area on non-AA edges.
//
// The vertices that describe each corner are placed so that edges, miters, and bevels calculate
// coverage by interpolating a varying and then clamping in the fragment shader. Triangles that
// cover the inner and outer curves calculate distance to the curve within the fragment shader.
//
// See https://docs.google.com/presentation/d/1MCPstNsSlDBhR8CrsJo0r-cZNbu-sEJEvU9W94GOJoY/edit?usp=sharing
// for diagrams and explanation of how the geometry is defined.

namespace skgpu::graphite {

static skvx::float4 loadXRadii(const SkRRect& rrect) {
    return skvx::float4{rrect.radii(SkRRect::kUpperLeft_Corner).fX,
                        rrect.radii(SkRRect::kUpperRight_Corner).fX,
                        rrect.radii(SkRRect::kLowerRight_Corner).fX,
                        rrect.radii(SkRRect::kLowerLeft_Corner).fX};
}
static skvx::float4 loadYRadii(const SkRRect& rrect) {
    return skvx::float4{rrect.radii(SkRRect::kUpperLeft_Corner).fY,
                        rrect.radii(SkRRect::kUpperRight_Corner).fY,
                        rrect.radii(SkRRect::kLowerRight_Corner).fY,
                        rrect.radii(SkRRect::kLowerLeft_Corner).fY};
}

AnalyticRRectRenderStep::AnalyticRRectRenderStep()
        : RenderStep("AnalyticRRectRenderStep",
                     "",
                     Flags::kPerformsShading,
                     /*uniforms=*/{},
                     PrimitiveType::kTriangles,
                     kDirectDepthGreaterPass,
                     /*vertexAttrs=*/{},
                     /*instanceAttrs=*/
                            {{"xRadiiOrFlags", VertexAttribType::kFloat4, SkSLType::kFloat4},
                             {"radiiOrQuadXs", VertexAttribType::kFloat4, SkSLType::kFloat4},
                             {"ltrbOrQuadYs", VertexAttribType::kFloat4, SkSLType::kFloat4},
                             // TODO: pack depth and ssboIndex into 32-bits
                             {"depth", VertexAttribType::kFloat, SkSLType::kFloat},
                             {"ssboIndex", VertexAttribType::kInt, SkSLType::kInt},
                             {"maxScale", VertexAttribType::kFloat, SkSLType::kFloat},
                             {"mat0", VertexAttribType::kFloat3, SkSLType::kFloat3},
                             {"mat0", VertexAttribType::kFloat3, SkSLType::kFloat3},
                             {"mat0", VertexAttribType::kFloat3, SkSLType::kFloat3}}) {}

AnalyticRRectRenderStep::~AnalyticRRectRenderStep() {}

const char* AnalyticRRectRenderStep::vertexSkSL() const {
    return R"(
        float3x3 matrix = float3x3(mat0, mat1, mat2);

        if (xRadiiOrFlags.x < -1.0) {
            // Stroked rect or round rect
        } else if (any(xRadiiOrFlags > 0.0)) {
            // Filled round rect
        } else {
            // Per-edge quadrilateral
        }

        // TODO: Implement this
    )";
}

void AnalyticRRectRenderStep::writeVertices(DrawWriter* writer,
                                           const DrawParams& params,
                                           int ssboIndex) const {
    SkASSERT(params.geometry().isShape());
    const Shape& shape = params.geometry().shape();

    // TODO: Fill out fixed vertex and index buffer and vertex count per instance.
    DrawWriter::Instances instance{*writer, {}, {}, 0};
    auto vw = instance.append(1);

    if (params.isStroke()) {
        SkASSERT(params.strokeStyle().halfWidth() >= 0.f);
        SkASSERT(shape.isRect() ||
                 (shape.isRRect() && SkRRectPriv::AllCornersCircular(shape.rrect())));

        // Write a negative value outside [-1, 0] to signal a stroked shape, and write style params
        vw << -2.f << params.strokeStyle().halfWidth() << params.strokeStyle().joinLimit() << 0.f;
        if (shape.isRRect()) {
            // X and Y radii are the same, but each corner could be different. Take X arbitrarily.
            const SkRRect& rrect = shape.rrect();
            vw << loadXRadii(rrect) << rrect.getBounds();
        } else {
            // All four corner radii are 0s, then write the LTRB of the rect directly
            vw << skvx::float4(0.f) << shape.rect().ltrb();
        }
    } else {
        // TODO: Add quadrilateral support to Shape with per-edge flags.
        if (shape.isRect() || (shape.isRRect() && shape.rrect().isRect())) {
            // Rectangles (or rectangles embedded in an SkRRect) are converted to the quadrilateral
            // case, but with all edges anti-aliased (== -1).
            skvx::float4 ltrb = shape.bounds().ltrb();
            vw << /*edge flags*/ skvx::float4(-1.f)
               << /*xs*/ skvx::shuffle<0,2,2,0>(ltrb)
               << /*ys*/ skvx::shuffle<1,1,3,3>(ltrb);
        } else {
            // A filled rounded rectangle
            const SkRRect& rrect = shape.rrect();
            SkASSERT(any(loadXRadii(rrect) > 0.f)); // If not the shader won't detect this case
            vw << loadXRadii(rrect) << loadYRadii(rrect) << shape.rrect().rect();
        }
    }

    // All instance types share the remaining instance attribute definitions
    const SkM44& m = params.transform().matrix();
    vw << params.order().depthAsFloat()
       << ssboIndex
       << params.transform().maxScaleFactor()
       << m.rc(0,0) << m.rc(1,0) << m.rc(3,0)
       << m.rc(0,1) << m.rc(1,1) << m.rc(3,1)
       << m.rc(0,3) << m.rc(1,3) << m.rc(3,3);
}

void AnalyticRRectRenderStep::writeUniformsAndTextures(const DrawParams&,
                                                       SkPipelineDataGatherer*) const {
    // All data is uploaded as instance attributes, so no uniforms are needed.
}

}  // namespace skgpu::graphite
