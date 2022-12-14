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
//
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
//
// AnalyticRRectRenderStep uses the common technique of approximating distance to the level set by
// one expansion of the Taylor's series for the level set's equation. Given a level set function
// C(x,y), this amounts to calculating C(px,py)/|∇C(px,py)|. For a round-rect's linear edges, C is
// linear and the gradient is a constant, so can be computed in the vertex shader and interpolated
// exactly. For the curved corners, C's inputs are interpolated and then evaluated in the fragment
// shader, but the gradient is linear and can be interpolated exactly as well. A separate varying
// is used so that the more expensive non-linear equations are only computed on triangles that cover
// the curved corners.
//
// However, for both linear and curved corners, C is much easier to define in a local space instead
// of the pixel-space required for final anti-aliasing. (px,py) is the projected point of (u,v)
// transformed by a 4x4 matrix:            [x]   [m00 m01 * m03] [u]
//                [x(u,v) / w(u,v)]        [y]   [m10 m11 * m13]X[v]
//      (px,py) = [y(u,v) / w(u,v)] where  [*] = [ *   *  *  * ] [0] = M*(u,v,0,1)
//                                         [w]   [m30 m31 * m33] [1]
//
// C(px,py) can be defined in terms of a local Cl(u,v) as C(px,py) = Cl(p^-1(px,py)), where p^-1 =
//                                              [x']   [m00' m01' * m03'] [px]
//               [x'(px,py) / w'(px,py)]        [y']   [m10' m11' * m13'] [py]
//      (u,v) =  [y'(px,py) / w'(px,py)] where  [* ] = [ *    *   *  *  ]X[ 0] = M^-1*(px,py,0,1)
//                                              [w']   [m30' m31' * m33'] [ 1]
//
// Using the chain rule, then ∇C(px,py)                             [m00' m01']
//   =  ∇Cl(u,v)X[1/w'(px,py)     0       0 -x'(px,py)/w'(px,py)^2]X[m10' m11']
//               [    0       1/w'(px,py) 0 -y'(px,py)/w'(px,py)^2] [ *     * ]
//                                                                  [m30' m31']
//
//                                                       [m00' m01']
//   = 1/w'(px,py)*∇Cl(u,v)X[1 0 0 -x'(px,py)/w'(px,py)]X[m10' m11']
//                          [0 1 0 -y'(px,py)/w'(px,py)] [ *    *  ]
//                                                       [m30' m31']
//                                [m00' m01']
//   = w(u,v)*∇Cl(u,v)X[1 0 0 -u]X[m10' m11']
//                     [0 1 0 -v] [ *    *  ]
//                                [m30' m31']
//
//   = w(u,v)*∇Cl(u,v)X[m00'-m30'u m01'-m31'u]
//                     [m10'-m30'v m11'-m31'v]
//
// For AnalyticRRectRenderStep, the "local" space used for these calculations is the normalized
// position scaled by the corner radii. This provides a stable local space for all vertices within a
// corner (which is not the case for the original normalized space and strokes, since the inner and
// outer curves differ). The main impact of this is that the M and M^-1 used in the above derivation
// are not the exact local-to-device matrix of the draw, but includes an additional basis
// adjustment in the form of A = [x0 y0 0 tx] so M^-1=A^-1x(L2D)^-1.
//                               [x1 y1 0 ty]
//                               [0  0  1  0]
//                               [0  0  0  1]
//
// A is different for each corner. This makes interpolating the Jacobian and (u,v) over the entire
// mesh a little more complex. Luckily, per-pixel coverage calculations can be contained entirely
// to triangles assigned to each corner, so within their area, the interpolated values are correct.
// All the triangles connecting adjacent corners are always linear edges so their coverage can
// be calculated by computing C/|∇C| in the vertex shader instead of the fragment shader. This
// final edge distance is stable across changes to A so as long as we carry another control
// attribute to identify whether or not a triangle uses the linear edge distance or the varying
// Jacobian and (u,v), it will all work out. To assist with this, though, and to avoid branching,
// the Jacobian, edge distance, and (u,v) coordinates are calculated for every vertex since it
// could belong to both linear and per-pixel coverage triangles.
//
// Within each corner, however, we do need to evaluate up to two circle equations: one with
// radius = corner-radius(r)+stroke-radius(s), and another with radius = r-s. This can be
// consolidated into a common evaluation against a circle of radius sqrt(r^2+s^2) as follows:
//
// (x/(r+/-s))^2 + (y/(r+/-s))^2 = 1
// x^2 + y^2 = (r+/-s)^2
// x^2 + y^2 = r^2 + s^2 +/- 2rs
// (x/sqrt(r^2+s^2))^2 + (y/sqrt(r^2+s^2)) = 1 +/- 2rs/(r^2+s^2)
//
// We pass r/sqrt(r^2+s^2) and s/sqrt(r^2+s^2) down as two varyings and then add and subtract
// their product to get coverage values for the inner and outer circles of the stroked corner.
// However, if their difference is negative, this is consistent with (r-s) < 0 and we have a
// collapsed inner corner so only the outer coverage should be used. This happens automatically
// for strokes with small corners or for rectangular corners (r=0) with round joins. For fills
// and hairlines, the normal s=0 so as expected 2rs is 0 and we're effectively testing a single
// circle curve. However, we have need to differentiate the two cases by setting r=0,s=1 for
// hairlines and r=1,s=0 for filled round rects.
namespace skgpu::graphite {

static skvx::float4 loadXRadii(const SkRRect& rrect) {
    // We swizzle X and Y radii for the anti-diagonal corners to match overall winding of the rrect
    // when processed as vertices on the GPU.
    return skvx::float4{rrect.radii(SkRRect::kUpperLeft_Corner).fX,
                        rrect.radii(SkRRect::kUpperRight_Corner).fY,
                        rrect.radii(SkRRect::kLowerRight_Corner).fX,
                        rrect.radii(SkRRect::kLowerLeft_Corner).fY};
}
static skvx::float4 loadYRadii(const SkRRect& rrect) {
    return skvx::float4{rrect.radii(SkRRect::kUpperLeft_Corner).fY,
                        rrect.radii(SkRRect::kUpperRight_Corner).fX,
                        rrect.radii(SkRRect::kLowerRight_Corner).fY,
                        rrect.radii(SkRRect::kLowerLeft_Corner).fX};
}

static float localAARadius(const Transform& t, const SkV2& p) {
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

static float localAARadius(const Transform& t, const Rect& bounds) {
    // Use the maximum radius of the 4 corners so that every local vertex uses the same offset
    // even if it's more conservative on some corners (when the min/max scale isn't constant due
    // to perspective).
    if (t.type() < Transform::Type::kProjection) {
        // Scale factors are constant, so the point doesn't really matter
        return localAARadius(t, SkV2{0.f, 0.f});
    } else {
        // TODO can we share calculation here?
        float tl = localAARadius(t, SkV2{bounds.left(), bounds.top()});
        float tr = localAARadius(t, SkV2{bounds.right(), bounds.top()});
        float br = localAARadius(t, SkV2{bounds.right(), bounds.bot()});
        float bl = localAARadius(t, SkV2{bounds.left(), bounds.bot()});
        return std::max(std::max(tl, tr), std::max(bl, br));
    }
}

static bool cornerInsetsIntersect(const SkRRect& rrect, float strokeRadius, float aaRadius) {
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

// Represents the per-vertex attributes used in each instance.
struct Vertex {
    SkV2 fPosition;
    SkV2 fNormal;
    // TODO: fNormalScale is only needed while we have vertices with stroke-control == 0 so that
    // those vertices can define a non-zero normal for distance calculations but not be moved.
    // A follow-up CL will switch to interpolating opposing edge distances, which means we can
    // entirely remove the vertices that lie on the center of the stroke.
    float fNormalScale;
    float fStrokeControl;
    float fMirrorOffset;
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

static constexpr int kCornerVertexCount = 19; // sk_VertexID is divided by this in SkSL
static constexpr int kVertexCount = 4 * kCornerVertexCount;
static constexpr int kIndexCount = 153;

static size_t indexBufferSize() { return kIndexCount * sizeof(uint16_t); }
static void writeIndexBuffer(VertexWriter writer, size_t size) {
    SkASSERT(indexBufferSize() == size);

    static constexpr uint16_t kTL = 0 * kCornerVertexCount;
    static constexpr uint16_t kTR = 1 * kCornerVertexCount;
    static constexpr uint16_t kBR = 2 * kCornerVertexCount;
    static constexpr uint16_t kBL = 3 * kCornerVertexCount;

    static const uint16_t kIndices[kIndexCount] = {
        // Exterior AA ramp outset
        kTL+0,kTL+6,kTL+1,kTL+7,kTL+2,kTL+8,kTL+3,kTL+8,kTL+4,kTL+9,kTL+5,kTL+9,
        kTR+0,kTR+6,kTR+1,kTR+7,kTR+2,kTR+8,kTR+3,kTR+8,kTR+4,kTR+9,kTR+5,kTR+9,
        kBR+0,kBR+6,kBR+1,kBR+7,kBR+2,kBR+8,kBR+3,kBR+8,kBR+4,kBR+9,kBR+5,kBR+9,
        kBL+0,kBL+6,kBL+1,kBL+7,kBL+2,kBL+8,kBL+3,kBL+8,kBL+4,kBL+9,kBL+5,kBL+9,
        kTL+0,kTL+6,kTL+6, // close and extra vertex to jump to next strip
        // Outer to central curve
        kTL+6,kTL+10,kTL+7,kTL+11,kTL+8,kTL+12,kTL+9,kTL+13,
        kTR+6,kTR+10,kTR+7,kTR+11,kTR+8,kTR+12,kTR+9,kTR+13,
        kBR+6,kBR+10,kBR+7,kBR+11,kBR+8,kBR+12,kBR+9,kBR+13,
        kBL+6,kBL+10,kBL+7,kBL+11,kBL+8,kBL+12,kBL+9,kBL+13,
        kTL+6,kTL+10,kTL+10, // close and extra vertex to jump to next strip
        // Center to inner curve's insets
        kTL+10,kTL+14,kTL+11,kTL+15,kTL+12,kTL+16,kTL+13,kTL+16,
        kTR+10,kTR+14,kTR+11,kTR+15,kTR+12,kTR+16,kTR+13,kTR+16,
        kBR+10,kBR+14,kBR+11,kBR+15,kBR+12,kBR+16,kBR+13,kBR+16,
        kBL+10,kBL+14,kBL+11,kBL+15,kBL+12,kBL+16,kBL+13,kBL+16,
        kTL+10,kTL+14,kTL+14, // close and extra vertex to jump to next strip
        // Inner inset to center of shape
        kTL+14,kTL+17,kTL+15,kTL+17,kTL+16,kTL+16,kTL+18,kTR+14,
        kTR+14,kTR+17,kTR+15,kTR+17,kTR+16,kTR+16,kTR+18,kBR+14,
        kBR+14,kBR+17,kBR+15,kBR+17,kBR+16,kBR+16,kBR+18,kBL+14,
        kBL+14,kBL+17,kBL+15,kBL+17,kBL+16,kBL+16,kBL+18,kTL+14 // close
    };

    writer << kIndices;
}

static size_t vertexBufferSize() { return kVertexCount * sizeof(Vertex); }
static void writeVertexBuffer(VertexWriter writer, size_t size) {
    SkASSERT(vertexBufferSize() == size);

    // Allowed values for the stroke control vertex attribute. This is multiplied with the stroke
    // radius of the instance to get the final effect on the corner positions. When the stroke
    // radius is zero (for fills or hairlines), the three possible curves are coincident.
    static constexpr float kOuterStroke = 1.f;
    static constexpr float kInnerStroke = -1.f;
    static constexpr float kCenterStroke = 0.f;

    // Allowed values for the mirror offset attribute. The full normalized position for a vertex is
    // position + join-scale*mirror-offset*position.yx; where join-scale is a scalar determined by
    // the bevel, round, or miter join style of the instance and corner's radii.
    static constexpr float kNoOffset = 0.f;
    static constexpr float kMirrorOffset = 1.f;

    // Allowed values for the center weight attribute.
    static constexpr float kRegularVertex = 0.f;
    static constexpr float kCenterVertex = 1.f;

    static constexpr float kHR2 = 0.5f * SK_FloatSqrt2; // "half root 2"

    // This template is repeated 4 times in the vertex buffer, for each of the four corners.
    // The vertex ID is used to determine which corner the normalized position is transformed to.
    static constexpr Vertex kCornerTemplate[kCornerVertexCount] = {
        // Device-space AA outsets from outer curve
        { {1.0f, 0.0f}, { 1.0f, 0.0f}, 1.f, kOuterStroke,  kNoOffset,      kRegularVertex },
        { {1.0f, 0.0f}, { 1.0f, 0.0f}, 1.f, kOuterStroke,  kMirrorOffset,  kRegularVertex },
        { {1.0f, 0.0f}, { kHR2, kHR2}, 1.f, kOuterStroke,  kMirrorOffset,  kRegularVertex },
        { {0.0f, 1.0f}, { kHR2, kHR2}, 1.f, kOuterStroke,  kMirrorOffset,  kRegularVertex },
        { {0.0f, 1.0f}, { 0.0f, 1.0f}, 1.f, kOuterStroke,  kMirrorOffset,  kRegularVertex },
        { {0.0f, 1.0f}, { 0.0f, 1.0f}, 1.f, kOuterStroke,  kNoOffset,      kRegularVertex },

        // Outer anchors (no local or device-space normal outset)
        { {1.0f, 0.0f}, { 1.0f, 0.0f}, 0.f, kOuterStroke,  kNoOffset,      kRegularVertex },
        { {1.0f, 0.0f}, { kHR2, kHR2}, 0.f, kOuterStroke,  kMirrorOffset,  kRegularVertex },
        { {0.0f, 1.0f}, { kHR2, kHR2}, 0.f, kOuterStroke,  kMirrorOffset,  kRegularVertex },
        { {0.0f, 1.0f}, { 0.0f, 1.0f}, 0.f, kOuterStroke,  kNoOffset,      kRegularVertex },

        // TODO: Remove the kCenterStroke vertices from the mesh.
        // Center of stroke (equivalent to outer anchors when filling)
        { {1.0f, 0.0f}, { 1.0f, 0.0f}, 0.f, kCenterStroke, kNoOffset,      kRegularVertex },
        { {1.0f, 0.0f}, { kHR2, kHR2}, 0.f, kCenterStroke, kMirrorOffset,  kRegularVertex },
        { {0.0f, 1.0f}, { kHR2, kHR2}, 0.f, kCenterStroke, kMirrorOffset,  kRegularVertex },
        { {0.0f, 1.0f}, { 0.0f, 1.0f}, 0.f, kCenterStroke, kNoOffset,      kRegularVertex },

        // Inner curve (with additional AA inset in the common case)
        { {1.0f, 0.0f}, { 1.0f, 0.0f}, -1.f, kInnerStroke,  kNoOffset,     kRegularVertex },
        { {0.5f, 0.5f}, { kHR2, kHR2}, -1.f, kInnerStroke,  kMirrorOffset, kRegularVertex },
        { {0.0f, 1.0f}, { 0.0f, 1.0f}, -1.f, kInnerStroke,  kNoOffset,     kRegularVertex },

        // Center filling vertices (equal to inner AA insets unless 'center' triggers a fill).
        { {0.5f, 0.5f}, { kHR2, kHR2}, -1.f, kInnerStroke,  kMirrorOffset, kCenterVertex  },
        { {0.0f, 1.0f}, { 0.0f, 1.0f}, -1.f, kInnerStroke,  kNoOffset,     kCenterVertex  },
    };

    writer << kCornerTemplate  // TL
           << kCornerTemplate  // TR
           << kCornerTemplate  // BR
           << kCornerTemplate; // BL
}

AnalyticRRectRenderStep::AnalyticRRectRenderStep()
        : RenderStep("AnalyticRRectRenderStep",
                     "",
                     Flags::kPerformsShading | Flags::kEmitsCoverage,
                     /*uniforms=*/{},
                     PrimitiveType::kTriangleStrip,
                     kDirectDepthGreaterPass,
                     /*vertexAttrs=*/{
                            {"position", VertexAttribType::kFloat2, SkSLType::kFloat2},
                            {"normal", VertexAttribType::kFloat2, SkSLType::kFloat2},
                            {"normalScale", VertexAttribType::kFloat, SkSLType::kFloat},
                            {"strokeControl", VertexAttribType::kFloat, SkSLType::kFloat},
                            {"mirrorOffset", VertexAttribType::kFloat, SkSLType::kFloat},
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
                             {"mat2", VertexAttribType::kFloat3, SkSLType::kFloat3},
                             // We need the first two columns of the inverse transform to calculate
                             // the normal matrix.
                             {"invMat0", VertexAttribType::kFloat3, SkSLType::kFloat3},
                             {"invMat1", VertexAttribType::kFloat3, SkSLType::kFloat3}},
                     /*varyings=*/{
                             {"jacobian", SkSLType::kFloat4}, // float2x2
                             {"coverageWidth", SkSLType::kFloat2},
                             // TODO: Switch to interpolating distance from two opposing edges,
                             // which also lets us drop 16 vertices and many more triangles.
                             {"edgeDistance", SkSLType::kFloat},
                             // TODO: With flat shading and careful control of indices for provoking
                             // vertex we can detect linear/circle coverage using just coverageWidth
                             {"perPixelControl", SkSLType::kFloat},
                             {"uv", SkSLType::kFloat2},
                     }) {}

AnalyticRRectRenderStep::~AnalyticRRectRenderStep() {}

const char* AnalyticRRectRenderStep::vertexSkSL() const {
    // TODO: Move this into a module
    return R"(
        const float kMiterScale = 1.0;
        const float kBevelScale = 0.0;
        const float kRoundScale = 0.41421356237; // sqrt(2)-1

        const float kEpsilon = 0.00024; // SK_ScalarNearlyZero

        int cornerID = sk_VertexID / 19; // KEEP IN SYNC WITH kCornerVertexCount

        // Instance-level state that controls how the interior is filled (or not).
        // There are two rings of inner vertices differentiated by the centerWeight attribute so
        // that we can control per-pixel coverage carefully when we need to fill the interior. For
        // typical stroked shapes, the "center" vertices are co-located with the inner vertices.
        // See note on 'center's declaration for how its components are interpreted.
        bool innerVertex = strokeControl < 0.0;
        bool snapToCenter = innerVertex &&
                            (center.w < 0.0 || centerWeight*center.z != 0.0);
        float innerCoverageDirection = center.z <= 0.0 ? -1.0 : 1.0; // -1 for strokes, +1 for fills
        float localAARadius = snapToCenter ? 0.0 : center.w;

        // Corner variables that are the same for all vertices in a corner, but depend on style.
        float4 xs, ys; // should be TL, TR, BR, BL
        float2 cornerRadii = float2(0.0);
        float strokeRadius = 0.0; // fill and hairline are differentiated by center weighting
        float joinScale; // the amount of mirror offseting to apply based on corner/stroke join
        float2 uvScale; // Normalization for circular corners.

        if (xRadiiOrFlags.x < -1.0) {
            // Stroked rect or round rect
            xs = ltrbOrQuadYs.LRRL;
            ys = ltrbOrQuadYs.TTBB;

            strokeRadius = xRadiiOrFlags.y;
            cornerRadii = float2(radiiOrQuadXs[cornerID]);  // strokes require circular corners

            // Configure corner shape based on style, defaulting to kRoundStyle since any circular
            // corner remains round regardless.
            // TODO should this analysis be done on the CPU and we upload 4 joinScales instead of
            // 1 join style that has to be combined with the per-corner radii?
            joinScale = kRoundScale;
            if (cornerRadii.x <= kEpsilon) {
                // Join type only affects rectangular corners. For simplicity, hairline rect corners
                // are always mitered.
                if (strokeRadius == 0.0 || xRadiiOrFlags.z > 0.0) {
                    joinScale = kMiterScale;
                } else if (xRadiiOrFlags.z == 0.0) {
                    joinScale = kBevelScale;
                } // else remain rounded
            }

            // When not rounded, this will be overwritten to sensible default values later on.
            uvScale = inversesqrt(cornerRadii*cornerRadii + strokeRadius*strokeRadius);
            coverageWidth = float2(cornerRadii.x, strokeRadius) * uvScale;

            // For strokes with a solid interior due to large stroke radius, adjust the inner
            // curve to be at the outer curve to limit the amount of per-pixel coverage (e.g. very
            // similar to a filled round rect except a corner could be beveled instead).
            if (center.z > 0.0 && innerVertex) {
                // TODO: Once the central vertices (sc = 0) are removed, this should be updated to
                // sc = 1 instead.
                strokeControl = 0.0;
            }
        } else if (any(greaterThan(xRadiiOrFlags, float4(0.0)))) {
            // Filled round rect
            xs = ltrbOrQuadYs.LRRL;
            ys = ltrbOrQuadYs.TTBB;

            cornerRadii = float2(xRadiiOrFlags[cornerID], radiiOrQuadXs[cornerID]);
            if (cornerRadii.x > kEpsilon || cornerRadii.y > kEpsilon) {
                joinScale = kRoundScale;
                uvScale = 1.0 / cornerRadii;
                // The final width is 0, but "r-s"<0 so we skip an inner circle calculation later.
                coverageWidth = float2(0.0, 1.0);
            } else {
                // This specific corner is rectangular
                joinScale = kMiterScale;
            }
        } else {
            // Per-edge quadrilateral, so we have to calculate the corner's basis from the
            // quad's edges.
            xs = radiiOrQuadXs;
            ys = ltrbOrQuadYs;
            joinScale = kMiterScale;
        }

        bool isCurve = strokeControl >= 0 && mirrorOffset != 0.0;
        if (joinScale != kRoundScale) {
            // Provide sensible default values for these, although the vertex structure should
            // ensure they aren't used (or always multiplied with a 0 term).
            isCurve = false;
            uvScale = float2(1.0);
            coverageWidth = float2(0.0);
        }

        float2 corner    = float2(xs.xyzw[cornerID], ys.xyzw[cornerID]);
        float2 cornerCW  = float2(xs.yzwx[cornerID], ys.yzwx[cornerID]);
        float2 cornerCCW = float2(xs.wxyz[cornerID], ys.wxyz[cornerID]);
        float2 xAxis = normalize(corner - cornerCW);
        float2 yAxis = normalize(corner - cornerCCW);

        // Additional transform from the local corner space to the standard local coordinates
        float2x2 basis = float2x2(xAxis, yAxis);
        float2 translate = corner - basis*cornerRadii;
        float2x2 invBasis = inverse(basis);

        float2 localPos;

        float2 scale = cornerRadii + strokeRadius*strokeControl;
        float2 p = scale*(position + joinScale*mirrorOffset*position.yx);
        if (innerVertex) {
            // An inset, so check for and avoid self-intersections
            float2 maxInset = scale - localAARadius;
            if (any(lessThan(maxInset, float2(0.0)))) {
                p = min(maxInset, float2(0.0));
                normal = position.xy; // a miter's normal is equal to its initial position
            } else {
                p -= localAARadius * normal;
            }
        } else if (strokeControl == 0.0 && joinScale == kMiterScale) {
            // TODO: When the sc=0 vertices are removed, this branch can be removed.
            normal = position.xy; // correct vertex attribute that was set for bevel/round cases
        }

        float sx = (scale.y + kEpsilon) / (scale.x + kEpsilon);
        normal = transpose(invBasis) * float2(sx * normal.x, normal.y);

        // Orient and place p relative to the corner's location in the local rectangle
        if (snapToCenter) {
            localPos = center.xy;
            uv = invBasis * (localPos - translate) * uvScale;

            // We can't just use 'corner' since for bevels, corner isn't actually on the line that
            // we're computing distance to. So this reconstructs the base vertex before any stroking
            float2 edgePoint = basis*(cornerRadii * (position + joinScale*mirrorOffset*position.yx))
                    + translate;

            float c = -dot(normal, edgePoint);
            edgeDistance = -(dot(normal, localPos) + c);
            // TODO: This can be simplified when (sc=0) verts are removed from the mesh.
            // +D for fills, or (strokeRadius - D) for strokes
            edgeDistance = innerCoverageDirection*edgeDistance
                    + 0.5*(1 - innerCoverageDirection)*(strokeRadius);
        } else {
            localPos = basis*p + translate;
            uv = p * uvScale;

            if (strokeControl > 0) {
                // We are on the outside of the fill or stroke, so distance is always 0
                edgeDistance = 0.0;
            } else {
                // The vertex is either the center of the stroke or the inner edge of the stroke,
                // so we calculate a local distance and then transform to device-space before it's
                // interpolated by the GPU.
                // TODO: This can be simplified when (sc=0) verts are removed from the mesh.
                // == strokeRadius when at the center, == -localAARadius for inner stroke,
                // == +localAARadius for a filled inset.
                edgeDistance = strokeRadius - (innerCoverageDirection * strokeControl)
                        * (strokeRadius + localAARadius);
            }
        }

        // Some vertices are adjacent to both linear and per-pixel coverage triangles, so we
        // always calculate a valid uv and jacobian. For pixels on the linear side, the
        // interpolation goes between two different corners so the jacobian is invalid, but it
        // is ignored and edgeDistance is used directly in that case.
        jacobian = float4(invMat0.xy, invMat1.xy);
        jacobian.xy -= invMat0.z*localPos;
        jacobian.zw -= invMat1.z*localPos;

        float2 gradient = float2(dot(normal, jacobian.xy), dot(normal, jacobian.zw));
        edgeDistance *= inversesqrt(dot(gradient, gradient));
        jacobian *= uvScale.xyxy;

        float3 devPos = float3x3(mat0, mat1, mat2)*localPos.xy1;
        if (strokeControl > 0.0 && normalScale > 0.0) {
            // Note that when there's no perspective, the jacobian is equivalent to the normal
            // matrix (inverse transpose), but produces correct results when there is perspective
            // because it accounts for the position's influence on a line's projected direction.
            float2 nx = normal.x * jacobian.xz;
            float2 ny = normal.y * jacobian.yw;

            if (joinScale == kMiterScale && all(greaterThan(normal, float2(0.0)))) {
                // Produce a bisecting vector in device space.
                nx = normalize(nx);
                ny = normalize(ny);
                if (dot(nx, ny) < -0.8) {
                    // Normals are in nearly opposite directions, so adjust to avoid float error.
                    float s = sign(cross_length_2d(nx, ny));
                    nx =  s*ortho(nx);
                    ny = -s*ortho(ny);
                }
            }
            // Adding the normal components together directly results in what we'd have
            // calculated if we'd just transformed 'normal' in one go, assuming they weren't
            // normalized in the if-block above. If they were normalized, the sum equals the
            // bisector between the original nx and ny.
            //
            // We multiply by W so that after perspective division the new point is offset by the
            // normal.
            devPos.xy += devPos.z * normalize(nx + ny);
            // By construction these points are 1px away from the original edge, but we multiply
            // by W since to get screen-space linear interpolation.
            edgeDistance = -devPos.z;
        }

        // Write out final results
        stepLocalCoords = localPos;
        float4 devPosition = float4(devPos.xy, devPos.z*depth, devPos.z);
        perPixelControl = isCurve ? 1.0 : 0.0;
    )";
}

const char* AnalyticRRectRenderStep::fragmentCoverageSkSL() const {
    // TODO: Actually implement this for linear edges (that get clamp a varying to [0,1]) and for
    // corners calculating distance to an ellipse.
    return R"(
        float c;
        if (perPixelControl > 0.0 && uv.x >= 0.0 && uv.y >= 0.0) {
            // Inside a triangle that covers the curve, so we coverage is non-linear.
            float2 gradient = float2(dot(uv, jacobian.xy), dot(uv, jacobian.zw));
            float invGradLength = 0.5 * inversesqrt(dot(gradient, gradient)) * sk_FragCoord.w;
            float f = dot(uv, uv) - 1.0;
            float width = 2 * coverageWidth.x * coverageWidth.y;

            // This is the coverage to the outside of the rounded corner's edge, including any
            // outset by stroke radius and the extra 1px linear coverage ramp surrounding the shape,
            // which is necessary when pixels are near the tangents to the circle.
            // TODO: This doesn't differentiate negative edge distances from stroke insets. Under
            // extreme perspective these can overlap with the outer curve and mess up the coverage
            // calculation. When edgeDistance is a float2, revisit to see if we can identify the
            // outset triangles w/o any additional data passed to the FS. Outset triangles are the
            // only case where we actually want to add the edgeDistance to f, and min(d,0) is a
            // faulty proxy for that.
            c = clamp(0.5 - (f - width) * invGradLength
                      + min(edgeDistance*sk_FragCoord.w, 0.0), 0.0, 1.0);
            if (coverageWidth.x >= coverageWidth.y) {
                // There is an interior circular edge that needs to be evaluated as well.
                float innerC = clamp(0.5 + (f + width) * invGradLength, 0.0, 1.0);
                c = min(c, innerC);
            }
        } else {
            // Inside a triangle that covers a linear section, so coverage is linear and we just
            // need to adjust from edge distance to [0, 1].
            // We multiply by sk_FragCoord.w (really 1/w) to either adjust the distance to linear
            // (for outside edge triangles), or to account for W in the length of the gradient we
            // had earlier divided by.
            // FIXME hairlines need to add 1.0 instead...
            c = clamp(edgeDistance*sk_FragCoord.w + 0.5, 0.0, 1.0);
        }
        outputCoverage = half4(c);
    )";
}

void AnalyticRRectRenderStep::writeVertices(DrawWriter* writer,
                                           const DrawParams& params,
                                           int ssboIndex) const {
    SkASSERT(params.geometry().isShape());
    const Shape& shape = params.geometry().shape();

    BindBufferInfo vertices =
        writer->bufferManager()->getStaticBuffer(BufferType::kVertex,
            writeVertexBuffer, vertexBufferSize);
    BindBufferInfo indices = writer->bufferManager()->getStaticBuffer(BufferType::kIndex,
                                                                      writeIndexBuffer,
                                                                      indexBufferSize);
    DrawWriter::Instances instance{*writer, vertices, indices, kIndexCount};
    auto vw = instance.append(1);

    // The bounds of a rect is the rect, and the bounds of a rrect is tight (== SkRRect::getRect()).
    Rect bounds = params.geometry().bounds();

    // aaRadius will be set to a negative value to signal a complex self-intersection that has to
    // be calculated in the vertex shader.
    float aaRadius = localAARadius(params.transform(), bounds);
    float centerWeight;

    if (params.isStroke()) {
        SkASSERT(params.strokeStyle().halfWidth() >= 0.f);
        SkASSERT(shape.isRect() ||
                 (shape.isRRect() && SkRRectPriv::AllCornersCircular(shape.rrect())));

        const float strokeRadius = params.strokeStyle().halfWidth();
        skvx::float2 innerGap = bounds.size() - 2.f * params.strokeStyle().halfWidth();
        if (any(innerGap <= 0.f)) {
            centerWeight = kSolidInterior;

            // For strokes that overlap so much the interior is solid, we move the inset vertices
            // to match the "filled" cases.
            if (shape.isRRect()) {
                // Check if insets from the outer curve would also overlap
                // TODO: Use -halfWidth() for stroke radius when (sc=0) vertices are removed.
                if (cornerInsetsIntersect(shape.rrect(), 0.f, aaRadius)) {
                    aaRadius = kComplexAAInsets;
                } // else VS will adjust stroke-control attribute to place at outer curve+aa inset.
            } else {
                // Insets for quads/filled-rects are always at the center
                aaRadius = kComplexAAInsets; // Match quad case instead of filled rrect case
            }
        } else {
            if (any(innerGap <= 2.f * aaRadius) ||
                (shape.isRRect() && cornerInsetsIntersect(shape.rrect(), strokeRadius, aaRadius))) {
                // When the insets intersect we separate the inner vertices from the center vertices
                // by placing the inner vertices on the base inner curve w/o the AA inset. However,
                // if the inner curves would intersect then we switch to a complex interior.
                centerWeight = kFilledStrokeInterior;
                if (shape.isRRect() && cornerInsetsIntersect(shape.rrect(), strokeRadius, 0.f)) {
                    aaRadius = kComplexAAInsets;
                } else {
                    aaRadius = 0.f;
                }
            } else {
                centerWeight = kStrokeInterior;
            }
        }

        skvx::float4 cornerRadii;
        if (shape.isRRect()) {
            // X and Y radii are the same, but each corner could be different. Take X arbitrarily.
            cornerRadii = loadXRadii(shape.rrect());
        } else {
            // All four corner radii are 0s for a rectangle
            cornerRadii = 0.f;
        }

        // Write a negative value outside [-1, 0] to signal a stroked shape, then the style params,
        // followed by corner radii and bounds.
        vw << -2.f << strokeRadius << params.strokeStyle().joinLimit() << /*unused*/0.f
           << cornerRadii << bounds.ltrb();
    } else {
        // TODO: Add quadrilateral support to Shape with per-edge flags.
        if (shape.isRect() || (shape.isRRect() && shape.rrect().isRect())) {
            // Rectangles (or rectangles embedded in an SkRRect) are converted to the quadrilateral
            // case, but with all edges anti-aliased (== -1).
            skvx::float4 ltrb = bounds.ltrb();
            vw << /*edge flags*/ skvx::float4(-1.f)
               << /*xs*/ skvx::shuffle<0,2,2,0>(ltrb)
               << /*ys*/ skvx::shuffle<1,1,3,3>(ltrb);
            // For simplicity, it's assumed arbitrary quad insets could self-intersect, so force
            // all interior vertices to the center.
            centerWeight = kSolidInterior;
            aaRadius = kComplexAAInsets;
        } else {
            // A filled rounded rectangle
            const SkRRect& rrect = shape.rrect();
            SkASSERT(any(loadXRadii(rrect) > 0.f)); // If not, the shader won't detect this case

            vw << loadXRadii(rrect) << loadYRadii(rrect) << bounds.ltrb();
            centerWeight = kSolidInterior;
            if (cornerInsetsIntersect(rrect, 0.f, aaRadius)) {
                aaRadius = kComplexAAInsets;
            }
        }
    }

    // All instance types share the remaining instance attribute definitions
    const SkM44& m = params.transform().matrix();
    const SkM44& invM = params.transform().inverse();
    vw << bounds.center() << centerWeight << aaRadius
       << params.order().depthAsFloat()
       << ssboIndex
       << m.rc(0,0) << m.rc(1,0) << m.rc(3,0) // mat0
       << m.rc(0,1) << m.rc(1,1) << m.rc(3,1) // mat1
       << m.rc(0,3) << m.rc(1,3) << m.rc(3,3) // mat2
       << invM.rc(0,0) << invM.rc(1,0) << invM.rc(3,0)  // invMat0
       << invM.rc(0,1) << invM.rc(1,1) << invM.rc(3,1); // invMat1
}

void AnalyticRRectRenderStep::writeUniformsAndTextures(const DrawParams&,
                                                       PipelineDataGatherer*) const {
    // All data is uploaded as instance attributes, so no uniforms are needed.
}

}  // namespace skgpu::graphite
