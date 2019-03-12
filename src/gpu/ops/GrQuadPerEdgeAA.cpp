/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrQuadPerEdgeAA.h"
#include "GrQuad.h"
#include "GrVertexWriter.h"
#include "glsl/GrGLSLColorSpaceXformHelper.h"
#include "glsl/GrGLSLGeometryProcessor.h"
#include "glsl/GrGLSLPrimitiveProcessor.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"
#include "glsl/GrGLSLVarying.h"
#include "glsl/GrGLSLVertexGeoBuilder.h"
#include "SkNx.h"

#define AI SK_ALWAYS_INLINE

namespace {

// Helper data types since there is a lot of information that needs to be passed around to
// avoid recalculation in the different procedures for tessellating an AA quad.

struct Vertices {
    // X, Y, and W coordinates in device space. If not perspective, w should be set to 1.f
    Sk4f fX, fY, fW;
    // U, V, and R coordinates representing local quad. Ignored depending on uvrCount (0, 1, 2).
    Sk4f fU, fV, fR;
    int fUVRCount;
};

struct QuadMetadata {
    // Normalized edge vectors of the device space quad, ordered L, B, T, R (i.e. nextCCW(x) - x).
    Sk4f fDX, fDY;
    // 1 / edge length of the device space quad
    Sk4f fInvLengths;
    // Edge mask (set to all 1s if aa flags is kAll), otherwise 1.f if edge was AA, 0.f if non-AA.
    Sk4f fMask;
};

struct Edges {
    // a * x + b * y + c = 0; positive distance is inside the quad; ordered LBTR.
    Sk4f fA, fB, fC;
    // Whether or not the edge normals had to be flipped to preserve positive distance on the inside
    bool fFlipped;
};

static constexpr float kTolerance = 1e-2f;

static AI Sk4f fma(const Sk4f& f, const Sk4f& m, const Sk4f& a) {
    return SkNx_fma<4, float>(f, m, a);
}

// These rotate the points/edge values either clockwise or counterclockwise assuming tri strip
// order.
static AI Sk4f nextCW(const Sk4f& v) {
    return SkNx_shuffle<2, 0, 3, 1>(v);
}

static AI Sk4f nextCCW(const Sk4f& v) {
    return SkNx_shuffle<1, 3, 0, 2>(v);
}

// Replaces zero-length 'bad' edge vectors with the reversed opposite edge vector.
// e3 may be null if only 2D edges need to be corrected for.
static AI void correct_bad_edges(const Sk4f& bad, Sk4f* e1, Sk4f* e2, Sk4f* e3) {
    if (bad.anyTrue()) {
        // Want opposite edges, L B T R -> R T B L but with flipped sign to preserve winding
        *e1 = bad.thenElse(-SkNx_shuffle<3, 2, 1, 0>(*e1), *e1);
        *e2 = bad.thenElse(-SkNx_shuffle<3, 2, 1, 0>(*e2), *e2);
        if (e3) {
            *e3 = bad.thenElse(-SkNx_shuffle<3, 2, 1, 0>(*e3), *e3);
        }
    }
}

// Replace 'bad' coordinates by rotating CCW to get the next point. c3 may be null for 2D points.
static AI void correct_bad_coords(const Sk4f& bad, Sk4f* c1, Sk4f* c2, Sk4f* c3) {
    if (bad.anyTrue()) {
        *c1 = bad.thenElse(nextCCW(*c1), *c1);
        *c2 = bad.thenElse(nextCCW(*c2), *c2);
        if (c3) {
            *c3 = bad.thenElse(nextCCW(*c3), *c3);
        }
    }
}

static AI QuadMetadata get_metadata(const Vertices& vertices, GrQuadAAFlags aaFlags) {
    Sk4f dx = nextCCW(vertices.fX) - vertices.fX;
    Sk4f dy = nextCCW(vertices.fY) - vertices.fY;
    Sk4f invLengths = fma(dx, dx, dy * dy).rsqrt();

    Sk4f mask = aaFlags == GrQuadAAFlags::kAll ? Sk4f(1.f) :
            Sk4f((GrQuadAAFlags::kLeft & aaFlags) ? 1.f : 0.f,
                 (GrQuadAAFlags::kBottom & aaFlags) ? 1.f : 0.f,
                 (GrQuadAAFlags::kTop & aaFlags) ? 1.f : 0.f,
                 (GrQuadAAFlags::kRight & aaFlags) ? 1.f : 0.f);
    return { dx * invLengths, dy * invLengths, invLengths, mask };
}

static AI Edges get_edge_equations(const QuadMetadata& metadata, const Vertices& vertices) {
    Sk4f dx = metadata.fDX;
    Sk4f dy = metadata.fDY;
    // Correct for bad edges by copying adjacent edge information into the bad component
    correct_bad_edges(metadata.fInvLengths >= 1.f / kTolerance, &dx, &dy, nullptr);

    Sk4f c = fma(dx, vertices.fY, -dy * vertices.fX);
    // Make sure normals point into the shape
    Sk4f test = fma(dy, nextCW(vertices.fX), fma(-dx, nextCW(vertices.fY), c));
    if ((test < -kTolerance).anyTrue()) {
        return {-dy, dx, -c, true};
    } else {
        return {dy, -dx, c, false};
    }
}

// Sets 'outset' to the magnitude of outset/inset to adjust each corner of a quad given the
// edge angles and lengths. If the quad is too small, has empty edges, or too sharp of angles,
// false is returned and the degenerate slow-path should be used.
static bool get_optimized_outset(const QuadMetadata& metadata, bool rectilinear, Sk4f* outset) {
    if (rectilinear) {
        *outset = 0.5f;
        // Stay in the fast path as long as all edges are at least a pixel long (so 1/len <= 1)
        return (metadata.fInvLengths <= 1.f).allTrue();
    }

    if ((metadata.fInvLengths >= 1.f / kTolerance).anyTrue()) {
        // Have an empty edge from a degenerate quad, so there's no hope
        return false;
    }

    // The distance the point needs to move is 1/2sin(theta), where theta is the angle between the
    // two edges at that point. cos(theta) is equal to dot(dxy, nextCW(dxy))
    Sk4f cosTheta = fma(metadata.fDX, nextCW(metadata.fDX), metadata.fDY * nextCW(metadata.fDY));
    // If the angle is too shallow between edges, go through the degenerate path, otherwise adding
    // and subtracting very large vectors in almost opposite directions leads to float errors
    if ((cosTheta.abs() >= 0.9f).anyTrue()) {
        return false;
    }
    *outset = 0.5f * (1.f - cosTheta * cosTheta).rsqrt(); // 1/2sin(theta)

    // When outsetting or insetting, the current edge's AA adds to the length:
    //   cos(pi - theta)/2sin(theta) + cos(pi-ccw(theta))/2sin(ccw(theta))
    // Moving an adjacent edge updates the length by 1/2sin(theta|ccw(theta))
    Sk4f halfTanTheta = -cosTheta * (*outset); // cos(pi - theta) = -cos(theta)
    Sk4f edgeAdjust = metadata.fMask * (halfTanTheta + nextCCW(halfTanTheta)) +
                      nextCCW(metadata.fMask) * nextCCW(*outset) +
                      nextCW(metadata.fMask) * (*outset);
    // If either outsetting (plus edgeAdjust) or insetting (minus edgeAdjust) make edgeLen negative
    // then use the slow path
    Sk4f threshold = 0.1f - metadata.fInvLengths.invert();
    return (edgeAdjust > threshold).allTrue() && (edgeAdjust < -threshold).allTrue();
}

// Ignores the quad's fW, use outset_projected_vertices if it's known to need 3D.
static AI void outset_vertices(const Sk4f& outset, const QuadMetadata& metadata, Vertices* quad) {
    // The mask is rotated compared to the outsets and edge vectors, since if the edge is "on"
    // both its points need to be moved along their other edge vectors.
    auto maskedOutset = -outset * nextCW(metadata.fMask);
    auto maskedOutsetCW = outset * metadata.fMask;
    // x = x + outset * mask * nextCW(xdiff) - outset * nextCW(mask) * xdiff
    quad->fX += fma(maskedOutsetCW, nextCW(metadata.fDX), maskedOutset * metadata.fDX);
    quad->fY += fma(maskedOutsetCW, nextCW(metadata.fDY), maskedOutset * metadata.fDY);
    if (quad->fUVRCount > 0) {
        // We want to extend the texture coords by the same proportion as the positions.
        maskedOutset *= metadata.fInvLengths;
        maskedOutsetCW *= nextCW(metadata.fInvLengths);
        Sk4f du = nextCCW(quad->fU) - quad->fU;
        Sk4f dv = nextCCW(quad->fV) - quad->fV;
        quad->fU += fma(maskedOutsetCW, nextCW(du), maskedOutset * du);
        quad->fV += fma(maskedOutsetCW, nextCW(dv), maskedOutset * dv);
        if (quad->fUVRCount == 3) {
            Sk4f dr = nextCCW(quad->fR) - quad->fR;
            quad->fR += fma(maskedOutsetCW, nextCW(dr), maskedOutset * dr);
        }
    }
}

// Updates (x,y,w) to be at (x2d,y2d) once projected. Updates (u,v,r) to match if provided.
// Gracefully handles 2D content if *w holds all 1s.
static void outset_projected_vertices(const Sk4f& x2d, const Sk4f& y2d,
                                      GrQuadAAFlags aaFlags, Vertices* quad) {
    // Left to right, in device space, for each point
    Sk4f e1x = SkNx_shuffle<2, 3, 2, 3>(quad->fX) - SkNx_shuffle<0, 1, 0, 1>(quad->fX);
    Sk4f e1y = SkNx_shuffle<2, 3, 2, 3>(quad->fY) - SkNx_shuffle<0, 1, 0, 1>(quad->fY);
    Sk4f e1w = SkNx_shuffle<2, 3, 2, 3>(quad->fW) - SkNx_shuffle<0, 1, 0, 1>(quad->fW);
    correct_bad_edges(fma(e1x, e1x, e1y * e1y) < kTolerance * kTolerance, &e1x, &e1y, &e1w);

    // // Top to bottom, in device space, for each point
    Sk4f e2x = SkNx_shuffle<1, 1, 3, 3>(quad->fX) - SkNx_shuffle<0, 0, 2, 2>(quad->fX);
    Sk4f e2y = SkNx_shuffle<1, 1, 3, 3>(quad->fY) - SkNx_shuffle<0, 0, 2, 2>(quad->fY);
    Sk4f e2w = SkNx_shuffle<1, 1, 3, 3>(quad->fW) - SkNx_shuffle<0, 0, 2, 2>(quad->fW);
    correct_bad_edges(fma(e2x, e2x, e2y * e2y) < kTolerance * kTolerance, &e2x, &e2y, &e2w);

    // Can only move along e1 and e2 to reach the new 2D point, so we have
    // x2d = (x + a*e1x + b*e2x) / (w + a*e1w + b*e2w) and
    // y2d = (y + a*e1y + b*e2y) / (w + a*e1w + b*e2w) for some a, b
    // This can be rewritten to a*c1x + b*c2x + c3x = 0; a * c1y + b*c2y + c3y = 0, where
    // the cNx and cNy coefficients are:
    Sk4f c1x = e1w * x2d - e1x;
    Sk4f c1y = e1w * y2d - e1y;
    Sk4f c2x = e2w * x2d - e2x;
    Sk4f c2y = e2w * y2d - e2y;
    Sk4f c3x = quad->fW * x2d - quad->fX;
    Sk4f c3y = quad->fW * y2d - quad->fY;

    // Solve for a and b
    Sk4f a, b, denom;
    if (aaFlags == GrQuadAAFlags::kAll) {
        // When every edge is outset/inset, each corner can use both edge vectors
        denom = c1x * c2y - c2x * c1y;
        a = (c2x * c3y - c3x * c2y) / denom;
        b = (c3x * c1y - c1x * c3y) / denom;
    } else {
        // Force a or b to be 0 if that edge cannot be used due to non-AA
        // FIXME requires the extra > 0.f, since Sk4f's thenElse only works if true values have
        // all their bits set to 1.
        Sk4f aMask = Sk4f((aaFlags & GrQuadAAFlags::kLeft) ? 1.f : 0.f,
                   (aaFlags & GrQuadAAFlags::kLeft) ? 1.f : 0.f,
                   (aaFlags & GrQuadAAFlags::kRight) ? 1.f : 0.f,
                   (aaFlags & GrQuadAAFlags::kRight) ? 1.f : 0.f) > 0.f;
        Sk4f bMask = Sk4f((aaFlags & GrQuadAAFlags::kTop) ? 1.f : 0.f,
                   (aaFlags & GrQuadAAFlags::kBottom) ? 1.f : 0.f,
                   (aaFlags & GrQuadAAFlags::kTop) ? 1.f : 0.f,
                   (aaFlags & GrQuadAAFlags::kBottom) ? 1.f : 0.f) > 0.f;

        // When aMask[i]&bMask[i], then a[i], b[i], denom[i] match the kAll case.
        // When aMask[i]&!bMask[i], then b[i] = 0, a[i] = -c3x/c1x or -c3y/c1y, using better denom
        // When !aMask[i]&bMask[i], then a[i] = 0, b[i] = -c3x/c2x or -c3y/c2y, ""
        // When !aMask[i]&!bMask[i], then both a[i] = 0 and b[i] = 0
        Sk4f useC1x = c1x.abs() > c1y.abs();
        Sk4f useC2x = c2x.abs() > c2y.abs();
        //                                    -------- A & B ------      --------- A & !B ---------
        denom = aMask.thenElse(bMask.thenElse(c1x * c2y - c2x * c1y,     useC1x.thenElse(c1x, c1y)),
        //                                    ------- !A & B ----------  - !A & !B -
                               bMask.thenElse(useC2x.thenElse(c2x, c2y), 1.0f));
        //                                -------- A & B ------  ---------- A & !B ----------
        a = aMask.thenElse(bMask.thenElse(c2x * c3y - c3x * c2y, useC1x.thenElse(-c3x, -c3y)),
        //                 - !A -
                           0.0f) / denom;
        //                                -------- A & B ------  ---------- !A & B ----------
        b = bMask.thenElse(aMask.thenElse(c3x * c1y - c1x * c3y, useC2x.thenElse(-c3x, -c3y)),
        //                 - !B -
                           0.0f) / denom;
    }

    quad->fX += a * e1x + b * e2x;
    quad->fY += a * e1y + b * e2y;
    quad->fW += a * e1w + b * e2w;
    correct_bad_coords(denom.abs() < kTolerance, &quad->fX, &quad->fY, &quad->fW);

    if (quad->fUVRCount > 0) {
        // Calculate R here so it can be corrected with U and V in case it's needed later
        Sk4f e1u = SkNx_shuffle<2, 3, 2, 3>(quad->fU) - SkNx_shuffle<0, 1, 0, 1>(quad->fU);
        Sk4f e1v = SkNx_shuffle<2, 3, 2, 3>(quad->fV) - SkNx_shuffle<0, 1, 0, 1>(quad->fV);
        Sk4f e1r = SkNx_shuffle<2, 3, 2, 3>(quad->fR) - SkNx_shuffle<0, 1, 0, 1>(quad->fR);
        correct_bad_edges(fma(e1u, e1u, e1v * e1v) < kTolerance * kTolerance, &e1u, &e1v, &e1r);

        Sk4f e2u = SkNx_shuffle<1, 1, 3, 3>(quad->fU) - SkNx_shuffle<0, 0, 2, 2>(quad->fU);
        Sk4f e2v = SkNx_shuffle<1, 1, 3, 3>(quad->fV) - SkNx_shuffle<0, 0, 2, 2>(quad->fV);
        Sk4f e2r = SkNx_shuffle<1, 1, 3, 3>(quad->fR) - SkNx_shuffle<0, 0, 2, 2>(quad->fR);
        correct_bad_edges(fma(e2u, e2u, e2v * e2v) < kTolerance * kTolerance, &e2u, &e2v, &e2r);

        quad->fU += a * e1u + b * e2u;
        quad->fV += a * e1v + b * e2v;
        if (quad->fUVRCount == 3) {
            quad->fR += a * e1r + b * e2r;
            correct_bad_coords(denom.abs() < kTolerance, &quad->fU, &quad->fV, &quad->fR);
        } else {
            correct_bad_coords(denom.abs() < kTolerance, &quad->fU, &quad->fV, nullptr);
        }
    }
}

// Calculate area of intersection between quad (xs, ys) and a pixel at 'pixelCenter'.
// a, b, c are edge equations of the quad, flipped is true if the line equations had their normals
// reversed to correct for matrix transforms.
static float get_exact_coverage(const SkPoint& pixelCenter, const Vertices& quad,
                                const Edges& edges) {
     // Ordering of vertices given default tri-strip that produces CCW points
    static const int kCCW[] = {0, 1, 3, 2};
    // Ordering of vertices given inverted tri-strip that produces CCW
    static const int kFlippedCCW[] = {0, 2, 3, 1};

    // Edge boundaries of the pixel
    float left = pixelCenter.fX - 0.5f;
    float right = pixelCenter.fX + 0.5f;
    float top = pixelCenter.fY - 0.5f;
    float bot = pixelCenter.fY + 0.5f;

    // Whether or not the 4 corners of the pixel are inside the quad geometry. Variable names are
    // intentional to work easily with the helper macros.
    bool topleftInside = ((edges.fA * left + edges.fB * top + edges.fC) >= 0.f).allTrue();
    bool botleftInside = ((edges.fA * left + edges.fB * bot + edges.fC) >= 0.f).allTrue();
    bool botrightInside = ((edges.fA * right + edges.fB * bot + edges.fC) >= 0.f).allTrue();
    bool toprightInside = ((edges.fA * right + edges.fB * top + edges.fC) >= 0.f).allTrue();
    if (topleftInside && botleftInside && botrightInside && toprightInside) {
        // Quad fully contains the pixel, so we know the area will be 1.f
        return 1.f;
    }

    // Track whether or not the quad vertices in (xs, ys) are on the proper sides of l, t, r, and b
    Sk4i leftValid = SkNx_cast<int32_t>(quad.fX >= left);
    Sk4i rightValid = SkNx_cast<int32_t>(quad.fX <= right);
    Sk4i topValid = SkNx_cast<int32_t>(quad.fY >= top);
    Sk4i botValid = SkNx_cast<int32_t>(quad.fY <= bot);

    // Intercepts of quad lines with the 4 pixel edges
    Sk4f leftCross = -(edges.fC + edges.fA * left) / edges.fB;
    Sk4f rightCross = -(edges.fC + edges.fA * right) / edges.fB;
    Sk4f topCross = -(edges.fC + edges.fB * top) / edges.fA;
    Sk4f botCross = -(edges.fC + edges.fB * bot) / edges.fA;

    // State for implicitly tracking the intersection boundary and area
    SkPoint firstPoint = {0.f, 0.f};
    SkPoint lastPoint = {0.f, 0.f};
    bool intersected = false;
    float area = 0.f;

    // Adds a point to the intersection hull, remembering first point (for closing) and the
    // current point, and updates the running area total.
    // See http://mathworld.wolfram.com/PolygonArea.html
    auto accumulate = [&](const SkPoint& p) {
        if (intersected) {
            float da = lastPoint.fX * p.fY - p.fX * lastPoint.fY;
            area += da;
        } else {
            firstPoint = p;
            intersected = true;
        }
        lastPoint = p;
    };

    // Used during iteration over the quad points to check if edge intersections are valid and
    // should be accumulated.
#define ADD_EDGE_CROSSING_X(SIDE) \
    do { \
        if (SIDE##Cross[ei] >= top && SIDE##Cross[ei] <= bot) { \
            accumulate({SIDE, SIDE##Cross[ei]}); \
            addedIntersection = true; \
        } \
    } while(false)
#define ADD_EDGE_CROSSING_Y(SIDE) \
    do { \
        if (SIDE##Cross[ei] >= left && SIDE##Cross[ei] <= right) { \
            accumulate({SIDE##Cross[ei], SIDE}); \
            addedIntersection = true; \
        } \
    } while(false)
#define TEST_EDGES(SIDE, AXIS, I, NI) \
    do { \
        if (!SIDE##Valid[I] && SIDE##Valid[NI]) { \
            ADD_EDGE_CROSSING_##AXIS(SIDE); \
            crossedEdges = true; \
        } \
    } while(false)
    // Used during iteration over the quad points to check if a pixel corner should be included
    // in the intersection boundary
#define ADD_CORNER(CHECK, SIDE_LR, SIDE_TB) \
    if (!CHECK##Valid[i] || !CHECK##Valid[ni]) { \
        if (SIDE_TB##SIDE_LR##Inside) { \
            accumulate({SIDE_LR, SIDE_TB}); \
        } \
    }
#define TEST_CORNER_X(SIDE, I, NI) \
    do { \
        if (!SIDE##Valid[I] && SIDE##Valid[NI]) { \
            ADD_CORNER(top, SIDE, top) else ADD_CORNER(bot, SIDE, bot) \
        } \
    } while(false)
#define TEST_CORNER_Y(SIDE, I, NI) \
    do { \
        if (!SIDE##Valid[I] && SIDE##Valid[NI]) { \
            ADD_CORNER(left, left, SIDE) else ADD_CORNER(right, right, SIDE) \
        } \
    } while(false)

    // Iterate over the 4 points of the quad, adding valid intersections with the pixel edges
    // or adding interior pixel corners as it goes. This automatically keeps all accumulated points
    // in CCW ordering so the area can be calculated on the fly and there's no need to store the
    // list of hull points. This is somewhat inspired by the Sutherland-Hodgman algorithm but since
    // there are only 4 points in each source polygon, there is no point list maintenance.
    for (int j = 0; j < 4; ++j) {
        // Current vertex
        int i = edges.fFlipped ? kFlippedCCW[j] : kCCW[j];
        // Moving to this vertex
        int ni = edges.fFlipped ? kFlippedCCW[(j + 1) % 4] : kCCW[(j + 1) % 4];
        // Index in edge vectors corresponding to move from i to ni
        int ei = edges.fFlipped ? ni : i;

        bool crossedEdges = false;
        bool addedIntersection = false;

        // First check if there are any outside -> inside edge crossings. There can be 0, 1, or 2.
        // 2 can occur if one crossing is still outside the pixel, or if they both go through
        // the corner (in which case a duplicate point is added, but that doesn't change area).

        // Outside to inside crossing
        TEST_EDGES(left, X, i, ni);
        TEST_EDGES(right, X, i, ni);
        TEST_EDGES(top, Y, i, ni);
        TEST_EDGES(bot, Y, i, ni);
        // Inside to outside crossing (swapping ni and i in the boolean test)
        TEST_EDGES(left, X, ni, i);
        TEST_EDGES(right, X, ni, i);
        TEST_EDGES(top, Y, ni, i);
        TEST_EDGES(bot, Y, ni, i);

        // If we crossed edges but didn't add any intersections, check the corners of the pixel.
        // If the pixel corners are inside the quad, include them in the boundary.
        if (crossedEdges && !addedIntersection) {
            // This can lead to repeated points, but those just accumulate zero area
            TEST_CORNER_X(left, i, ni);
            TEST_CORNER_X(right, i, ni);
            TEST_CORNER_Y(top, i, ni);
            TEST_CORNER_Y(bot, i, ni);

            TEST_CORNER_X(left, ni, i);
            TEST_CORNER_X(right, ni, i);
            TEST_CORNER_Y(top, ni, i);
            TEST_CORNER_Y(bot, ni, i);
        }

        // Lastly, if the next point is completely inside the pixel it gets included in the boundary
        if (leftValid[ni] && rightValid[ni] && topValid[ni] && botValid[ni]) {
            accumulate({quad.fX[ni], quad.fY[ni]});
        }
    }

#undef TEST_CORNER_Y
#undef TEST_CORNER_X
#undef ADD_CORNER

#undef TEST_EDGES
#undef ADD_EDGE_CROSSING_Y
#undef ADD_EDGE_CROSSING_X

    // After all points have been considered, close the boundary to get final area. If we never
    // added any points, it means the quad didn't intersect the pixel rectangle.
    if (intersected) {
        // Final equation for area of convex polygon is to multiply by -1/2 (minus since the points
        // were in CCW order).
        accumulate(firstPoint);
        return -0.5f * area;
    } else {
        return 0.f;
    }
}

// Outsets or insets xs/ys in place. To be used when the interior is very small, edges are near
// parallel, or edges are very short/zero-length. Returns coverage for each vertex.
// Requires (dx, dy) to already be fixed for empty edges.
static Sk4f compute_degenerate_quad(GrQuadAAFlags aaFlags, const Sk4f& mask, const Edges& edges,
                                    bool outset, Vertices* quad) {
    // Move the edge 1/2 pixel in or out depending on 'outset'.
    Sk4f oc = edges.fC + mask * (outset ? 0.5f : -0.5f);

    // There are 6 points that we care about to determine the final shape of the polygon, which
    // are the intersections between (e0,e2), (e1,e0), (e2,e3), (e3,e1) (corresponding to the
    // 4 corners), and (e1, e2), (e0, e3) (representing the intersections of opposite edges).
    Sk4f denom = edges.fA * nextCW(edges.fB) - edges.fB * nextCW(edges.fA);
    Sk4f px = (edges.fB * nextCW(oc) - oc * nextCW(edges.fB)) / denom;
    Sk4f py = (oc * nextCW(edges.fA) - edges.fA * nextCW(oc)) / denom;
    correct_bad_coords(denom.abs() < kTolerance, &px, &py, nullptr);

    // Calculate the signed distances from these 4 corners to the other two edges that did not
    // define the intersection. So p(0) is compared to e3,e1, p(1) to e3,e2 , p(2) to e0,e1, and
    // p(3) to e0,e2
    Sk4f dists1 = px * SkNx_shuffle<3, 3, 0, 0>(edges.fA) +
                  py * SkNx_shuffle<3, 3, 0, 0>(edges.fB) +
                  SkNx_shuffle<3, 3, 0, 0>(oc);
    Sk4f dists2 = px * SkNx_shuffle<1, 2, 1, 2>(edges.fA) +
                  py * SkNx_shuffle<1, 2, 1, 2>(edges.fB) +
                  SkNx_shuffle<1, 2, 1, 2>(oc);

    // If all the distances are >= 0, the 4 corners form a valid quadrilateral, so use them as
    // the 4 points. If any point is on the wrong side of both edges, the interior has collapsed
    // and we need to use a central point to represent it. If all four points are only on the
    // wrong side of 1 edge, one edge has crossed over another and we use a line to represent it.
    // Otherwise, use a triangle that replaces the bad points with the intersections of
    // (e1, e2) or (e0, e3) as needed.
    Sk4f d1v0 = dists1 < kTolerance;
    Sk4f d2v0 = dists2 < kTolerance;
    // FIXME(michaelludwig): Sk4f has anyTrue() and allTrue(), but not & or |. Sk4i has & or | but
    // not anyTrue() and allTrue(). Moving to SkVx from SkNx will clean this up.
    Sk4i d1And2 = SkNx_cast<int32_t>(d1v0) & SkNx_cast<int32_t>(d2v0);
    Sk4i d1Or2 = SkNx_cast<int32_t>(d1v0) | SkNx_cast<int32_t>(d2v0);

    Sk4f coverage;
    if (!d1Or2[0] && !d1Or2[1] && !d1Or2[2] && !d1Or2[3]) {
        // Every dists1 and dists2 >= kTolerance so it's not degenerate, use all 4 corners as-is
        // and use full coverage
        coverage = 1.f;
    } else if (d1And2[0] || d1And2[1] || d1And2[2] || d1And2[3]) {
        // A point failed against two edges, so reduce the shape to a single point, which we take as
        // the center of the original quad to ensure it is contained in the intended geometry. Since
        // it has collapsed, we know the shape cannot cover a pixel so update the coverage.
        SkPoint center = {0.25f * (quad->fX[0] + quad->fX[1] + quad->fX[2] + quad->fX[3]),
                          0.25f * (quad->fY[0] + quad->fY[1] + quad->fY[2] + quad->fY[3])};
        coverage = get_exact_coverage(center, *quad, edges);
        px = center.fX;
        py = center.fY;
    } else if (d1Or2[0] && d1Or2[1] && d1Or2[2] && d1Or2[3]) {
        // Degenerates to a line. Compare p[2] and p[3] to edge 0. If they are on the wrong side,
        // that means edge 0 and 3 crossed, and otherwise edge 1 and 2 crossed.
        if (dists1[2] < kTolerance && dists1[3] < kTolerance) {
            // Edges 0 and 3 have crossed over, so make the line from average of (p0,p2) and (p1,p3)
            px = 0.5f * (SkNx_shuffle<0, 1, 0, 1>(px) + SkNx_shuffle<2, 3, 2, 3>(px));
            py = 0.5f * (SkNx_shuffle<0, 1, 0, 1>(py) + SkNx_shuffle<2, 3, 2, 3>(py));
            float mc02 = get_exact_coverage({px[0], py[0]}, *quad, edges);
            float mc13 = get_exact_coverage({px[1], py[1]}, *quad, edges);
            coverage = Sk4f(mc02, mc13, mc02, mc13);
        } else {
            // Edges 1 and 2 have crossed over, so make the line from average of (p0,p1) and (p2,p3)
            px = 0.5f * (SkNx_shuffle<0, 0, 2, 2>(px) + SkNx_shuffle<1, 1, 3, 3>(px));
            py = 0.5f * (SkNx_shuffle<0, 0, 2, 2>(py) + SkNx_shuffle<1, 1, 3, 3>(py));
            float mc01 = get_exact_coverage({px[0], py[0]}, *quad, edges);
            float mc23 = get_exact_coverage({px[2], py[2]}, *quad, edges);
            coverage = Sk4f(mc01, mc01, mc23, mc23);
        }
    } else {
        // This turns into a triangle. Replace corners as needed with the intersections between
        // (e0,e3) and (e1,e2), which must now be calculated
        Sk2f eDenom = SkNx_shuffle<0, 1>(edges.fA) * SkNx_shuffle<3, 2>(edges.fB) -
                      SkNx_shuffle<0, 1>(edges.fB) * SkNx_shuffle<3, 2>(edges.fA);
        Sk2f ex = (SkNx_shuffle<0, 1>(edges.fB) * SkNx_shuffle<3, 2>(oc) -
                   SkNx_shuffle<0, 1>(oc) * SkNx_shuffle<3, 2>(edges.fB)) / eDenom;
        Sk2f ey = (SkNx_shuffle<0, 1>(oc) * SkNx_shuffle<3, 2>(edges.fA) -
                   SkNx_shuffle<0, 1>(edges.fA) * SkNx_shuffle<3, 2>(oc)) / eDenom;

        if (SkScalarAbs(eDenom[0]) > kTolerance) {
            px = d1v0.thenElse(ex[0], px);
            py = d1v0.thenElse(ey[0], py);
        }
        if (SkScalarAbs(eDenom[1]) > kTolerance) {
            px = d2v0.thenElse(ex[1], px);
            py = d2v0.thenElse(ey[1], py);
        }

        coverage = 1.f;
    }

    outset_projected_vertices(px, py, aaFlags, quad);
    return coverage;
}

// Computes the vertices for the two nested quads used to create AA edges. The original single quad
// should be duplicated as input in 'inner' and 'outer', and the resulting quad frame will be
// stored in-place on return. Returns per-vertex coverage for the inner vertices.
static Sk4f compute_nested_quad_vertices(GrQuadAAFlags aaFlags, bool rectilinear,
                                         Vertices* inner, Vertices* outer) {
    SkASSERT(inner->fUVRCount == 0 || inner->fUVRCount == 2 || inner->fUVRCount == 3);
    SkASSERT(outer->fUVRCount == inner->fUVRCount);

    QuadMetadata metadata = get_metadata(*inner, aaFlags);

    // When outsetting, we want the new edge to be .5px away from the old line, which means the
    // corners may need to be adjusted by more than .5px if the matrix had sheer. This adjustment
    // is only computed if there are no empty edges, and it may signal going through the slow path.
    Sk4f outset = 0.5f;
    if (get_optimized_outset(metadata, rectilinear, &outset)) {
       // Since it's not subpixel, outsetting and insetting are trivial vector additions.
        outset_vertices(outset, metadata, outer);
        outset_vertices(-outset, metadata, inner);
        return 1.f;
    }

    // Only compute edge equations once since they are the same for inner and outer quads
    Edges edges = get_edge_equations(metadata, *inner);

    // Calculate both outset and inset, returning the coverage reported for the inset, since the
    // outset will always have 0.0f.
    compute_degenerate_quad(aaFlags, metadata.fMask, edges, true, outer);
    return compute_degenerate_quad(aaFlags, metadata.fMask, edges, false, inner);
}

// Generalizes compute_nested_quad_vertices to extrapolate local coords such that after perspective
// division of the device coordinates, the original local coordinate value is at the original
// un-outset device position.
static Sk4f compute_nested_persp_quad_vertices(const GrQuadAAFlags aaFlags, Vertices* inner,
                                               Vertices* outer) {
    SkASSERT(inner->fUVRCount == 0 || inner->fUVRCount == 2 || inner->fUVRCount == 3);
    SkASSERT(outer->fUVRCount == inner->fUVRCount);

    // Calculate the projected 2D quad and use it to form projeccted inner/outer quads
    // Don't use Sk4f.invert() here because it does not preserve 1/1 == 1, which creates rendering
    // mismatches for 2D content that was batched into a 3D op, vs. 2D on its own.
    Sk4f iw = 1.0f / inner->fW;
    Sk4f x2d = inner->fX * iw;
    Sk4f y2d = inner->fY * iw;

    Vertices inner2D = { x2d, y2d, /*w*/ 1.f, 0.f, 0.f, 0.f, 0 }; // No uvr outsetting in 2D
    Vertices outer2D = inner2D;

    Sk4f coverage = compute_nested_quad_vertices(aaFlags, /* rect */ false, &inner2D, &outer2D);

    // Now map from the 2D inset/outset back to 3D and update the local coordinates as well
    outset_projected_vertices(inner2D.fX, inner2D.fY, aaFlags, inner);
    outset_projected_vertices(outer2D.fX, outer2D.fY, aaFlags, outer);

    return coverage;
}

enum class CoverageMode {
    kNone,
    kWithPosition,
    kWithColor
};

static CoverageMode get_mode_for_spec(const GrQuadPerEdgeAA::VertexSpec& spec) {
    if (spec.usesCoverageAA()) {
        if (spec.compatibleWithAlphaAsCoverage() && spec.hasVertexColors()) {
            return CoverageMode::kWithColor;
        } else {
            return CoverageMode::kWithPosition;
        }
    } else {
        return CoverageMode::kNone;
    }
}

// Writes four vertices in triangle strip order, including the additional data for local
// coordinates, domain, color, and coverage as needed to satisfy the vertex spec.
static void write_quad(GrVertexWriter* vb, const GrQuadPerEdgeAA::VertexSpec& spec,
                       CoverageMode mode, Sk4f coverage,
                       SkPMColor4f color4f, bool wideColor,
                       const SkRect& domain,
                       const Vertices& quad) {
    static constexpr auto If = GrVertexWriter::If<float>;

    for (int i = 0; i < 4; ++i) {
        // save position, this is a float2 or float3 or float4 depending on the combination of
        // perspective and coverage mode.
        vb->write(quad.fX[i], quad.fY[i],
                  If(spec.deviceQuadType() == GrQuadType::kPerspective, quad.fW[i]),
                  If(mode == CoverageMode::kWithPosition, coverage[i]));

        // save color
        if (spec.hasVertexColors()) {
            vb->write(GrVertexColor(
                    color4f * (mode == CoverageMode::kWithColor ? coverage[i] : 1.f), wideColor));
        }

        // save local position
        if (spec.hasLocalCoords()) {
            vb->write(quad.fU[i], quad.fV[i],
                      If(spec.localQuadType() == GrQuadType::kPerspective, quad.fR[i]));
        }

        // save the domain
        if (spec.hasDomain()) {
            vb->write(domain);
        }
    }
}

GR_DECLARE_STATIC_UNIQUE_KEY(gAAFillRectIndexBufferKey);

static const int kVertsPerAAFillRect = 8;
static const int kIndicesPerAAFillRect = 30;

static sk_sp<const GrGpuBuffer> get_index_buffer(GrResourceProvider* resourceProvider) {
    GR_DEFINE_STATIC_UNIQUE_KEY(gAAFillRectIndexBufferKey);

    // clang-format off
    static const uint16_t gFillAARectIdx[] = {
        0, 1, 2, 1, 3, 2,
        0, 4, 1, 4, 5, 1,
        0, 6, 4, 0, 2, 6,
        2, 3, 6, 3, 7, 6,
        1, 5, 3, 3, 5, 7,
    };
    // clang-format on

    GR_STATIC_ASSERT(SK_ARRAY_COUNT(gFillAARectIdx) == kIndicesPerAAFillRect);
    return resourceProvider->findOrCreatePatternedIndexBuffer(
            gFillAARectIdx, kIndicesPerAAFillRect, GrQuadPerEdgeAA::kNumAAQuadsInIndexBuffer,
            kVertsPerAAFillRect, gAAFillRectIndexBufferKey);
}

} // anonymous namespace

namespace GrQuadPerEdgeAA {

////////////////// Tessellate Implementation

void* Tessellate(void* vertices, const VertexSpec& spec, const GrPerspQuad& deviceQuad,
                 const SkPMColor4f& color4f, const GrPerspQuad& localQuad, const SkRect& domain,
                 GrQuadAAFlags aaFlags) {
    bool wideColor = GrQuadPerEdgeAA::ColorType::kHalf == spec.colorType();
    CoverageMode mode = get_mode_for_spec(spec);

    // Load position data into Sk4fs (always x, y, and load w to avoid branching down the road)
    Vertices outer;
    outer.fX = deviceQuad.x4f();
    outer.fY = deviceQuad.y4f();
    outer.fW = deviceQuad.w4f(); // Guaranteed to be 1f if it's not perspective

    // Load local position data into Sk4fs (either none, just u,v or all three)
    outer.fUVRCount = spec.localDimensionality();
    if (spec.hasLocalCoords()) {
        outer.fU = localQuad.x4f();
        outer.fV = localQuad.y4f();
        outer.fR = localQuad.w4f(); // Will be ignored if the local quad type isn't perspective
    }

    GrVertexWriter vb{vertices};
    if (spec.usesCoverageAA()) {
        SkASSERT(mode == CoverageMode::kWithPosition || mode == CoverageMode::kWithColor);
        // Must calculate two new quads, an outset and inset by .5 in projected device space, so
        // duplicate the original quad for the inner space
        Vertices inner = outer;

        Sk4f maxCoverage = 1.f;
        if (spec.deviceQuadType() == GrQuadType::kPerspective) {
            // For perspective, send quads with all edges non-AA through the tessellation to ensure
            // their corners are processed the same as adjacent quads. This approach relies on
            // solving edge equations to reconstruct corners, which can create seams if an inner
            // fully non-AA quad is not similarly processed.
            maxCoverage = compute_nested_persp_quad_vertices(aaFlags, &inner, &outer);
        } else if (aaFlags != GrQuadAAFlags::kNone) {
            // In 2D, the simpler corner math does not cause issues with seaming against non-AA
            // inner quads.
            maxCoverage = compute_nested_quad_vertices(
                    aaFlags, spec.deviceQuadType() <= GrQuadType::kRectilinear, &inner, &outer);
        }
        // NOTE: could provide an even more optimized tessellation function for axis-aligned
        // rects since the positions can be outset by constants without doing vector math,
        // except it must handle identifying the winding of the quad vertices if the transform
        // applied a mirror, etc. The current 2D case is already adequately fast.

        // Write two quads for inner and outer, inner will use the
        write_quad(&vb, spec, mode, maxCoverage, color4f, wideColor, domain, inner);
        write_quad(&vb, spec, mode, 0.f, color4f, wideColor, domain, outer);
    } else {
        // No outsetting needed, just write a single quad with full coverage
        SkASSERT(mode == CoverageMode::kNone);
        write_quad(&vb, spec, mode, 1.f, color4f, wideColor, domain, outer);
    }

    return vb.fPtr;
}

bool ConfigureMeshIndices(GrMeshDrawOp::Target* target, GrMesh* mesh, const VertexSpec& spec,
                          int quadCount) {
    if (spec.usesCoverageAA()) {
        // AA quads use 8 vertices, basically nested rectangles
        sk_sp<const GrGpuBuffer> ibuffer = get_index_buffer(target->resourceProvider());
        if (!ibuffer) {
            return false;
        }

        mesh->setPrimitiveType(GrPrimitiveType::kTriangles);
        mesh->setIndexedPatterned(std::move(ibuffer), kIndicesPerAAFillRect, kVertsPerAAFillRect,
                                  quadCount, kNumAAQuadsInIndexBuffer);
    } else {
        // Non-AA quads use 4 vertices, and regular triangle strip layout
        if (quadCount > 1) {
            sk_sp<const GrGpuBuffer> ibuffer = target->resourceProvider()->refQuadIndexBuffer();
            if (!ibuffer) {
                return false;
            }

            mesh->setPrimitiveType(GrPrimitiveType::kTriangles);
            mesh->setIndexedPatterned(std::move(ibuffer), 6, 4, quadCount,
                                      GrResourceProvider::QuadCountOfQuadBuffer());
        } else {
            mesh->setPrimitiveType(GrPrimitiveType::kTriangleStrip);
            mesh->setNonIndexedNonInstanced(4);
        }
    }

    return true;
}

////////////////// VertexSpec Implementation

int VertexSpec::deviceDimensionality() const {
    return this->deviceQuadType() == GrQuadType::kPerspective ? 3 : 2;
}

int VertexSpec::localDimensionality() const {
    return fHasLocalCoords ? (this->localQuadType() == GrQuadType::kPerspective ? 3 : 2) : 0;
}

////////////////// Geometry Processor Implementation

class QuadPerEdgeAAGeometryProcessor : public GrGeometryProcessor {
public:

    static sk_sp<GrGeometryProcessor> Make(const VertexSpec& spec) {
        return sk_sp<QuadPerEdgeAAGeometryProcessor>(new QuadPerEdgeAAGeometryProcessor(spec));
    }

    static sk_sp<GrGeometryProcessor> Make(const VertexSpec& vertexSpec, const GrShaderCaps& caps,
                                           GrTextureType textureType, GrPixelConfig textureConfig,
                                           const GrSamplerState& samplerState,
                                           uint32_t extraSamplerKey,
                                           sk_sp<GrColorSpaceXform> textureColorSpaceXform) {
        return sk_sp<QuadPerEdgeAAGeometryProcessor>(new QuadPerEdgeAAGeometryProcessor(
                vertexSpec, caps, textureType, textureConfig, samplerState, extraSamplerKey,
                std::move(textureColorSpaceXform)));
    }

    const char* name() const override { return "QuadPerEdgeAAGeometryProcessor"; }

    void getGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder* b) const override {
        // domain, texturing, device-dimensions are single bit flags
        uint32_t x = fDomain.isInitialized() ? 0 : 1;
        x |= fSampler.isInitialized() ? 0 : 2;
        x |= fNeedsPerspective ? 0 : 4;
        // local coords require 2 bits (3 choices), 00 for none, 01 for 2d, 10 for 3d
        if (fLocalCoord.isInitialized()) {
            x |= kFloat3_GrVertexAttribType == fLocalCoord.cpuType() ? 8 : 16;
        }
        // similar for colors, 00 for none, 01 for bytes, 10 for half-floats
        if (fColor.isInitialized()) {
            x |= kUByte4_norm_GrVertexAttribType == fColor.cpuType() ? 32 : 64;
        }
        // and coverage mode, 00 for none, 01 for withposition, 10 for withcolor
        if (fCoverageMode != CoverageMode::kNone) {
            x |= CoverageMode::kWithPosition == fCoverageMode ? 128 : 256;
        }

        b->add32(GrColorSpaceXform::XformKey(fTextureColorSpaceXform.get()));
        b->add32(x);
    }

    GrGLSLPrimitiveProcessor* createGLSLInstance(const GrShaderCaps& caps) const override {
        class GLSLProcessor : public GrGLSLGeometryProcessor {
        public:
            void setData(const GrGLSLProgramDataManager& pdman, const GrPrimitiveProcessor& proc,
                         FPCoordTransformIter&& transformIter) override {
                const auto& gp = proc.cast<QuadPerEdgeAAGeometryProcessor>();
                if (gp.fLocalCoord.isInitialized()) {
                    this->setTransformDataHelper(SkMatrix::I(), pdman, &transformIter);
                }
                fTextureColorSpaceXformHelper.setData(pdman, gp.fTextureColorSpaceXform.get());
            }

        private:
            void onEmitCode(EmitArgs& args, GrGPArgs* gpArgs) override {
                using Interpolation = GrGLSLVaryingHandler::Interpolation;

                const auto& gp = args.fGP.cast<QuadPerEdgeAAGeometryProcessor>();
                fTextureColorSpaceXformHelper.emitCode(args.fUniformHandler,
                                                       gp.fTextureColorSpaceXform.get());

                args.fVaryingHandler->emitAttributes(gp);

                if (gp.fCoverageMode == CoverageMode::kWithPosition) {
                    // Strip last channel from the vertex attribute to remove coverage and get the
                    // actual position
                    if (gp.fNeedsPerspective) {
                        args.fVertBuilder->codeAppendf("float3 position = %s.xyz;",
                                                       gp.fPosition.name());
                    } else {
                        args.fVertBuilder->codeAppendf("float2 position = %s.xy;",
                                                       gp.fPosition.name());
                    }
                    gpArgs->fPositionVar = {"position",
                                            gp.fNeedsPerspective ? kFloat3_GrSLType
                                                                 : kFloat2_GrSLType,
                                            GrShaderVar::kNone_TypeModifier};
                } else {
                    // No coverage to eliminate
                    gpArgs->fPositionVar = gp.fPosition.asShaderVar();
                }

                // Handle local coordinates if they exist
                if (gp.fLocalCoord.isInitialized()) {
                    // NOTE: If the only usage of local coordinates is for the inline texture fetch
                    // before FPs, then there are no registered FPCoordTransforms and this ends up
                    // emitting nothing, so there isn't a duplication of local coordinates
                    this->emitTransforms(args.fVertBuilder,
                                         args.fVaryingHandler,
                                         args.fUniformHandler,
                                         gp.fLocalCoord.asShaderVar(),
                                         args.fFPCoordTransformHandler);
                }

                // Solid color before any texturing gets modulated in
                if (gp.fColor.isInitialized()) {
                    // The color cannot be flat if the varying coverage has been modulated into it
                    args.fVaryingHandler->addPassThroughAttribute(gp.fColor, args.fOutputColor,
                            gp.fCoverageMode == CoverageMode::kWithColor ?
                            Interpolation::kInterpolated : Interpolation::kCanBeFlat);
                } else {
                    // Output color must be initialized to something
                    args.fFragBuilder->codeAppendf("%s = half4(1);", args.fOutputColor);
                }

                // If there is a texture, must also handle texture coordinates and reading from
                // the texture in the fragment shader before continuing to fragment processors.
                if (gp.fSampler.isInitialized()) {
                    // Texture coordinates clamped by the domain on the fragment shader; if the GP
                    // has a texture, it's guaranteed to have local coordinates
                    args.fFragBuilder->codeAppend("float2 texCoord;");
                    if (gp.fLocalCoord.cpuType() == kFloat3_GrVertexAttribType) {
                        // Can't do a pass through since we need to perform perspective division
                        GrGLSLVarying v(gp.fLocalCoord.gpuType());
                        args.fVaryingHandler->addVarying(gp.fLocalCoord.name(), &v);
                        args.fVertBuilder->codeAppendf("%s = %s;",
                                                       v.vsOut(), gp.fLocalCoord.name());
                        args.fFragBuilder->codeAppendf("texCoord = %s.xy / %s.z;",
                                                       v.fsIn(), v.fsIn());
                    } else {
                        args.fVaryingHandler->addPassThroughAttribute(gp.fLocalCoord, "texCoord");
                    }

                    // Clamp the now 2D localCoordName variable by the domain if it is provided
                    if (gp.fDomain.isInitialized()) {
                        args.fFragBuilder->codeAppend("float4 domain;");
                        args.fVaryingHandler->addPassThroughAttribute(gp.fDomain, "domain",
                                                                      Interpolation::kCanBeFlat);
                        args.fFragBuilder->codeAppend(
                                "texCoord = clamp(texCoord, domain.xy, domain.zw);");
                    }

                    // Now modulate the starting output color by the texture lookup
                    args.fFragBuilder->codeAppendf("%s = ", args.fOutputColor);
                    args.fFragBuilder->appendTextureLookupAndModulate(
                        args.fOutputColor, args.fTexSamplers[0], "texCoord", kFloat2_GrSLType,
                        &fTextureColorSpaceXformHelper);
                    args.fFragBuilder->codeAppend(";");
                }

                // And lastly, output the coverage calculation code
                if (gp.fCoverageMode == CoverageMode::kWithPosition) {
                    GrGLSLVarying coverage(kFloat_GrSLType);
                    args.fVaryingHandler->addVarying("coverage", &coverage);
                    if (gp.fNeedsPerspective) {
                        args.fVertBuilder->codeAppendf("%s = %s.w;",
                                                       coverage.vsOut(), gp.fPosition.name());
                    } else {
                        args.fVertBuilder->codeAppendf("%s = %s.z;",
                                                       coverage.vsOut(), gp.fPosition.name());
                    }

                    args.fFragBuilder->codeAppendf("%s = half4(half(%s));",
                                                   args.fOutputCoverage, coverage.fsIn());
                } else {
                    // Set coverage to 1, since it's either non-AA or the coverage was already
                    // folded into the output color
                    args.fFragBuilder->codeAppendf("%s = half4(1);", args.fOutputCoverage);
                }
            }
            GrGLSLColorSpaceXformHelper fTextureColorSpaceXformHelper;
        };
        return new GLSLProcessor;
    }

private:
    QuadPerEdgeAAGeometryProcessor(const VertexSpec& spec)
            : INHERITED(kQuadPerEdgeAAGeometryProcessor_ClassID)
            , fTextureColorSpaceXform(nullptr) {
        SkASSERT(!spec.hasDomain());
        this->initializeAttrs(spec);
        this->setTextureSamplerCnt(0);
    }

    QuadPerEdgeAAGeometryProcessor(const VertexSpec& spec, const GrShaderCaps& caps,
                                   GrTextureType textureType, GrPixelConfig textureConfig,
                                   const GrSamplerState& samplerState,
                                   uint32_t extraSamplerKey,
                                   sk_sp<GrColorSpaceXform> textureColorSpaceXform)
            : INHERITED(kQuadPerEdgeAAGeometryProcessor_ClassID)
            , fTextureColorSpaceXform(std::move(textureColorSpaceXform))
            , fSampler(textureType, textureConfig, samplerState, extraSamplerKey) {
        SkASSERT(spec.hasLocalCoords());
        this->initializeAttrs(spec);
        this->setTextureSamplerCnt(1);
    }

    void initializeAttrs(const VertexSpec& spec) {
        fNeedsPerspective = spec.deviceDimensionality() == 3;
        fCoverageMode = get_mode_for_spec(spec);

        if (fCoverageMode == CoverageMode::kWithPosition) {
            if (fNeedsPerspective) {
                fPosition = {"positionWithCoverage", kFloat4_GrVertexAttribType, kFloat4_GrSLType};
            } else {
                fPosition = {"positionWithCoverage", kFloat3_GrVertexAttribType, kFloat3_GrSLType};
            }
        } else {
            if (fNeedsPerspective) {
                fPosition = {"position", kFloat3_GrVertexAttribType, kFloat3_GrSLType};
            } else {
                fPosition = {"position", kFloat2_GrVertexAttribType, kFloat2_GrSLType};
            }
        }

        int localDim = spec.localDimensionality();
        if (localDim == 3) {
            fLocalCoord = {"localCoord", kFloat3_GrVertexAttribType, kFloat3_GrSLType};
        } else if (localDim == 2) {
            fLocalCoord = {"localCoord", kFloat2_GrVertexAttribType, kFloat2_GrSLType};
        } // else localDim == 0 and attribute remains uninitialized

        if (ColorType::kByte == spec.colorType()) {
            fColor = {"color", kUByte4_norm_GrVertexAttribType, kHalf4_GrSLType};
        } else if (ColorType::kHalf == spec.colorType()) {
            fColor = {"color", kHalf4_GrVertexAttribType, kHalf4_GrSLType};
        }

        if (spec.hasDomain()) {
            fDomain = {"domain", kFloat4_GrVertexAttribType, kFloat4_GrSLType};
        }

        this->setVertexAttributes(&fPosition, 4);
    }

    const TextureSampler& onTextureSampler(int) const override { return fSampler; }

    Attribute fPosition; // May contain coverage as last channel
    Attribute fColor; // May have coverage modulated in if the FPs support it
    Attribute fLocalCoord;
    Attribute fDomain;

    // The positions attribute may have coverage built into it, so float3 is an ambiguous type
    // and may mean 2d with coverage, or 3d with no coverage
    bool fNeedsPerspective;
    CoverageMode fCoverageMode;

    // Color space will be null and fSampler.isInitialized() returns false when the GP is configured
    // to skip texturing.
    sk_sp<GrColorSpaceXform> fTextureColorSpaceXform;
    TextureSampler fSampler;

    typedef GrGeometryProcessor INHERITED;
};

sk_sp<GrGeometryProcessor> MakeProcessor(const VertexSpec& spec) {
    return QuadPerEdgeAAGeometryProcessor::Make(spec);
}

sk_sp<GrGeometryProcessor> MakeTexturedProcessor(const VertexSpec& spec, const GrShaderCaps& caps,
        GrTextureType textureType, GrPixelConfig textureConfig,
        const GrSamplerState& samplerState, uint32_t extraSamplerKey,
        sk_sp<GrColorSpaceXform> textureColorSpaceXform) {
    return QuadPerEdgeAAGeometryProcessor::Make(spec, caps, textureType, textureConfig,
                                                samplerState, extraSamplerKey,
                                                std::move(textureColorSpaceXform));
}

} // namespace GrQuadPerEdgeAA
