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

enum class LineType {
    kDot,
    kDash,
    kSolid
};

// Draw a line through the two points, outset by a fixed length in screen space
static void draw_line(SkCanvas* canvas, SkColor color, SkScalar width, LineType type,
                             const SkPoint& p0, const SkPoint& p1, SkScalar extension = 0.f,
                             bool showNormal = false) {
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setColor(color);
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(width);
    if (type == LineType::kDot) {
        SkScalar intervals[2] = {width, 2 * width};
        paint.setPathEffect(SkDashPathEffect::Make(intervals, 2, 0.f));
    } else if (type == LineType::kDash) {
        SkScalar intervals[2] = {5 * width, 2 * width};
        paint.setPathEffect(SkDashPathEffect::Make(intervals, 2, 0.f));
    } // else no path effect

    SkVector v = p1 - p0;
    v.setLength(v.length() + extension);
    if (v.length() + extension > 0.001f) {
        canvas->drawLine(p1 - v, p0 + v, paint);

        // Draw normal vector too
        if (showNormal) {
            SkVector n = {v.fY, -v.fX};
            n.setLength(0.25f);
            paint.setPathEffect(nullptr);
            paint.setStrokeWidth(width / 4.f);
            SkPoint m = (p0 + p1) * 0.5f;
            canvas->drawLine(m, m + n, paint);
        }
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

static void get_shape(const SkPoint lines[8], const SkPoint baseCorners[4],
                      SkPoint points[4]) {
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
            SkDebugf("%d %d - %g\n", i, j, d);
            if (d > 1e-5f) {
                sd[i] = sd[i] + 1;
            }
        }

        SkDebugf("%d - %d\n", i, sd[i]);
        SkASSERT(sd[i] >= 0 && sd[i] <= 2);
        edgePosCounts[sd[i]] = edgePosCounts[sd[i]] + 1;
    }
    SkDebugf("pos counts: %d %d %d\n", edgePosCounts[0], edgePosCounts[1], edgePosCounts[2]);

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
}

static SkScalar get_edge_dist_coverage(const SkPoint outsetLines[8], const SkPoint& point) {
    SkScalar minCoverage = 1.f;
    for (int i = 0; i < 4; ++i) {
        // Multiply by negative 1 so that outside points have negative distances
        SkScalar d = -signed_distance(point, outsetLines[2 * i], outsetLines[2 * i + 1]);
        if (d < minCoverage) {
            minCoverage = d;
            if (minCoverage < 0.f) {
                break; // Outside the shape
            }
        }
    }
    return minCoverage < 0.f ? 0.f : minCoverage;
}

static bool inside_triangle(const SkPoint& point, const SkPoint& t0, const SkPoint& t1, const SkPoint& t2,
        SkScalar bary[3]) {
    // Triangle normals arranged so that out is positive, flip sign since we want interior
    SkScalar d0 = -signed_distance(point, t0, t1);
    SkScalar d1 = -signed_distance(point, t1, t2);
    SkScalar d2 = -signed_distance(point, t2, t0);
    if (d0 < 0.f) {
        return false;
    } else if (d1 < 0.f) {
        return false;
    } else if (d2 < 0.f) {
        return false;
    }

    // Inside, so calculate barycentric coords from the sideline distances
    SkScalar d01 = (t0 - t1).length();
    SkScalar d12 = (t1 - t2).length();
    SkScalar d20 = (t2 - t0).length();
    bary[0] = d0 * d01;
    bary[1] = d1 * d12;
    bary[2] = d2 * d20;
    // And normalize
    SkScalar sum = bary[0] + bary[1] + bary[2];
    bary[0] /= sum;
    bary[1] /= sum;
    bary[2] /= sum;

    SkDebugf("inside triangle: %.2f %.2f %.2f -> %.2f %.2f %.2f\n",
        d0, d1, d2, bary[0], bary[1], bary[2]);
    return true;
}

static SkScalar get_framed_coverage(const SkPoint outer[4], const SkScalar outerCoverages[4],
                                    const SkPoint inner[4], const SkScalar innerCoverages[4],
                                    const SkPoint& point) {
    // Triangles are ordered clock wise. Indices >= 4 refer to inner[i - 4]. Otherwise its outer[i].
    static const int kFrameTris[3] = {
        // 0, 1, 4,   4, 1, 5,
        // 1, 2, 5,   5, 2, 6,
        // 2, 3, 6,   6, 3, 7,
        // 3, 0, 7,   7, 0, 4,
        // 4, 5, 7,   7, 5, 6
        4, 5, 7
    };
    static const int kNumTris = 1; //10;

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
            , fShowGPU(true) {
        fOuterRect.toQuad(fCorners);
        for (int i = 0; i < 4; ++i) {
            fEdgeAA[i] = true;
        }
    }

    void onDrawContent(SkCanvas* canvas) override {
        SkPaint circlePaint;
        circlePaint.setAntiAlias(true);

        canvas->translate(kViewOffset, kViewOffset);
        canvas->scale(kViewScale, kViewScale);

        // Draw the outer rectangle as a dotted line
        draw_line(canvas, SK_ColorBLACK, 2.f / kViewScale, LineType::kDot,
                   {fOuterRect.fLeft, fOuterRect.fTop}, {fOuterRect.fRight, fOuterRect.fTop});
        draw_line(canvas, SK_ColorBLACK, 2.f / kViewScale, LineType::kDot,
                   {fOuterRect.fRight, fOuterRect.fTop}, {fOuterRect.fRight, fOuterRect.fBottom});
        draw_line(canvas, SK_ColorBLACK, 2.f / kViewScale, LineType::kDot,
                   {fOuterRect.fLeft, fOuterRect.fBottom}, {fOuterRect.fRight, fOuterRect.fBottom});
        draw_line(canvas, SK_ColorBLACK, 2.f / kViewScale, LineType::kDot,
                   {fOuterRect.fLeft, fOuterRect.fBottom}, {fOuterRect.fLeft, fOuterRect.fTop});

        // FIXME instead, draw a pixel grid with dotted lines
        // Then evaluate the pixel centers depending on the coverage mode
        // either use the ideal geometry, in which case it is the minimum coverage value, based on
        // signed distances to the different edges, or based on finding which
        // triangle contains the center in the gpu quads, and then interpolating the stored
        // vertex coverage to get the center's coverage. Draw that coverage by filling in the
        // rectangle.
        // FIXME switch to calculating data only when updated
        // FIXME for the ideal mode, look at the signed distance for the intersection points
        // along adjacent edges to detect when we need to switch to a triangle and update what is
        // drawn to highlight the fact that the inner triangle is used
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
            get_shape(outsets, fCorners, idealOutset);
            get_shape(insets, fCorners, idealInset);

            // Visualize the coverage values across the clamping rectangle
            SkPaint pixelPaint;
            pixelPaint.setAntiAlias(true);
            bool useMinDist = false;
            SkRect covRect = fOuterRect.makeOutset(2.f, 2.f);
            for (SkScalar py = covRect.fTop; py < covRect.fBottom; py += 1.f) {
                for (SkScalar px = covRect.fLeft; px < covRect.fRight; px += 1.f) {
                    // px and py are the top-left corner of the current pixel, so get center's coordinate
                    SkPoint pixelCenter = {px + 0.5f, py + 0.5f};
                    SkScalar coverage = useMinDist ?
                            get_edge_dist_coverage(outsets, pixelCenter) :
                            get_framed_coverage(idealOutset, idealOutsetCoverages, idealInset, idealInsetCoverages, pixelCenter);
                    SkRect pixelRect = SkRect::MakeXYWH(px, py, 1.f, 1.f);
                    pixelRect.inset(0.1f, 0.1f);

                    SkScalar a = 1.f - 0.5f * coverage;
                    pixelPaint.setColor4f({a, a, a, 1.f}, nullptr);
                    canvas->drawRect(pixelRect, pixelPaint);
                }
            }

            // Draw the inset/outset "infinite" lines
            for (int i = 0; i < 4; ++i) {
                if (fEdgeAA[i]) {
                    draw_line(canvas, SK_ColorBLUE, 2.f / kViewScale, LineType::kDash,
                                outsets[i * 2], outsets[i * 2 + 1], 5.f, true);
                    draw_line(canvas, SK_ColorGREEN, 2.f / kViewScale, LineType::kDash,
                                insets[i * 2], insets[i * 2 + 1], 5.f, true);
                } else {
                    // Both outset and inset are the same line, so only draw one in cyan
                    draw_line(canvas, SK_ColorCYAN, 2.f / kViewScale, LineType::kDash,
                                outsets[i * 2], outsets[i * 2 + 1], 5.f, true);
                }
            }

            // Draw the polygon boundaries of the ideal shape with corner circles
            for (int i = 0; i < 4; ++i) {
                if (fEdgeAA[i]) {
                    draw_line(canvas, SK_ColorBLUE, 7.f / kViewScale, LineType::kSolid, idealOutset[i], idealOutset[i == 3 ? 0 : i + 1]);
                    draw_line(canvas, SK_ColorGREEN, 7.f / kViewScale, LineType::kSolid, idealInset[i], idealInset[i == 3 ? 0 : i + 1]);
                } else {
                    draw_line(canvas, SK_ColorCYAN, 7.f / kViewScale, LineType::kSolid, idealOutset[i], idealOutset[i == 3 ? 0 : i + 1]);
                    draw_line(canvas, SK_ColorCYAN, 7.f / kViewScale, LineType::kSolid, idealInset[i], idealInset[i == 3 ? 0 : i + 1]);
                }
            }

            for (int i = 0; i < 4; ++i) {
                circlePaint.setColor(SK_ColorBLUE);
                canvas->drawCircle(idealOutset[i], 7.f / kViewScale, circlePaint);
                circlePaint.setColor(SK_ColorGREEN);
                canvas->drawCircle(idealInset[i], 7.f / kViewScale, circlePaint);
            }

            // What is tessellated using GrQuadPerEdgeAA
            if (fShowGPU) {
                SkPoint gpuOutset[4];
                SkPoint gpuInset[4];
                this->getTessellatedPoints(gpuInset, gpuOutset);
                for (int i = 0; i < 4; ++i) {
                    draw_line(canvas, SK_ColorRED, 4.f / kViewScale, LineType::kSolid, gpuOutset[i], gpuOutset[i == 3 ? 0 : i + 1]);
                    draw_line(canvas, SK_ColorMAGENTA, 4.f / kViewScale, LineType::kSolid, gpuInset[i], gpuInset[i == 3 ? 0 : i + 1]);
                }
            }

            // Draw the edges as a solid line
            for (int i = 0; i < 4; ++i) {
                draw_line(canvas, SK_ColorBLACK, 2.f / kViewScale, LineType::kSolid,
                             fCorners[i], fCorners[i == 3 ? 0 : i + 1]);
            }
        } else {
            // Draw the edges as a solid line
            for (int i = 0; i < 4; ++i) {
                draw_line(canvas, SK_ColorRED, 2.f / kViewScale, LineType::kSolid,
                             fCorners[i], fCorners[i == 3 ? 0 : i + 1]);
            }
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

    const SkRect fOuterRect;
    SkPoint fCorners[4]; // TL, TR, BR, BL
    bool fEdgeAA[4]; // T, R, B, L
    bool fShowGPU;

    bool isValid() const {
        SkPath path;
        path.addPoly(fCorners, 4, true);
        return path.isConvex();
    }

    void getTessellatedPoints(SkPoint inset[4], SkPoint outset[4]) const {
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
        inset[3] = {vertices[3], vertices[4]}; // BL
        inset[1] = {vertices[6], vertices[7]}; // TR
        inset[2] = {vertices[9], vertices[10]}; // BR

        outset[0] = {vertices[12], vertices[13]}; // TL
        outset[3] = {vertices[15], vertices[16]}; // BL
        outset[1] = {vertices[18], vertices[19]}; // TR
        outset[2] = {vertices[21], vertices[22]}; // BR
    }

    typedef Sample INHERITED;
};

class DegenerateQuadSample::Click : public Sample::Click {
public:
    Click(Sample* target, const SkRect& clamp, SkPoint* point)
            : Sample::Click(target)
            , fOuterRect(clamp)
            , fPoint(point) {}

    void doClick() {
        if (fPoint) {
            SkIPoint delta = fICurr - fIPrev;
            *fPoint += SkPoint::Make(delta.x() / kViewScale, delta.y() / kViewScale);
            fPoint->fX = SkMinScalar(fOuterRect.fRight, SkMaxScalar(fPoint->fX, fOuterRect.fLeft));
            fPoint->fY = SkMinScalar(fOuterRect.fBottom, SkMaxScalar(fPoint->fY, fOuterRect.fTop));
        }
    }

private:
    SkRect fOuterRect;
    SkPoint* fPoint;
};

Sample::Click* DegenerateQuadSample::onFindClickHandler(SkScalar x, SkScalar y, unsigned) {
    SkPoint inCTM = SkPoint::Make((x - kViewOffset) / kViewScale, (y - kViewOffset) / kViewScale);
    for (int i = 0; i < 4; ++i) {
        if ((fCorners[i] - inCTM).length() < 10.f / kViewScale) {
            return new Click(this, fOuterRect, &fCorners[i]);
        }
    }
    return new Click(this, fOuterRect, nullptr);
}

bool DegenerateQuadSample::onClick(Sample::Click* click) {
    Click* myClick = (Click*) click;
    myClick->doClick();
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
            case ' ':
                fShowGPU = !fShowGPU;
                return true;
        }
    }
    return this->INHERITED::onQuery(event);
}

DEF_SAMPLE(return new DegenerateQuadSample(SkRect::MakeWH(4.f, 4.f));)
