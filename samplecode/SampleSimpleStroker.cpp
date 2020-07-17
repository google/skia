/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkPath.h"
#include "include/utils/SkParsePath.h"
#include "samplecode/Sample.h"

#include "src/core/SkGeometry.h"

namespace {

//////////////////////////////////////////////////////////////////////////////

static SkPoint rotate90(const SkPoint& p) { return {-p.fY, p.fX}; }
static SkPoint rotate180(const SkPoint& p) { return p * -1; }
static SkPoint setLength(SkPoint p, float len) {
    if (!p.setLength(len)) {
        SkDebugf("Failed to set point length\n");
    }
    return p;
}
static bool isClockwise(const SkPoint& a, const SkPoint& b) { return a.cross(b) > 0; }

//////////////////////////////////////////////////////////////////////////////

// Testing ground for a new stroker implementation
class SkPathStroker2 {
public:
    // Returns the fill path
    SkPath getFillPath(const SkPath& path, const SkPaint& paint);

private:
    struct PathSegment {
        SkPath::Verb fVerb;
        SkPoint fPoints[4];
    };

    float fRadius;
    SkPaint::Cap fCap;
    SkPaint::Join fJoin;
    SkPath fInnerPath, fOuterPath;
    SkPath *fInner = &fInnerPath, *fOuter = &fOuterPath;

    // Initialize stroker state
    void initForPath(const SkPath& path, const SkPaint& paint);

    // Strokes a line segment
    void strokeLine(const PathSegment& line, bool needsMove);

    // Adds an endcap to fOuter
    enum class CapLocation { Start, End };
    void endcap(CapLocation loc);

    // Adds a join between the two segments
    void join(const PathSegment& prev, const PathSegment& curr);

    // Appends path in reverse to result
    static void appendPathReversed(const SkPath* path, SkPath* result);

    // Returns the segment unit normal
    static SkPoint unitNormal(const PathSegment& seg, float t);
};

void SkPathStroker2::initForPath(const SkPath& path, const SkPaint& paint) {
    fRadius = paint.getStrokeWidth() / 2;
    fCap = paint.getStrokeCap();
    fJoin = paint.getStrokeJoin();
    fInnerPath.rewind();
    fOuterPath.rewind();
    fInner = &fInnerPath;
    fOuter = &fOuterPath;
}

SkPath SkPathStroker2::getFillPath(const SkPath& path, const SkPaint& paint) {
    initForPath(path, paint);

    // Trace the inner and outer paths simultaneously. Inner will therefore be
    // recorded in reverse from how we trace the outline.
    SkPath::Iter it(path, false);
    PathSegment segment, prevSegment;
    bool firstSegment = true;
    while ((segment.fVerb = it.next(segment.fPoints)) != SkPath::kDone_Verb) {
        // Join to the previous segment
        if (!firstSegment) {
            join(prevSegment, segment);
        }

        // Stroke the current segment
        switch (segment.fVerb) {
            case SkPath::kLine_Verb:
                strokeLine(segment, firstSegment);
                break;
            case SkPath::kMove_Verb:
                // Don't care about multiple contours currently
                continue;
            default:
                SkDebugf("Unhandled path verb %d\n", segment.fVerb);
                break;
        }

        std::swap(segment, prevSegment);
        firstSegment = false;
    }

    // Open contour => endcap at the end
    const bool isClosed = path.isLastContourClosed();
    if (isClosed) {
        SkDebugf("Unhandled closed contour\n");
    } else {
        endcap(CapLocation::End);
    }

    // Walk inner path in reverse, appending to result
    appendPathReversed(fInner, fOuter);
    endcap(CapLocation::Start);

    return fOuterPath;
}

void SkPathStroker2::strokeLine(const PathSegment& line, bool needsMove) {
    const SkPoint tangent = line.fPoints[1] - line.fPoints[0];
    const SkPoint normal = rotate90(tangent);
    const SkPoint offset = setLength(normal, fRadius);
    if (needsMove) {
        fOuter->moveTo(line.fPoints[0] + offset);
        fInner->moveTo(line.fPoints[0] - offset);
    }
    fOuter->lineTo(line.fPoints[1] + offset);
    fInner->lineTo(line.fPoints[1] - offset);
}

void SkPathStroker2::endcap(CapLocation loc) {
    const auto buttCap = [this](CapLocation loc) {
        if (loc == CapLocation::Start) {
            // Back at the start of the path: just close the stroked outline
            fOuter->close();
        } else {
            // Inner last pt == first pt when appending in reverse
            SkPoint innerLastPt;
            fInner->getLastPt(&innerLastPt);
            fOuter->lineTo(innerLastPt);
        }
    };

    switch (fCap) {
        case SkPaint::kButt_Cap:
            buttCap(loc);
            break;
        default:
            SkDebugf("Unhandled endcap %d\n", fCap);
            buttCap(loc);
            break;
    }
}

void SkPathStroker2::join(const PathSegment& prev, const PathSegment& curr) {
    const auto miterJoin = [this](const PathSegment& prev, const PathSegment& curr) {
        const SkPoint miterMidpt = curr.fPoints[0];
        SkPoint before = unitNormal(prev, 1);
        SkPoint after = unitNormal(curr, 0);

        // Check who's inside and who's outside.
        SkPath *outer = fOuter, *inner = fInner;
        if (!isClockwise(before, after)) {
            std::swap(inner, outer);
            before = rotate180(before);
            after = rotate180(after);
        }

        const float cosTheta = before.dot(after);
        if (SkScalarNearlyZero(1 - cosTheta)) {
            // Nearly identical normals: don't bother.
            return;
        }

        // Before and after have the same origin and magnitude, so before+after is the diagonal of
        // their rhombus. Origin of this vector is the midpoint of the miter line.
        SkPoint miterVec = before + after;

        // Note the relationship (draw a right triangle with the miter line as its hypoteneuse):
        //     sin(theta/2) = strokeWidth / miterLength
        // so miterLength = strokeWidth / sin(theta/2)
        // where miterLength is the length of the miter from outer point to inner corner.
        // miterVec's origin is the midpoint of the miter line, so we use strokeWidth/2.
        // Sqrt is just an application of half-angle identities.
        const float sinHalfTheta = sqrtf(0.5 * (1 + cosTheta));
        const float halfMiterLength = fRadius / sinHalfTheta;
        miterVec.setLength(halfMiterLength);  // TODO: miter length limit

        outer->lineTo(miterMidpt + miterVec);
        inner->lineTo(miterMidpt - miterVec);
    };

    switch (fJoin) {
        case SkPaint::kMiter_Join:
            miterJoin(prev, curr);
            break;
        default:
            SkDebugf("Unhandled join %d\n", fJoin);
            miterJoin(prev, curr);
            break;
    }
}

void SkPathStroker2::appendPathReversed(const SkPath* path, SkPath* result) {
    const int numVerbs = path->countVerbs();
    const int numPoints = path->countPoints();
    std::unique_ptr<uint8_t[]> verbs = std::make_unique<uint8_t[]>(numVerbs);
    std::unique_ptr<SkPoint[]> points = std::make_unique<SkPoint[]>(numPoints);

    path->getVerbs(verbs.get(), numVerbs);
    path->getPoints(points.get(), numPoints);

    for (int i = numVerbs - 1, j = numPoints; i >= 0; i--) {
        auto verb = static_cast<SkPath::Verb>(verbs[i]);
        switch (verb) {
            case SkPath::kLine_Verb: {
                j -= 1;
                SkASSERT(j >= 1);
                result->lineTo(points[j - 1]);
                break;
            }
            case SkPath::kMove_Verb:
                // Ignore
                break;
            default:
                SkASSERT(false);
                break;
        }
    }
}

SkPoint SkPathStroker2::unitNormal(const PathSegment& seg, float t) {
    if (seg.fVerb != SkPath::kLine_Verb) {
        SkDebugf("Unhandled verb for unit normal %d\n", seg.fVerb);
    }

    (void)t;  // Not needed for lines
    const SkPoint tangent = seg.fPoints[1] - seg.fPoints[0];
    const SkPoint normal = rotate90(tangent);
    return setLength(normal, 1);
}

}  // namespace

//////////////////////////////////////////////////////////////////////////////

class SimpleStroker : public Sample {
    bool fShowSkiaStroke, fShowHidden;
    float fWidth = 175;
    SkPaint fPtsPaint, fStrokePaint, fNewFillPaint, fHiddenPaint;
    static constexpr int kN = 3;

public:
    SkPoint fPts[kN];

    SimpleStroker() : fShowSkiaStroke(true), fShowHidden(false) {
        fPts[0] = {500, 200};
        fPts[1] = {300, 200};
        fPts[2] = {100, 100};

        fPtsPaint.setAntiAlias(true);
        fPtsPaint.setStrokeWidth(10);
        fPtsPaint.setStrokeCap(SkPaint::kRound_Cap);

        fStrokePaint.setAntiAlias(true);
        fStrokePaint.setStyle(SkPaint::kStroke_Style);
        fStrokePaint.setColor(0x80FF0000);

        fNewFillPaint.setAntiAlias(true);
        fNewFillPaint.setColor(0x8000FF00);

        fHiddenPaint.setAntiAlias(true);
        fHiddenPaint.setStyle(SkPaint::kStroke_Style);
        fHiddenPaint.setColor(0xFF0000FF);
    }

    void toggle(bool& value) { value = !value; }

protected:
    SkString name() override { return SkString("SimpleStroker"); }

    bool onChar(SkUnichar uni) override {
        switch (uni) {
            case '1':
                this->toggle(fShowSkiaStroke);
                return true;
            case '2':
                this->toggle(fShowHidden);
                return true;
            case '-':
                fWidth -= 5;
                return true;
            case '=':
                fWidth += 5;
                return true;
            default:
                break;
        }
        return false;
    }

    void makePath(SkPath* path) {
        path->moveTo(fPts[0]);
        for (int i = 1; i < kN; ++i) {
            path->lineTo(fPts[i]);
        }
    }

    void onDrawContent(SkCanvas* canvas) override {
        canvas->drawColor(0xFFEEEEEE);

        SkPath path;
        this->makePath(&path);

        fStrokePaint.setStrokeWidth(fWidth);

        // The correct result
        if (fShowSkiaStroke) {
            canvas->drawPath(path, fStrokePaint);
        }

        // Simple stroker result
        SkPathStroker2 stroker;
        SkPath fillPath = stroker.getFillPath(path, fStrokePaint);
        canvas->drawPath(fillPath, fNewFillPaint);

        if (fShowHidden) {
            canvas->drawPath(fillPath, fHiddenPaint);
        }

        canvas->drawPoints(SkCanvas::kPoints_PointMode, kN, fPts, fPtsPaint);
    }

    Sample::Click* onFindClickHandler(SkScalar x, SkScalar y, skui::ModifierKey modi) override {
        const SkScalar tol = 4;
        const SkRect r = SkRect::MakeXYWH(x - tol, y - tol, tol * 2, tol * 2);
        for (int i = 0; i < kN; ++i) {
            if (r.intersects(SkRect::MakeXYWH(fPts[i].fX, fPts[i].fY, 1, 1))) {
                return new Click([this, i](Click* c) {
                    fPts[i] = c->fCurr;
                    return true;
                });
            }
        }
        return nullptr;
    }

private:
    typedef Sample INHERITED;
};

DEF_SAMPLE(return new SimpleStroker;)
