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
    // df/du = (dx/du*w - x*dw/du)/w^2 = (m00*w - m30*x)/w^2
    // df/dv = (dx/dv*w - x*dw/dv)/w^2 = (m01*w - m31*x)/w^2
    // dg/du = (dy/du*w - y*dw/du)/w^2 = (m10*w - m30*y)/w^2
    // dg/dv = (dy/dv*w - y*dw/du)/w^2 = (m11*w - m31*y)/w^2
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

static bool cornerInsetsIntersect(const SkRRect& rrect, float maxInset) {
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
    float fStrokeControl;
    float fMirrorOffset;
    float fCenterWeight;
};

// Allowed values for the center weight instance value (selected at record time based on style
// and transform), and are defined such that when (insance-weight > vertex-weight) is true, the
// vertex should be snapped to the center instead of its regular calculation.
static constexpr float kDontSnapToCenter = 0.f;
static constexpr float kFillCenter = 1.f;
static constexpr float kInsetsIntersect = 2.f;

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

    // Allowed values for each vertex's center weight (assuming only the allowed instance center
    // weights are used, defined earlier).
    static constexpr float kNeverSnapToCenter = 2.f;
    static constexpr float kSnapIfInsetsIntersect = 1.f;
    static constexpr float kSnapForFills = 0.f;
    static constexpr float kHR2 = 0.5f * SK_FloatSqrt2; // "half root 2"

    // This template is repeated 4 times in the vertex buffer, for each of the four corners.
    // The vertex ID is used to determine which corner the normalized position is transformed to.
    static constexpr Vertex kCornerTemplate[kCornerVertexCount] = {
        // Device-space AA outsets from outer curve
        { {1.0f, 0.0f}, { 1.0f,  0.0f}, kOuterStroke,  kNoOffset,     kNeverSnapToCenter },
        { {1.0f, 0.0f}, { 1.0f,  0.0f}, kOuterStroke,  kMirrorOffset, kNeverSnapToCenter },
        { {1.0f, 0.0f}, { kHR2,  kHR2}, kOuterStroke,  kMirrorOffset, kNeverSnapToCenter },
        { {0.0f, 1.0f}, { kHR2,  kHR2}, kOuterStroke,  kMirrorOffset, kNeverSnapToCenter },
        { {0.0f, 1.0f}, { 0.0f,  1.0f}, kOuterStroke,  kMirrorOffset, kNeverSnapToCenter },
        { {0.0f, 1.0f}, { 0.0f,  1.0f}, kOuterStroke,  kNoOffset,     kNeverSnapToCenter },

        // Outer anchors (no local or device-space normal outset)
        { {1.0f, 0.0f}, { 0.0f,  0.0f}, kOuterStroke,  kNoOffset,     kNeverSnapToCenter },
        { {1.0f, 0.0f}, { 0.0f,  0.0f}, kOuterStroke,  kMirrorOffset, kNeverSnapToCenter },
        { {0.0f, 1.0f}, { 0.0f,  0.0f}, kOuterStroke,  kMirrorOffset, kNeverSnapToCenter },
        { {0.0f, 1.0f}, { 0.0f,  0.0f}, kOuterStroke,  kNoOffset,     kNeverSnapToCenter },

        // Center of stroke (equivalent to outer anchors when filling)
        { {1.0f, 0.0f}, { 0.0f,  0.0f}, kCenterStroke, kNoOffset,     kNeverSnapToCenter },
        { {1.0f, 0.0f}, { 0.0f,  0.0f}, kCenterStroke, kMirrorOffset, kNeverSnapToCenter },
        { {0.0f, 1.0f}, { 0.0f,  0.0f}, kCenterStroke, kMirrorOffset, kNeverSnapToCenter },
        { {0.0f, 1.0f}, { 0.0f,  0.0f}, kCenterStroke, kNoOffset,     kNeverSnapToCenter },

        // Inner AA insets from inner curve
        { {1.0f, 0.0f}, {-1.0f,  0.0f}, kInnerStroke,  kNoOffset,     kSnapIfInsetsIntersect },
        { {0.5f, 0.5f}, {-kHR2, -kHR2}, kInnerStroke,  kMirrorOffset, kSnapIfInsetsIntersect },
        { {0.0f, 1.0f}, { 0.0f, -1.0f}, kInnerStroke,  kNoOffset,     kSnapIfInsetsIntersect },

        // Center filling vertices (equal to inner AA insets unless instance weight = kFillCenter)
        { {0.5f, 0.5f}, {-kHR2, -kHR2}, kInnerStroke,  kMirrorOffset, kSnapForFills },
        { {0.0f, 1.0f}, { 0.0f, -1.0f}, kInnerStroke,  kNoOffset,     kSnapForFills },
    };

    writer << kCornerTemplate  // TL
           << kCornerTemplate  // TR
           << kCornerTemplate  // BR
           << kCornerTemplate; // BL
}

AnalyticRRectRenderStep::AnalyticRRectRenderStep()
        : RenderStep("AnalyticRRectRenderStep",
                     "",
                     Flags::kPerformsShading |
                     Flags::kEmitsCoverage,
                     /*uniforms=*/{},
                     PrimitiveType::kTriangleStrip,
                     kDirectDepthGreaterPass,
                     /*vertexAttrs=*/{
                            {"position", VertexAttribType::kFloat2, SkSLType::kFloat2},
                            {"normal", VertexAttribType::kFloat2, SkSLType::kFloat2},
                            {"strokeControl", VertexAttribType::kFloat, SkSLType::kFloat},
                            {"mirrorOffset", VertexAttribType::kFloat, SkSLType::kFloat},
                            {"centerWeight", VertexAttribType::kFloat, SkSLType::kFloat}
                     },
                     /*instanceAttrs=*/
                            {{"xRadiiOrFlags", VertexAttribType::kFloat4, SkSLType::kFloat4},
                             {"radiiOrQuadXs", VertexAttribType::kFloat4, SkSLType::kFloat4},
                             {"ltrbOrQuadYs", VertexAttribType::kFloat4, SkSLType::kFloat4},
                             // xy stores center of rrect in local coords, z stores a control value
                             // added to each vertex's centerWeight to handle snapping when needed.
                             // w stores the local AA radius calculated from the full transform.
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
                     /*varyings=*/{}) {}

AnalyticRRectRenderStep::~AnalyticRRectRenderStep() {}

const char* AnalyticRRectRenderStep::vertexSkSL() const {
    // TODO: Move this into a module
    return R"(
        const float kMiterScale = 1.0;
        const float kBevelScale = 0.0;
        const float kRoundScale = 0.41421356237; // sqrt(2)-1

        int cornerID = sk_VertexID / 19; // KEEP IN SYNC WITH kCornerVertexCount

        float4 xs, ys; // should be BR, TR, TL, BL
        float2 cornerRadii = float2(0.0);
        float strokeRadius = 0.0; // fill and hairline are differentiated by center weighting
        if (xRadiiOrFlags.x < -1.0) {
            // Stroked rect or round rect
            strokeRadius = xRadiiOrFlags.y;
            cornerRadii = float2(radiiOrQuadXs[cornerID]);  // strokes require circular corners
            xs = ltrbOrQuadYs.LRRL;
            ys = ltrbOrQuadYs.TTBB;
        } else if (any(greaterThan(xRadiiOrFlags, float4(0.0)))) {
            // Filled round rect
            cornerRadii = float2(xRadiiOrFlags[cornerID], radiiOrQuadXs[cornerID]);
            xs = ltrbOrQuadYs.LRRL;
            ys = ltrbOrQuadYs.TTBB;
        } else {
            // Per-edge quadrilateral, so we have to calculate the corner's basis from the
            // quad's edges.
            xs = radiiOrQuadXs;
            ys = ltrbOrQuadYs;
        }

        float2 corner    = float2(xs.xyzw[cornerID], ys.xyzw[cornerID]);
        float2 cornerCW  = float2(xs.yzwx[cornerID], ys.yzwx[cornerID]);
        float2 cornerCCW = float2(xs.wxyz[cornerID], ys.wxyz[cornerID]);
        float2 xAxis = normalize(corner - cornerCW);
        float2 yAxis = normalize(corner - cornerCCW);

        // Determine the amount of mirror offsetting to apply based on fill/hairline/stroke join
        // TODO should this analysis be done on the CPU and we upload 4 joinScales instead of
        // 1 join style that has to be combined with the per-corner radii?
        float joinScale;
        if (cornerRadii.x > 0.000124 || cornerRadii.y > 0.000124) {
            // A rounded corner is always rounded regardless of style
            joinScale = kRoundScale;
        } else if (strokeRadius > 0.0 && xRadiiOrFlags.z <= 0.0) {
            // A rect corner of a non-hairline stroke changes based on the join type
            joinScale = xRadiiOrFlags.z == 0.0 ? kBevelScale : kRoundScale;
        } else {
            // A filled or hairline rect corner is always mitered
            joinScale = kMiterScale;
        }

        float2 scale = cornerRadii + strokeRadius*strokeControl;
        float2 localPos;
        if (center.z > centerWeight) {
            // It's either a fill and this vertex is designated to fill to the center, or it's a
            // self-intersecting stroke that snaps to the center so geometry stays well-defined.
            localPos = center.xy;
        } else {
            float2 p = scale*(position + joinScale*mirrorOffset*position.yx);

            if (strokeControl < 0.0) {
                // An inset, so check for and avoid self-intersections
                float localAARadius = center.w;
                float2 maxInset = scale - localAARadius;
                if (any(lessThan(maxInset, float2(0.0)))) {
                    p = min(maxInset, float2(0.0));
                } else {
                    p += localAARadius * normal;
                }
            }

            // Orient and place p relative to the corner's location in the local rectangle
            localPos = float2x2(xAxis, yAxis)*(p - cornerRadii) + corner;
        }

        float3 devPos = float3x3(mat0, mat1, mat2)*localPos.xy1;
        if (strokeControl > 0.0 && (normal.x > 0.0 || normal.y > 0.0)) {
            // We need a device-space normal added to devPos. Transform the local normal by the
            // normal matrix (A^-1)^T where A = M*T(corner)*scale; but since we know the structure
            // of T(corner)*scale and that we're in 2D, we can skip computing the entire matrix.
            float3 localNx = calc_line_eq(-yAxis, corner);
            float3 localNy = calc_line_eq( xAxis, corner);

            float sx = (scale.y + 0.000124) / (scale.x + 0.000124);
            float2 nx = sx * normal.x * float2(dot(invMat0, localNx), dot(invMat1, localNx));
            float2 ny =      normal.y * float2(dot(invMat0, localNy), dot(invMat1, localNy));

            if (joinScale == 1.0 && all(greaterThan(normal, float2(0.0)))) {
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
        }

        // Write out final results
        stepLocalCoords = localPos;
        float4 devPosition = float4(devPos.xy, devPos.z*depth, devPos.z);
    )";
}

const char* AnalyticRRectRenderStep::fragmentCoverageSkSL() const {
    // TODO: Actually implement this for linear edges (that get clamp a varying to [0,1]) and for
    // corners calculating distance to an ellipse.
    return R"(
        outputCoverage = half4(1.0);
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

    // The bounds of a rect is the rect, and the bounds of a rrect is tight
    Rect bounds = params.geometry().bounds();
    const float aaRadius = localAARadius(params.transform(), bounds);
    float centerWeight;

    if (params.isStroke()) {
        SkASSERT(params.strokeStyle().halfWidth() >= 0.f);
        SkASSERT(shape.isRect() ||
                 (shape.isRRect() && SkRRectPriv::AllCornersCircular(shape.rrect())));

        const float maxInset = aaRadius + params.strokeStyle().halfWidth();
        bool insetsIntersect = any(2.f * maxInset >= bounds.size());

        skvx::float4 cornerRadii;
        if (shape.isRRect()) {
            // X and Y radii are the same, but each corner could be different. Take X arbitrarily.
            cornerRadii = loadXRadii(shape.rrect());
            insetsIntersect |= cornerInsetsIntersect(shape.rrect(), maxInset);
        } else {
            // All four corner radii are 0s for a rectangle
            cornerRadii = 0.f;
        }

        // Write a negative value outside [-1, 0] to signal a stroked shape, then the style params,
        // followed by corner radii and bounds.
        vw << -2.f << params.strokeStyle().halfWidth() << params.strokeStyle().joinLimit() << 0.f
           << cornerRadii << bounds.ltrb();
        centerWeight = insetsIntersect ? kInsetsIntersect : kDontSnapToCenter;
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
            centerWeight = kInsetsIntersect;
        } else {
            // A filled rounded rectangle
            const SkRRect& rrect = shape.rrect();
            SkASSERT(any(loadXRadii(rrect) > 0.f)); // If not, the shader won't detect this case

            vw << loadXRadii(rrect) << loadYRadii(rrect) << bounds.ltrb();
            centerWeight = cornerInsetsIntersect(rrect, aaRadius) ? kInsetsIntersect : kFillCenter;
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
