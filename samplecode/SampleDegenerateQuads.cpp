/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Sample.h"

#include "GrQuad.h"
#include "ops/GrQuadPerEdgeAA.h"

#include "SkCanvas.h"
#include "SkDashPathEffect.h"
#include "SkNx.h"
#include "SkPaint.h"
#include "SkPathOps.h"

// Draw a line through the two points, outset by a fixed length in screen space
static void draw_extended_line(SkCanvas* canvas, const SkPaint paint,
                              const SkPoint& p0, const SkPoint& p1, SkScalar extension = 0.f,
                              bool showNormal = false) {
    SkVector v = p1 - p0;
    v.setLength(v.length() + extension);
    canvas->drawLine(p1 - v, p0 + v, paint);

    // Draw normal vector too
    if (showNormal) {
        SkPaint normalPaint = paint;
        normalPaint.setPathEffect(nullptr);
        normalPaint.setStrokeWidth(paint.getStrokeWidth() / 4.f);

        SkVector n = {v.fY, -v.fX};
        n.setLength(.25f);
        SkPoint m = (p0 + p1) * 0.5f;
        canvas->drawLine(m, m + n, normalPaint);
    }
}

static void make_aa_line(const SkPoint& p0, const SkPoint& p1, bool aaOn,
                         bool outset, SkPoint line[2]) {
    SkVector n = {0.f, 0.f};
    if (aaOn) {
        SkVector v = p1 - p0;
        n = outset ? SkVector::Make(v.fY, -v.fX) : SkVector::Make(-v.fY, v.fX);
        n.setLength(0.5f);
    }

    line[0] = p0 + n;
    line[1] = p1 + n;
}

// To the line through l0-l1, not capped at the end points of the segment
static SkScalar signed_distance(const SkPoint& p, const SkPoint& l0, const SkPoint& l1) {
    SkVector v = l1 - l0;
    v.normalize();
    SkVector n = {v.fY, -v.fX};
    SkScalar c = -n.dot(l0);
    return n.dot(p) + c;
}

#define AI SK_ALWAYS_INLINE

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

static void dumpSk4f(const char* prefix, const Sk4f& f) {
    SkDebugf("%s: [%.4f %.4f %.4f %.4f]\n", prefix, f[0], f[1], f[2], f[3]);
}

static void dumpSk4i_bool(const char* prefix, const Sk4i& f) {
    SkDebugf("%s: [%d %d %d %d]\n", prefix, f[0] != 0, f[1] != 0, f[2] != 0, f[3] != 0);
}

static float get_max_coverage(float w, float h, float d1, float d2) {
    float cov1 = SkMinScalar(w, 1.f) * SkMinScalar(h, 1.f);
    float cov2 = SkMinScalar(d1, 1.f) * SkMinScalar(d2, 1.f);
    return SkMinScalar(cov1, cov2);
}

static Sk2f get_diagonals(const Sk4f& xs, const Sk4f& ys) {
    Sk2f dx(xs[0] - xs[3], xs[1] - xs[2]);
    Sk2f dy(ys[0] - ys[3], ys[1] - ys[2]);
    return (dx * dx + dy * dy).sqrt();
}

static int get_shape_sk4f(SkCanvas* canvas, const bool edgeAA[4], const SkPoint corners[4],
                          bool outset, SkPoint points[4], SkScalar coverage[4]) {
    // Load base corners into Sk4fs and calculate edge vectors and angles
    Sk4f xs(corners[0].fX, corners[1].fX, corners[2].fX, corners[3].fX);
    Sk4f ys(corners[0].fY, corners[1].fY, corners[2].fY, corners[3].fY);
    // Reshuffle from demo's TL, TR, BR, BL into GPUs TL, BL, TR, BR ordering
    xs = SkNx_shuffle<0, 3, 1, 2>(xs);
    ys = SkNx_shuffle<0, 3, 1, 2>(ys);

    // Edge ordering goes from T, R, B, L to L, B, T, R
    Sk4f mask(edgeAA[0] ? 1.f : 0.f, edgeAA[1] ? 1.f : 0.f, edgeAA[2] ? 1.f : 0.f, edgeAA[3] ? 1.f : 0.f);
    mask = SkNx_shuffle<3, 2, 0, 1>(mask);

    Sk4f dx = nextCCW(xs) - xs;
    Sk4f dy = nextCCW(ys) - ys;
    Sk4f edgeLengths = fma(dx, dx, dy * dy).sqrt();
    // Need to normalize, but we also have to make sure to handle edges that are too short.
    // What is the best way for that? Copy adjacent edge? Set it to (0,0) after normalization?
    auto emptyEdge = edgeLengths < 1e-2f;
    // Copy adjacent edge into empty edges (this assumes a triangle, so one bad edge; other degenerate
    // initial shapes should already be detected as empty geometry). FIXME not actually true in Skia currently for
    // the general quad API.
    // FIXME once fast path and slow path are separate functions, really only do these corrections
    // to the edge vectors in the slow path, and add the additional emptyedge check as a short cut
    // into the slow path.
    edgeLengths = emptyEdge.thenElse(nextCW(edgeLengths), edgeLengths);
    dx = emptyEdge.thenElse(nextCW(dx), dx) / edgeLengths;
    dy = emptyEdge.thenElse(nextCW(dy), dy) / edgeLengths;
    Sk4f edgeCosTheta = fma(dx, nextCW(dx), dy * nextCW(dy));
    Sk4f edgeSinTheta = (1.f - edgeCosTheta * edgeCosTheta).sqrt();

    // current edge AA adds .5cos(pi-theta)/sin(theta) + .5cos(pi-theta_ccw)/sin(theta_ccw)
    // cos(pi-theta) = -cos(theta).
    // ccw and cw edges add .5/sin(theta_x)
    Sk4f halfSinInv = 0.5f * edgeSinTheta.invert();
    Sk4f halfTanInv = edgeCosTheta * halfSinInv;
    Sk4f edgeAdjust = mask * (-halfTanInv - nextCCW(halfTanInv)) + nextCCW(mask) * nextCCW(halfSinInv) + nextCW(mask) * halfSinInv;
    Sk4f newEdges = edgeLengths + (outset ? 1.f : -1.f) * edgeAdjust;

    // Conservative heuristic for when quads are likely to be degenerate and need additional
    // analysis to properly inset and outset. Any side after inset/outset becoming negative, or
    // any angle is too close to 0 or 180 degrees
    // (in which case it's possible to make a very thin shape where all edges are large).
    bool useFastPath = (newEdges > 0.1f).allTrue() &&
            (edgeCosTheta.abs() < 0.9f).allTrue();
            if (!outset) {
                dumpSk4f("hsinI", halfSinInv);
                dumpSk4f("htanI", halfTanInv);
            dumpSk4f("edgeAdjust", edgeAdjust);
            dumpSk4f("len", edgeLengths);
            dumpSk4f("newEdges", newEdges);
            dumpSk4f("cos", edgeCosTheta);
            dumpSk4i_bool("edge test", SkNx_cast<int32_t>(newEdges > 1e-2f));
            dumpSk4i_bool("ang test", SkNx_cast<int32_t>(edgeCosTheta.abs() < 0.9f));
            SkDebugf("%d %d %d\n", useFastPath, (newEdges > 1e-2f).allTrue(), (edgeCosTheta.abs() < 0.9f).allTrue());
}
    int pts = 4;
    Sk4f coverage4f = 1.f;
    if (useFastPath) {
        // We can assume the final shape will be a quad without any issues, and can be calculated
        // by moving along dx,dy at each vertex based on edgeAA.
        Sk4f outsetV = 0.5f / edgeSinTheta * (outset ? 1.f : -1.f);

        // FIXME clamp outsetV to be a small amount of pixels

        Sk4f maskedOut = -outsetV * nextCW(mask);
        Sk4f maskedOutCW = outsetV * mask;
        xs += fma(maskedOutCW, nextCW(dx), maskedOut * dx);
        ys += fma(maskedOutCW, nextCW(dy), maskedOut * dy);
    } else {
        // Calculate original edge equations, correcting for any winding inversion from transform
        // then adjust by insetting or outsetting. With adjusted edge equations, calculate the
        // convex interior region bounded by their half planes.
        Sk4f a, b; // ax + by + c = 0
        Sk4f c = fma(dx, ys, -dy * xs);
        // Make sure normals point into the shape
        bool flipped = false;
        Sk4f test = fma(dy, nextCW(xs), fma(-dx, nextCW(ys), c));
        if ((test < 0.f).anyTrue()) {
            // Flip
            a = -dy;
            b = dx;
            c = -c;
            flipped = true;
        } else {
            a = dy;
            b = -dx;
        }

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
        emptyEdge = denom.abs() < 1e-2f;
        px = emptyEdge.thenElse(nextCCW(px), px);
        py = emptyEdge.thenElse(nextCCW(py), py);

        // Calculate the signed distances from these 4 corners to the other two edges that did not
        // define the intersection. So p(0) is compared to e1+e3, p(1) to e2+e3, p(2) to e0+e1, p(3) to e0+e2
        Sk4f dists1 = px * SkNx_shuffle<3, 3, 0, 0>(a) + py * SkNx_shuffle<3, 3, 0, 0>(b) + SkNx_shuffle<3, 3, 0, 0>(c);
        Sk4f dists2 = px * SkNx_shuffle<1, 2, 1, 2>(a) + py * SkNx_shuffle<1, 2, 1, 2>(b) + SkNx_shuffle<1, 2, 1, 2>(c);

        // If all the distances are >= 0, the 4 corners form a valid quadrilateral, so use them as
        // the 4 points. If any point is on the wrong side of both edges, the interior has collapsed
        // and we need to use a central point to represent it. If all four points are only on the
        // wrong side of 1 edge, one edge has crossed over another and we use a line to represent it.
        // Otherwise, use a triangle that replaces the bad vertices with the intersections of
        // (e1, e2) or (e0, e3) as needed.

        // FIXME(michaelludwig): Currently only Sk4f has anyTrue() and allTrue() defined, but
        // only Sk4i has the & and | operators implemented, so we have to cast. Casting back to
        // Sk4f and relyin gon anyTrue/allTrue also seems buggy, so manually combine components.
        Sk4i d1v0 = SkNx_cast<int32_t>(dists1 < 1e-2f);
        Sk4i d2v0 = SkNx_cast<int32_t>(dists2 < 1e-2f);
        Sk4i d1And2 = d1v0 & d2v0;
        Sk4i d1Or2 = d1v0 | d2v0;

        if ((dists1 >= 1e-2f).allTrue() && (dists2 >= 1e-2f).allTrue()) {
            // Not actually degenerate, use all 4 corners
            xs = px;
            ys = py;
        } else if (d1And2[0] || d1And2[1] || d1And2[2] || d1And2[3]) {
            // A point fails on two edges, so degenerates to a single point, which we take as the
            // center of the original quad to ensure it is contained in the intended geometry.
            Sk2f diag = get_diagonals(xs, ys);
            float maxCoverage = get_max_coverage(SkMinScalar(edgeLengths[0], edgeLengths[3]),
                    SkMinScalar(edgeLengths[1], edgeLengths[2]), diag[0], diag[1]);
            coverage4f = maxCoverage; // All points share the reduced coverage
            xs = 0.25f * (xs[0] + xs[1] + xs[2] + xs[3]);
            ys = 0.25f * (ys[0] + ys[1] + ys[2] + ys[3]);
            pts = 1;
        } else if (d1Or2[0] && d1Or2[1] && d1Or2[2] && d1Or2[3]) {
            // Degenerates to a line. Compare p[2] and p[3] to edge 0. If they are on the wrong
            // side, that means edge 0 and 3 crossed, and otherwise edge 1 and 2 crossed.
            Sk2f diag = get_diagonals(xs, ys);
            if (dists1[2] < 1e-2f && dists1[3] < 1e-2f) {
                // Edge 0 and edge 3 have crossed over, so make the line from average of
                // (p0,p2) and (p1,p3)
                float mc02 = get_max_coverage(SkMinScalar(edgeLengths[0], edgeLengths[3]), edgeLengths[2], diag[0], diag[1]);
                float mc13 = get_max_coverage(SkMinScalar(edgeLengths[0], edgeLengths[3]), edgeLengths[1], diag[0], diag[1]);
                coverage4f = Sk4f(mc02, mc13, mc02, mc13);
                xs = 0.5f * (SkNx_shuffle<0, 1, 0, 1>(px) + SkNx_shuffle<2, 3, 2, 3>(px));
                ys = 0.5f * (SkNx_shuffle<0, 1, 0, 1>(py) + SkNx_shuffle<2, 3, 2, 3>(py));
            } else {
                // Edge 1 and edge 2 have crossed over, so form a line between the average
                // of (p0,p1) and (p2,p3)
                float mc01 = get_max_coverage(edgeLengths[0], SkMinScalar(edgeLengths[1], edgeLengths[2]), diag[0], diag[1]);
                float mc23 = get_max_coverage(edgeLengths[3], SkMinScalar(edgeLengths[1], edgeLengths[2]), diag[0], diag[1]);
                coverage4f = Sk4f(mc01, mc01, mc23, mc23);
                xs = 0.5f * (SkNx_shuffle<0, 0, 2, 2>(px) + SkNx_shuffle<1, 1, 3, 3>(px));
                ys = 0.5f * (SkNx_shuffle<0, 0, 2, 2>(py) + SkNx_shuffle<1, 1, 3, 3>(py));
            }
            pts = 2;
        } else {
            // This turns into a triangle. Replace corners as needed with the intersections between
            // (e1,e2) and (e0,e3), which must now be calculated

            // While we're only calculating 2 intersection points, so store them as (e1,e2), (e0,e3)
            Sk2f eDenom = SkNx_shuffle<1,0>(a) * SkNx_shuffle<2,3>(b) - SkNx_shuffle<1,0>(b) * SkNx_shuffle<2,3>(a);
            Sk2f ex = (SkNx_shuffle<1,0>(b) * SkNx_shuffle<2,3>(c) - SkNx_shuffle<1,0>(c) * SkNx_shuffle<2,3>(b)) / eDenom;
            Sk2f ey = (SkNx_shuffle<1,0>(c) * SkNx_shuffle<2,3>(a) - SkNx_shuffle<1,0>(a) * SkNx_shuffle<2,3>(c)) / eDenom;
            // Replace original corners that are on the wrong side of lines with the intersection
            // points. Assuming we started with a valid convex quadrilateral, these replacements
            // are the only ones needed to reconstruct the inner triangle.
            xs = px;
            ys = py;
            if (eDenom[0] > 1e-3f || eDenom[0] < -1e-3f) {
                xs = (dists2 < -1e-4f).thenElse(ex[0], xs);
                ys = (dists2 < -1e-4f).thenElse(ey[0], ys);
            }
            if (eDenom[1] > 1e-3f || eDenom[1] < -1e-3f) {
                xs = (dists1 < -1e-4f).thenElse(ex[1], xs);
                ys = (dists1 < -1e-4f).thenElse(ey[1], ys);
            }
            pts = 3;
            // But keep coverage at 1.0 since there is a non-empty area that can have full coverage
            // FIXME clamp distance that a point moves out to something small
        }
    }

    // Re-arrange for demo ordering: TL,BL,TR,BR -> TL,TR,BR,BL
    xs = SkNx_shuffle<0,2,3,1>(xs);
    ys = SkNx_shuffle<0,2,3,1>(ys);
    coverage4f = SkNx_shuffle<0,2,3,1>(coverage4f);
    for (int i = 0; i < 4; ++i) {
        points[i] = {xs[i], ys[i]};
        coverage[i] = outset ? 0.f : coverage4f[i];
    }

    return pts;
}

// FIXME take into account max coverage properly,
static SkScalar get_edge_dist_coverage(const bool edgeAA[4], const SkPoint corners[4],
                                       const SkPoint outsetLines[8], const SkPoint insetLines[8], const SkPoint& point) {
    bool flip = false;
    // If the quad has been inverted, the original corners will not all be on the negative side of
    // every outset line. When that happens, calculate coverage using the "inset" lines and flip
    // the signed distance
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            SkScalar d = signed_distance(corners[i], outsetLines[2 * j], outsetLines[2 * j + 1]);
            if (d >= 0.f) {
                flip = true;
                break;
            }
        }
        if (flip) {
            break;
        }
    }

    const SkPoint* lines = flip ? insetLines : outsetLines;

    SkScalar minCoverage = 1.f;
    for (int i = 0; i < 4; ++i) {
        // Multiply by negative 1 so that outside points have negative distances
        SkScalar d = (flip ? 1 : -1) * signed_distance(point, lines[2 * i], lines[2 * i + 1]);
        if (!edgeAA[i] && d >= -1e-4f) {
            d = 1.f;
        }
        if (d < minCoverage) {
            minCoverage = d;
            if (minCoverage < 0.f) {
                break; // Outside the shape
            }
        }
    }
    return minCoverage < 0.f ? 0.f : minCoverage;
}

// Move point along the line until it is contained in the pixel boundary, implicitly represented
// by 'pixelCenter'. If the line doesn't intersect the pixel bounds, the point will be snapped to
// the nearest corner.
static void snap_to_pixel(const SkPoint& pixelCenter, float a, float b, float c, SkPoint* point) {
    float left = pixelCenter.fX - 0.5f;
    float right = pixelCenter.fX + 0.5f;
    float top = pixelCenter.fY - 0.5f;
    float bot = pixelCenter.fY + 0.5f;

    if (point->fX < left) {
        float ny = -(c + a * left) / b;
        if (!SkScalarNearlyZero(b) && ny >= top && ny <= bot) {
            point->fX = left;
            point->fY = ny;
        }
    } else if (point->fX > right) {
        float ny = -(c + a * right) / b;
        if (!SkScalarNearlyZero(b) && ny >= top && ny <= bot) {
            point->fX = right;
            point->fY = ny;
        }
    }

    if (point->fY < top) {
        float nx = -(c + b * top) / a;
        if (!SkScalarNearlyZero(a) && nx >= left && nx <= right) {
            point->fY = top;
            point->fX = nx;
        }
    } else if (point->fY > bot) {
        float nx = -(c + b * bot) / a;
        if (!SkScalarNearlyZero(a) && nx >= left && nx <= right) {
            point->fY = bot;
            point->fX = nx;
        }
    }

    // Pin to corners
    point->fX = SkScalarPin(point->fX, left, right);
    point->fY = SkScalarPin(point->fY, top, bot);
}

static float get_area_coverage_sk4f(const Sk4f& xs, const Sk4f& ys, const Sk4f& a, const Sk4f& b,
                                    const Sk4f& c, const SkPoint& pixelCenter) {
    // Walk CCW over corners of the quad, starting at xs[0], ys[] -> 1 -> 3 -> 2 (total CCW, not
    // triangle-strip CCW).
    float area = 0.f;
    SkPoint firstVertex;
    SkPoint lastVertex;
    bool log = pixelCenter.fX == 0.5f;
    if (log)
        SkDebugf("Starting pixel [%.2f %.2f]\n", pixelCenter.fX, pixelCenter.fY);
    static constexpr int kCCW[4] = {0, 1, 3, 2};
    for (int j = 0; j < 4; ++j) {
        int i = kCCW[j];

        // Snap the point to the pixel, following the current edge if possible
        SkPoint vertex = {xs[i], ys[i]};
        snap_to_pixel(pixelCenter, a[i], b[i], c[i], &vertex);
        if (j > 0) {
            // Accumulate any area that came from moving from the last vertex to this snap?ped vertex
            float darea = (lastVertex.fX * vertex.fY - vertex.fX * lastVertex.fY);
                if (log)

            SkDebugf("Accumulating %d  [%.3f, %.3f] -> [%.3f, %.3f], %.3f + %.3f\n",
                 j, lastVertex.fX, lastVertex.fY, vertex.fX, vertex.fY, area, darea);
            area += darea;
        } else {
            // Remember for when we close the contour
            firstVertex = vertex;
        }

        int ni = kCCW[(j + 1) % 4];
        SkPoint next = {xs[ni], ys[ni]};
        snap_to_pixel(pixelCenter, a[i], b[i], c[i], &next);
        // And accumulate area from vertex to next, then remember next for the next iteration
        float da = (vertex.fX * next.fY - next.fX * vertex.fY);
            if (log)

        SkDebugf("Accumulating %d' [%.3f, %.3f] -> [%.3f, %.3f], %.3f + %.3f\n",
                j, vertex.fX, vertex.fY, next.fX, next.fY, area, da);
        area += da;
        lastVertex = next;
    }

    // Close the convex polygon by going from lastVertex to firstVertex
    float da = (lastVertex.fX * firstVertex.fY - firstVertex.fX * lastVertex.fY);
        if (log)

        SkDebugf("Closing         [%.3f, %.3f] -> [%.3f, %.3f], %.3f + %.3f\n", lastVertex.fX, lastVertex.fY, firstVertex.fX, firstVertex.fY, area, da);
    area += da;
    // May have actually been CW if there was a mirror transform, in which case the sign is flipped
    if (log)

    SkDebugf("Final area for pixel [%.2f %.2f] = %.4f\n", pixelCenter.fX, pixelCenter.fY, SkScalarAbs(0.5f * area));
    return SkScalarAbs(0.5f * area);
}

static SkScalar get_area_coverage(const bool edgeAA[4], const SkPoint corners[4], const SkPoint& point) {
    /*SkPath shape;
    shape.addPoly(corners, 4, true);
    SkASSERT(shape.isConvex());
    SkPath pixel;
    pixel.addRect(SkRect::MakeXYWH(point.fX - 0.5f, point.fY - 0.5f, 1.f, 1.f));

    SkPath intersection;
    if (!Op(shape, pixel, kIntersect_SkPathOp, &intersection) || intersection.isEmpty()) {
        return 0.f;
    }
    SkASSERT(intersection.isConvex());

    // Calculate area of the convex polygon, assuming points are still in
    // CW order (since that's the order both shape and pixel were made in).
    SkScalar area = 0.f;
    for (int i = 0; i < intersection.countPoints(); ++i) {
        SkPoint p0 = intersection.getPoint(i);
        SkPoint p1 = intersection.getPoint(i == intersection.countPoints() - 1 ? 0 : i + 1);
        SkScalar det = p0.fX * p1.fY - p1.fX * p0.fY;
        area += det;
    }

    // Scale by 1/2, then take abs value (this area formula is signed based on point winding,
    // but since it's convex, just make it positive).
    area = SkScalarAbs(0.5f * area);
    SkASSERT(area >= 0.f && area <= 1.f);
    */
    Sk4f xs(corners[0].fX, corners[1].fX, corners[2].fX, corners[3].fX);
    Sk4f ys(corners[0].fY, corners[1].fY, corners[2].fY, corners[3].fY);
    // Reshuffle from demo's TL, TR, BR, BL into GPUs TL, BL, TR, BR ordering
    xs = SkNx_shuffle<0, 3, 1, 2>(xs);
    ys = SkNx_shuffle<0, 3, 1, 2>(ys);
    Sk4f dx = nextCCW(xs) - xs;
    Sk4f dy = nextCCW(ys) - ys;
    Sk4f edgeLengths = fma(dx, dx, dy * dy).sqrt();
    // Need to normalize, but we also have to make sure to handle edges that are too short.
    // What is the best way for that? Copy adjacent edge? Set it to (0,0) after normalization?
    auto emptyEdge = edgeLengths < 1e-2f;
    // Copy adjacent edge into empty edges (this assumes a triangle, so one bad edge; other degenerate
    // initial shapes should already be detected as empty geometry). FIXME not actually true in Skia currently for
    // the general quad API.
    // FIXME once fast path and slow path are separate functions, really only do these corrections
    // to the edge vectors in the slow path, and add the additional emptyedge check as a short cut
    // into the slow path.
    edgeLengths = emptyEdge.thenElse(nextCW(edgeLengths), edgeLengths);
    dx = emptyEdge.thenElse(nextCW(dx), dx) / edgeLengths;
    dy = emptyEdge.thenElse(nextCW(dy), dy) / edgeLengths;

    Sk4f a, b; // ax + by + c = 0
    Sk4f c = fma(dx, ys, -dy * xs);
    // Make sure normals point into the shape
    bool flipped = false;
    Sk4f test = fma(dy, nextCW(xs), fma(-dx, nextCW(ys), c));
    if ((test < 0.f).anyTrue()) {
        // Flip
        a = -dy;
        b = dx;
        c = -c;
        flipped = true;
    } else {
        a = dy;
        b = -dx;
    }

    float area = get_area_coverage_sk4f(xs, ys, a, b, c, point);

    // Now account for the edge AA. If the pixel center is outside of a non-AA edge, turn of its
    // coverage. If the pixel only intersects non-AA edges, then set coverage to 1.
    bool needsNonAA = false;
    SkScalar edgeD[4];
    for (int i = 0; i < 4; ++i) {
        SkPoint e0 = corners[i];
        SkPoint e1 = corners[i == 3 ? 0 : i + 1];
        edgeD[i] = -signed_distance(point, e0, e1);
        if (!edgeAA[i]) {
            if (edgeD[i] < -1e-4f) {
                return 0.f; // Outside
            }
            needsNonAA = true;
        }
    }
    // Otherwise inside the shape, so check if any AA edge is close enough to exert influence over nonAA
    if (needsNonAA) {
        for (int i = 0; i < 4; i++) {
            if (edgeAA[i] && edgeD[i] < 0.5f) {
                needsNonAA = false;
                break;
            }
        }
    }
    return needsNonAA ? 1.f : area;
}

// FIXME handle CW triangle windings
static bool inside_triangle(const SkPoint& point, const SkPoint& t0, const SkPoint& t1, const SkPoint& t2,
                            SkScalar bary[3]) {
    // Check sign of t0 to (t1,t2). If it is positive, that means the normals point into the triangle
    // otherwise the normals point outside the triangle so update edge distances as necessary
    bool flip = signed_distance(t0, t1, t2) < 0.f;

    SkScalar d0 = (flip ? -1 : 1) * signed_distance(point, t0, t1);
    SkScalar d1 = (flip ? -1 : 1) * signed_distance(point, t1, t2);
    SkScalar d2 = (flip ? -1 : 1) * signed_distance(point, t2, t0);
    // Be a little forgiving
    if (d0 < -1e-4f || d1 < -1e-4f || d2 < -1e-4f) {
        return false;
    }

    // Inside, so calculate barycentric coords from the sideline distances
    SkScalar d01 = (t0 - t1).length();
    SkScalar d12 = (t1 - t2).length();
    SkScalar d20 = (t2 - t0).length();

    if (SkScalarNearlyZero(d12) || SkScalarNearlyZero(d20) || SkScalarNearlyZero(d01)) {
        // Empty degenerate triangle
        return false;
    }

    // Coordinates for a vertex use distances to the opposite edge
    bary[0] = d1 * d12;
    bary[1] = d2 * d20;
    bary[2] = d0 * d01;
    // And normalize
    SkScalar sum = bary[0] + bary[1] + bary[2];
    bary[0] /= sum;
    bary[1] /= sum;
    bary[2] /= sum;

    return true;
}

static SkScalar get_framed_coverage(const SkPoint outer[4], const SkScalar outerCoverages[4],
                                    const SkPoint inner[4], const SkScalar innerCoverages[4],
                                    const SkPoint& point) {
    // Triangles are ordered clock wise. Indices >= 4 refer to inner[i - 4]. Otherwise its outer[i].
    static const int kFrameTris[] = {
        0, 1, 4,   4, 1, 5,
        1, 2, 5,   5, 2, 6,
        2, 3, 6,   6, 3, 7,
        3, 0, 7,   7, 0, 4,
        4, 5, 7,   7, 5, 6
    };
    static const int kNumTris = 10;

    SkScalar bary[3];
    for (int i = 0; i < kNumTris; ++i) {
        int i0 = kFrameTris[i * 3];
        int i1 = kFrameTris[i * 3 + 1];
        int i2 = kFrameTris[i * 3 + 2];

        SkPoint t0 = i0 >= 4 ? inner[i0 - 4] : outer[i0];
        SkPoint t1 = i1 >= 4 ? inner[i1 - 4] : outer[i1];
        SkPoint t2 = i2 >= 4 ? inner[i2 - 4] : outer[i2];
        if (inside_triangle(point, t0, t1, t2, bary)) {
            // Calculate coverage by barycentric interpolation of coverages
            SkScalar c0 = i0 >= 4 ? innerCoverages[i0 - 4] : outerCoverages[i0];
            SkScalar c1 = i1 >= 4 ? innerCoverages[i1 - 4] : outerCoverages[i1];
            SkScalar c2 = i2 >= 4 ? innerCoverages[i2 - 4] : outerCoverages[i2];

            return bary[0] * c0 + bary[1] * c1 + bary[2] * c2;
        }
    }
    // Not inside any triangle
    return 0.f;
}

static constexpr SkScalar kViewScale = 100.f;
static constexpr SkScalar kViewOffset = 200.f;

class DegenerateQuadSample : public Sample {
public:
    DegenerateQuadSample(const SkRect& rect)
            : fOuterRect(rect)
            , fCoverageMode(CoverageMode::kArea) {
        fOuterRect.toQuad(fCorners);
        for (int i = 0; i < 4; ++i) {
            fEdgeAA[i] = true;
        }
    }

    void onDrawContent(SkCanvas* canvas) override {
        static const SkScalar kDotParams[2] = {1.f / kViewScale, 12.f / kViewScale};
        sk_sp<SkPathEffect> dots = SkDashPathEffect::Make(kDotParams, 2, 0.f);
        static const SkScalar kDashParams[2] = {8.f / kViewScale, 12.f / kViewScale};
        sk_sp<SkPathEffect> dashes = SkDashPathEffect::Make(kDashParams, 2, 0.f);

        SkPaint circlePaint;
        circlePaint.setAntiAlias(true);

        SkPaint linePaint;
        linePaint.setAntiAlias(true);
        linePaint.setStyle(SkPaint::kStroke_Style);
        linePaint.setStrokeWidth(4.f / kViewScale);
        linePaint.setStrokeJoin(SkPaint::kRound_Join);
        linePaint.setStrokeCap(SkPaint::kRound_Cap);

        canvas->translate(kViewOffset, kViewOffset);
        canvas->scale(kViewScale, kViewScale);

        // Draw the outer rectangle as a dotted line
        linePaint.setPathEffect(dots);
        canvas->drawRect(fOuterRect, linePaint);

        bool valid = this->isValid();

        if (valid) {
            SkPoint outsets[8];
            SkPoint insets[8];
            // calculate inset and outset lines
            for (int i = 0; i < 4; ++i) {
                make_aa_line(fCorners[i], fCorners[i == 3 ? 0 : i + 1], fEdgeAA[i], true, outsets + i * 2);
                make_aa_line(fCorners[i], fCorners[i == 3 ? 0 : i + 1], fEdgeAA[i], false, insets + i * 2);
            }

            // Calculate the valid shape for the interior and the exterior
            SkPoint idealOutset[4];
            SkScalar idealOutsetCoverages[4];
            int numOutsetPoints = get_shape_sk4f(canvas, fEdgeAA, fCorners, true, idealOutset, idealOutsetCoverages);;

            SkPoint idealInset[4];
            SkScalar idealInsetCoverages[4];
            int numInsetPoints = get_shape_sk4f(canvas, fEdgeAA, fCorners, false, idealInset, idealInsetCoverages);

            SkPoint gpuOutset[4];
            SkScalar gpuOutsetCoverage[4];
            SkPoint gpuInset[4];
            SkScalar gpuInsetCoverage[4];
            this->getTessellatedPoints(gpuInset, gpuInsetCoverage, gpuOutset, gpuOutsetCoverage);

            // Visualize the coverage values across the clamping rectangle
            SkPaint pixelPaint;
            pixelPaint.setAntiAlias(true);
            SkRect covRect = fOuterRect.makeOutset(2.f, 2.f);
            for (SkScalar py = covRect.fTop; py < covRect.fBottom; py += 1.f) {
                for (SkScalar px = covRect.fLeft; px < covRect.fRight; px += 1.f) {
                    // px and py are the top-left corner of the current pixel, so get center's coordinate
                    SkPoint pixelCenter = {px + 0.5f, py + 0.5f};
                    SkScalar coverage;
                    if (fCoverageMode == CoverageMode::kArea) {
                        coverage = get_area_coverage(fEdgeAA, fCorners, pixelCenter);
                    } else if (fCoverageMode == CoverageMode::kEdgeDistance) {
                        coverage = get_edge_dist_coverage(fEdgeAA, fCorners, outsets, insets, pixelCenter);
                    } else if (fCoverageMode == CoverageMode::kIdealMesh) {
                        coverage = get_framed_coverage(idealOutset, idealOutsetCoverages, idealInset, idealInsetCoverages, pixelCenter);
                    } else {
                        SkASSERT(fCoverageMode == CoverageMode::kGPUMesh);
                        coverage = get_framed_coverage(gpuOutset, gpuOutsetCoverage, gpuInset, gpuInsetCoverage, pixelCenter);
                    }

                    SkRect pixelRect = SkRect::MakeXYWH(px, py, 1.f, 1.f);
                    pixelRect.inset(0.1f, 0.1f);

                    SkScalar a = 1.f - 0.5f * coverage;
                    pixelPaint.setColor4f({a, a, a, 1.f}, nullptr);
                    canvas->drawRect(pixelRect, pixelPaint);

                    pixelPaint.setColor(coverage > 0.f ? SK_ColorGREEN : SK_ColorRED);
                    pixelRect.inset(0.38f, 0.38f);
                    canvas->drawRect(pixelRect, pixelPaint);
                }
            }

            linePaint.setPathEffect(dashes);
            // Draw the inset/outset "infinite" lines
            if (fCoverageMode == CoverageMode::kEdgeDistance) {
                for (int i = 0; i < 4; ++i) {
                    if (fEdgeAA[i]) {
                        linePaint.setColor(SK_ColorBLUE);
                        draw_extended_line(canvas, linePaint, outsets[i * 2], outsets[i * 2 + 1], 3.f, true);
                        linePaint.setColor(SK_ColorGREEN);
                        draw_extended_line(canvas, linePaint, insets[i * 2], insets[i * 2 + 1], 3.f, true);
                    } else {
                        // Both outset and inset are the same line, so only draw one in cyan
                        linePaint.setColor(SK_ColorCYAN);
                        draw_extended_line(canvas, linePaint, outsets[i * 2], outsets[i * 2 + 1], 3.f, true);
                    }
                }
            }

            linePaint.setPathEffect(nullptr);
            // Draw the polygon boundaries of the ideal shape with corner circles
            if (fCoverageMode == CoverageMode::kIdealMesh) {
                if (numOutsetPoints > 1) {
                    SkPath outsetPath;
                    outsetPath.addPoly(idealOutset, 4, true);
                    linePaint.setColor(SK_ColorBLUE);
                    canvas->drawPath(outsetPath, linePaint);
                } else {
                    canvas->drawPoint(idealOutset[0], linePaint);
                }

                linePaint.setColor(SK_ColorGREEN);
                if (numInsetPoints > 1) {
                    SkPath insetPath;
                    insetPath.addPoly(idealInset, 4, true);
                    canvas->drawPath(insetPath, linePaint);
                } else {
                    canvas->drawPoint(idealInset[0], linePaint);
                }
            }

            // What is tessellated using GrQuadPerEdgeAA
            if (fCoverageMode == CoverageMode::kGPUMesh) {
                SkPath outsetPath;
                outsetPath.addPoly(gpuOutset, 4, true);
                linePaint.setColor(SK_ColorRED);
                canvas->drawPath(outsetPath, linePaint);

                SkPath insetPath;
                insetPath.addPoly(gpuInset, 4, true);
                linePaint.setColor(SK_ColorMAGENTA);
                canvas->drawPath(insetPath, linePaint);
            }

            // Draw the edges as a solid line
            SkPath path;
            path.addPoly(fCorners, 4, true);
            linePaint.setColor(SK_ColorBLACK);
            canvas->drawPath(path, linePaint);
        } else {
            // Draw the edges as a solid line
            SkPath path;
            path.addPoly(fCorners, 4, true);
            linePaint.setColor(SK_ColorRED);
            linePaint.setPathEffect(nullptr);
            canvas->drawPath(path, linePaint);
        }

        // Draw the four clickable corners as circles
        circlePaint.setColor(valid ? SK_ColorBLACK : SK_ColorRED);
        for (int i = 0; i < 4; ++i) {
            canvas->drawCircle(fCorners[i], 5.f / kViewScale, circlePaint);
        }
    }

    Sample::Click* onFindClickHandler(SkScalar x, SkScalar y, unsigned) override;
    bool onClick(Sample::Click*) override;
    bool onQuery(Sample::Event* evt) override;

private:
    class Click;

    enum class CoverageMode {
        kArea, kEdgeDistance, kIdealMesh, kGPUMesh
    };

    const SkRect fOuterRect;
    SkPoint fCorners[4]; // TL, TR, BR, BL
    bool fEdgeAA[4]; // T, R, B, L
    CoverageMode fCoverageMode;

    bool isValid() const {
        SkPath path;
        path.addPoly(fCorners, 4, true);
        return path.isConvex();
    }

    void getTessellatedPoints(SkPoint inset[4], SkScalar insetCoverage[4], SkPoint outset[4],
                              SkScalar outsetCoverage[4]) const {
        // Fixed vertex spec for extracting the picture frame geometry
        static const GrQuadPerEdgeAA::VertexSpec kSpec =
            {GrQuadType::kStandard, GrQuadPerEdgeAA::ColorType::kNone,
             GrQuadType::kRect, false, GrQuadPerEdgeAA::Domain::kNo,
             GrAAType::kCoverage, false};
        static const GrPerspQuad kIgnored(SkRect::MakeEmpty());

        GrQuadAAFlags flags = GrQuadAAFlags::kNone;
        flags |= fEdgeAA[0] ? GrQuadAAFlags::kTop : GrQuadAAFlags::kNone;
        flags |= fEdgeAA[1] ? GrQuadAAFlags::kRight : GrQuadAAFlags::kNone;
        flags |= fEdgeAA[2] ? GrQuadAAFlags::kBottom : GrQuadAAFlags::kNone;
        flags |= fEdgeAA[3] ? GrQuadAAFlags::kLeft : GrQuadAAFlags::kNone;

        GrPerspQuad quad = GrPerspQuad::MakeFromSkQuad(fCorners, SkMatrix::I());

        float vertices[24]; // 2 quads, with x, y, and coverage
        GrQuadPerEdgeAA::Tessellate(vertices, kSpec, quad, {1.f, 1.f, 1.f, 1.f},
                GrPerspQuad(SkRect::MakeEmpty()), SkRect::MakeEmpty(), flags);

        // The first quad in vertices is the inset, then the outset, but they
        // are ordered TL, BL, TR, BR so un-interleave coverage and re-arrange
        inset[0] = {vertices[0], vertices[1]}; // TL
        insetCoverage[0] = vertices[2];
        inset[3] = {vertices[3], vertices[4]}; // BL
        insetCoverage[3] = vertices[5];
        inset[1] = {vertices[6], vertices[7]}; // TR
        insetCoverage[1] = vertices[8];
        inset[2] = {vertices[9], vertices[10]}; // BR
        insetCoverage[2] = vertices[11];

        outset[0] = {vertices[12], vertices[13]}; // TL
        outsetCoverage[0] = vertices[14];
        outset[3] = {vertices[15], vertices[16]}; // BL
        outsetCoverage[3] = vertices[17];
        outset[1] = {vertices[18], vertices[19]}; // TR
        outsetCoverage[1] = vertices[20];
        outset[2] = {vertices[21], vertices[22]}; // BR
        outsetCoverage[2] = vertices[23];
    }

    typedef Sample INHERITED;
};

class DegenerateQuadSample::Click : public Sample::Click {
public:
    Click(Sample* target, const SkRect& clamp, int index)
            : Sample::Click(target)
            , fOuterRect(clamp)
            , fIndex(index) {}

    void doClick(SkPoint points[4]) {
        if (fIndex >= 0) {
            this->drag(&points[fIndex]);
        } else {
            for (int i = 0; i < 4; ++i) {
                this->drag(&points[i]);
            }
        }
    }

private:
    SkRect fOuterRect;
    int fIndex;

    void drag(SkPoint* point) {
        SkIPoint delta = fICurr - fIPrev;
        *point += SkPoint::Make(delta.x() / kViewScale, delta.y() / kViewScale);
        point->fX = SkMinScalar(fOuterRect.fRight, SkMaxScalar(point->fX, fOuterRect.fLeft));
        point->fY = SkMinScalar(fOuterRect.fBottom, SkMaxScalar(point->fY, fOuterRect.fTop));
    }
};

Sample::Click* DegenerateQuadSample::onFindClickHandler(SkScalar x, SkScalar y, unsigned) {
    SkPoint inCTM = SkPoint::Make((x - kViewOffset) / kViewScale, (y - kViewOffset) / kViewScale);
    for (int i = 0; i < 4; ++i) {
        if ((fCorners[i] - inCTM).length() < 10.f / kViewScale) {
            return new Click(this, fOuterRect, i);
        }
    }
    return new Click(this, fOuterRect, -1);
}

bool DegenerateQuadSample::onClick(Sample::Click* click) {
    Click* myClick = (Click*) click;
    myClick->doClick(fCorners);
    return true;
}

bool DegenerateQuadSample::onQuery(Sample::Event* event) {
    if (Sample::TitleQ(*event)) {
        Sample::TitleR(event, "DegenerateQuad");
        return true;
    }
    SkUnichar code;
    if (Sample::CharQ(*event, &code)) {
        switch(code) {
            case '1':
                fEdgeAA[0] = !fEdgeAA[0];
                return true;
            case '2':
                fEdgeAA[1] = !fEdgeAA[1];
                return true;
            case '3':
                fEdgeAA[2] = !fEdgeAA[2];
                return true;
            case '4':
                fEdgeAA[3] = !fEdgeAA[3];
                return true;
            case 'q':
                fCoverageMode = CoverageMode::kArea;
                return true;
            case 'w':
                fCoverageMode = CoverageMode::kEdgeDistance;
                return true;
            case 'e':
                fCoverageMode = CoverageMode::kIdealMesh;
                return true;
            case 'r':
                fCoverageMode = CoverageMode::kGPUMesh;
                return true;
        }
    }
    return this->INHERITED::onQuery(event);
}

DEF_SAMPLE(return new DegenerateQuadSample(SkRect::MakeWH(4.f, 4.f));)
