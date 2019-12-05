/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/geometry/GrQuadUtils.h"

#include "include/core/SkRect.h"
#include "include/private/GrTypesPriv.h"
#include "include/private/SkVx.h"
#include "src/gpu/geometry/GrQuad.h"

using V4f = skvx::Vec<4, float>;
using M4f = skvx::Vec<4, int32_t>;

#define AI SK_ALWAYS_INLINE

static constexpr float kTolerance = 1e-2f;

// These rotate the points/edge values either clockwise or counterclockwise assuming tri strip
// order.
static AI V4f next_cw(const V4f& v) {
    return skvx::shuffle<2, 0, 3, 1>(v);
}

static AI V4f next_ccw(const V4f& v) {
    return skvx::shuffle<1, 3, 0, 2>(v);
}

// Replaces zero-length 'bad' edge vectors with the reversed opposite edge vector.
// e3 may be null if only 2D edges need to be corrected for.
static AI void correct_bad_edges(const M4f& bad, V4f* e1, V4f* e2, V4f* e3) {
    if (any(bad)) {
        // Want opposite edges, L B T R -> R T B L but with flipped sign to preserve winding
        *e1 = if_then_else(bad, -skvx::shuffle<3, 2, 1, 0>(*e1), *e1);
        *e2 = if_then_else(bad, -skvx::shuffle<3, 2, 1, 0>(*e2), *e2);
        if (e3) {
            *e3 = if_then_else(bad, -skvx::shuffle<3, 2, 1, 0>(*e3), *e3);
        }
    }
}

// Replace 'bad' coordinates by rotating CCW to get the next point. c3 may be null for 2D points.
static AI void correct_bad_coords(const M4f& bad, V4f* c1, V4f* c2, V4f* c3) {
    if (any(bad)) {
        *c1 = if_then_else(bad, next_ccw(*c1), *c1);
        *c2 = if_then_else(bad, next_ccw(*c2), *c2);
        if (c3) {
            *c3 = if_then_else(bad, next_ccw(*c3), *c3);
        }
    }
}

// Since the local quad may not be type kRect, this uses the opposites for each vertex when
// interpolating, and calculates new ws in addition to new xs, ys.
static void interpolate_local(float alpha, int v0, int v1, int v2, int v3,
                              float lx[4], float ly[4], float lw[4]) {
    SkASSERT(v0 >= 0 && v0 < 4);
    SkASSERT(v1 >= 0 && v1 < 4);
    SkASSERT(v2 >= 0 && v2 < 4);
    SkASSERT(v3 >= 0 && v3 < 4);

    float beta = 1.f - alpha;
    lx[v0] = alpha * lx[v0] + beta * lx[v2];
    ly[v0] = alpha * ly[v0] + beta * ly[v2];
    lw[v0] = alpha * lw[v0] + beta * lw[v2];

    lx[v1] = alpha * lx[v1] + beta * lx[v3];
    ly[v1] = alpha * ly[v1] + beta * ly[v3];
    lw[v1] = alpha * lw[v1] + beta * lw[v3];
}

// Crops v0 to v1 based on the clipDevRect. v2 is opposite of v0, v3 is opposite of v1.
// It is written to not modify coordinates if there's no intersection along the edge.
// Ideally this would have been detected earlier and the entire draw is skipped.
static bool crop_rect_edge(const SkRect& clipDevRect, int v0, int v1, int v2, int v3,
                           float x[4], float y[4], float lx[4], float ly[4], float lw[4]) {
    SkASSERT(v0 >= 0 && v0 < 4);
    SkASSERT(v1 >= 0 && v1 < 4);
    SkASSERT(v2 >= 0 && v2 < 4);
    SkASSERT(v3 >= 0 && v3 < 4);

    if (SkScalarNearlyEqual(x[v0], x[v1])) {
        // A vertical edge
        if (x[v0] < clipDevRect.fLeft && x[v2] >= clipDevRect.fLeft) {
            // Overlapping with left edge of clipDevRect
            if (lx) {
                float alpha = (x[v2] - clipDevRect.fLeft) / (x[v2] - x[v0]);
                interpolate_local(alpha, v0, v1, v2, v3, lx, ly, lw);
            }
            x[v0] = clipDevRect.fLeft;
            x[v1] = clipDevRect.fLeft;
            return true;
        } else if (x[v0] > clipDevRect.fRight && x[v2] <= clipDevRect.fRight) {
            // Overlapping with right edge of clipDevRect
            if (lx) {
                float alpha = (clipDevRect.fRight - x[v2]) / (x[v0] - x[v2]);
                interpolate_local(alpha, v0, v1, v2, v3, lx, ly, lw);
            }
            x[v0] = clipDevRect.fRight;
            x[v1] = clipDevRect.fRight;
            return true;
        }
    } else {
        // A horizontal edge
        SkASSERT(SkScalarNearlyEqual(y[v0], y[v1]));
        if (y[v0] < clipDevRect.fTop && y[v2] >= clipDevRect.fTop) {
            // Overlapping with top edge of clipDevRect
            if (lx) {
                float alpha = (y[v2] - clipDevRect.fTop) / (y[v2] - y[v0]);
                interpolate_local(alpha, v0, v1, v2, v3, lx, ly, lw);
            }
            y[v0] = clipDevRect.fTop;
            y[v1] = clipDevRect.fTop;
            return true;
        } else if (y[v0] > clipDevRect.fBottom && y[v2] <= clipDevRect.fBottom) {
            // Overlapping with bottom edge of clipDevRect
            if (lx) {
                float alpha = (clipDevRect.fBottom - y[v2]) / (y[v0] - y[v2]);
                interpolate_local(alpha, v0, v1, v2, v3, lx, ly, lw);
            }
            y[v0] = clipDevRect.fBottom;
            y[v1] = clipDevRect.fBottom;
            return true;
        }
    }

    // No overlap so don't crop it
    return false;
}

// Updates x and y to intersect with clipDevRect.  lx, ly, and lw are updated appropriately and may
// be null to skip calculations. Returns bit mask of edges that were clipped.
static GrQuadAAFlags crop_rect(const SkRect& clipDevRect, float x[4], float y[4],
                               float lx[4], float ly[4], float lw[4]) {
    GrQuadAAFlags clipEdgeFlags = GrQuadAAFlags::kNone;

    // The quad's left edge may not align with the SkRect notion of left due to 90 degree rotations
    // or mirrors. So, this processes the logical edges of the quad and clamps it to the 4 sides of
    // clipDevRect.

    // Quad's left is v0 to v1 (op. v2 and v3)
    if (crop_rect_edge(clipDevRect, 0, 1, 2, 3, x, y, lx, ly, lw)) {
        clipEdgeFlags |= GrQuadAAFlags::kLeft;
    }
    // Quad's top edge is v0 to v2 (op. v1 and v3)
    if (crop_rect_edge(clipDevRect, 0, 2, 1, 3, x, y, lx, ly, lw)) {
        clipEdgeFlags |= GrQuadAAFlags::kTop;
    }
    // Quad's right edge is v2 to v3 (op. v0 and v1)
    if (crop_rect_edge(clipDevRect, 2, 3, 0, 1, x, y, lx, ly, lw)) {
        clipEdgeFlags |= GrQuadAAFlags::kRight;
    }
    // Quad's bottom edge is v1 to v3 (op. v0 and v2)
    if (crop_rect_edge(clipDevRect, 1, 3, 0, 2, x, y, lx, ly, lw)) {
        clipEdgeFlags |= GrQuadAAFlags::kBottom;
    }

    return clipEdgeFlags;
}

// Similar to crop_rect, but assumes that both the device coordinates and optional local coordinates
// geometrically match the TL, BL, TR, BR vertex ordering, i.e. axis-aligned but not flipped, etc.
static GrQuadAAFlags crop_simple_rect(const SkRect& clipDevRect, float x[4], float y[4],
                                      float lx[4], float ly[4]) {
    GrQuadAAFlags clipEdgeFlags = GrQuadAAFlags::kNone;

    // Update local coordinates proportionately to how much the device rect edge was clipped
    const SkScalar dx = lx ? (lx[2] - lx[0]) / (x[2] - x[0]) : 0.f;
    const SkScalar dy = ly ? (ly[1] - ly[0]) / (y[1] - y[0]) : 0.f;
    if (clipDevRect.fLeft > x[0]) {
        if (lx) {
            lx[0] += (clipDevRect.fLeft - x[0]) * dx;
            lx[1] = lx[0];
        }
        x[0] = clipDevRect.fLeft;
        x[1] = clipDevRect.fLeft;
        clipEdgeFlags |= GrQuadAAFlags::kLeft;
    }
    if (clipDevRect.fTop > y[0]) {
        if (ly) {
            ly[0] += (clipDevRect.fTop - y[0]) * dy;
            ly[2] = ly[0];
        }
        y[0] = clipDevRect.fTop;
        y[2] = clipDevRect.fTop;
        clipEdgeFlags |= GrQuadAAFlags::kTop;
    }
    if (clipDevRect.fRight < x[2]) {
        if (lx) {
            lx[2] -= (x[2] - clipDevRect.fRight) * dx;
            lx[3] = lx[2];
        }
        x[2] = clipDevRect.fRight;
        x[3] = clipDevRect.fRight;
        clipEdgeFlags |= GrQuadAAFlags::kRight;
    }
    if (clipDevRect.fBottom < y[1]) {
        if (ly) {
            ly[1] -= (y[1] - clipDevRect.fBottom) * dy;
            ly[3] = ly[1];
        }
        y[1] = clipDevRect.fBottom;
        y[3] = clipDevRect.fBottom;
        clipEdgeFlags |= GrQuadAAFlags::kBottom;
    }

    return clipEdgeFlags;
}
// Consistent with GrQuad::asRect()'s return value but requires fewer operations since we don't need
// to calculate the bounds of the quad.
static bool is_simple_rect(const GrQuad& quad) {
    if (quad.quadType() != GrQuad::Type::kAxisAligned) {
        return false;
    }
    // v0 at the geometric top-left is unique, so we only need to compare x[0] < x[2] for left
    // and y[0] < y[1] for top, but add a little padding to protect against numerical precision
    // on R90 and R270 transforms tricking this check.
    return ((quad.x(0) + SK_ScalarNearlyZero) < quad.x(2)) &&
           ((quad.y(0) + SK_ScalarNearlyZero) < quad.y(1));
}

// Calculates barycentric coordinates for each point in (testX, testY) in the triangle formed by
// (x0,y0) - (x1,y1) - (x2, y2) and stores them in u, v, w.
static bool barycentric_coords(float x0, float y0, float x1, float y1, float x2, float y2,
                               const V4f& testX, const V4f& testY,
                               V4f* u, V4f* v, V4f* w) {
    // Modeled after SkPathOpsQuad::pointInTriangle() but uses float instead of double, is
    // vectorized and outputs normalized barycentric coordinates instead of inside/outside test
    float v0x = x2 - x0;
    float v0y = y2 - y0;
    float v1x = x1 - x0;
    float v1y = y1 - y0;

    float dot00 = v0x * v0x + v0y * v0y;
    float dot01 = v0x * v1x + v0y * v1y;
    float dot11 = v1x * v1x + v1y * v1y;

    // Not yet 1/d, first check d != 0 with a healthy tolerance (worst case is we end up not
    // cropping something we could have, which is better than cropping something we shouldn't have).
    // The tolerance is partly so large because these comparisons operate in device px^4 units,
    // with plenty of subtractions thrown in. The SkPathOpsQuad code's use of doubles helped, and
    // because it only needed to return "inside triangle", it could compare against [0, denom] and
    // skip the normalization entirely.
    float invDenom = dot00 * dot11 - dot01 * dot01;
    static constexpr SkScalar kEmptyTriTolerance = SK_Scalar1 / (1 << 5);
    if (SkScalarNearlyZero(invDenom, kEmptyTriTolerance)) {
        // The triangle was degenerate/empty, which can cause the following UVW calculations to
        // return (0,0,1) for every test point. This in turn makes the cropping code think that the
        // empty triangle contains the crop rect and we turn the draw into a fullscreen clear, which
        // is definitely the utter opposite of what we'd expect for an empty shape.
        return false;
    } else {
        // Safe to divide
        invDenom = sk_ieee_float_divide(1.f, invDenom);
    }

    V4f v2x = testX - x0;
    V4f v2y = testY - y0;

    V4f dot02 = v0x * v2x + v0y * v2y;
    V4f dot12 = v1x * v2x + v1y * v2y;

    *u = (dot11 * dot02 - dot01 * dot12) * invDenom;
    *v = (dot00 * dot12 - dot01 * dot02) * invDenom;
    *w = 1.f - *u - *v;

    return true;
}

static M4f inside_triangle(const V4f& u, const V4f& v, const V4f& w) {
    return ((u >= 0.f) & (u <= 1.f)) & ((v >= 0.f) & (v <= 1.f)) & ((w >= 0.f) & (w <= 1.f));
}

namespace GrQuadUtils {

void ResolveAAType(GrAAType requestedAAType, GrQuadAAFlags requestedEdgeFlags, const GrQuad& quad,
                   GrAAType* outAAType, GrQuadAAFlags* outEdgeFlags) {
    // Most cases will keep the requested types unchanged
    *outAAType = requestedAAType;
    *outEdgeFlags = requestedEdgeFlags;

    switch (requestedAAType) {
        // When aa type is coverage, disable AA if the edge configuration doesn't actually need it
        case GrAAType::kCoverage:
            if (requestedEdgeFlags == GrQuadAAFlags::kNone) {
                // Turn off anti-aliasing
                *outAAType = GrAAType::kNone;
            } else {
                // For coverage AA, if the quad is a rect and it lines up with pixel boundaries
                // then overall aa and per-edge aa can be completely disabled
                if (quad.quadType() == GrQuad::Type::kAxisAligned && !quad.aaHasEffectOnRect()) {
                    *outAAType = GrAAType::kNone;
                    *outEdgeFlags = GrQuadAAFlags::kNone;
                }
            }
            break;
        // For no or msaa anti aliasing, override the edge flags since edge flags only make sense
        // when coverage aa is being used.
        case GrAAType::kNone:
            *outEdgeFlags = GrQuadAAFlags::kNone;
            break;
        case GrAAType::kMSAA:
            *outEdgeFlags = GrQuadAAFlags::kAll;
            break;
    }
}

bool CropToRect(const SkRect& cropRect, GrAA cropAA, GrQuadAAFlags* edgeFlags, GrQuad* quad,
                GrQuad* local) {
    SkASSERT(quad->isFinite());

    if (quad->quadType() == GrQuad::Type::kAxisAligned) {
        // crop_rect and crop_rect_simple keep the rectangles as rectangles, so the intersection
        // of the crop and quad can be calculated exactly. Some care must be taken if the quad
        // is axis-aligned but does not satisfy asRect() due to flips, etc.
        GrQuadAAFlags clippedEdges;
        if (local) {
            if (is_simple_rect(*quad) && is_simple_rect(*local)) {
                clippedEdges = crop_simple_rect(cropRect, quad->xs(), quad->ys(),
                                                local->xs(), local->ys());
            } else {
                clippedEdges = crop_rect(cropRect, quad->xs(), quad->ys(),
                                         local->xs(), local->ys(), local->ws());
            }
        } else {
            if (is_simple_rect(*quad)) {
                clippedEdges = crop_simple_rect(cropRect, quad->xs(), quad->ys(), nullptr, nullptr);
            } else {
                clippedEdges = crop_rect(cropRect, quad->xs(), quad->ys(),
                                         nullptr, nullptr, nullptr);
            }
        }

        // Apply the clipped edge updates to the original edge flags
        if (cropAA == GrAA::kYes) {
            // Turn on all edges that were clipped
            *edgeFlags |= clippedEdges;
        } else {
            // Turn off all edges that were clipped
            *edgeFlags &= ~clippedEdges;
        }
        return true;
    }

    if (local) {
        // FIXME (michaelludwig) Calculate cropped local coordinates when not kAxisAligned
        return false;
    }

    V4f devX = quad->x4f();
    V4f devY = quad->y4f();
    V4f devIW = quad->iw4f();
    // Project the 3D coordinates to 2D
    if (quad->quadType() == GrQuad::Type::kPerspective) {
        devX *= devIW;
        devY *= devIW;
    }

    V4f clipX = {cropRect.fLeft, cropRect.fLeft, cropRect.fRight, cropRect.fRight};
    V4f clipY = {cropRect.fTop, cropRect.fBottom, cropRect.fTop, cropRect.fBottom};

    // Calculate barycentric coordinates for the 4 rect corners in the 2 triangles that the quad
    // is tessellated into when drawn.
    V4f u1, v1, w1;
    V4f u2, v2, w2;
    if (!barycentric_coords(devX[0], devY[0], devX[1], devY[1], devX[2], devY[2], clipX, clipY,
                            &u1, &v1, &w1) ||
        !barycentric_coords(devX[1], devY[1], devX[3], devY[3], devX[2], devY[2], clipX, clipY,
                            &u2, &v2, &w2)) {
        // Bad triangles, skip cropping
        return false;
    }

    // clipDevRect is completely inside this quad if each corner is in at least one of two triangles
    M4f inTri1 = inside_triangle(u1, v1, w1);
    M4f inTri2 = inside_triangle(u2, v2, w2);
    if (all(inTri1 | inTri2)) {
        // We can crop to exactly the clipDevRect.
        // FIXME (michaelludwig) - there are other ways to have determined quad covering the clip
        // rect, but the barycentric coords will be useful to derive local coordinates in the future

        // Since we are cropped to exactly clipDevRect, we have discarded any perspective and the
        // type becomes kRect. If updated locals were requested, they will incorporate perspective.
        // FIXME (michaelludwig) - once we have local coordinates handled, it may be desirable to
        // keep the draw as perspective so that the hardware does perspective interpolation instead
        // of pushing it into a local coord w and having the shader do an extra divide.
        clipX.store(quad->xs());
        clipY.store(quad->ys());
        quad->ws()[0] = 1.f;
        quad->ws()[1] = 1.f;
        quad->ws()[2] = 1.f;
        quad->ws()[3] = 1.f;
        quad->setQuadType(GrQuad::Type::kAxisAligned);

        // Update the edge flags to match the clip setting since all 4 edges have been clipped
        *edgeFlags = cropAA == GrAA::kYes ? GrQuadAAFlags::kAll : GrQuadAAFlags::kNone;

        return true;
    }

    // FIXME (michaelludwig) - use the GrQuadPerEdgeAA tessellation inset/outset math to move
    // edges to the closest clip corner they are outside of

    return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// TessellationHelper implementation and helper struct implementations
///////////////////////////////////////////////////////////////////////////////////////////////////

//** EdgeVectors implementation

void TessellationHelper::EdgeVectors::reset(const skvx::Vec<4, float>& xs,
                                            const skvx::Vec<4, float>& ys,
                                            const skvx::Vec<4, float>& ws,
                                            GrQuad::Type quadType) {
    // Calculate all projected edge vector values for this quad.
    if (quadType == GrQuad::Type::kPerspective) {
        V4f iw = 1.0 / ws;
        fX2D = xs * iw;
        fY2D = ys * iw;
    } else {
        fX2D = xs;
        fY2D = ys;
    }

    fDX = next_ccw(fX2D) - fX2D;
    fDY = next_ccw(fY2D) - fY2D;
    fInvLengths = rsqrt(mad(fDX, fDX, fDY * fDY));

    // Normalize edge vectors
    fDX *= fInvLengths;
    fDY *= fInvLengths;

    // Calculate angles between vectors
    if (quadType <= GrQuad::Type::kRectilinear) {
        fCosTheta = 0.f;
        fInvSinTheta = 1.f;
    } else {
        fCosTheta = mad(fDX, next_cw(fDX), fDY * next_cw(fDY));
        // NOTE: if cosTheta is close to 1, inset/outset math will avoid the fast paths that rely
        // on thefInvSinTheta since it will approach infinity.
        fInvSinTheta = rsqrt(1.f - fCosTheta * fCosTheta);
    }
}

//** EdgeEquations implementation

void TessellationHelper::EdgeEquations::reset(const EdgeVectors& edgeVectors) {
    V4f dx = edgeVectors.fDX;
    V4f dy = edgeVectors.fDY;
    // Correct for bad edges by copying adjacent edge information into the bad component
    correct_bad_edges(edgeVectors.fInvLengths >= 1.f / kTolerance, &dx, &dy, nullptr);

    V4f c = mad(dx, edgeVectors.fY2D, -dy * edgeVectors.fX2D);
    // Make sure normals point into the shape
    V4f test = mad(dy, next_cw(edgeVectors.fX2D), mad(-dx, next_cw(edgeVectors.fY2D), c));
    if (any(test < -kTolerance)) {
        fA = -dy;
        fB = dx;
        fC = -c;
    } else {
        fA = dy;
        fB = -dx;
        fC = c;
    }
}

V4f TessellationHelper::EdgeEquations::estimateCoverage(const V4f& x2d, const V4f& y2d) const {
    // Calculate distance of the 4 inset points (px, py) to the 4 edges
    V4f d0 = mad(fA[0], x2d, mad(fB[0], y2d, fC[0]));
    V4f d1 = mad(fA[1], x2d, mad(fB[1], y2d, fC[1]));
    V4f d2 = mad(fA[2], x2d, mad(fB[2], y2d, fC[2]));
    V4f d3 = mad(fA[3], x2d, mad(fB[3], y2d, fC[3]));

    // For each point, pretend that there's a rectangle that touches e0 and e3 on the horizontal
    // axis, so its width is "approximately" d0 + d3, and it touches e1 and e2 on the vertical axis
    // so its height is d1 + d2. Pin each of these dimensions to [0, 1] and approximate the coverage
    // at each point as clamp(d0+d3, 0, 1) x clamp(d1+d2, 0, 1). For rectilinear quads this is an
    // accurate calculation of its area clipped to an aligned pixel. For arbitrary quads it is not
    // mathematically accurate but qualitatively provides a stable value proportional to the size of
    // the shape.
    V4f w = max(0.f, min(1.f, d0 + d3));
    V4f h = max(0.f, min(1.f, d1 + d2));
    return w * h;
}

int TessellationHelper::EdgeEquations::computeDegenerateQuad(const V4f& signedEdgeDistances,
                                                             V4f* x2d, V4f* y2d) const {
    // Move the edge by the signed edge adjustment.
    V4f oc = fC + signedEdgeDistances;

    // There are 6 points that we care about to determine the final shape of the polygon, which
    // are the intersections between (e0,e2), (e1,e0), (e2,e3), (e3,e1) (corresponding to the
    // 4 corners), and (e1, e2), (e0, e3) (representing the intersections of opposite edges).
    V4f denom = fA * next_cw(fB) - fB * next_cw(fA);
    V4f px = (fB * next_cw(oc) - oc * next_cw(fB)) / denom;
    V4f py = (oc * next_cw(fA) - fA * next_cw(oc)) / denom;
    correct_bad_coords(abs(denom) < kTolerance, &px, &py, nullptr);

    // Calculate the signed distances from these 4 corners to the other two edges that did not
    // define the intersection. So p(0) is compared to e3,e1, p(1) to e3,e2 , p(2) to e0,e1, and
    // p(3) to e0,e2
    V4f dists1 = px * skvx::shuffle<3, 3, 0, 0>(fA) +
                 py * skvx::shuffle<3, 3, 0, 0>(fB) +
                 skvx::shuffle<3, 3, 0, 0>(oc);
    V4f dists2 = px * skvx::shuffle<1, 2, 1, 2>(fA) +
                 py * skvx::shuffle<1, 2, 1, 2>(fB) +
                 skvx::shuffle<1, 2, 1, 2>(oc);

    // If all the distances are >= 0, the 4 corners form a valid quadrilateral, so use them as
    // the 4 points. If any point is on the wrong side of both edges, the interior has collapsed
    // and we need to use a central point to represent it. If all four points are only on the
    // wrong side of 1 edge, one edge has crossed over another and we use a line to represent it.
    // Otherwise, use a triangle that replaces the bad points with the intersections of
    // (e1, e2) or (e0, e3) as needed.
    M4f d1v0 = dists1 < kTolerance;
    M4f d2v0 = dists2 < kTolerance;
    M4f d1And2 = d1v0 & d2v0;
    M4f d1Or2 = d1v0 | d2v0;

    if (!any(d1Or2)) {
        // Every dists1 and dists2 >= kTolerance so it's not degenerate, use all 4 corners as-is
        // and use full coverage
        *x2d = px;
        *y2d = py;
        return 4;
    } else if (any(d1And2)) {
        // A point failed against two edges, so reduce the shape to a single point, which we take as
        // the center of the original quad to ensure it is contained in the intended geometry. Since
        // it has collapsed, we know the shape cannot cover a pixel so update the coverage.
        SkPoint center = {0.25f * ((*x2d)[0] + (*x2d)[1] + (*x2d)[2] + (*x2d)[3]),
                          0.25f * ((*y2d)[0] + (*y2d)[1] + (*y2d)[2] + (*y2d)[3])};
        *x2d = center.fX;
        *y2d = center.fY;
        return 1;
    } else if (all(d1Or2)) {
        // Degenerates to a line. Compare p[2] and p[3] to edge 0. If they are on the wrong side,
        // that means edge 0 and 3 crossed, and otherwise edge 1 and 2 crossed.
        if (dists1[2] < kTolerance && dists1[3] < kTolerance) {
            // Edges 0 and 3 have crossed over, so make the line from average of (p0,p2) and (p1,p3)
            *x2d = 0.5f * (skvx::shuffle<0, 1, 0, 1>(px) + skvx::shuffle<2, 3, 2, 3>(px));
            *y2d = 0.5f * (skvx::shuffle<0, 1, 0, 1>(py) + skvx::shuffle<2, 3, 2, 3>(py));
        } else {
            // Edges 1 and 2 have crossed over, so make the line from average of (p0,p1) and (p2,p3)
            *x2d = 0.5f * (skvx::shuffle<0, 0, 2, 2>(px) + skvx::shuffle<1, 1, 3, 3>(px));
            *y2d = 0.5f * (skvx::shuffle<0, 0, 2, 2>(py) + skvx::shuffle<1, 1, 3, 3>(py));
        }
        return 2;
    } else {
        // This turns into a triangle. Replace corners as needed with the intersections between
        // (e0,e3) and (e1,e2), which must now be calculated
        using V2f = skvx::Vec<2, float>;
        V2f eDenom = skvx::shuffle<0, 1>(fA) * skvx::shuffle<3, 2>(fB) -
                     skvx::shuffle<0, 1>(fB) * skvx::shuffle<3, 2>(fA);
        V2f ex = (skvx::shuffle<0, 1>(fB) * skvx::shuffle<3, 2>(oc) -
                  skvx::shuffle<0, 1>(oc) * skvx::shuffle<3, 2>(fB)) / eDenom;
        V2f ey = (skvx::shuffle<0, 1>(oc) * skvx::shuffle<3, 2>(fA) -
                  skvx::shuffle<0, 1>(fA) * skvx::shuffle<3, 2>(oc)) / eDenom;

        if (SkScalarAbs(eDenom[0]) > kTolerance) {
            px = if_then_else(d1v0, V4f(ex[0]), px);
            py = if_then_else(d1v0, V4f(ey[0]), py);
        }
        if (SkScalarAbs(eDenom[1]) > kTolerance) {
            px = if_then_else(d2v0, V4f(ex[1]), px);
            py = if_then_else(d2v0, V4f(ey[1]), py);
        }

        *x2d = px;
        *y2d = py;
        return 3;
    }
}

//** OutsetRequest implementation

void TessellationHelper::OutsetRequest::reset(const EdgeVectors& edgeVectors, GrQuad::Type quadType,
                                              const skvx::Vec<4, float>& edgeDistances) {
    fEdgeDistances = edgeDistances;

    // Based on the edge distances, determine if it's acceptable to use fInvSinTheta to
    // calculate the inset or outset geometry.
    if (quadType <= GrQuad::Type::kRectilinear) {
        // Since it's rectangular, the width (edge[1] or edge[2]) collapses if subtracting
        // (dist[0] + dist[3]) makes the new width negative (minus for inset, outsetting will
        // never be degenerate in this case). The same applies for height (edge[0] or edge[3])
        // and (dist[1] + dist[2]).
        fOutsetDegenerate = false;
        float widthChange = edgeDistances[0] + edgeDistances[3];
        float heightChange = edgeDistances[1] + edgeDistances[2];
        // (1/len > 1/(edge sum) implies len - edge sum < 0.
        fInsetDegenerate =
                (widthChange > 0.f  && edgeVectors.fInvLengths[1] > 1.f / widthChange) ||
                (heightChange > 0.f && edgeVectors.fInvLengths[0] > 1.f / heightChange);
    } else if (any(edgeVectors.fInvLengths >= 1.f / kTolerance)) {
        // Have an edge that is effectively length 0, so we're dealing with a triangle, which
        // must always go through the degenerate code path.
        fOutsetDegenerate = true;
        fInsetDegenerate = true;
    } else {
        // If possible, the corners will move +/-edgeDistances * 1/sin(theta). The entire
        // request is degenerate if 1/sin(theta) -> infinity (or cos(theta) -> 1).
        if (any(abs(edgeVectors.fCosTheta) >= 0.9f)) {
            fOutsetDegenerate = true;
            fInsetDegenerate = true;
        } else {
            // With an edge-centric view, an edge's length changes by
            // edgeDistance * cos(pi - theta) / sin(theta) for each of its corners (the second
            // corner uses ccw theta value). An edge's length also changes when its adjacent
            // edges move, in which case it's updated by edgeDistance / sin(theta)
            // (or cos(theta) for the other edge).

            // cos(pi - theta) = -cos(theta)
            V4f halfTanTheta = -edgeVectors.fCosTheta * edgeVectors.fInvSinTheta;
            V4f edgeAdjust = edgeDistances * (halfTanTheta + next_ccw(halfTanTheta)) +
                             next_ccw(edgeDistances) * next_ccw(edgeVectors.fInvSinTheta) +
                             next_cw(edgeDistances) * edgeVectors.fInvSinTheta;

            // If either outsetting (plus edgeAdjust) or insetting (minus edgeAdjust) make
            // the edge lengths negative, then it's degenerate.
            V4f threshold = 0.1f - (1.f / edgeVectors.fInvLengths);
            fOutsetDegenerate = any(edgeAdjust < threshold);
            fInsetDegenerate = any(edgeAdjust > -threshold);
        }
    }
}

//** Vertices implementation

void TessellationHelper::Vertices::reset(const GrQuad& deviceQuad, const GrQuad* localQuad) {
    // Set vertices to match the device and local quad
    fX = deviceQuad.x4f();
    fY = deviceQuad.y4f();
    fW = deviceQuad.w4f();

    if (localQuad) {
        fU = localQuad->x4f();
        fV = localQuad->y4f();
        fR = localQuad->w4f();
        fUVRCount = localQuad->hasPerspective() ? 3 : 2;
    } else {
        fUVRCount = 0;
    }
}

void TessellationHelper::Vertices::asGrQuads(GrQuad* deviceOut, GrQuad::Type deviceType,
                                             GrQuad* localOut, GrQuad::Type localType) const {
    SkASSERT(deviceOut);
    SkASSERT(fUVRCount == 0 || localOut);

    fX.store(deviceOut->xs());
    fY.store(deviceOut->ys());
    if (deviceType == GrQuad::Type::kPerspective) {
        fW.store(deviceOut->ws());
    }
    deviceOut->setQuadType(deviceType); // This sets ws == 1 when device type != perspective

    if (fUVRCount > 0) {
        fU.store(localOut->xs());
        fV.store(localOut->ys());
        if (fUVRCount == 3) {
            fR.store(localOut->ws());
        }
        localOut->setQuadType(localType);
    }
}

void TessellationHelper::Vertices::moveAlong(const EdgeVectors& edgeVectors,
                                             const V4f& signedEdgeDistances) {
    // This shouldn't be called if fInvSinTheta is close to infinity (cosTheta close to 1).
    SkASSERT(all(abs(edgeVectors.fCosTheta) < 0.9f));

    // When the projected device quad is not degenerate, the vertex corners can move
    // cornerOutsetLen along their edge and their cw-rotated edge. The vertex's edge points
    // inwards and the cw-rotated edge points outwards, hence the minus-sign.
    // The edge distances are rotated compared to the corner outsets and (dx, dy), since if
    // the edge is "on" both its corners need to be moved along their other edge vectors.
    V4f signedOutsets = -edgeVectors.fInvSinTheta * next_cw(signedEdgeDistances);
    V4f signedOutsetsCW = edgeVectors.fInvSinTheta * signedEdgeDistances;

    // x = x + outset * mask * next_cw(xdiff) - outset * next_cw(mask) * xdiff
    fX += mad(signedOutsetsCW, next_cw(edgeVectors.fDX), signedOutsets * edgeVectors.fDX);
    fY += mad(signedOutsetsCW, next_cw(edgeVectors.fDY), signedOutsets * edgeVectors.fDY);
    if (fUVRCount > 0) {
        // We want to extend the texture coords by the same proportion as the positions.
        signedOutsets *= edgeVectors.fInvLengths;
        signedOutsetsCW *= next_cw(edgeVectors.fInvLengths);
        V4f du = next_ccw(fU) - fU;
        V4f dv = next_ccw(fV) - fV;
        fU += mad(signedOutsetsCW, next_cw(du), signedOutsets * du);
        fV += mad(signedOutsetsCW, next_cw(dv), signedOutsets * dv);
        if (fUVRCount == 3) {
            V4f dr = next_ccw(fR) - fR;
            fR += mad(signedOutsetsCW, next_cw(dr), signedOutsets * dr);
        }
    }
}

void TessellationHelper::Vertices::moveTo(const V4f& x2d, const V4f& y2d, const M4f& mask) {
    // Left to right, in device space, for each point
    V4f e1x = skvx::shuffle<2, 3, 2, 3>(fX) - skvx::shuffle<0, 1, 0, 1>(fX);
    V4f e1y = skvx::shuffle<2, 3, 2, 3>(fY) - skvx::shuffle<0, 1, 0, 1>(fY);
    V4f e1w = skvx::shuffle<2, 3, 2, 3>(fW) - skvx::shuffle<0, 1, 0, 1>(fW);
    correct_bad_edges(mad(e1x, e1x, e1y * e1y) < kTolerance * kTolerance, &e1x, &e1y, &e1w);

    // // Top to bottom, in device space, for each point
    V4f e2x = skvx::shuffle<1, 1, 3, 3>(fX) - skvx::shuffle<0, 0, 2, 2>(fX);
    V4f e2y = skvx::shuffle<1, 1, 3, 3>(fY) - skvx::shuffle<0, 0, 2, 2>(fY);
    V4f e2w = skvx::shuffle<1, 1, 3, 3>(fW) - skvx::shuffle<0, 0, 2, 2>(fW);
    correct_bad_edges(mad(e2x, e2x, e2y * e2y) < kTolerance * kTolerance, &e2x, &e2y, &e2w);

    // Can only move along e1 and e2 to reach the new 2D point, so we have
    // x2d = (x + a*e1x + b*e2x) / (w + a*e1w + b*e2w) and
    // y2d = (y + a*e1y + b*e2y) / (w + a*e1w + b*e2w) for some a, b
    // This can be rewritten to a*c1x + b*c2x + c3x = 0; a * c1y + b*c2y + c3y = 0, where
    // the cNx and cNy coefficients are:
    V4f c1x = e1w * x2d - e1x;
    V4f c1y = e1w * y2d - e1y;
    V4f c2x = e2w * x2d - e2x;
    V4f c2y = e2w * y2d - e2y;
    V4f c3x = fW * x2d - fX;
    V4f c3y = fW * y2d - fY;

    // Solve for a and b
    V4f a, b, denom;
    if (all(mask)) {
        // When every edge is outset/inset, each corner can use both edge vectors
        denom = c1x * c2y - c2x * c1y;
        a = (c2x * c3y - c3x * c2y) / denom;
        b = (c3x * c1y - c1x * c3y) / denom;
    } else {
        // Force a or b to be 0 if that edge cannot be used due to non-AA
        M4f aMask = skvx::shuffle<0, 0, 3, 3>(mask);
        M4f bMask = skvx::shuffle<2, 1, 2, 1>(mask);

        // When aMask[i]&bMask[i], then a[i], b[i], denom[i] match the kAll case.
        // When aMask[i]&!bMask[i], then b[i] = 0, a[i] = -c3x/c1x or -c3y/c1y, using better denom
        // When !aMask[i]&bMask[i], then a[i] = 0, b[i] = -c3x/c2x or -c3y/c2y, ""
        // When !aMask[i]&!bMask[i], then both a[i] = 0 and b[i] = 0
        M4f useC1x = abs(c1x) > abs(c1y);
        M4f useC2x = abs(c2x) > abs(c2y);

        denom = if_then_else(aMask,
                        if_then_else(bMask,
                                c1x * c2y - c2x * c1y,            /* A & B   */
                                if_then_else(useC1x, c1x, c1y)),  /* A & !B  */
                        if_then_else(bMask,
                                if_then_else(useC2x, c2x, c2y),   /* !A & B  */
                                V4f(1.f)));                       /* !A & !B */

        a = if_then_else(aMask,
                    if_then_else(bMask,
                            c2x * c3y - c3x * c2y,                /* A & B   */
                            if_then_else(useC1x, -c3x, -c3y)),    /* A & !B  */
                    V4f(0.f)) / denom;                            /* !A      */
        b = if_then_else(bMask,
                    if_then_else(aMask,
                            c3x * c1y - c1x * c3y,                /* A & B   */
                            if_then_else(useC2x, -c3x, -c3y)),    /* !A & B  */
                    V4f(0.f)) / denom;                            /* !B      */
    }

    V4f newW = fW + a * e1w + b * e2w;
    // If newW < 0, scale a and b such that the point reaches the infinity plane instead of crossing
    // This breaks orthogonality of inset/outsets, but GPUs don't handle negative Ws well so this
    // is far less visually disturbing (likely not noticeable since it's at extreme perspective).
    // The alternative correction (multiply xyw by -1) has the disadvantage of changing how local
    // coordinates would be interpolated.
    static const float kMinW = 1e-6f;
    if (any(newW < 0.f)) {
        V4f scale = if_then_else(newW < kMinW, (kMinW - fW) / (newW - fW), V4f(1.f));
        a *= scale;
        b *= scale;
    }

    fX += a * e1x + b * e2x;
    fY += a * e1y + b * e2y;
    fW += a * e1w + b * e2w;
    correct_bad_coords(abs(denom) < kTolerance, &fX, &fY, &fW);

    if (fUVRCount > 0) {
        // Calculate R here so it can be corrected with U and V in case it's needed later
        V4f e1u = skvx::shuffle<2, 3, 2, 3>(fU) - skvx::shuffle<0, 1, 0, 1>(fU);
        V4f e1v = skvx::shuffle<2, 3, 2, 3>(fV) - skvx::shuffle<0, 1, 0, 1>(fV);
        V4f e1r = skvx::shuffle<2, 3, 2, 3>(fR) - skvx::shuffle<0, 1, 0, 1>(fR);
        correct_bad_edges(mad(e1u, e1u, e1v * e1v) < kTolerance * kTolerance, &e1u, &e1v, &e1r);

        V4f e2u = skvx::shuffle<1, 1, 3, 3>(fU) - skvx::shuffle<0, 0, 2, 2>(fU);
        V4f e2v = skvx::shuffle<1, 1, 3, 3>(fV) - skvx::shuffle<0, 0, 2, 2>(fV);
        V4f e2r = skvx::shuffle<1, 1, 3, 3>(fR) - skvx::shuffle<0, 0, 2, 2>(fR);
        correct_bad_edges(mad(e2u, e2u, e2v * e2v) < kTolerance * kTolerance, &e2u, &e2v, &e2r);

        fU += a * e1u + b * e2u;
        fV += a * e1v + b * e2v;
        if (fUVRCount == 3) {
            fR += a * e1r + b * e2r;
            correct_bad_coords(abs(denom) < kTolerance, &fU, &fV, &fR);
        } else {
            correct_bad_coords(abs(denom) < kTolerance, &fU, &fV, nullptr);
        }
    }
}

//** TessellationHelper implementation

void TessellationHelper::reset(const GrQuad& deviceQuad, const GrQuad* localQuad) {
    // Record basic state that isn't recorded on the Vertices struct itself
    fDeviceType = deviceQuad.quadType();
    fLocalType = localQuad ? localQuad->quadType() : GrQuad::Type::kAxisAligned;

    // Reset metadata validity
    fOutsetRequestValid = false;
    fEdgeEquationsValid = false;

    // Compute vertex properties that are always needed for a quad, so no point in doing it lazily.
    fOriginal.reset(deviceQuad, localQuad);
    fEdgeVectors.reset(fOriginal.fX, fOriginal.fY, fOriginal.fW, fDeviceType);

    fVerticesValid = true;
}

V4f TessellationHelper::inset(const skvx::Vec<4, float>& edgeDistances,
                              GrQuad* deviceInset, GrQuad* localInset) {
    SkASSERT(fVerticesValid);

    Vertices inset = fOriginal;
    const OutsetRequest& request = this->getOutsetRequest(edgeDistances);
    int vertexCount;
    if (request.fInsetDegenerate) {
        vertexCount = this->adjustDegenerateVertices(-request.fEdgeDistances, &inset);
    } else {
        this->adjustVertices(-request.fEdgeDistances, &inset);
        vertexCount = 4;
    }

    inset.asGrQuads(deviceInset, fDeviceType, localInset, fLocalType);
    if (vertexCount < 3) {
        // The interior has less than a full pixel's area so estimate reduced coverage using
        // the distance of the inset's projected corners to the original edges.
        return this->getEdgeEquations().estimateCoverage(inset.fX / inset.fW,
                                                         inset.fY / inset.fW);
    } else {
        return 1.f;
    }
}

void TessellationHelper::outset(const skvx::Vec<4, float>& edgeDistances,
                                GrQuad* deviceOutset, GrQuad* localOutset) {
    SkASSERT(fVerticesValid);

    Vertices outset = fOriginal;
    const OutsetRequest& request = this->getOutsetRequest(edgeDistances);
    if (request.fOutsetDegenerate) {
        this->adjustDegenerateVertices(request.fEdgeDistances, &outset);
    } else {
        this->adjustVertices(request.fEdgeDistances, &outset);
    }

    outset.asGrQuads(deviceOutset, fDeviceType, localOutset, fLocalType);
}

const TessellationHelper::OutsetRequest& TessellationHelper::getOutsetRequest(
        const skvx::Vec<4, float>& edgeDistances) {
    // Much of the code assumes that we start from positive distances and apply it unmodified to
    // create an outset; knowing that it's outset simplifies degeneracy checking.
    SkASSERT(all(edgeDistances >= 0.f));

    // Rebuild outset request if invalid or if the edge distances have changed.
    if (!fOutsetRequestValid || any(edgeDistances != fOutsetRequest.fEdgeDistances)) {
        fOutsetRequest.reset(fEdgeVectors, fDeviceType, edgeDistances);
        fOutsetRequestValid = true;
    }
    return fOutsetRequest;
}

const TessellationHelper::EdgeEquations& TessellationHelper::getEdgeEquations() {
    if (!fEdgeEquationsValid) {
        fEdgeEquations.reset(fEdgeVectors);
        fEdgeEquationsValid = true;
    }
    return fEdgeEquations;
}

void TessellationHelper::adjustVertices(const skvx::Vec<4, float>& signedEdgeDistances,
                                        Vertices* vertices) {
    SkASSERT(vertices);
    SkASSERT(vertices->fUVRCount == 0 || vertices->fUVRCount == 2 || vertices->fUVRCount == 3);

    if (fDeviceType < GrQuad::Type::kPerspective) {
        // For non-perspective, non-degenerate quads, moveAlong is correct and most efficient
        vertices->moveAlong(fEdgeVectors, signedEdgeDistances);
    } else {
        // For perspective, non-degenerate quads, use moveAlong for the projected points and then
        // reconstruct Ws with moveTo.
        Vertices projected = { fEdgeVectors.fX2D, fEdgeVectors.fY2D, /*w*/ 1.f, 0.f, 0.f, 0.f, 0 };
        projected.moveAlong(fEdgeVectors, signedEdgeDistances);
        vertices->moveTo(projected.fX, projected.fY, signedEdgeDistances != 0.f);
    }
}

int TessellationHelper::adjustDegenerateVertices(const skvx::Vec<4, float>& signedEdgeDistances,
                                                 Vertices* vertices) {
    SkASSERT(vertices);
    SkASSERT(vertices->fUVRCount == 0 || vertices->fUVRCount == 2 || vertices->fUVRCount == 3);

    if (fDeviceType <= GrQuad::Type::kRectilinear) {
        // For rectilinear, degenerate quads, can use moveAlong if the edge distances are adjusted
        // to not cross over each other.
        SkASSERT(all(signedEdgeDistances <= 0.f)); // Only way rectilinear can degenerate is insets
        V4f halfLengths = -0.5f / next_cw(fEdgeVectors.fInvLengths); // Negate to inset
        M4f crossedEdges = halfLengths > signedEdgeDistances;
        V4f safeInsets = if_then_else(crossedEdges, halfLengths, signedEdgeDistances);
        vertices->moveAlong(fEdgeVectors, safeInsets);

        // A degenerate rectilinear quad is either a point (both w and h crossed), or a line
        return all(crossedEdges) ? 1 : 2;
    } else {
        // Degenerate non-rectangular shape, must go through slowest path (which automatically
        // handles perspective).
        V4f x2d = fEdgeVectors.fX2D;
        V4f y2d = fEdgeVectors.fY2D;
        int vertexCount = this->getEdgeEquations().computeDegenerateQuad(signedEdgeDistances,
                                                                         &x2d, &y2d);
        vertices->moveTo(x2d, y2d, signedEdgeDistances != 0.f);
        return vertexCount;
    }
}

}; // namespace GrQuadUtils
