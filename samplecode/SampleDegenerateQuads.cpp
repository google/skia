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
        n.setLength(0.25f);
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

// Intersect the two infinite lines that go through (p0-p1) and (l0-l1).
// True is returned if the lines intersect at 1 or infinite locations. False
// is returned if they are parallel but distinct lines. When the lines are
// identical, the intersect point is set to the average of the 4 points
static bool intersect_lines(const SkPoint& p0, const SkPoint& p1,
                            const SkPoint& l0, const SkPoint& l1,
                            SkPoint* intersect) {
    SkVector pv = p1 - p0;
    pv.normalize();
    SkVector pn = {pv.fY, -pv.fX};
    SkScalar pc = -pn.dot(p0);

    SkVector lv = l1 - l0;
    lv.normalize();
    SkVector ln = {lv.fY, -lv.fX};
    SkScalar lc = -ln.dot(l0);

    SkScalar denom = pn.fX * ln.fY - pn.fY * ln.fX;
    SkScalar xNum = pn.fY * lc - pc * ln.fY;
    SkScalar yNum = pc * ln.fX - pn.fX * lc;

    if (SkScalarNearlyZero(denom)) {
        // Check if p0 is on the other line
        SkScalar d = ln.dot(p0) + lc;
        if (SkScalarNearlyZero(d)) {
            // Same lines, so return the average of 4 points
            *intersect = (p0 + p1 + l0 + l1) * 0.25f;
            return true;
        } else {
            // Parallel but separated
            return false;
        }
    } else {
        *intersect = {xNum / denom, yNum / denom};
        return true;
    }
}

// Return false if needs restricted coverage
static bool get_shape(const SkPoint lines[8], const SkPoint baseCorners[4], SkPoint points[4]) {
    // FIXME what happens when one edge is null? currently we crash, or we calculate
    // the wrong intersections. need some way to skipping over the bad edge.
    // - What if we rewrote this to fill in 4 sets of line equation parameters, a, b, and c
    //   Then for edges that are bad, we can set them to one of their neighbors

    // Calculate original 4 corners based on adjacent edges
    for (int i = 0; i < 4; ++i) {
        // Current edge
        SkPoint e0 = lines[i * 2];
        SkPoint e1 = lines[i * 2 + 1];
        // Prev edge in rotation
        int pi = i == 0 ? 3 : i - 1;
        SkPoint p0 = lines[pi * 2];
        SkPoint p1 = lines[pi * 2 + 1];
        if (!intersect_lines(e0, e1, p0, p1, &points[i])) {
            // The edges are parallel and not intersecting, so set the point to
            // be in the middle between e0 and p1
            points[i] = e0 * 0.5f + p1 * 0.5f;
        }
    }

    // Calculate how many edges have positive signed distances per point
    int sd[4];
    int edgePosCounts[3] = {0, 0, 0}; // Index refers to # of corners with that many pos. distances to edges
    for (int i = 0; i < 4; ++i) {
        sd[i] = 0;
        for (int j = 0; j < 4; ++j) {
            SkPoint e0 = lines[j * 2];
            SkPoint e1 = lines[j * 2 + 1];
            SkScalar d = signed_distance(points[i], e0, e1);
            if (d > 1e-5f) {
                sd[i] = sd[i] + 1;
            }
        }

        SkASSERT(sd[i] >= 0 && sd[i] <= 2);
        edgePosCounts[sd[i]] = edgePosCounts[sd[i]] + 1;
    }

    // Classify the interior shape based on these edge counts
    if (edgePosCounts[2] > 0) {
        // At least one point is on the wrong side of two lines. It may be the case that all
        // lines have crossed over, or two have crossed and the remaining two intersect.
        // Either case, the proper interior geometry is a point. Use the average of the
        // original geometry, which keeps the point in its interior when under high sheer.
        // If we used the inset/outset line intersections' averages, that point could fall
        // outside the original shape.
        SkPoint center = {0.f, 0.f};
        for (int i = 0; i < 4; ++i) {
            center = center + baseCorners[i] * 0.25f;
        }
        for (int i = 0; i < 4; ++i) {
            points[i] = center;
        }

        return false;
    }

    if (edgePosCounts[1] > 0) {
        if (edgePosCounts[1] == 4) {
            // Two opposing edges have passed each other, so the interior is collapsed to a line
            // Look at signed distances for pt 2 and 3 against edge 0
            SkASSERT(edgePosCounts[0] == 0 && edgePosCounts[2] == 0);
            SkPoint e0 = lines[0];
            SkPoint e1 = lines[1];
            double d2 = signed_distance(points[2], e0, e1);
            double d3 = signed_distance(points[3], e0, e1);
            if (d2 > 0.f && d3 > 0.f) {
                // Edge 0 and edge 2 have crossed over, so form a line between the average
                // of (p0,p3) and (p1,p2)
                SkPoint l0 = (points[0] + points[3]) * 0.5f;
                SkPoint l1 = (points[1] + points[2]) * 0.5f;
                points[0] = l0;
                points[1] = l1;
                points[2] = l1;
                points[3] = l0;
            } else {
                // Edge 1 and edge 3 have crossed over, so make the line from average of
                // (p0,p1) and (p2,p3)
                SkPoint l0 = (points[0] + points[1]) * 0.5f;
                SkPoint l1 = (points[2] + points[3]) * 0.5f;
                points[0] = l0;
                points[1] = l0;
                points[2] = l1;
                points[3] = l1;
            }

            return false;
        } else {
            // This is a triangle of some sort, so find the new intersection points that
            // form the interior region
            for (int i = 0; i < 4; ++i) {
                if (sd[i] == 0) {
                    continue;
                }
                // Must move points[i] so that it has one fewer positive distances with respect
                // to the two edges that didn't form it in the first place.
                //
                // Look at intersection of (prev) and (next) edges, and compare distance
                // to the (next)^2 edge. If negative, then that's the snap point.
                int pi = i == 0 ? 3 : i - 1;
                int ni = (i + 1) % 4;
                int n2i = (i + 2) % 4;
                SkPoint snap;

                bool snapValid = false;
                if (intersect_lines(lines[pi * 2], lines[pi * 2 + 1], lines[ni * 2], lines[ni * 2 + 1], &snap)) {
                    // Distance to opposing edge
                    SkScalar dn2i = signed_distance(snap, lines[n2i * 2], lines[n2i * 2 + 1]);
                    // Distance to current edge
                    SkScalar dc = signed_distance(snap, lines[i * 2], lines[i * 2 + 1]);
                    // Opposing lines still point outwards, so we want the interior
                    // point that has 2 negative distances.
                    snapValid = dn2i <= 0.f && dc <= 0.f;
                }
                // Next, look at intersection of (curr) and (next)^2 edges, and compare
                // distance to the (prev) edge.
                if (!snapValid && intersect_lines(lines[i * 2], lines[i * 2 + 1], lines[n2i * 2], lines[n2i * 2 + 1], &snap)) {
                    SkScalar dp = signed_distance(snap, lines[pi * 2], lines[pi * 2 + 1]);
                    SkScalar dn = signed_distance(snap, lines[ni * 2], lines[ni * 2 + 1]);
                    snapValid = dp <= 0.f && dn <= 0.f;
                }

                if (snapValid) {
                    points[i] = snap;
                }
            }
        }
    }

    return true;
}

static SkScalar get_max_coverage(const SkPoint corners[4]) {
    SkScalar w0 = (corners[1] - corners[0]).length();
    SkScalar w1 = (corners[2] - corners[1]).length();
    SkScalar w2 = (corners[3] - corners[2]).length();
    SkScalar w3 = (corners[3] - corners[0]).length();

// FIXME min and max of these dimensions is a bad approximation when general quads can be used
    // we can have really long edges of a parallelogram that is still very skinny.
    // FIXME the thin rect coverage business looks really good, but it's extra outsetting really
    // doesn't work in the context of a compositor, since we need to ensure very careful coverage
    // FIXME could we calculate the max coverage to clamp per vertex, based on a theoretic pixel
    // located at the vertex and what area of the quad intersects it?
    // Or use a "pixel" at the center of the shape and its intersection area? That is nice because
    // it can be a constant for the whole shape, but not sure how that works for things where
    // most of it will be a 1, and a thin corner needs something less? Maybe interplation takes care of that
    //
    // FIXME can we calculate a true coverage value for the original shape and draw that during
    // the kNone draw mode? Look at shape's intersection with each pixel and calculate the area
    // of it. The challenge will be for mixed edge modes.
    // In full-aa, we can just use area. For non-aa, we can say any non-zero intersection means
    // full? (or switch back to pixel center tests).
    // - maybe that's how per-edge AA would work too? We calculate initial coverage as if full-aa
    // and then for each edge that's non-AA we see if the pixel center is on the proper side
    // to be switched to 1 or 0. That's a little tricky because, at a certain distance into the
    // interior of the shape, it needs to know how to switch to using the theoretic coverage.
    SkScalar w = SkMaxScalar(w0, w2);
    SkScalar h = SkMaxScalar(w1, w3);
    return SkMinScalar(w, 1.f) * SkMinScalar(h, 1.f);
}

// FIXME take into account max coverage properly
static SkScalar get_edge_dist_coverage(const bool edgeAA[4], const SkPoint outsetLines[8], const SkPoint& point) {
    SkScalar minCoverage = 1.f;
    for (int i = 0; i < 4; ++i) {
        // Multiply by negative 1 so that outside points have negative distances
        SkScalar d = -signed_distance(point, outsetLines[2 * i], outsetLines[2 * i + 1]);
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

static SkScalar get_area_coverage(const bool edgeAA[4], const SkPoint corners[4], const SkPoint& point) {
    SkPath shape;
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
    // return needsAA ? area : 1.f;
}

static bool inside_triangle(const SkPoint& point, const SkPoint& t0, const SkPoint& t1, const SkPoint& t2,
                            SkScalar bary[3]) {
    // Triangle normals arranged so that out is positive, flip sign since we want interior
    SkScalar d0 = -signed_distance(point, t0, t1);
    SkScalar d1 = -signed_distance(point, t1, t2);
    SkScalar d2 = -signed_distance(point, t2, t0);
    // Be a little forgiving
    if (d0 < -1e-4f) {
        return false;
    } else if (d1 < -1e-4f) {
        return false;
    } else if (d2 < -1e-4f) {
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

        //
        // FIXME switch to calculating data only when updated
        // FIXME in the tessellator, we need to check if cos(theta) is really close to 1 or -1
        // because that can lead to catastrophic cancellation when calculating the outset positions.
        // - in that case, we need to calculate the edge intersection coordinate to use instead
        // - similarly, if we detect that our edge distances have gone negative, it is necessary
        //   to calculate based on edge intersections. Ideally, maybe we can classify these issues
        //   per edge, and perform the vertex updates as necessary. That way any degenerate edge
        //   could similarly activate the edge detecion fallbacks for connected verts

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
            SkScalar idealOutsetCoverages[4] = {0.f, 0.f, 0.f, 0.f};
            SkPoint idealInset[4];
            SkScalar idealInsetCoverages[4] = {1.f, 1.f, 1.f, 1.f};
            SkScalar maxCoverage = get_max_coverage(fCorners);

            get_shape(outsets, fCorners, idealOutset);
            if (!get_shape(insets, fCorners, idealInset)) {
                for (int i = 0; i < 4; ++i) {
                    idealInsetCoverages[i] = maxCoverage;
                }
            }

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
                        coverage = get_edge_dist_coverage(fEdgeAA, outsets, pixelCenter);
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
                        draw_extended_line(canvas, linePaint, outsets[i * 2], outsets[i * 2 + 1], true);
                        linePaint.setColor(SK_ColorGREEN);
                        draw_extended_line(canvas, linePaint, insets[i * 2], insets[i * 2 + 1], true);
                    } else {
                        // Both outset and inset are the same line, so only draw one in cyan
                        linePaint.setColor(SK_ColorCYAN);
                        draw_extended_line(canvas, linePaint, outsets[i * 2], outsets[i * 2 + 1], true);
                    }
                }
            }

            linePaint.setPathEffect(nullptr);
            // Draw the polygon boundaries of the ideal shape with corner circles
            if (fCoverageMode == CoverageMode::kIdealMesh) {
                SkPath outsetPath;
                outsetPath.addPoly(idealOutset, 4, true);
                linePaint.setColor(SK_ColorBLUE);
                canvas->drawPath(outsetPath, linePaint);

                SkPath insetPath;
                insetPath.addPoly(idealInset, 4, true);
                linePaint.setColor(SK_ColorGREEN);
                canvas->drawPath(insetPath, linePaint);
                // FIXME when there's a point, drawPath collapses and doesn't draw so would be
                // nice to add drawCircle or drawPoint?
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
