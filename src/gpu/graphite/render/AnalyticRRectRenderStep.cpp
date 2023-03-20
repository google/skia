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
// and stroked rounded rectangles with circular corners (each corner can be different or square).
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
//    Otherwise, if .x < -1, the instance represents a stroked or hairline [round] rect, where .y
//    differentiates hairline vs. stroke. If .y is negative, then it is a hairline and xRadiiOrFlags
//    stores (-2 - X radii); otherwise it is a regular stroke and .z holds the stroke radius and
//    .w stores the join limit (matching StrokeStyle's conventions).
//    Else it's a filled quadrilateral with per-edge AA defined by each component: aa != 0.
// float4 radiiOrQuadXs - if in filled round rect or hairline [round] rect mode, these values
//    provide the Y radii in top-left CW order. If in stroked [round] rect mode, these values
//    provide the circular corner radii (same order). Otherwise, when in per-edge quad mode, these
//    values provide the X coordinates of the quadrilateral (same order).
// float4 ltrbOrQuadYs - if in filled round rect mode or stroked [round] rect mode, these values
//    define the LTRB edge coordinates of the rectangle surrounding the round rect (or the
//    rect itself when the radii are 0s). Otherwise, in per-edge quad mode, these values provide
//    the Y coordinates of the quadrilateral.
//
// From the other direction, shapes produce instance values like:
//  - filled rect:    [-1 -1 -1 -1]            [L R R L]             [T T B B]
//  - stroked rect:   [-2 0 stroke join]       [0 0 0 0]             [L T R B]
//  - hairline rect:  [-2 -2 -2 -2]            [0 0 0 0]             [L T R B]
//  - filled rrect:   [xRadii(tl,tr,br,bl)]    [yRadii(tl,tr,br,bl)] [L T R B]
//  - stroked rrect:  [-2 0 stroke join]       [radii(tl,tr,br,bl)]  [L T R B]
//  - hairline rrect: [-2-xRadii(tl,tr,br,bl)] [radii(tl,tr,br,bl)]  [L T R B]
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

static float local_aa_radius(const Transform& t, const SkV2& p) {
    // TODO: This should be the logic for Transform::scaleFactor()
    //              [m00 m01 * m03]                                 [f(u,v)]
    // Assuming M = [m10 m11 * m13], define the projected p'(u,v) = [g(u,v)] where
    //              [ *   *  *  * ]
    //              [m30 m31 * m33]
    //                                                        [x]     [u]
    // f(u,v) = x(u,v) / w(u,v), g(u,v) = y(u,v) / w(u,v) and [y] = M*[v]
    //                                                        [*] =   [0]
    //                                                        [w]     [1]
    //
    // x(u,v) = m00*u + m01*v + m03
    // y(u,v) = m10*u + m11*v + m13
    // w(u,v) = m30*u + m31*v + m33
    //
    // dx/du = m00, dx/dv = m01,
    // dy/du = m10, dy/dv = m11
    // dw/du = m30, dw/dv = m31
    //
    // df/du = (dx/du*w - x*dw/du)/w^2 = (m00*w - m30*x)/w^2 = (m00 - m30*f)/w
    // df/dv = (dx/dv*w - x*dw/dv)/w^2 = (m01*w - m31*x)/w^2 = (m01 - m31*f)/w
    // dg/du = (dy/du*w - y*dw/du)/w^2 = (m10*w - m30*y)/w^2 = (m10 - m30*g)/w
    // dg/dv = (dy/dv*w - y*dw/du)/w^2 = (m11*w - m31*y)/w^2 = (m11 - m31*g)/w
    //
    // Singular values of [df/du df/dv] define perspective correct minimum and maximum scale factors
    //                    [dg/du dg/dv]
    // for M evaluated at  (u,v)
    const SkM44& matrix = t.matrix();
    SkV4 devP = matrix.map(p.x, p.y, 0.f, 1.f);

    const float dxdu = matrix.rc(0,0);
    const float dxdv = matrix.rc(0,1);
    const float dydu = matrix.rc(1,0);
    const float dydv = matrix.rc(1,1);
    const float dwdu = matrix.rc(3,0);
    const float dwdv = matrix.rc(3,1);

    float invW2 = sk_ieee_float_divide(1.f, (devP.w * devP.w));
    // non-persp has invW2 = 1, devP.w = 1, dwdu = 0, dwdv = 0
    float dfdu = (devP.w*dxdu - devP.x*dwdu) * invW2; // non-persp -> dxdu -> m00
    float dfdv = (devP.w*dxdv - devP.x*dwdv) * invW2; // non-persp -> dxdv -> m01
    float dgdu = (devP.w*dydu - devP.y*dwdu) * invW2; // non-persp -> dydu -> m10
    float dgdv = (devP.w*dydv - devP.y*dwdv) * invW2; // non-persp -> dydv -> m11

    // no-persp, these are the singular values of [m00,m01][m10,m11], which is just the upper 2x2
    // and equivalent to SkMatrix::getMinmaxScales().
    float s1 = dfdu*dfdu + dfdv*dfdv + dgdu*dgdu + dgdv*dgdv;

    float e = dfdu*dfdu + dfdv*dfdv - dgdu*dgdu - dgdv*dgdv;
    float f = dfdu*dgdu + dfdv*dgdv;
    float s2 = SkScalarSqrt(e*e + 4*f*f);

    float singular1 = SkScalarSqrt(0.5f * (s1 + s2));
    float singular2 = SkScalarSqrt(0.5f * (s1 - s2));

    // singular1 and 2 represent the minimum and maximum scale factors at that transformed point.
    // Moving 1 from 'p' before transforming will move at least minimum and at most maximum from
    // the transformed point. Thus moving between [1/max, 1/min] pre-transformation means post
    // transformation moves between [1,max/min] so using 1/min as the local AA radius ensures that
    // the post-transformed point is at least 1px away from the original.
    float aaRadius = sk_ieee_float_divide(1.f, std::min(singular1, singular2));
    if (sk_float_isfinite(aaRadius)) {
        return aaRadius;
    } else {
        // Treat NaNs and infinities as +inf, which will always trigger the inset self-intersection
        // logic that snaps inner vertices to the center instead of insetting by the local AA radius
        return SK_FloatInfinity;
    }
}

static float local_aa_radius(const Transform& t, const Rect& bounds) {
    // Use the maximum radius of the 4 corners so that every local vertex uses the same offset
    // even if it's more conservative on some corners (when the min/max scale isn't constant due
    // to perspective).
    if (t.type() < Transform::Type::kProjection) {
        // Scale factors are constant, so the point doesn't really matter
        return local_aa_radius(t, SkV2{0.f, 0.f});
    } else {
        // TODO can we share calculation here?
        float tl = local_aa_radius(t, SkV2{bounds.left(), bounds.top()});
        float tr = local_aa_radius(t, SkV2{bounds.right(), bounds.top()});
        float br = local_aa_radius(t, SkV2{bounds.right(), bounds.bot()});
        float bl = local_aa_radius(t, SkV2{bounds.left(), bounds.bot()});
        return std::max(std::max(tl, tr), std::max(bl, br));
    }
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
        if (shape.isRect()) {
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

    writer << kIndices;
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

    writer << kCornerTemplate  // TL
           << kCornerTemplate  // TR
           << kCornerTemplate  // BR
           << kCornerTemplate; // BL
}

AnalyticRRectRenderStep::AnalyticRRectRenderStep(StaticBufferManager* bufferManager)
        : RenderStep("AnalyticRRectRenderStep",
                     "",
                     Flags::kPerformsShading | Flags::kEmitsCoverage,
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

                             // TODO: pack depth and ssboIndex into 32-bits
                             {"depth", VertexAttribType::kFloat, SkSLType::kFloat},
                             {"ssboIndex", VertexAttribType::kInt, SkSLType::kInt},

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
    // TODO: Move this into a module
    return R"(
        const int kCornerVertexCount = 9; // KEEP IN SYNC WITH C++'s kCornerVertexCount
        const float kMiterScale = 1.0;
        const float kBevelScale = 0.0;
        const float kRoundScale = 0.41421356237; // sqrt(2)-1

        const float kEpsilon = 0.00024; // SK_ScalarNearlyZero

        // Default to miter'ed vertex positioning. Corners with sufficiently large corner radii, or
        // bevel'ed strokes will adjust vertex placement on a per corner basis. This will not affect
        // the final coverage calculations in the fragment shader.
        float joinScale = kMiterScale;

        // Unpack instance-level state that determines the vertex placement and style of shape.
        bool bidirectionalCoverage = center.z <= 0.0;
        bool deviceSpaceDistances = false;
        float4 xs, ys; // ordered TL, TR, BR, BL
        float4 edgeAA = float4(1.0); // ordered L,T,R,B. 1 = AA, 0 = no AA
        if (xRadiiOrFlags.x < -1.0) {
            // Stroked rect or round rect
            xs = ltrbOrQuadYs.LRRL;
            ys = ltrbOrQuadYs.TTBB;

            if (xRadiiOrFlags.y < 0.0) {
                // A hairline so the X radii are encoded as negative values in this field, and Y
                // radii are stored directly in the subsequent float4.
                xRadii = -xRadiiOrFlags - 2.0;
                yRadii = radiiOrQuadXs;

                // All hairlines use miter joins (join style > 0)
                strokeParams = float2(0.0, 1.0);
            } else {
                xRadii = radiiOrQuadXs;
                yRadii = xRadii; // regular strokes are circular
                strokeParams = xRadiiOrFlags.zw;

                if (strokeParams.y < 0.0) {
                    joinScale = kRoundScale; // the stroke radius rounds rectangular corners
                }  else if (strokeParams.y == 0.0) {
                    joinScale = kBevelScale;
                } // else stay mitered
            }
        } else if (any(greaterThan(xRadiiOrFlags, float4(0.0)))) {
            // Filled round rect
            xs = ltrbOrQuadYs.LRRL;
            ys = ltrbOrQuadYs.TTBB;

            xRadii = xRadiiOrFlags;
            yRadii = radiiOrQuadXs;

            strokeParams = float2(0.0, -1.0); // A negative join style is "round"
        } else {
            // Per-edge quadrilateral, so we have to calculate the corner's basis from the
            // quad's edges.
            xs = radiiOrQuadXs;
            ys = ltrbOrQuadYs;
            edgeAA = -xRadiiOrFlags; // AA flags needed to be < 0 on upload, so flip the sign.

            xRadii = float4(0.0);
            yRadii = float4(0.0);

            strokeParams = float2(0.0, 1.0); // Will be ignored, but set to a "miter"
            deviceSpaceDistances = true;
        }

        // Adjust state on a per-corner basis
        int cornerID = sk_VertexID / kCornerVertexCount;
        float strokeRadius = strokeParams.x; // alias
        float2 cornerRadii = float2(xRadii[cornerID], yRadii[cornerID]);
        if (cornerID % 2 != 0) {
            // Corner radii are uploaded in the local coordinate frame, but vertex placement happens
            // in a consistent winding before transforming to final local coords, so swap the
            // radii for odd corners.
            cornerRadii = cornerRadii.yx;
        }

        float2 cornerAspectRatio = float2(1.0);
        if (cornerRadii.x > kEpsilon && cornerRadii.y > kEpsilon) {
            // Position vertices for an elliptical corner; overriding any previous join style since
            // that only applies when radii are 0.
            joinScale = kRoundScale;
            cornerAspectRatio = cornerRadii.yx;
        } else if (cornerRadii.x != 0 && cornerRadii.y != 0) {
            // A very small rounded corner, which technically ignores style (i.e. should not be
            // beveled or mitered), but place the vertices as a miter to fully cover it and let
            // the fragment shader evaluate the curve per pixel.
            joinScale = kMiterScale;
            cornerAspectRatio = cornerRadii.yx;
            cornerRadii = float2(0.0);
        } else if (strokeRadius > 0.0 && strokeRadius <= kEpsilon) {
            // A stroked rectangular corner that could have a very small bevel or round join,
            // so place vertices as a miter.
            joinScale = kMiterScale;
        }

        // Calculate the local edge vectors, ordered L, T, R, B starting from the bottom left point.
        // For quadrilaterals these are not necessarily axis-aligned, but in all cases they orient
        // the +X/+Y normalized vertex template for each corner.
        float4 dx = xs - xs.wxyz;
        float4 dy = ys - ys.wxyz;
        float4 edgeLen = sqrt(dx*dx + dy*dy);

        float4 edgeMask = sign(edgeLen); // 0 for zero-length edge, 1 for non-zero edge.
        if (any(equal(edgeMask, float4(0.0)))) {
            // Must clean up (dx,dy) depending on the empty edge configuration
            if (all(equal(edgeMask, float4(0.0)))) {
                // A point so use the canonical basis
                dx = float4( 0.0, 1.0, 0.0, -1.0);
                dy = float4(-1.0, 0.0, 1.0,  0.0);
                edgeLen = float4(1.0);
            } else {
                // Triangles (3 non-zero edges) copy the adjacent edge. Otherwise it's a line so
                // replace empty edges with the left-hand normal vector of the adjacent edge.
                bool triangle = (edgeMask[0] + edgeMask[1] + edgeMask[2] + edgeMask[3]) > 2.5;
                float4 edgeX = triangle ? dx.yzwx :  dy.yzwx;
                float4 edgeY = triangle ? dy.yzwx : -dx.yzwx;

                dx = mix(edgeX, dx, edgeMask);
                dy = mix(edgeY, dy, edgeMask);
                edgeLen = mix(edgeLen.yzwx, edgeLen, edgeMask);
                edgeAA = mix(edgeAA.yzwx, edgeAA, edgeMask);
            }
        }

        dx /= edgeLen;
        dy /= edgeLen;

        // Calculate local coordinate for the vertex (relative to xAxis and yAxis at first).
        float2 xAxis = -float2(dx.yzwx[cornerID], dy.yzwx[cornerID]);
        float2 yAxis =  float2(dx.xyzw[cornerID], dy.xyzw[cornerID]);
        float2 localPos;
        bool snapToCenter = false;
        if (normalScale < 0.0) {
            // Vertex is inset from the base shape, so we scale by (cornerRadii - strokeRadius)
            // and have to check for the possibility of an inner miter. It is always inset by an
            // additional conservative AA amount.
            if (center.w < 0.0 || centerWeight * center.z != 0.0) {
                snapToCenter = true;
            } else {
                float localAARadius = center.w;
                float2 insetRadii =
                        cornerRadii + (bidirectionalCoverage ? -strokeRadius : strokeRadius);
                if (joinScale == kMiterScale ||
                    insetRadii.x <= localAARadius || insetRadii.y <= localAARadius) {
                    // Miter the inset position
                    localPos = (insetRadii - localAARadius);
                } else {
                    localPos = insetRadii*position - localAARadius*normal;
                }
            }
        } else {
            // Vertex is outset from the base shape (and possibly with an additional AA outset later
            // in device space).
            localPos = (cornerRadii + strokeRadius) * (position + joinScale*position.yx);
        }

        if (snapToCenter) {
            // Center is already relative to true local coords, not the corner basis.
            localPos = center.xy;
        } else {
            // Transform from corner basis to true local coords.
            localPos -= cornerRadii;
            localPos = float2(xs[cornerID], ys[cornerID]) + xAxis*localPos.x + yAxis*localPos.y;
        }

        // Calculate edge distances and device space coordinate for the vertex
        // TODO: Apply edge AA flags to these values to turn off AA when necessary.
        edgeDistances = dy*(xs - localPos.x) - dx*(ys - localPos.y);

        float3x3 localToDevice = float3x3(mat0, mat1, mat2);
        // NOTE: This 3x3 inverse is different than just taking the 1st two columns of the 4x4
        // inverse of the original SkM44 local-to-device matrix. We could calculate the 3x3 inverse
        // and upload it, but it does not seem to be a bottleneck and saves on bandwidth to
        // calculate it here instead.
        float3x3 deviceToLocal = inverse(localToDevice);
        float3 devPos = localToDevice * localPos.xy1;
        jacobian = float4(deviceToLocal[0].xy - deviceToLocal[0].z*localPos,
                          deviceToLocal[1].xy - deviceToLocal[1].z*localPos);

        if (deviceSpaceDistances) {
            // Apply the Jacobian in the vertex shader so any quadrilateral normals do not have to
            // be passed to the fragment shader. However, it's important to use the Jacobian at a
            // vertex on the edge, not the current vertex's Jacobian.
            float4 gx = -dy*(deviceToLocal[0].x - deviceToLocal[0].z*xs) +
                         dx*(deviceToLocal[0].y - deviceToLocal[0].z*ys);
            float4 gy = -dy*(deviceToLocal[1].x - deviceToLocal[1].z*xs) +
                         dx*(deviceToLocal[1].y - deviceToLocal[1].z*ys);
            // NOTE: The gradient is missing a W term so edgeDistances must still be multiplied by
            // 1/w in the fragment shader. The same goes for the encoded coverage scale.
            edgeDistances *= inversesqrt(gx*gx + gy*gy);

            // Bias non-AA edge distances by device W so its coverage contribution is >= 1.0
            edgeDistances += (1 - edgeAA)*abs(devPos.z);

            // Mixed edge AA shapes do not use subpixel scale+bias for coverage, since they tile
            // to a large shape of unknown--but likely not subpixel--size. Triangles and quads do
            // not use subpixel coverage since the scale+bias is not constant over the shape, but
            // we can't evaluate per-fragment since we aren't passing down their arbitrary normals.
            bool subpixelCoverage = edgeAA == float4(1.0) &&
                                    dot(abs(dx*dx.yzwx + dy*dy.yzwx), float4(1.0)) < kEpsilon;
            if (subpixelCoverage) {
                // Reconstructs the actual device-space width and height for all rectangle vertices.
                float2 dim = edgeDistances.xy + edgeDistances.zw;
                perPixelControl.y = 1.0 + min(min(dim.x, dim.y), abs(devPos.z));
            } else {
                perPixelControl.y = 1.0 + abs(devPos.z); // standard 1px width pre W division.
            }
        }

        // Only outset for a vertex that is in front of the w=0 plane to avoid dealing with outset
        // triangles rasterizing differently from the main triangles as w crosses 0.
        if (normalScale > 0.0 && devPos.z > 0.0) {
            // Note that when there's no perspective, the jacobian is equivalent to the normal
            // matrix (inverse transpose), but produces correct results when there's perspective
            // because it accounts for the position's influence on a line's projected direction.
            float2x2 J = float2x2(jacobian.xy, jacobian.zw);

            float2 edgeAANormal = float2(edgeAA[cornerID], edgeAA.yzwx[cornerID]) * normal;
            float2 nx = cornerAspectRatio.x * edgeAANormal.x * perp(-yAxis) * J;
            float2 ny = cornerAspectRatio.y * edgeAANormal.y * perp( xAxis) * J;

            bool isMidVertex = edgeAANormal.x != 0.0 && edgeAANormal.y != 0.0;
            if (joinScale == kMiterScale && isMidVertex) {
                // Produce a bisecting vector in device space (ignoring 'normal' since that was
                // previously corrected to match the mitered edge normals).
                nx = normalize(nx);
                ny = normalize(ny);
                if (dot(nx, ny) < -0.8) {
                    // Normals are in nearly opposite directions, so adjust to avoid float error.
                    float s = sign(cross_length_2d(nx, ny));
                    nx =  s*perp(nx);
                    ny = -s*perp(ny);
                }
            }
            // Adding the normal components together directly results in what we'd have
            // calculated if we'd just transformed 'normal' in one go, assuming they weren't
            // normalized in the if-block above. If they were normalized, the sum equals the
            // bisector between the original nx and ny.
            //
            // We multiply by W so that after perspective division the new point is offset by the
            // now-unit normal.
            // NOTE: (nx + ny) can become the zero vector if the device outset is for an edge
            // marked as non-AA. In this case normalize() could produce the zero vector or NaN.
            // Until a counter-example is found, GPUs seem to discard triangles with NaN vertices,
            // which has the same effect as outsetting by the zero vector with this mesh, so we
            // don't bother guarding the normalize() (yet).
            devPos.xy += devPos.z * normalize(nx + ny);

            // By construction these points are 1px away from the outer edge in device space.
            if (deviceSpaceDistances) {
                // Apply directly to edgeDistances to save work per pixel later on.
                edgeDistances -= devPos.z;
            } else {
                // Otherwise store separately so edgeDistances can be used to reconstruct corner pos
                perPixelControl.y = -devPos.z;
            }
        } else if (!deviceSpaceDistances) {
            // Triangles are within the original shape so there's no additional outsetting to
            // take into account for coverage calculations.
            perPixelControl.y = 0.0;
        }

        if (centerWeight != 0.0) {
            // A positive value signals that a pixel is trivially full coverage.
            perPixelControl.x = 1.0;
        } else {
            // A negative value signals bidirectional coverage, and a zero value signals a solid
            // interior with per-pixel coverage.
            perPixelControl.x = bidirectionalCoverage ? -1.0 : 0.0;
        }

        // Write out final results
        stepLocalCoords = localPos;
        float4 devPosition = float4(devPos.xy, devPos.z*depth, devPos.z);
    )";
}

const char* AnalyticRRectRenderStep::fragmentCoverageSkSL() const {
    // TODO: Further modularize this
    return R"(
        if (perPixelControl.x > 0.0) {
            // A trivially solid interior pixel, either from a filled rect or round rect, or a
            // stroke with sufficiently large width that the interior completely overlaps itself.
            outputCoverage = half4(1.0);
        } else if (perPixelControl.y > 1.0) {
            // This represents a filled rectangle or quadrilateral, where the distances have already
            // been converted to device space. Mitered strokes cannot use this optimization because
            // their scale and bias is not uniform over the shape; Rounded shapes cannot use this
            // because they rely on the edge distances being in local space to reconstruct the
            // per-corner positions for the elliptical implicit functions.
            float2 outerDist = min(edgeDistances.xy, edgeDistances.zw);
            float c = min(outerDist.x, outerDist.y) * sk_FragCoord.w;
            float scale = (perPixelControl.y - 1.0) * sk_FragCoord.w;
            float bias = coverage_bias(scale);
            outputCoverage = half4(clamp(scale * (c + bias), 0.0, 1.0));
        } else {
            // Compute per-pixel coverage, mixing four outer edge distances, possibly four inner
            // edge distances, and per-corner elliptical distances into a final coverage value.
            // The Jacobian needs to be multiplied by W, but sk_FragCoord.w stores 1/w.
            float2x2 J = float2x2(jacobian.xy, jacobian.zw) / sk_FragCoord.w;

            float2 invGradLen = float2(inverse_grad_len(float2(1.0, 0.0), J),
                                       inverse_grad_len(float2(0.0, 1.0), J));
            float2 outerDist = invGradLen * (strokeParams.x + min(edgeDistances.xy,
                                                                  edgeDistances.zw));

            // d.x tracks minimum outer distance (pre scale-and-biasing to a coverage value).
            // d.y tracks negative maximum inner distance (so min() over c accumulates min and outer
            // and max inner simultaneously).)
            float2 d = float2(min(outerDist.x, outerDist.y), -1.0);
            float scale, bias;

            // Check for bidirectional coverage, which is is marked as a -1 from the vertex shader.
            // We don't just check for < 0 since extrapolated fill triangle samples can have small
            // negative values.
            if (perPixelControl.x > -0.95) {
                // A solid interior, so update scale and bias based on full width and height
                float2 dim = invGradLen * (edgeDistances.xy + edgeDistances.zw + 2*strokeParams.xx);
                scale = min(min(dim.x, dim.y), 1.0);
                bias = coverage_bias(scale);
                // Since we leave d.y = -1.0, no inner curve coverage will adjust it closer to 0,
                // so 'finalCoverage' is based solely on outer edges and curves.
            } else {
                // Bidirectional coverage, so we modify c.y to hold the negative of the maximum
                // interior coverage, and update scale and bias based on stroke width.
                float2 strokeWidth = 2.0 * strokeParams.x * invGradLen;
                float2 innerDist = strokeWidth - outerDist;

                d.y = -max(innerDist.x, innerDist.y);
                if (strokeParams.x > 0.0) {
                    float strokeDim = min(strokeWidth.x, strokeWidth.y);
                    if (innerDist.y >= -0.5 && strokeWidth.y > strokeDim) {
                        strokeDim = strokeWidth.y;
                    }
                    if (innerDist.x >= -0.5 && strokeWidth.x > strokeDim) {
                        strokeDim = strokeWidth.x;
                    }
                    scale = min(strokeDim, 1.0);
                    bias = coverage_bias(scale);
                } else {
                    // A hairline, so scale and bias should both be 1
                     scale = bias = 1.0;
                }
            }

            // Check all corners, although most pixels should only be influenced by 1.
            corner_distances(d, J, strokeParams, edgeDistances, xRadii, yRadii);

            float outsetDist = min(perPixelControl.y, 0.0) * sk_FragCoord.w;
            float finalCoverage = scale * (min(d.x + outsetDist, -d.y) + bias);

            outputCoverage = half4(clamp(finalCoverage, 0.0, 1.0));
        }
    )";
}

void AnalyticRRectRenderStep::writeVertices(DrawWriter* writer,
                                           const DrawParams& params,
                                           int ssboIndex) const {
    SkASSERT(params.geometry().isShape() || params.geometry().isEdgeAAQuad());

    DrawWriter::Instances instance{*writer, fVertexBuffer, fIndexBuffer, kIndexCount};
    auto vw = instance.append(1);

    // The bounds of a rect is the rect, and the bounds of a rrect is tight (== SkRRect::getRect()).
    Rect bounds = params.geometry().bounds();
    const skvx::float2 size = bounds.size();

    // aaRadius will be set to a negative value to signal a complex self-intersection that has to
    // be calculated in the vertex shader.
    float aaRadius = local_aa_radius(params.transform(), bounds);
    float strokeInset = 0.f;
    float centerWeight = kSolidInterior;

    if (params.isStroke()) {
        const Shape& shape = params.geometry().shape(); // EdgeAAQuads are not stroked

        SkASSERT(params.strokeStyle().halfWidth() >= 0.f);
        SkASSERT(shape.isRect() || params.strokeStyle().halfWidth() == 0.f ||
                 (shape.isRRect() && SkRRectPriv::AllCornersCircular(shape.rrect())));

        float strokeRadius = params.strokeStyle().halfWidth();
        skvx::float2 innerGap = size - 2.f * params.strokeStyle().halfWidth();
        if (any(innerGap <= 0.f)) {
            // AA inset intersections are measured from the *outset*
            strokeInset = -strokeRadius;
        } else {
            // This will be upgraded to kFilledStrokeInterior if insets intersect
            centerWeight = kStrokeInterior;
            strokeInset = strokeRadius;
        }

        skvx::float4 xRadii = shape.isRRect() ? load_x_radii(shape.rrect()) : skvx::float4(0.f);
        if (strokeRadius > 0.f) {
            float joinStyle = params.strokeStyle().joinLimit();
            if (params.strokeStyle().isMiterJoin()) {
                // All corners are 90-degrees so become beveled if the miter limit is < sqrt(2).
                if (params.strokeStyle().miterLimit() < SK_ScalarSqrt2) {
                    joinStyle = 0.f; // == bevel
                } else {
                    // Discard actual miter limit because a 90-degree corner never exceeds it.
                    joinStyle = 1.f;
                }
            }
            // Stroked lines or point needs some upfront cleanup for the vertex shader to work.
            auto empty = size == 0.f;
            if (all(empty)) {
                // A point, so update join style based on the cap geometry. Butt caps should have
                // been discarded earlier.
                SkASSERT(params.strokeStyle().cap() != SkPaint::kButt_Cap);
                joinStyle = params.strokeStyle().cap() == SkPaint::kRound_Cap ? -1.f : 1.f;
            } else if (any(empty) && joinStyle >= 0.f) {
                // A line with miter or bevel joins, but "corners" are now 180 degree turns so the
                // miter limit is always exceeded and the bevel matches that of a butt cap. The
                // vertex shader can't handle that so manually inset the uploaded geometry so a
                // stroke-radius miter join produces the expected line.
                float strokeDelta = std::min(0.f, std::max(innerGap.x(), innerGap.y()));
                auto adjust = strokeDelta + if_then_else(empty, skvx::float2(0.f),
                                                                skvx::float2(strokeRadius));
                bounds.inset(adjust);
                strokeRadius += strokeDelta;
                joinStyle = 1.f;

                // Since we are distorting the uploaded geometry, the normal catch-all complex
                // interior check doesn't work.
                if (opposite_insets_intersect(bounds, strokeRadius, aaRadius)) {
                    aaRadius = kComplexAAInsets;
                    SkASSERT(centerWeight == kSolidInterior);
                }
            } // Else a non-empty or line+round join, which do not need any style cleanup

            // Write a negative value outside [-1, 0] to signal a stroked shape, then the style
            // params, followed by corner radii and bounds.
            vw << -2.f << 0.f << strokeRadius << joinStyle << xRadii << bounds.ltrb();
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
       << ssboIndex
       << m.rc(0,0) << m.rc(1,0) << m.rc(3,0)  // mat0
       << m.rc(0,1) << m.rc(1,1) << m.rc(3,1)  // mat1
       << m.rc(0,3) << m.rc(1,3) << m.rc(3,3); // mat2
}

void AnalyticRRectRenderStep::writeUniformsAndTextures(const DrawParams&,
                                                       PipelineDataGatherer*) const {
    // All data is uploaded as instance attributes, so no uniforms are needed.
}

}  // namespace skgpu::graphite
