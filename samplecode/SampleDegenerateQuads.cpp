/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "samplecode/Sample.h"

#include "src/gpu/geometry/GrQuad.h"
#include "src/gpu/ops/GrQuadPerEdgeAA.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "include/effects/SkDashPathEffect.h"
#include "include/pathops/SkPathOps.h"

// Draw a line through the two points, outset by a fixed length in screen space
static void draw_extended_line(SkCanvas* canvas, const SkPaint paint,
                              const SkPoint& p0, const SkPoint& p1) {
    SkVector v = p1 - p0;
    v.setLength(v.length() + 3.f);
    canvas->drawLine(p1 - v, p0 + v, paint);

    // Draw normal vector too
    SkPaint normalPaint = paint;
    normalPaint.setPathEffect(nullptr);
    normalPaint.setStrokeWidth(paint.getStrokeWidth() / 4.f);

    SkVector n = {v.fY, -v.fX};
    n.setLength(.25f);
    SkPoint m = (p0 + p1) * 0.5f;
    canvas->drawLine(m, m + n, normalPaint);
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

static SkScalar get_area_coverage(const bool edgeAA[4], const SkPoint corners[4],
                                  const SkPoint& point) {
    SkPath shape;
    shape.addPoly(corners, 4, true);
    SkPath pixel;
    pixel.addRect(SkRect::MakeXYWH(point.fX - 0.5f, point.fY - 0.5f, 1.f, 1.f));

    SkPath intersection;
    if (!Op(shape, pixel, kIntersect_SkPathOp, &intersection) || intersection.isEmpty()) {
        return 0.f;
    }

    // Calculate area of the convex polygon
    SkScalar area = 0.f;
    for (int i = 0; i < intersection.countPoints(); ++i) {
        SkPoint p0 = intersection.getPoint(i);
        SkPoint p1 = intersection.getPoint((i + 1) % intersection.countPoints());
        SkScalar det = p0.fX * p1.fY - p1.fX * p0.fY;
        area += det;
    }

    // Scale by 1/2, then take abs value (this area formula is signed based on point winding, but
    // since it's convex, just make it positive).
    area = SkScalarAbs(0.5f * area);

    // Now account for the edge AA. If the pixel center is outside of a non-AA edge, turn of its
    // coverage. If the pixel only intersects non-AA edges, then set coverage to 1.
    bool needsNonAA = false;
    SkScalar edgeD[4];
    for (int i = 0; i < 4; ++i) {
        SkPoint e0 = corners[i];
        SkPoint e1 = corners[(i + 1) % 4];
        edgeD[i] = -signed_distance(point, e0, e1);
        if (!edgeAA[i]) {
            if (edgeD[i] < -1e-4f) {
                return 0.f; // Outside of non-AA line
            }
            needsNonAA = true;
        }
    }
    // Otherwise inside the shape, so check if any AA edge exerts influence over nonAA
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

// FIXME take into account max coverage properly,
static SkScalar get_edge_dist_coverage(const bool edgeAA[4], const SkPoint corners[4],
                                       const SkPoint outsetLines[8], const SkPoint insetLines[8],
                                       const SkPoint& point) {
    bool flip = false;
    // If the quad has been inverted, the original corners will not all be on the negative side of
    // every outset line. When that happens, calculate coverage using the "inset" lines and flip
    // the signed distance
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            SkScalar d = signed_distance(corners[i], outsetLines[j * 2], outsetLines[j * 2 + 1]);
            if (d > 1e-4f) {
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
        SkScalar d = (flip ? 1 : -1) * signed_distance(point, lines[i * 2], lines[i * 2 + 1]);
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

static bool inside_triangle(const SkPoint& point, const SkPoint& t0, const SkPoint& t1,
                            const SkPoint& t2, SkScalar bary[3]) {
    // Check sign of t0 to (t1,t2). If it is positive, that means the normals point into the
    // triangle otherwise the normals point outside the triangle so update edge distances as
    // necessary
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
                                    const SkRect& geomDomain, const SkPoint& point) {
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

            SkScalar coverage = bary[0] * c0 + bary[1] * c1 + bary[2] * c2;
            if (coverage < 0.5f) {
                // Check distances to domain
                SkScalar l = SkScalarPin(point.fX - geomDomain.fLeft, 0.f, 1.f);
                SkScalar t = SkScalarPin(point.fY - geomDomain.fTop, 0.f, 1.f);
                SkScalar r = SkScalarPin(geomDomain.fRight - point.fX, 0.f, 1.f);
                SkScalar b = SkScalarPin(geomDomain.fBottom - point.fY, 0.f, 1.f);
                coverage = SkMinScalar(coverage, l * t * r * b);
            }
            return coverage;
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
            // Calculate inset and outset lines for edge-distance visualization
            for (int i = 0; i < 4; ++i) {
                make_aa_line(fCorners[i], fCorners[(i + 1) % 4], fEdgeAA[i], true, outsets + i * 2);
                make_aa_line(fCorners[i], fCorners[(i + 1) % 4], fEdgeAA[i], false, insets + i * 2);
            }

            // Calculate inner and outer meshes for GPU visualization
            SkPoint gpuOutset[4];
            SkScalar gpuOutsetCoverage[4];
            SkPoint gpuInset[4];
            SkScalar gpuInsetCoverage[4];
            SkRect gpuDomain;
            this->getTessellatedPoints(gpuInset, gpuInsetCoverage, gpuOutset, gpuOutsetCoverage,
                                       &gpuDomain);

            // Visualize the coverage values across the clamping rectangle, but test pixels outside
            // of the "outer" rect since some quad edges can be outset extra far.
            SkPaint pixelPaint;
            pixelPaint.setAntiAlias(true);
            SkRect covRect = fOuterRect.makeOutset(2.f, 2.f);
            for (SkScalar py = covRect.fTop; py < covRect.fBottom; py += 1.f) {
                for (SkScalar px = covRect.fLeft; px < covRect.fRight; px += 1.f) {
                    // px and py are the top-left corner of the current pixel, so get center's
                    // coordinate
                    SkPoint pixelCenter = {px + 0.5f, py + 0.5f};
                    SkScalar coverage;
                    if (fCoverageMode == CoverageMode::kArea) {
                        coverage = get_area_coverage(fEdgeAA, fCorners, pixelCenter);
                    } else if (fCoverageMode == CoverageMode::kEdgeDistance) {
                        coverage = get_edge_dist_coverage(fEdgeAA, fCorners, outsets, insets,
                                                          pixelCenter);
                    } else {
                        SkASSERT(fCoverageMode == CoverageMode::kGPUMesh);
                        coverage = get_framed_coverage(gpuOutset, gpuOutsetCoverage,
                                                       gpuInset, gpuInsetCoverage, gpuDomain,
                                                       pixelCenter);
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
                        draw_extended_line(canvas, linePaint, outsets[i * 2], outsets[i * 2 + 1]);
                        linePaint.setColor(SK_ColorGREEN);
                        draw_extended_line(canvas, linePaint, insets[i * 2], insets[i * 2 + 1]);
                    } else {
                        // Both outset and inset are the same line, so only draw one in cyan
                        linePaint.setColor(SK_ColorCYAN);
                        draw_extended_line(canvas, linePaint, outsets[i * 2], outsets[i * 2 + 1]);
                    }
                }
            }

            linePaint.setPathEffect(nullptr);
            // What is tessellated using GrQuadPerEdgeAA
            if (fCoverageMode == CoverageMode::kGPUMesh) {
                SkPath outsetPath;
                outsetPath.addPoly(gpuOutset, 4, true);
                linePaint.setColor(SK_ColorBLUE);
                canvas->drawPath(outsetPath, linePaint);

                SkPath insetPath;
                insetPath.addPoly(gpuInset, 4, true);
                linePaint.setColor(SK_ColorGREEN);
                canvas->drawPath(insetPath, linePaint);

                SkPaint domainPaint = linePaint;
                domainPaint.setStrokeWidth(2.f / kViewScale);
                domainPaint.setPathEffect(dashes);
                domainPaint.setColor(SK_ColorMAGENTA);
                canvas->drawRect(gpuDomain, domainPaint);
            }

            // Draw the edges of the true quad as a solid line
            SkPath path;
            path.addPoly(fCorners, 4, true);
            linePaint.setColor(SK_ColorBLACK);
            canvas->drawPath(path, linePaint);
        } else {
            // Draw the edges of the true quad as a solid *red* line
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

    Sample::Click* onFindClickHandler(SkScalar x, SkScalar y,
                                      unsigned) override;
    bool onClick(Sample::Click*) override;
    bool onChar(SkUnichar) override;
    SkString name() override { return SkString("DegenerateQuad"); }

private:
    class Click;

    enum class CoverageMode {
        kArea, kEdgeDistance, kGPUMesh
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
                              SkScalar outsetCoverage[4], SkRect* domain) const {
        // Fixed vertex spec for extracting the picture frame geometry
        static const GrQuadPerEdgeAA::VertexSpec kSpec =
            {GrQuad::Type::kGeneral, GrQuadPerEdgeAA::ColorType::kNone,
             GrQuad::Type::kAxisAligned, false, GrQuadPerEdgeAA::Domain::kNo,
             GrAAType::kCoverage, false};
        static const GrQuad kIgnored(SkRect::MakeEmpty());

        GrQuadAAFlags flags = GrQuadAAFlags::kNone;
        flags |= fEdgeAA[0] ? GrQuadAAFlags::kTop : GrQuadAAFlags::kNone;
        flags |= fEdgeAA[1] ? GrQuadAAFlags::kRight : GrQuadAAFlags::kNone;
        flags |= fEdgeAA[2] ? GrQuadAAFlags::kBottom : GrQuadAAFlags::kNone;
        flags |= fEdgeAA[3] ? GrQuadAAFlags::kLeft : GrQuadAAFlags::kNone;

        GrQuad quad = GrQuad::MakeFromSkQuad(fCorners, SkMatrix::I());

        float vertices[56]; // 2 quads, with x, y, coverage, and geometry domain (7 floats x 8 vert)
        GrQuadPerEdgeAA::Tessellate(vertices, kSpec, quad, {1.f, 1.f, 1.f, 1.f},
                GrQuad(SkRect::MakeEmpty()), SkRect::MakeEmpty(), flags);

        // The first quad in vertices is the inset, then the outset, but they
        // are ordered TL, BL, TR, BR so un-interleave coverage and re-arrange
        inset[0] = {vertices[0], vertices[1]}; // TL
        insetCoverage[0] = vertices[2];
        inset[3] = {vertices[7], vertices[8]}; // BL
        insetCoverage[3] = vertices[9];
        inset[1] = {vertices[14], vertices[15]}; // TR
        insetCoverage[1] = vertices[16];
        inset[2] = {vertices[21], vertices[22]}; // BR
        insetCoverage[2] = vertices[23];

        outset[0] = {vertices[28], vertices[29]}; // TL
        outsetCoverage[0] = vertices[30];
        outset[3] = {vertices[35], vertices[36]}; // BL
        outsetCoverage[3] = vertices[37];
        outset[1] = {vertices[42], vertices[43]}; // TR
        outsetCoverage[1] = vertices[44];
        outset[2] = {vertices[49], vertices[50]}; // BR
        outsetCoverage[2] = vertices[51];

        *domain = {vertices[52], vertices[53], vertices[54], vertices[55]};
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

bool DegenerateQuadSample::onChar(SkUnichar code) {
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
                fCoverageMode = CoverageMode::kGPUMesh;
                return true;
        }
        return false;
}

DEF_SAMPLE(return new DegenerateQuadSample(SkRect::MakeWH(4.f, 4.f));)
