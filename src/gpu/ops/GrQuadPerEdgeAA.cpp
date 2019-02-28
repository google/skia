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

static AI Sk4f fma(const Sk4f& f, const Sk4f& m, const Sk4f& a) {
    return SkNx_fma<4, float>(f, m, a);
}

// Calculate area of intersection between quad (xs, ys) and a pixel at 'pixelCenter'.
// a, b, c are edge equations of the quad, flipped is true if the line equations had their normals
// reversed to correct for matrix transforms.
static float get_exact_coverage(const SkPoint& pixelCenter, const Sk4f& xs, const Sk4f& ys,
                                const Sk4f& a, const Sk4f& b, const Sk4f& c, bool flipped) {
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
    bool topleftInside = ((a * left + b * top + c) >= 0.f).allTrue();
    bool botleftInside = ((a * left + b * bot + c) >= 0.f).allTrue();
    bool botrightInside = ((a * right + b * bot + c) >= 0.f).allTrue();
    bool toprightInside = ((a * right + b * top + c) >= 0.f).allTrue();
    if (topleftInside && botleftInside && botrightInside && toprightInside) {
        // Quad fully contains the pixel, so we know the area will be 1.f
        return 1.f;
    }

    // Track whether or not the quad vertices in (xs, ys) are on the proper sides of l, t, r, and b
    Sk4i leftValid = SkNx_cast<int32_t>(xs >= left);
    Sk4i rightValid = SkNx_cast<int32_t>(xs <= right);
    Sk4i topValid = SkNx_cast<int32_t>(ys >= top);
    Sk4i botValid = SkNx_cast<int32_t>(ys <= bot);

    // Intercepts of quad lines with the 4 pixel edges
    Sk4f leftCross = -(c + a * left) / b;
    Sk4f rightCross = -(c + a * right) / b;
    Sk4f topCross = -(c + b * top) / a;
    Sk4f botCross = -(c + b * bot) / a;

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
    if (SIDE##Cross[ei] >= top && SIDE##Cross[ei] <= bot) { \
        accumulate({SIDE, SIDE##Cross[ei]}); \
        addedIntersection = true; \
    }
#define ADD_EDGE_CROSSING_Y(SIDE) \
    if (SIDE##Cross[ei] >= left && SIDE##Cross[ei] <= right) { \
        accumulate({SIDE##Cross[ei], SIDE}); \
        addedIntersection = true; \
    }
#define TEST_EDGES(SIDE, AXIS, I, NI) \
    if (!SIDE##Valid[I] && SIDE##Valid[NI]) { \
        ADD_EDGE_CROSSING_##AXIS(SIDE); \
        crossedEdges = true; \
    }
    // Used during iteration over the quad points to check if a pixel corner should be included
    // in the intersection boundary
#define ADD_CORNER(CHECK, SIDE_LR, SIDE_TB) \
    if (!CHECK##Valid[i] || !CHECK##Valid[ni]) { \
        if (SIDE_TB##SIDE_LR##Inside) { \
            accumulate({SIDE_LR, SIDE_TB}); \
        } \
    }
#define TEST_CORNER_X(SIDE, I, NI) \
    if (!SIDE##Valid[I] && SIDE##Valid[NI]) { \
        ADD_CORNER(top, SIDE, top) else ADD_CORNER(bot, SIDE, bot); \
    }
#define TEST_CORNER_Y(SIDE, I, NI) \
    if (!SIDE##Valid[I] && SIDE##Valid[NI]) { \
        ADD_CORNER(left, left, SIDE) else ADD_CORNER(right, right, SIDE); \
    }

    // Iterate over the 4 points of the quad, adding valid intersections with the pixel edges
    // or adding interior pixel corners as it goes. This automatically keeps all accumulated points
    // in CCW ordering so the area can be calculated on the fly and there's no need to store the
    // list of hull points. This is somewhat inspired by the Sutherland-Hodgman algorithm but since
    // there are only 4 points in each source polygon, there is no point list maintenance.
    for (int j = 0; j < 4; ++j) {
        // Current vertex
        int i = flipped ? kFlippedCCW[j] : kCCW[j];
        // Moving to this vertex
        int ni = flipped ? kFlippedCCW[(j + 1) % 4] : kCCW[(j + 1) % 4];
        // Index in edge vectors corresponding to move from i to ni
        int ei = flipped ? ni : i;

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
            accumulate({xs[ni], ys[ni]});
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

// These rotate the points/edge values either clockwise or counterclockwise assuming tri strip
// order.
static AI Sk4f nextCW(const Sk4f& v) {
    return SkNx_shuffle<2, 0, 3, 1>(v);
}

static AI Sk4f nextCCW(const Sk4f& v) {
    return SkNx_shuffle<1, 3, 0, 2>(v);
}

// Fills Sk4f with 1f if edge bit is set, 0f otherwise. Edges are ordered LBTR to match CCW ordering
// of vertices in the quad.
static AI Sk4f compute_edge_mask(GrQuadAAFlags aaFlags) {
    return Sk4f((GrQuadAAFlags::kLeft & aaFlags) ? 1.f : 0.f,
                (GrQuadAAFlags::kBottom & aaFlags) ? 1.f : 0.f,
                (GrQuadAAFlags::kTop & aaFlags) ? 1.f : 0.f,
                (GrQuadAAFlags::kRight & aaFlags) ? 1.f : 0.f);
}

// Outputs normalized edge vectors in xdiff and ydiff, as well as the reciprocal of the original
// edge lengths in invLengths
static AI void compute_edge_vectors(const Sk4f& x, const Sk4f& y, const Sk4f& xnext,
                                    const Sk4f& ynext, Sk4f* xdiff, Sk4f* ydiff, Sk4f* invLengths) {
    *xdiff = xnext - x;
    *ydiff = ynext - y;
    *invLengths = fma(*xdiff, *xdiff, *ydiff * *ydiff).rsqrt();
    *xdiff *= *invLengths;
    *ydiff *= *invLengths;
}

static AI void outset_masked_vertices(const Sk4f& outset, const Sk4f& xdiff, const Sk4f& ydiff,
                                      const Sk4f& invLengths, const Sk4f& mask,
                                      Sk4f* x, Sk4f* y, Sk4f* u, Sk4f* v, Sk4f* r, int uvrCount) {
    // The mask is rotated compared to the outsets and edge vectors, since if the edge is "on"
    // both its points need to be moved along their other edge vectors.
    auto maskedOutset = -outset * nextCW(mask);
    auto maskedOutsetCW = outset * mask;
    // x = x + outset * mask * nextCW(xdiff) - outset * nextCW(mask) * xdiff
    *x += fma(maskedOutsetCW, nextCW(xdiff), maskedOutset * xdiff);
    *y += fma(maskedOutsetCW, nextCW(ydiff), maskedOutset * ydiff);
    if (uvrCount > 0) {
        // We want to extend the texture coords by the same proportion as the positions.
        maskedOutset *= invLengths;
        maskedOutsetCW *= nextCW(invLengths);
        Sk4f udiff = nextCCW(*u) - *u;
        Sk4f vdiff = nextCCW(*v) - *v;
        *u += fma(maskedOutsetCW, nextCW(udiff), maskedOutset * udiff);
        *v += fma(maskedOutsetCW, nextCW(vdiff), maskedOutset * vdiff);
        if (uvrCount == 3) {
            Sk4f rdiff = nextCCW(*r) - *r;
            *r += fma(maskedOutsetCW, nextCW(rdiff), maskedOutset * rdiff);
        }
    }
}

static AI void outset_vertices(const Sk4f& outset, const Sk4f& xdiff, const Sk4f& ydiff,
                               const Sk4f& invLengths, Sk4f* x, Sk4f* y, Sk4f* u, Sk4f* v, Sk4f* r,
                               int uvrCount) {
    // x = x + outset * nextCW(xdiff) - outset * xdiff (as above, but where mask = (1,1,1,1))
    *x += fma(outset, nextCW(xdiff), -outset * xdiff);
    *y += fma(outset, nextCW(ydiff), -outset * ydiff);
    if (uvrCount > 0) {
        Sk4f t = -outset * invLengths; // Bake minus sign in here
        Sk4f tCW = outset * nextCW(invLengths);
        Sk4f udiff = nextCCW(*u) - *u;
        Sk4f vdiff = nextCCW(*v) - *v;
        *u += fma(tCW, nextCW(udiff), t * udiff);
        *v += fma(tCW, nextCW(vdiff), t * vdiff);
        if (uvrCount == 3) {
            Sk4f rdiff = nextCCW(*r) - *r;
            *r += fma(tCW, nextCW(rdiff), t * rdiff);
        }
    }
}

// Updates (x,y,w) to be at (x2d,y2d) once projected. Updates (u,v,r) to match if provided.
// Gracefully handles 2D content if *w holds all 1s. Doesn't need aa flags since (x2d,y2d)
// encodes that information already.
static void outset_projected_vertices(const Sk4f& x2d, const Sk4f& y2d, Sk4f* x, Sk4f* y, Sk4f* w,
                                      Sk4f* u, Sk4f* v, Sk4f* r, int uvrCount) {
    // Calculate 3D quad edges, but don't bother normalizing since the lengths will be baked into
    // the solved equations.
    Sk4f ex = nextCCW(*x) - *x;
    Sk4f ey = nextCCW(*y) - *y;
    Sk4f ew = nextCCW(*w) - *w;
    // Can only move along -e and nextCW(e) to reach the new 2D point, which means we have
    // x2d = (x - a*ex + b*cw(ex)) / (w - a*ew + b*cw(ew)) and
    // y2d = (y - a*ey + b*cw(ey)) / (w - a*ew + b*cw(ew)) for some a, b.
    // Solve for a and b to get scale factors when adding edges to (x,y,w) to get the outset point.
    Sk4f c1x = x2d * ew - ex;
    Sk4f c1y = y2d * ew - ey;
    Sk4f c2x = nextCW(ex) - x2d * nextCW(ew);
    Sk4f c2y = nextCW(ey) - y2d * nextCW(ew);
    Sk4f c3x = *x - x2d * (*w);
    Sk4f c3y = *y - y2d * (*w);

    // a * c1x + b * c2x + c3x = 0, a * c1y + b * c2y + c3y = 0, solved for a,b
    Sk4f denom = c1x * c2y - c2x * c1y;
    Sk4f a = (c2x * c3y - c3x * c2y) / denom;
    Sk4f b = (c3x * c1y - c1x * c3y) / denom;

    *x = *x - a * ex + b * nextCW(ex);
    *y = *y - a * ey + b * nextCW(ey);
    *w = *w - a * ew + b * nextCW(ew);

    // Now outset the texture coordinates to match new positions
    if (uvrCount > 0) {
        Sk4f eu = nextCCW(*u) - *u;
        Sk4f ev = nextCCW(*v) - *v;
        *u = *u - a * eu + b * nextCW(eu);
        *v = *v - a * ev + b * nextCW(ev);
        if (uvrCount == 3) {
            Sk4f er = nextCCW(*r) - *r;
            *r = *r - a * er + b * nextCW(er);
        }
    }
}

// Updates outset in place to account for non-90 degree angles of the quad edges stored in
// xdiff, ydiff (which are assumed to be normalized). Returns true if the fast path for
// inset/outsetting is still appropriate.
static bool adjust_non_rectilinear_outset(const Sk4f& xdiff, const Sk4f& ydiff, Sk4f* outset) {
    // The distance the point needs to move is outset/sqrt(1-cos^2(theta)), where theta is the angle
    // between the two edges at that point. cos(theta) is equal to dot(xydiff, nextCW(xydiff)),
    Sk4f cosTheta = fma(xdiff, nextCW(xdiff), ydiff * nextCW(ydiff));
    *outset *= (1.f - cosTheta * cosTheta).rsqrt();
    // But clamp to make sure we don't expand by a giant amount if the sheer is really high
    *outset = Sk4f::Max(-100.f, Sk4f::Min(*outset, 100.f));
}

static constexpr float kTolerance = 1e-2f;

// Assumes no zero-length edges. Returns true if the normals have been flipped.
static bool get_edge_equations(const Sk4f& xs, const Sk4f& ys, const Sk4f& dx, const Sk4f& dy,
                               Sk4f* a, Sk4f* b, Sk4f* c) {
    *c = fma(dx, ys, -dy * xs);
    // Make sure normals point into the shape
    Sk4f test = fma(dy, nextCW(xs), fma(-dx, nextCW(ys), *c));
    if ((test < -kTolerance).anyTrue()) {
        // Flip
        *a = -dy;
        *b = dx;
        *c = -*c;

        return true;
    } else {
        *a = dy;
        *b = -dx;

        return false;
    }
}

// Outsets or insets xs/ys in place. To be used when the interior is very small, edges are near
// parallel, or edges are very short/zero-length. Returns coverage for each vertex.
// FIXME take the u, v, r pointers and call outset_projected_vertices before rewriting xs,ys
// with the new points.
static Sk4f compute_degenerate_quad_vertices(Sk4f* xs, Sk4f* ys, const Sk4f& xdiff,
                                             const Sk4f& ydiff, const Sk4f& lengths,
                                             GrQuadAAFlags aaFlags, bool outset) {
    Sk4f mask = compute_edge_mask(aaFlags);

    Sk4f badEdge = lengths < kTolerance;
    // Correct for bad edges by copying adjacent edge information into the bad component
    // Sk4f edgeLengths = badEdge.thenElse(nextCW(lengths), lengths);
    Sk4f dx = badEdge.thenElse(nextCW(xdiff), xdiff);
    Sk4f dy = badEdge.thenElse(nextCW(ydiff), ydiff);

    // Calculate original edge equations, correcting for any winding inversion from transform then
    // adjust by insetting or outsetting.
    Sk4f a, b, c; // ax + by + c = 0
    bool flipped = get_edge_equations(*xs, *ys, dx, dy, &a, &b, &c);

    // Shift distance from the line to get the supporting lines of the convex polygon
    c += mask * (outset ? 0.5f : -0.5f);

    // There are 6 points that we care about to determine the final shape of the polygon, which
    // are the intersections between (e0,e2), (e1,e0), (e2,e3), (e3,e1) (corresponding to the
    // 4 corners), and (e1, e2), (e0, e3) (representing the intersections of opposite edges).
    Sk4f denom = a * nextCW(b) - b * nextCW(a);
    Sk4f px = (b * nextCW(c) - c * nextCW(b)) / denom;
    Sk4f py = (c * nextCW(a) - a * nextCW(c)) / denom;
    // The original empty edges have their edge equations replaced with the CW line, so their
    // intersection point will be invalid. Replace with the *CCW* point.
    badEdge = denom.abs() < kTolerance;
    px = badEdge.thenElse(nextCCW(px), px);
    py = badEdge.thenElse(nextCCW(py), py);

    // Calculate the signed distances from these 4 corners to the other two edges that did not
    // define the intersection. So p(0) is compared to e3,e1, p(1) to e3,e2 , p(2) to e0,e1, and
    // p(3) to e0,e2
    Sk4f dists1 = px * SkNx_shuffle<3, 3, 0, 0>(a) + py * SkNx_shuffle<3, 3, 0, 0>(b) +
                  SkNx_shuffle<3, 3, 0, 0>(c);
    Sk4f dists2 = px * SkNx_shuffle<1, 2, 1, 2>(a) + py * SkNx_shuffle<1, 2, 1, 2>(b) +
                  SkNx_shuffle<1, 2, 1, 2>(c);

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
    if (!d1Or2[0] && !d1Or2[1] && !d1Or2[2] && !d1Or2[3]) {
        // Every dists1 and dists2 >= kTolerance so it's not degenerate and use all 4 corners.
        *xs = px;
        *ys = py;
        return 1.f;
    } else if (d1And2[0] || d1And2[1] || d1And2[2] || d1And2[2]) {
        // A point failed against two edges, so reduce the shape to a single point, which we take as
        // the center of the original quad to ensure it is contained in the intended geometry. Since
        // it has collapsed, we know the shape cannot cover a pixel so update the coverage.
        SkPoint center = {0.25f * ((*xs)[0] + (*xs)[1] + (*xs)[2] + (*xs)[3]),
                          0.25f * ((*ys)[0] + (*ys)[1] + (*ys)[2] + (*ys)[3])};
                          // FIXME undo adjustment to c since this should be original edge equations
        float maxCoverage = get_exact_coverage(center, *xs, *ys, a, b, c, flipped);
        *xs = center.fX;
        *ys = center.fY;
        return maxCoverage;
    } else if (d1Or2[0] && d1Or2[1] && d1Or2[2] && d1Or2[3]) {
        // Degenerates to a line. Compare p[2] and p[3] to edge 0. If they are on the wrong side,
        // that means edge 0 and 3 crossed, and otherwise edge 1 and 2 crossed.
        if (dists1[2] < kTolerance && dists1[3] < kTolerance) {
            // Edges 0 and 3 have crossed over, so make the line from average of (p0,p2) and (p1,p3)
            SkPoint p1 = {0.5f * (px[0] + px[2]), 0.5f * (py[0] + py[2])};
            SkPoint p2 = {0.5f * (px[1] + px[3]), 0.5f * (py[1] + py[3])};
            // FIXME undo adjustment to c
            float mc02 = get_exact_coverage(p1, *xs, *ys, a, b, c, flipped);
            float mc13 = get_exact_coverage(p2, *xs, *ys, a, b, c, flipped);
            *xs = Sk4f(p1.fX, p2.fX, p1.fX, p2.fX);
            *ys = Sk4f(p1.fY, p2.fY, p1.fY, p2.fY);
            return Sk4f(mc02, mc13, mc02, mc13);
        } else {
            // Edges 1 and 2 have crossed over, so make the line from average of (p0,p1) and (p2,p3)
            SkPoint p1 = {0.5f * (px[0] + px[1]), 0.5f * (py[0] + py[1])};
            SkPoint p2 = {0.5f * (px[2] + px[3]), 0.5f * (py[2] + py[3])};
            // FIXME undo adjustment to c
            float mc01 = get_exact_coverage(p1, *xs, *ys, a, b, c, flipped);
            float mc23 = get_exact_coverage(p2, *xs, *ys, a, b, c, flipped);
            *xs = Sk4f(p1.fX, p1.fX, p2.fX, p2.fX);
            *ys = Sk4f(p1.fY, p1.fY, p2.fY, p2.fY);
            return Sk4f(mc01, mc01, mc23, mc23);
        }
    } else {
        // This turns into a triangle. Replace corners as needed with the intersections between
        // (e0,e3) and (e1,e2), which must now be calculated
        Sk2f eDenom = SkNx_shuffle<0, 1>(a) * SkNx_shuffle<3, 2>(b) -
                      SkNx_shuffle<0, 1>(b) * SkNx_shuffle<3, 2>(a);
        Sk2f ex = (SkNx_shuffle<0, 1>(b) * SkNx_shuffle<3, 2>(c) -
                   SkNx_shuffle<0, 1>(c) * SkNx_shuffle<3, 2>(b)) / eDenom;
        Sk2f ey = (SkNx_shuffle<0, 1>(c) * SkNx_shuffle<3, 2>(a) -
                   SkNx_shuffle<0, 1>(a) * SkNx_shuffle<3, 2>(c)) / eDenom;

        if (SkScalarAbs(eDenom[0]) > kTolerance) {
            px = d1v0.thenElse(ex[0], px);
            py = d1v0.thenElse(ey[0], py);
        }
        if (SkScalarAbs(eDenom[1]) > kTolerance) {
            px = d2v0.thenElse(ex[1], px);
            py = d2v0.thenElse(ey[1], py);
        }

        *xs = px;
        *ys = py;
        return 1.f;
    }
}

// Computes the vertices for the two nested quads used to create AA edges. The original single quad
// should be duplicated as input in x1 and x2, y1 and y2, and possibly u1|u2, v1|v2, [r1|r2]
// (controlled by uvrChannelCount).  While the values should be duplicated, they should be separate
// pointers. The outset quad is written in-place back to x1, y1, etc. and the inset inner quad is
// written to x2, y2, etc.
static Sk4f compute_nested_quad_vertices(GrQuadAAFlags aaFlags, Sk4f* x1, Sk4f* y1,
        Sk4f* u1, Sk4f* v1, Sk4f* r1, Sk4f* x2, Sk4f* y2, Sk4f* u2, Sk4f* v2, Sk4f* r2,
        int uvrCount, bool rectilinear) {
    SkASSERT(uvrCount == 0 || uvrCount == 2 || uvrCount == 3);

    // Compute edge vectors for the quad.
    auto xnext = nextCCW(*x1);
    auto ynext = nextCCW(*y1);
    // xdiff and ydiff will comprise the normalized vectors pointing along each quad edge.
    Sk4f xdiff, ydiff, invLengths;
    compute_edge_vectors(*x1, *y1, xnext, ynext, &xdiff, &ydiff, &invLengths);
    Sk4f lengths = invLengths.invert();

    // When outsetting, we want the new edge to be .5px away from the old line, which means the
    // corners may need to be adjusted by more than .5px if the matrix had sheer. This adjustment
    // is only computed if there are no empty edges, and it may signal going through the slow path.
    Sk4f outset = 0.5f;
    if ((lengths < 1.f).anyTrue() ||
        (!rectilinear && !adjust_non_rectilinear_outset(xdiff, ydiff, &outset))) {
        // Go through the slow path, returning the coverage for the inner quad
        // FIXME must correctly outset/inset the uvr coords for each too
        compute_degenerate_quad_vertices(x1, y1, xdiff, ydiff, lengths, aaFlags, true);
        return compute_degenerate_quad_vertices(x2, y2, xdiff, ydiff, lengths, aaFlags, false);
    }

    // Since it's not subpixel, outsetting and insetting are trivial vector additions.
    Sk4f inset = -outset;
    if (aaFlags != GrQuadAAFlags::kAll) {
        Sk4f mask = compute_edge_mask(aaFlags);
        outset_masked_vertices(outset, xdiff, ydiff, invLengths, mask, x1, y1,
                               u1, v1, r1, uvrCount);
        outset_masked_vertices(inset, xdiff, ydiff, invLengths, mask, x2, y2,
                               u2, v2, r2, uvrCount);
    } else {
        outset_vertices(outset, xdiff, ydiff, invLengths, x1, y1, u1, v1, r1, uvrCount);
        outset_vertices(inset, xdiff, ydiff, invLengths, x2, y2, u2, v2, r2, uvrCount);
    }

    return 1.f;
}

// For each device space corner, devP, label its left/right or top/bottom opposite device space
// point opDevPt. The new device space point is opDevPt + s (devPt - opDevPt) where s is
// (length(devPt - opDevPt) + outset) / length(devPt - opDevPt); This returns the interpolant s,
// adjusted for any subpixel corrections. If subpixel, it also updates the max coverage.
static Sk4f get_projected_interpolant(const Sk4f& len, const Sk4f& outsets, float* maxCoverage) {
    if ((len < 1.f).anyTrue()) {
        *maxCoverage *= len.min();

        // When insetting, the amount is clamped to be half the minimum edge length to prevent
        // overlap. When outsetting, the amount is padded to cover 2 pixels.
        if ((outsets < 0.f).anyTrue()) {
            return (len - 0.5f * len.min()) / len;
        } else {
            return (len + outsets * (2.f - len.min())) / len;
        }
    } else {
        return (len + outsets) / len;
    }
}

// Generalizes compute_nested_quad_vertices to extrapolate local coords such that
// after perspective division of the device coordinate, the original local coordinate value is at
// the original un-outset device position. r is the local coordinate's w component. However, since
// the projected edges will be different for inner and outer quads, there isn't much reuse between
// the calculations, so it's easier to just have this operate on one quad a time.
// FIXME take both inner and outer quad data
static float compute_quad_persp_vertices(GrQuadAAFlags aaFlags, Sk4f* x, Sk4f* y,
        Sk4f* w, Sk4f* u, Sk4f* v, Sk4f* r, int uvrCount, bool inset) {
    SkASSERT(uvrCount == 0 || uvrCount == 2 || uvrCount == 3);

    auto iw = (*w).invert();
    auto x2d = (*x) * iw;
    auto y2d = (*y) * iw;

    // FIXME get the projected inner and outer 2d quads by calling compute_nexted_quad_vertices,
    // but with no uvs specified.

    // Then call outset_projected_vertices for both, but pass in the original w's instead of 1s
    // and pass in the uvs this time as well.
    // Then should be able to delete all of the code below, and get_projected_interpolant.

    // Should return the max coverage returned from the inner 2d quad "outset".

    // Must compute non-rectilinear outset quantity using the projected 2d edge vectors
    Sk4f xdiff, ydiff, invLengths;
    compute_edge_vectors(x2d, y2d, nextCCW(x2d), nextCCW(y2d), &xdiff, &ydiff, &invLengths);
    Sk4f outset = inset ? -0.5f : 0.5f;
    adjust_non_rectilinear_outset(xdiff, ydiff, &outset);

    float maxProjectedCoverage = 1.f;

    if ((GrQuadAAFlags::kLeft | GrQuadAAFlags::kRight) & aaFlags) {
        // For each entry in x the equivalent entry in opX is the left/right opposite and so on.
        Sk4f opX = SkNx_shuffle<2, 3, 0, 1>(*x);
        Sk4f opW = SkNx_shuffle<2, 3, 0, 1>(*w);
        Sk4f opY = SkNx_shuffle<2, 3, 0, 1>(*y);
        // vx/vy holds the device space left-to-right vectors along top and bottom of the quad.
        Sk2f vx = SkNx_shuffle<2, 3>(x2d) - SkNx_shuffle<0, 1>(x2d);
        Sk2f vy = SkNx_shuffle<2, 3>(y2d) - SkNx_shuffle<0, 1>(y2d);
        Sk4f len = SkNx_shuffle<0, 1, 0, 1>(SkNx_fma(vx, vx, vy * vy).sqrt());

        // Compute t in homogeneous space from s using similar triangles so that we can produce
        // homogeneous outset vertices for perspective-correct interpolation.
        Sk4f s = get_projected_interpolant(len, outset, &maxProjectedCoverage);
        Sk4f sOpW = s * opW;
        Sk4f t = sOpW / (sOpW + (1.f - s) * (*w));
        // mask is used to make the t values be 1 when the left/right side is not antialiased.
        Sk4f mask(GrQuadAAFlags::kLeft & aaFlags  ? 1.f : 0.f,
                  GrQuadAAFlags::kLeft & aaFlags  ? 1.f : 0.f,
                  GrQuadAAFlags::kRight & aaFlags ? 1.f : 0.f,
                  GrQuadAAFlags::kRight & aaFlags ? 1.f : 0.f);
        t = t * mask + (1.f - mask);
        *x = opX + t * (*x - opX);
        *y = opY + t * (*y - opY);
        *w = opW + t * (*w - opW);

        if (uvrCount > 0) {
            Sk4f opU = SkNx_shuffle<2, 3, 0, 1>(*u);
            Sk4f opV = SkNx_shuffle<2, 3, 0, 1>(*v);
            *u = opU + t * (*u - opU);
            *v = opV + t * (*v - opV);
            if (uvrCount == 3) {
                Sk4f opR = SkNx_shuffle<2, 3, 0, 1>(*r);
                *r = opR + t * (*r - opR);
            }
        }

        if ((GrQuadAAFlags::kTop | GrQuadAAFlags::kBottom) & aaFlags) {
            // Update the 2D points for the top/bottom calculation.
            iw = (*w).invert();
            x2d = (*x) * iw;
            y2d = (*y) * iw;
        }
    }

    if ((GrQuadAAFlags::kTop | GrQuadAAFlags::kBottom) & aaFlags) {
        // This operates the same as above but for top/bottom rather than left/right.
        Sk4f opX = SkNx_shuffle<1, 0, 3, 2>(*x);
        Sk4f opW = SkNx_shuffle<1, 0, 3, 2>(*w);
        Sk4f opY = SkNx_shuffle<1, 0, 3, 2>(*y);

        Sk2f vx = SkNx_shuffle<1, 3>(x2d) - SkNx_shuffle<0, 2>(x2d);
        Sk2f vy = SkNx_shuffle<1, 3>(y2d) - SkNx_shuffle<0, 2>(y2d);
        Sk4f len = SkNx_shuffle<0, 0, 1, 1>(SkNx_fma(vx, vx, vy * vy).sqrt());

        Sk4f s = get_projected_interpolant(len, outset, &maxProjectedCoverage);
        Sk4f sOpW = s * opW;
        Sk4f t = sOpW / (sOpW + (1.f - s) * (*w));

        Sk4f mask(GrQuadAAFlags::kTop    & aaFlags ? 1.f : 0.f,
                  GrQuadAAFlags::kBottom & aaFlags ? 1.f : 0.f,
                  GrQuadAAFlags::kTop    & aaFlags ? 1.f : 0.f,
                  GrQuadAAFlags::kBottom & aaFlags ? 1.f : 0.f);
        t = t * mask + (1.f - mask);
        *x = opX + t * (*x - opX);
        *y = opY + t * (*y - opY);
        *w = opW + t * (*w - opW);

        if (uvrCount > 0) {
            Sk4f opU = SkNx_shuffle<1, 0, 3, 2>(*u);
            Sk4f opV = SkNx_shuffle<1, 0, 3, 2>(*v);
            *u = opU + t * (*u - opU);
            *v = opV + t * (*v - opV);
            if (uvrCount == 3) {
                Sk4f opR = SkNx_shuffle<1, 0, 3, 2>(*r);
                *r = opR + t * (*r - opR);
            }
        }
    }

    return maxProjectedCoverage;
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
                       CoverageMode mode, float coverage,
                       SkPMColor4f color4f, bool wideColor,
                       const SkRect& domain,
                       const Sk4f& x, const Sk4f& y, const Sk4f& w,
                       const Sk4f& u, const Sk4f& v, const Sk4f& r) {
    static constexpr auto If = GrVertexWriter::If<float>;

    if (mode == CoverageMode::kWithColor) {
        // Multiply the color by the coverage up front
        SkASSERT(spec.hasVertexColors());
        color4f = color4f * coverage;
    }
    GrVertexColor color(color4f, wideColor);

    for (int i = 0; i < 4; ++i) {
        // save position, this is a float2 or float3 or float4 depending on the combination of
        // perspective and coverage mode.
        vb->write(x[i], y[i], If(spec.deviceQuadType() == GrQuadType::kPerspective, w[i]),
                  If(mode == CoverageMode::kWithPosition, coverage));

        // save color
        if (spec.hasVertexColors()) {
            vb->write(color);
        }

        // save local position
        if (spec.hasLocalCoords()) {
            vb->write(u[i], v[i], If(spec.localQuadType() == GrQuadType::kPerspective, r[i]));
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
    Sk4f oX = deviceQuad.x4f();
    Sk4f oY = deviceQuad.y4f();
    Sk4f oW = deviceQuad.w4f(); // Guaranteed to be 1f if it's not perspective

    // Load local position data into Sk4fs (either none, just u,v or all three)
    Sk4f oU, oV, oR;
    if (spec.hasLocalCoords()) {
        oU = localQuad.x4f();
        oV = localQuad.y4f();
        oR = localQuad.w4f(); // Will be ignored if the local quad type isn't perspective
    }

    GrVertexWriter vb{vertices};
    if (spec.usesCoverageAA()) {
        SkASSERT(mode == CoverageMode::kWithPosition || mode == CoverageMode::kWithColor);

        // Must calculate two new quads, an outset and inset by .5 in projected device space, so
        // duplicate the original quad into new Sk4fs for the inset.
        Sk4f iX = oX, iY = oY, iW = oW;
        Sk4f iU = oU, iV = oV, iR = oR;

        float maxCoverage = 1.f;
        if (aaFlags != GrQuadAAFlags::kNone) {
            if (spec.deviceQuadType() == GrQuadType::kPerspective) {
                // Outset and inset the quads independently because perspective makes each shift
                // unique. Since iX copied pre-outset oX, this will compute the proper inset too.
                compute_quad_persp_vertices(aaFlags, &oX, &oY, &oW, &oU, &oV, &oW,
                                            spec.localDimensionality(), /* inset */ false);
                // Save coverage limit when computing inset quad
                maxCoverage = compute_quad_persp_vertices(aaFlags, &iX, &iY, &iW, &iU, &iV, &iW,
                                                          spec.localDimensionality(), true);
            } else {
                // In the 2D case, insetting and outsetting can reuse the edge vectors, so the
                // nested quads are computed together
                maxCoverage = compute_nested_quad_vertices(aaFlags, &oX, &oY, &oU, &oV, &oR,
                        &iX, &iY, &iU, &iV, &iR, spec.localDimensionality(),
                        spec.deviceQuadType() <= GrQuadType::kRectilinear);
            }
            // NOTE: could provide an even more optimized tessellation function for axis-aligned
            // rects since the positions can be outset by constants without doing vector math,
            // except it must handle identifying the winding of the quad vertices if the transform
            // applied a mirror, etc. The current 2D case is already adequately fast.
        } // else don't adjust any positions, let the outer quad form degenerate triangles

        // Write two quads for inner and outer, inner will use the
        write_quad(&vb, spec, mode, maxCoverage, color4f, wideColor, domain,
                   iX, iY, iW, iU, iV, iR);
        write_quad(&vb, spec, mode, 0.f, color4f, wideColor, domain, oX, oY, oW, oU, oV, oR);
    } else {
        // No outsetting needed, just write a single quad with full coverage
        SkASSERT(mode == CoverageMode::kNone);
        write_quad(&vb, spec, mode, 1.f, color4f, wideColor, domain, oX, oY, oW, oU, oV, oR);
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
