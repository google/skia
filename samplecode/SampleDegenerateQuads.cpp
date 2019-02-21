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
                             const SkPoint& p0, const SkPoint& p1, SkScalar extension = 0.f) {
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
    canvas->drawLine(p1 - v, p0 + v, paint);
}

static void make_aa_line(const SkPoint& p0, const SkPoint& p1, bool aaOn,
                         bool outset, SkPoint line[2]) {
    SkVector v = p1 - p0;
    SkVector n = outset ? SkVector::Make(v.fY, -v.fX) : SkVector::Make(-v.fY, v.fX);
    n.setLength(0.5f);
    v.setLength(2.f);
    if (!aaOn) {
        n = {0.f, 0.f};
    }
    line[0] = p0 + n - v;
    line[1] = p1 + n + v;
}

// Returns true if line segment (p0-p1) intersects with line segment (l0-l1); if true is returned,
// the intersection point is stored in 'intersect'.
static bool intersect_line_segments(const SkPoint& p0, const SkPoint& p1,
                                    const SkPoint& l0, const SkPoint& l1, SkPoint* intersect) {
    static constexpr SkScalar kHorizontalTolerance = 0.01f; // Pretty conservative

    // Use doubles for accuracy, since the clipping strategy used below can create T
    // junctions, and lower precision could artificially create gaps
    double pY = (double) p1.fY - (double) p0.fY;
    double pX = (double) p1.fX - (double) p0.fX;
    double lY = (double) l1.fY - (double) l0.fY;
    double lX = (double) l1.fX - (double) l0.fX;
    double plY = (double) p0.fY - (double) l0.fY;
    double plX = (double) p0.fX - (double) l0.fX;
    if (SkScalarNearlyZero(pY, kHorizontalTolerance)) {
        if (SkScalarNearlyZero(lY, kHorizontalTolerance)) {
            // Two horizontal lines
            return false;
        } else {
            // Recalculate but swap p and l
            return intersect_line_segments(l0, l1, p0, p1, intersect);
        }
    }

    // Up to now, the line segments do not form an invalid intersection
    double lNumerator = plX * pY - plY * pX;
    double lDenom = lX * pY - lY * pX;
    if (SkScalarNearlyZero(lDenom)) {
        // Parallel or identical
        return false;
    }

    // Calculate alphaL that provides the intersection point along (l0-l1), e.g. l0+alphaL*(l1-l0)
    double alphaL = lNumerator / lDenom;
    if (alphaL < 0.0 || alphaL > 1.0) {
        // Outside of the l segment
        return false;
    }

    // Calculate alphaP from the valid alphaL (since it could be outside p segment)
    // double alphaP = (alphaL * l.fY - pl.fY) / p.fY;
    double alphaP = (alphaL * lY - plY) / pY;
    if (alphaP < 0.0 || alphaP > 1.0) {
        // Outside of p segment
        return false;
    }

    // Is valid, so calculate the actual intersection point
    *intersect = l1 * SkScalar(alphaL) + l0 * SkScalar(1.0 - alphaL);
    return true;
}

static constexpr SkScalar kViewScale = 100.f;
static constexpr SkScalar kViewOffset = 200.f;

class DegenerateQuadSample : public Sample {
public:
    DegenerateQuadSample(const SkRect& rect)
            : fOuterRect(rect) {
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

        // Draw the edges as a solid line
        for (int i = 0; i < 4; ++i) {
            draw_line(canvas, SK_ColorBLACK, 2.f / kViewScale, LineType::kSolid,
                         fCorners[i], fCorners[i == 3 ? 0 : i + 1]);
        }

        // What is tessellated using GrQuadPerEdgeAA
        SkPoint gpuOutset[4];
        SkPoint gpuInset[4];
        this->getTessellatedPoints(gpuInset, gpuOutset);
        for (int i = 0; i < 4; ++i) {
            // With borders drawn
            draw_line(canvas, SK_ColorRED, 4.f / kViewScale, LineType::kSolid, gpuOutset[i], gpuOutset[i == 3 ? 0 : i + 1]);
            draw_line(canvas, SK_ColorMAGENTA, 4.f / kViewScale, LineType::kSolid, gpuInset[i], gpuInset[i == 3 ? 0 : i + 1]);
        }

        SkPoint outsets[8];
        SkPoint insets[8];
        // Draw the outsets as a dashed blue line, and insets as a dashed green line
        for (int i = 0; i < 4; ++i) {
            make_aa_line(fCorners[i], fCorners[i == 3 ? 0 : i + 1], fEdgeAA[i], true, outsets + i * 2);
            make_aa_line(fCorners[i], fCorners[i == 3 ? 0 : i + 1], fEdgeAA[i], false, insets + i * 2);
            draw_line(canvas, SK_ColorBLUE, 2.f / kViewScale, LineType::kDash,
                        outsets[i * 2], outsets[i * 2 + 1], 5.f);
            draw_line(canvas, SK_ColorGREEN, 2.f / kViewScale, LineType::kDash,
                        insets[i * 2], insets[i * 2 + 1], 5.f);
        }

        // Gpu points, on top of the ideal inset/outset geometry
        for (int i = 0; i < 4; ++i) {
            circlePaint.setColor(SK_ColorRED);
            canvas->drawCircle(gpuOutset[i], 5.f / kViewScale, circlePaint);
            circlePaint.setColor(SK_ColorMAGENTA);
            canvas->drawCircle(gpuInset[i], 5.f / kViewScale, circlePaint);
        }

        // Now draw the outset and inset intersection points
        for (int i = 0; i < 4; ++i) {
            SkPoint p;
            int ni = i == 3 ? 0 : i + 1;
            // Check edge i vs. i+1
            if (intersect_line_segments(outsets[i * 2], outsets[i * 2 + 1],
                    outsets[ni * 2], outsets[ni * 2 + 1], &p)) {
                circlePaint.setColor(SK_ColorBLUE);
                canvas->drawCircle(p, 3.f / kViewScale, circlePaint);
            }
            if (intersect_line_segments(insets[i * 2], insets[i * 2 + 1],
                    insets[ni * 2], insets[ni * 2 + 1], &p)) {
                circlePaint.setColor(SK_ColorGREEN);
                canvas->drawCircle(p, 3.f / kViewScale, circlePaint);
            }
        }

        // Draw the four corners as circles
        circlePaint.setColor(SK_ColorBLACK);
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

    void getTessellatedPoints(SkPoint inset[4], SkPoint outset[4]) {
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
        if ((fCorners[i] - inCTM).length() < 5.f / kViewScale) {
            return new Click(this, fOuterRect, &fCorners[i]);
        }
    }
    return nullptr;
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
        }
    }
    return this->INHERITED::onQuery(event);
}

DEF_SAMPLE(return new DegenerateQuadSample(SkRect::MakeWH(4.f, 4.f));)
