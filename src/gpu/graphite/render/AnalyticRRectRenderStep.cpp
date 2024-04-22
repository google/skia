/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/render/AnalyticRRectRenderStep.h"

#include "src/base/SkVx.h"
#include "src/core/SkRRectPriv.h"
#include "src/gpu/graphite/DrawParams.h"
#include "src/gpu/graphite/DrawWriter.h"
#include "src/gpu/graphite/render/CommonDepthStencilSettings.h"

// This RenderStep is flexible and can draw filled rectangles, filled quadrilaterals with per-edge
// AA, filled rounded rectangles with arbitrary corner radii, stroked rectangles with any join,
// stroked lines with any cap, stroked rounded rectangles with circular corners (each corner can be
// different or square), hairline rectangles, hairline lines, and hairline rounded rectangles with
// arbitrary corners.
//
// We combine all of these together to maximize batching across simple geometric draws and reduce
// the number pipeline specializations. Additionally, these primitives are the most common
// operations and help us avoid triggering MSAA.
//
// Each of these "primitives" is represented by a single instance. The instance attributes are
// flexible enough to describe any of the above shapes without relying on uniforms to define its
// operation. The attributes encode shape as follows:
//
// float4 xRadiiOrFlags - if any components is > 0, the instance represents a filled round rect
//    with elliptical corners and these values specify the X radii in top-left CW order.
//    Otherwise, if .x < -1, the instance represents a stroked or hairline [round] rect or line,
//    where .y differentiates hairline vs. stroke. If .y is negative, then it is a hairline [round]
//    rect and xRadiiOrFlags stores (-2 - X radii); if .y is zero, it is a regular stroked [round]
//    rect; if .y is positive, then it is a stroked *or* hairline line. For .y >= 0, .z holds the
//    stroke radius and .w stores the join limit (matching StrokeStyle's conventions).
//    Lastly, if -1 <= .x <= 0, it's a filled quadrilateral with per-edge AA defined by each by the
//    component: aa != 0.
// float4 radiiOrQuadXs - if in filled round rect or hairline [round] rect mode, these values
//    provide the Y radii in top-left CW order. If in stroked [round] rect mode, these values
//    provide the circular corner radii (same order). Otherwise, when in per-edge quad mode, these
//    values provide the X coordinates of the quadrilateral (same order).
// float4 ltrbOrQuadYs - if in filled round rect mode or stroked [round] rect mode, these values
//    define the LTRB edge coordinates of the rectangle surrounding the round rect (or the
//    rect itself when the radii are 0s). In stroked line mode, LTRB is treated as (x0,y0) and
//    (x1,y1) that defines the line. Otherwise, in per-edge quad mode, these values provide
//    the Y coordinates of the quadrilateral.
//
// From the other direction, shapes produce instance values like:
//  - filled rect:    [-1 -1 -1 -1]            [L R R L]             [T T B B]
//  - stroked rect:   [-2 0 stroke join]       [0 0 0 0]             [L T R B]
//  - hairline rect:  [-2 -2 -2 -2]            [0 0 0 0]             [L T R B]
//  - filled rrect:   [xRadii(tl,tr,br,bl)]    [yRadii(tl,tr,br,bl)] [L T R B]
//  - stroked rrect:  [-2 0 stroke join]       [radii(tl,tr,br,bl)]  [L T R B]
//  - hairline rrect: [-2-xRadii(tl,tr,br,bl)] [radii(tl,tr,br,bl)]  [L T R B]
//  - filled line:    N/A, discarded higher in the stack
//  - stroked line:   [-2 1 stroke cap]        [0 0 0 0]             [x0,y0,x1,y1]
//  - hairline line:  [-2 1 0 1]               [0 0 0 0]             [x0,y0,x1,y1]
//  - per-edge quad:  [aa(t,r,b,l) ? -1 : 0]   [xs(tl,tr,br,bl)]     [ys(tl,tr,br,bl)]
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
//
// AnalyticRRectRenderStep uses the common technique of approximating distance to the level set by
// one expansion of the Taylor's series for the level set's equation. Given a level set function
// C(x,y), this amounts to calculating C(px,py)/|∇C(px,py)|. For the straight edges the level set
// is linear and calculated in the vertex shader and then interpolated exactly over the rectangle.
// This provides distances to all four exterior edges within the fragment shader and allows it to
// reconstruct a relative position per elliptical corner. Unfortunately this requires the fragment
// shader to calculate the length of the gradient for straight edges instead of interpolating
// exact device-space distance.
//
// All four corner radii are potentially evaluated by the fragment shader although each corner's
// coverage is only calculated when the pixel is within the bounding box of its quadrant. For fills
// and simple strokes it's theoretically valid to have each pixel calculate a single corner's
// coverage that was controlled via the vertex shader. However, testing all four corners is
// necessary in order to correctly handle self-intersecting stroke interiors. Similarly, all four
// edges must be evaluated in order to handle extremely thin shapes; whereas often you could get
// away with tracking a single edge distance per pixel.
//
// Analytic derivatives are used so that a single pipeline can be used regardless of HW derivative
// support or for geometry that would prove difficult for forward differencing. The device-space
// gradient for ellipses is calculated per-pixel by transforming a per-pixel local gradient vector
// with the Jacobian of the inverse local-to-device transform:
//
// (px,py) is the projected point of (u,v) transformed by a 3x3 matrix, M:
//                [x(u,v) / w(u,v)]       [x]   [m00 m01 m02] [u]
//      (px,py) = [y(u,v) / w(u,v)] where [y] = [m10 m11 m12]X[v] = M*(u,v,1)
//                                        [w]   [m20 m21 m22] [1]
//
// C(px,py) can be defined in terms of a local Cl(u,v) as C(px,py) = Cl(p^-1(px,py)), where p^-1 =
//
//               [x'(px,py) / w'(px,py)]       [x']   [m00' m01' * m02'] [px]
//      (u,v) =  [y'(px,py) / w'(px,py)] where [y'] = [m10' m11' * m12']X[py] = M^-1*(px,py,0,1)
//                                             [w']   [m20' m21' * m22'] [ 1]
//
// Note that if the 3x3 M was arrived by dropping the 3rd row and column from a 4x4 since we assume
// a local 3rd coordinate of 0, M^-1 is not equal to the 4x4 inverse with dropped rows and columns.
//
// Using the chain rule, then ∇C(px,py)
//   =  ∇Cl(u,v)X[1/w'(px,py)     0       -x'(px,py)/w'(px,py)^2] [m00' m01']
//               [    0       1/w'(px,py) -y'(px,py)/w'(px,py)^2]X[m10' m11']
//                                                                [m20' m21']
//
//   = 1/w'(px,py)*∇Cl(u,v)X[1 0 -x'(px,py)/w'(px,py)] [m00' m01']
//                          [0 1 -y'(px,py)/w'(px,py)]X[m10' m11']
//                                                     [m20' m21']
//
//   = w(u,v)*∇Cl(u,v)X[1 0 0 -u] [m00' m01']
//                     [0 1 0 -v]X[m10' m11']
//                                [m20' m21']
//
//   = w(u,v)*∇Cl(u,v)X[m00'-m20'u m01'-m21'u]
//                     [m10'-m20'v m11'-m21'v]
//
// The vertex shader calculates the rightmost 2x2 matrix and interpolates it across the shape since
// each component is linear in (u,v). ∇Cl(u,v) is evaluated per pixel in the fragment shader and
// depends on which corner and edge being evaluated. w(u,v) is the device-space W coordinate, so
// its reciprocal is provided in sk_FragCoord.w.
namespace skgpu::graphite {

using AAFlags = EdgeAAQuad::Flags;

static skvx::float4 load_x_radii(const SkRRect& rrect) {
    return skvx::float4{rrect.radii(SkRRect::kUpperLeft_Corner).fX,
                        rrect.radii(SkRRect::kUpperRight_Corner).fX,
                        rrect.radii(SkRRect::kLowerRight_Corner).fX,
                        rrect.radii(SkRRect::kLowerLeft_Corner).fX};
}
static skvx::float4 load_y_radii(const SkRRect& rrect) {
    return skvx::float4{rrect.radii(SkRRect::kUpperLeft_Corner).fY,
                        rrect.radii(SkRRect::kUpperRight_Corner).fY,
                        rrect.radii(SkRRect::kLowerRight_Corner).fY,
                        rrect.radii(SkRRect::kLowerLeft_Corner).fY};
}

static bool opposite_insets_intersect(const SkRRect& rrect, float strokeRadius, float aaRadius) {
    // One AA inset per side
    const float maxInset = strokeRadius + 2.f * aaRadius;
    return // Horizontal insets would intersect opposite corner's curve
           maxInset >= rrect.width() - rrect.radii(SkRRect::kLowerLeft_Corner).fX   ||
           maxInset >= rrect.width() - rrect.radii(SkRRect::kLowerRight_Corner).fX  ||
           maxInset >= rrect.width() - rrect.radii(SkRRect::kUpperLeft_Corner).fX   ||
           maxInset >= rrect.width() - rrect.radii(SkRRect::kUpperRight_Corner).fX  ||
           // Vertical insets would intersect opposite corner's curve
           maxInset >= rrect.height() - rrect.radii(SkRRect::kLowerLeft_Corner).fY  ||
           maxInset >= rrect.height() - rrect.radii(SkRRect::kLowerRight_Corner).fY ||
           maxInset >= rrect.height() - rrect.radii(SkRRect::kUpperLeft_Corner).fY  ||
           maxInset >= rrect.height() - rrect.radii(SkRRect::kUpperRight_Corner).fY;
}

static bool opposite_insets_intersect(const Rect& rect, float strokeRadius, float aaRadius) {
    return any(rect.size() <= 2.f * (strokeRadius + aaRadius));
}

static bool opposite_insets_intersect(const Geometry& geometry,
                                      float strokeRadius,
                                      float aaRadius) {
    if (geometry.isEdgeAAQuad()) {
        SkASSERT(strokeRadius == 0.f);
        const EdgeAAQuad& quad = geometry.edgeAAQuad();
        if (quad.edgeFlags() == AAFlags::kNone) {
            // If all edges are non-AA, there won't be any insetting. This allows completely non-AA
            // quads to use the fill triangles for simpler fragment shader work.
            return false;
        } else if (quad.isRect() && quad.edgeFlags() == AAFlags::kAll) {
            return opposite_insets_intersect(quad.bounds(), 0.f, aaRadius);
        } else {
            // Quads with mixed AA edges are tiles where non-AA edges must seam perfectly together.
            // If we were to inset along just the axis with AA at a corner, two adjacent quads could
            // arrive at slightly different inset coordinates and then we wouldn't have a perfect
            // mesh. Forcing insets to snap to the center means all non-AA edges are formed solely
            // by the original quad coordinates and should seam perfectly assuming perfect input.
            // The only downside to this is the fill triangles cannot be used since they would
            // partially extend into the coverage ramp from adjacent AA edges.
            return true;
        }
    } else {
        const Shape& shape = geometry.shape();
        if (shape.isLine()) {
            return strokeRadius <= aaRadius;
        } else if (shape.isRect()) {
            return opposite_insets_intersect(shape.rect(), strokeRadius, aaRadius);
        } else {
            SkASSERT(shape.isRRect());
            return opposite_insets_intersect(shape.rrect(), strokeRadius, aaRadius);
        }
    }
}

static bool is_clockwise(const EdgeAAQuad& quad) {
    if (quad.isRect()) {
        return true; // by construction, these are always locally clockwise
    }

    // This assumes that each corner has a consistent winding, which is the case for convex inputs,
    // which is an assumption of the per-edge AA API. Check the sign of cross product between the
    // first two edges.
    const skvx::float4& xs = quad.xs();
    const skvx::float4& ys = quad.ys();

    float winding = (xs[0] - xs[3])*(ys[1] - ys[0]) - (ys[0] - ys[3])*(xs[1] - xs[0]);
    if (winding == 0.f) {
        // The input possibly forms a triangle with duplicate vertices, so check the opposite corner
        winding = (xs[2] - xs[1])*(ys[3] - ys[2]) - (ys[2] - ys[1])*(xs[3] - xs[2]);
    }

    // At this point if winding is < 0, the quad's vertices are CCW. If it's still 0, the vertices
    // form a line, in which case the vertex shader constructs a correct CW winding. Otherwise,
    // the quad or triangle vertices produce a positive winding and are CW.
    return winding >= 0.f;
}

static skvx::float2 quad_center(const EdgeAAQuad& quad) {
    // The center of the bounding box is *not* a good center to use. Take the average of the
    // four points instead (which is slightly biased if they form a triangle, but still okay).
    return skvx::float2(dot(quad.xs(), skvx::float4(0.25f)),
                        dot(quad.ys(), skvx::float4(0.25f)));
}

// Represents the per-vertex attributes used in each instance.
struct Vertex {
    SkV2 fPosition;
    SkV2 fNormal;
    float fNormalScale;
    float fCenterWeight;
};

// Allowed values for the center weight instance value (selected at record time based on style
// and transform), and are defined such that when (insance-weight > vertex-weight) is true, the
// vertex should be snapped to the center instead of its regular calculation.
static constexpr float kSolidInterior = 1.f;
static constexpr float kStrokeInterior = 0.f;
static constexpr float kFilledStrokeInterior = -1.f;

// Special value for local AA radius to signal when the self-intersections of a stroke interior
// need extra calculations in the vertex shader.
static constexpr float kComplexAAInsets = -1.f;

static constexpr int kCornerVertexCount = 9; // sk_VertexID is divided by this in SkSL
static constexpr int kVertexCount = 4 * kCornerVertexCount;
static constexpr int kIndexCount = 69;

static void write_index_buffer(VertexWriter writer) {
    static constexpr uint16_t kTL = 0 * kCornerVertexCount;
    static constexpr uint16_t kTR = 1 * kCornerVertexCount;
    static constexpr uint16_t kBR = 2 * kCornerVertexCount;
    static constexpr uint16_t kBL = 3 * kCornerVertexCount;

    static const uint16_t kIndices[kIndexCount] = {
        // Exterior AA ramp outset
        kTL+0,kTL+4,kTL+1,kTL+5,kTL+2,kTL+3,kTL+5,
        kTR+0,kTR+4,kTR+1,kTR+5,kTR+2,kTR+3,kTR+5,
        kBR+0,kBR+4,kBR+1,kBR+5,kBR+2,kBR+3,kBR+5,
        kBL+0,kBL+4,kBL+1,kBL+5,kBL+2,kBL+3,kBL+5,
        kTL+0,kTL+4, // close and jump to next strip
        // Outer to inner edges
        kTL+4,kTL+6,kTL+5,kTL+7,
        kTR+4,kTR+6,kTR+5,kTR+7,
        kBR+4,kBR+6,kBR+5,kBR+7,
        kBL+4,kBL+6,kBL+5,kBL+7,
        kTL+4,kTL+6, // close and jump to next strip
        // Fill triangles
        kTL+6,kTL+8,kTL+7, kTL+7,kTR+8,
        kTR+6,kTR+8,kTR+7, kTR+7,kBR+8,
        kBR+6,kBR+8,kBR+7, kBR+7,kBL+8,
        kBL+6,kBL+8,kBL+7, kBL+7,kTL+8,
        kTL+6 // close
    };

    if (writer) {
        writer << kIndices;
    } // otherwise static buffer creation failed, so do nothing; Context initialization will fail.
}

static void write_vertex_buffer(VertexWriter writer) {
    // Allowed values for the normal scale attribute. +1 signals a device-space outset along the
    // normal away from the outer edge of the stroke. 0 signals no outset, but placed on the outer
    // edge of the stroke. -1 signals a local inset along the normal from the inner edge.
    static constexpr float kOutset = 1.0;
    static constexpr float kInset  = -1.0;

    static constexpr float kCenter = 1.f; // "true" as a float

    // Zero, but named this way to help call out non-zero parameters.
    static constexpr float _______ = 0.f;

    static constexpr float kHR2 = 0.5f * SK_FloatSqrt2; // "half root 2"

    // This template is repeated 4 times in the vertex buffer, for each of the four corners.
    // The vertex ID is used to lookup per-corner instance properties such as corner radii or
    // positions, but otherwise this vertex data produces a consistent clockwise mesh from
    // TL -> TR -> BR -> BL.
    static constexpr Vertex kCornerTemplate[kCornerVertexCount] = {
        // Device-space AA outsets from outer curve
        { {1.0f, 0.0f}, {1.0f, 0.0f}, kOutset, _______ },
        { {1.0f, 0.0f}, {kHR2, kHR2}, kOutset, _______ },
        { {0.0f, 1.0f}, {kHR2, kHR2}, kOutset, _______ },
        { {0.0f, 1.0f}, {0.0f, 1.0f}, kOutset, _______ },

        // Outer anchors (no local or device-space normal outset)
        { {1.0f, 0.0f}, {kHR2, kHR2}, _______, _______ },
        { {0.0f, 1.0f}, {kHR2, kHR2}, _______, _______ },

        // Inner curve (with additional AA inset in the common case)
        { {1.0f, 0.0f}, {1.0f, 0.0f}, kInset, _______ },
        { {0.0f, 1.0f}, {0.0f, 1.0f}, kInset, _______ },

        // Center filling vertices (equal to inner AA insets unless 'center' triggers a fill).
        // TODO: On backends that support "cull" distances (and with SkSL support), these vertices
        // and their corresponding triangles can be completely removed. The inset vertices can
        // set their cull distance value to cause all filling triangles to be discarded or not
        // depending on the instance's style.
        { {1.0f, 0.0f}, {1.0f, 0.0f}, kInset,  kCenter },
    };

    if (writer) {
        writer << kCornerTemplate  // TL
               << kCornerTemplate  // TR
               << kCornerTemplate  // BR
               << kCornerTemplate; // BL
    } // otherwise static buffer creation failed, so do nothing; Context initialization will fail.
}

AnalyticRRectRenderStep::AnalyticRRectRenderStep(StaticBufferManager* bufferManager)
        : RenderStep("AnalyticRRectRenderStep",
                     "",
                     Flags::kPerformsShading | Flags::kEmitsCoverage | Flags::kOutsetBoundsForAA,
                     /*uniforms=*/{},
                     PrimitiveType::kTriangleStrip,
                     kDirectDepthGreaterPass,
                     /*vertexAttrs=*/{
                            {"position", VertexAttribType::kFloat2, SkSLType::kFloat2},
                            {"normal", VertexAttribType::kFloat2, SkSLType::kFloat2},
                            // TODO: These values are all +1/0/-1, or +1/0, so could be packed
                            // much more densely than as three floats.
                            {"normalScale", VertexAttribType::kFloat, SkSLType::kFloat},
                            {"centerWeight", VertexAttribType::kFloat, SkSLType::kFloat}
                     },
                     /*instanceAttrs=*/
                            {{"xRadiiOrFlags", VertexAttribType::kFloat4, SkSLType::kFloat4},
                             {"radiiOrQuadXs", VertexAttribType::kFloat4, SkSLType::kFloat4},
                             {"ltrbOrQuadYs", VertexAttribType::kFloat4, SkSLType::kFloat4},
                             // XY stores center of rrect in local coords. Z and W store values to
                             // control interior fill behavior. Z can be -1, 0, or 1:
                             //   -1: A stroked interior where AA insets overlap, but isn't solid.
                             //    0: A stroked interior with no complications.
                             //    1: A solid interior (fill or sufficiently large stroke width).
                             // W specifies the size of the AA inset if it's >= 0, or signals that
                             // the inner curves intersect in a complex manner (rare).
                             {"center", VertexAttribType::kFloat4, SkSLType::kFloat4},

                             // TODO: pack depth and ssbo index into one 32-bit attribute, if we can
                             // go without needing both render step and paint ssbo index attributes.
                             {"depth", VertexAttribType::kFloat, SkSLType::kFloat},
                             {"ssboIndices", VertexAttribType::kUShort2, SkSLType::kUShort2},

                             {"mat0", VertexAttribType::kFloat3, SkSLType::kFloat3},
                             {"mat1", VertexAttribType::kFloat3, SkSLType::kFloat3},
                             {"mat2", VertexAttribType::kFloat3, SkSLType::kFloat3}},
                     /*varyings=*/{
                             // TODO: If the inverse transform is part of the draw's SSBO, we can
                             // reconstruct the Jacobian in the fragment shader using the existing
                             // local coordinates varying
                             {"jacobian", SkSLType::kFloat4}, // float2x2
                             // Distance to LTRB edges of unstroked shape. Depending on
                             // 'perPixelControl' these will either be local or device-space values.
                             {"edgeDistances", SkSLType::kFloat4}, // distance to LTRB edges
                             // TODO: These are constant for all fragments for a given instance,
                             // could we store them in the draw's SSBO?
                             {"xRadii", SkSLType::kFloat4},
                             {"yRadii", SkSLType::kFloat4},
                             // Matches the StrokeStyle struct (X is radius, Y < 0 is round join,
                             // Y = 0 is bevel, Y > 0 is miter join).
                             // TODO: These could easily be considered part of the draw's uniforms.
                             {"strokeParams", SkSLType::kFloat2},
                             // 'perPixelControl' is a tightly packed description of how to
                             // evaluate the possible edges that influence coverage in a pixel.
                             // The decision points and encoded values are spread across X and Y
                             // so that they are consistent regardless of whether or not MSAA is
                             // used and does not require centroid sampling.
                             //
                             // The signs of values are used to determine the type of coverage to
                             // calculate in the fragment shader and depending on the state, extra
                             // varying state is encoded in the fields:
                             //  - A positive X value overrides all per-pixel coverage calculations
                             //    and sets the pixel to full coverage. Y is ignored in this case.
                             //  - A zero X value represents a solid interior shape.
                             //  - X much less than 0 represents bidirectional coverage for a
                             //    stroke, using a sufficiently negative value to avoid
                             //    extrapolation from fill triangles. For actual shapes with
                             //    bidirectional coverage, the fill triangles are zero area.
                             //
                             //  - Y much greater than 0 takes precedence over the latter two X
                             //    rules and signals that 'edgeDistances' holds device-space values
                             //    and does not require additional per-pixel calculations. The
                             //    coverage scale is encoded as (1+scale*w) and the bias is
                             //    reconstructed from that. X is always 0 for non-fill triangles
                             //    since device-space edge distance is only used for solid interiors
                             //  - Otherwise, any negative Y value represents an additional
                             //    reduction in coverage due to a device-space outset. It is clamped
                             //    below 0 to avoid adding coverage from extrapolation.
                             {"perPixelControl", SkSLType::kFloat2},
                     }) {
    // Initialize the static buffers we'll use when recording draw calls.
    // NOTE: Each instance of this RenderStep gets its own copy of the data. Since there should only
    // ever be one AnalyticRRectRenderStep at a time, this shouldn't be an issue.
    write_vertex_buffer(bufferManager->getVertexWriter(sizeof(Vertex) * kVertexCount,
                                                       &fVertexBuffer));
    write_index_buffer(bufferManager->getIndexWriter(sizeof(uint16_t) * kIndexCount,
                                                     &fIndexBuffer));
}

AnalyticRRectRenderStep::~AnalyticRRectRenderStep() {}

std::string AnalyticRRectRenderStep::vertexSkSL() const {
    // Returns the body of a vertex function, which must define a float4 devPosition variable and
    // must write to an already-defined float2 stepLocalCoords variable.
    return "float4 devPosition = analytic_rrect_vertex_fn("
                   // Vertex Attributes
                   "position, normal, normalScale, centerWeight, "
                   // Instance Attributes
                   "xRadiiOrFlags, radiiOrQuadXs, ltrbOrQuadYs, center, depth, "
                   "float3x3(mat0, mat1, mat2), "
                   // Varyings
                   "jacobian, edgeDistances, xRadii, yRadii, strokeParams, perPixelControl, "
                   // Render Step
                   "stepLocalCoords);\n";
}

const char* AnalyticRRectRenderStep::fragmentCoverageSkSL() const {
    // The returned SkSL must write its coverage into a 'half4 outputCoverage' variable (defined in
    // the calling code) with the actual coverage splatted out into all four channels.
    return "outputCoverage = analytic_rrect_coverage_fn(sk_FragCoord, "
                                                       "jacobian, "
                                                       "edgeDistances, "
                                                       "xRadii, "
                                                       "yRadii, "
                                                       "strokeParams, "
                                                       "perPixelControl);";
}

void AnalyticRRectRenderStep::writeVertices(DrawWriter* writer,
                                            const DrawParams& params,
                                            skvx::ushort2 ssboIndices) const {
    SkASSERT(params.geometry().isShape() || params.geometry().isEdgeAAQuad());

    DrawWriter::Instances instance{*writer, fVertexBuffer, fIndexBuffer, kIndexCount};
    auto vw = instance.append(1);

    // The bounds of a rect is the rect, and the bounds of a rrect is tight (== SkRRect::getRect()).
    Rect bounds = params.geometry().bounds();

    // aaRadius will be set to a negative value to signal a complex self-intersection that has to
    // be calculated in the vertex shader.
    float aaRadius = params.transform().localAARadius(bounds);
    float strokeInset = 0.f;
    float centerWeight = kSolidInterior;

    if (params.isStroke()) {
         // EdgeAAQuads are not stroked so we know it's a Shape, but we support rects, rrects, and
         // lines that all need to be converted to the same form.
        const Shape& shape = params.geometry().shape();

        SkASSERT(params.strokeStyle().halfWidth() >= 0.f);
        SkASSERT(shape.isRect() || shape.isLine() || params.strokeStyle().halfWidth() == 0.f ||
                 (shape.isRRect() && SkRRectPriv::AllCornersCircular(shape.rrect())));

        float strokeRadius = params.strokeStyle().halfWidth();

        skvx::float2 size = shape.isLine() ? skvx::float2(length(shape.p1() - shape.p0()), 0.f)
                                           : bounds.size(); // rect or [r]rect

        skvx::float2 innerGap = size - 2.f * params.strokeStyle().halfWidth();
        if (any(innerGap <= 0.f) && strokeRadius > 0.f) {
            // AA inset intersections are measured from the *outset* and remain marked as "solid"
            strokeInset = -strokeRadius;
        } else {
            // This will be upgraded to kFilledStrokeInterior if insets intersect
            centerWeight = kStrokeInterior;
            strokeInset = strokeRadius;
        }

        skvx::float4 xRadii = shape.isRRect() ? load_x_radii(shape.rrect()) : skvx::float4(0.f);
        if (strokeRadius > 0.f || shape.isLine()) {
            // Regular strokes only need to upload 4 corner radii; hairline lines can be uploaded in
            // the same manner since it has no real corner radii.
            float joinStyle = params.strokeStyle().joinLimit();
            float lineFlag = shape.isLine() ? 1.f : 0.f;
            auto empty = size == 0.f;

            // Points and lines produce caps instead of joins. However, the capped geometry is
            // visually equivalent to a joined, stroked [r]rect of the paired join style.
            if (shape.isLine() || all(empty)) {
                // However, butt-cap points are defined not to produce any geometry, so that combo
                // should have been rejected earlier.
                SkASSERT(shape.isLine() || params.strokeStyle().cap() != SkPaint::kButt_Cap);
                switch(params.strokeStyle().cap()) {
                    case SkPaint::kRound_Cap:  joinStyle = -1.f; break; // round cap == round join
                    case SkPaint::kButt_Cap:   joinStyle =  0.f; break; // butt cap == bevel join
                    case SkPaint::kSquare_Cap: joinStyle =  1.f; break; // square cap == miter join
                }
            } else if (params.strokeStyle().isMiterJoin()) {
                // Normal corners are 90-degrees so become beveled if the miter limit is < sqrt(2).
                // If the [r]rect has a width or height of 0, the corners are actually 180-degrees,
                // so the must always be beveled (or, equivalently, butt-capped).
                if (params.strokeStyle().miterLimit() < SK_ScalarSqrt2 || any(empty)) {
                    joinStyle = 0.f; // == bevel (or butt if width or height are zero)
                } else {
                    // Discard actual miter limit because a 90-degree corner never exceeds it.
                    joinStyle = 1.f;
                }
            } // else no join style correction needed for non-empty geometry or round joins

            // Write a negative value outside [-1, 0] to signal a stroked shape, the line flag, then
            // the style params, followed by corner radii and coords.
            vw << -2.f << lineFlag << strokeRadius << joinStyle << xRadii
               << (shape.isLine() ? shape.line() : bounds.ltrb());
        } else {
            // Write -2 - cornerRadii to encode the X radii in such a way to trigger stroking but
            // guarantee the 2nd field is non-zero to signal hairline. Then we upload Y radii as
            // well to allow for elliptical hairlines.
            skvx::float4 yRadii = shape.isRRect() ? load_y_radii(shape.rrect()) : skvx::float4(0.f);
            vw << (-2.f - xRadii) << yRadii << bounds.ltrb();
        }
    } else {
        // Empty fills should not have been recorded at all.
        SkASSERT(!bounds.isEmptyNegativeOrNaN());

        if (params.geometry().isEdgeAAQuad()) {
            // NOTE: If quad.isRect() && quad.edgeFlags() == kAll, the written data is identical to
            // Shape.isRect() case below.
            const EdgeAAQuad& quad = params.geometry().edgeAAQuad();

            // If all edges are non-AA, set localAARadius to 0 so that the fill triangles cover the
            // entire shape. Otherwise leave it as-is for the full AA rect case; in the event it's
            // mixed-AA or a quad, it'll be converted to complex insets down below.
            if (quad.edgeFlags() == EdgeAAQuad::Flags::kNone) {
                aaRadius = 0.f;
            }

            // -1 for AA on, 0 for AA off
            auto edgeSigns = skvx::float4{quad.edgeFlags() & AAFlags::kLeft   ? -1.f : 0.f,
                                          quad.edgeFlags() & AAFlags::kTop    ? -1.f : 0.f,
                                          quad.edgeFlags() & AAFlags::kRight  ? -1.f : 0.f,
                                          quad.edgeFlags() & AAFlags::kBottom ? -1.f : 0.f};

            // The vertex shader expects points to be in clockwise order. EdgeAAQuad is the only
            // shape that *might* have counter-clockwise input.
            if (is_clockwise(quad)) {
                vw << edgeSigns << quad.xs() << quad.ys();
            } else {
                vw << skvx::shuffle<2,1,0,3>(edgeSigns)  // swap left and right AA bits
                   << skvx::shuffle<1,0,3,2>(quad.xs())  // swap TL with TR, and BL with BR
                   << skvx::shuffle<1,0,3,2>(quad.ys()); //   ""
            }
        } else {
            const Shape& shape = params.geometry().shape();
            // Filled lines are empty by definition, so they shouldn't have been recorded
            SkASSERT(!shape.isLine());

            if (shape.isRect() || (shape.isRRect() && shape.rrect().isRect())) {
                // Rectangles (or rectangles embedded in an SkRRect) are converted to the
                // quadrilateral case, but with all edges anti-aliased (== -1).
                skvx::float4 ltrb = bounds.ltrb();
                vw << /*edge flags*/ skvx::float4(-1.f)
                   << /*xs*/ skvx::shuffle<0,2,2,0>(ltrb)
                   << /*ys*/ skvx::shuffle<1,1,3,3>(ltrb);
            } else {
                // A filled rounded rectangle, so make sure at least one corner radii > 0 or the
                // shader won't detect it as a rounded rect.
                SkASSERT(any(load_x_radii(shape.rrect()) > 0.f));

                vw << load_x_radii(shape.rrect()) << load_y_radii(shape.rrect()) << bounds.ltrb();
            }
        }
    }

    if (opposite_insets_intersect(params.geometry(), strokeInset, aaRadius)) {
        aaRadius = kComplexAAInsets;
        if (centerWeight == kStrokeInterior) {
            centerWeight = kFilledStrokeInterior;
        }
    }

    // All instance types share the remaining instance attribute definitions
    const SkM44& m = params.transform().matrix();
    auto center = params.geometry().isEdgeAAQuad() ? quad_center(params.geometry().edgeAAQuad())
                                                   : bounds.center();
    vw << center << centerWeight << aaRadius
       << params.order().depthAsFloat()
       << ssboIndices
       << m.rc(0,0) << m.rc(1,0) << m.rc(3,0)  // mat0
       << m.rc(0,1) << m.rc(1,1) << m.rc(3,1)  // mat1
       << m.rc(0,3) << m.rc(1,3) << m.rc(3,3); // mat2
}

void AnalyticRRectRenderStep::writeUniformsAndTextures(const DrawParams&,
                                                       PipelineDataGatherer*) const {
    // All data is uploaded as instance attributes, so no uniforms are needed.
}

}  // namespace skgpu::graphite
