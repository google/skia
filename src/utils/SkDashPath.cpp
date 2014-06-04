/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkDashPathPriv.h"
#include "SkPathMeasure.h"

static inline int is_even(int x) {
    return (~x) << 31;
}

static SkScalar find_first_interval(const SkScalar intervals[], SkScalar phase,
                                    int32_t* index, int count) {
    for (int i = 0; i < count; ++i) {
        if (phase > intervals[i]) {
            phase -= intervals[i];
        } else {
            *index = i;
            return intervals[i] - phase;
        }
    }
    // If we get here, phase "appears" to be larger than our length. This
    // shouldn't happen with perfect precision, but we can accumulate errors
    // during the initial length computation (rounding can make our sum be too
    // big or too small. In that event, we just have to eat the error here.
    *index = 0;
    return intervals[0];
}

void SkDashPath::CalcDashParameters(SkScalar phase, const SkScalar intervals[], int32_t count,
                                    SkScalar* initialDashLength, int32_t* initialDashIndex,
                                    SkScalar* intervalLength, SkScalar* adjustedPhase) {
    SkScalar len = 0;
    for (int i = 0; i < count; i++) {
        len += intervals[i];
    }
    *intervalLength = len;

    // watch out for values that might make us go out of bounds
    if ((len > 0) && SkScalarIsFinite(phase) && SkScalarIsFinite(len)) {

        // Adjust phase to be between 0 and len, "flipping" phase if negative.
        // e.g., if len is 100, then phase of -20 (or -120) is equivalent to 80
        if (adjustedPhase) {
            if (phase < 0) {
                phase = -phase;
                if (phase > len) {
                    phase = SkScalarMod(phase, len);
                }
                phase = len - phase;

                // Due to finite precision, it's possible that phase == len,
                // even after the subtract (if len >>> phase), so fix that here.
                // This fixes http://crbug.com/124652 .
                SkASSERT(phase <= len);
                if (phase == len) {
                    phase = 0;
                }
            } else if (phase >= len) {
                phase = SkScalarMod(phase, len);
            }
            *adjustedPhase = phase;
        }
        SkASSERT(phase >= 0 && phase < len);

        *initialDashLength = find_first_interval(intervals, phase,
                                                initialDashIndex, count);

        SkASSERT(*initialDashLength >= 0);
        SkASSERT(*initialDashIndex >= 0 && *initialDashIndex < count);
    } else {
        *initialDashLength = -1;    // signal bad dash intervals
    }
}

static void outset_for_stroke(SkRect* rect, const SkStrokeRec& rec) {
    SkScalar radius = SkScalarHalf(rec.getWidth());
    if (0 == radius) {
        radius = SK_Scalar1;    // hairlines
    }
    if (SkPaint::kMiter_Join == rec.getJoin()) {
        radius = SkScalarMul(radius, rec.getMiter());
    }
    rect->outset(radius, radius);
}

// Only handles lines for now. If returns true, dstPath is the new (smaller)
// path. If returns false, then dstPath parameter is ignored.
static bool cull_path(const SkPath& srcPath, const SkStrokeRec& rec,
                      const SkRect* cullRect, SkScalar intervalLength,
                      SkPath* dstPath) {
    if (NULL == cullRect) {
        return false;
    }

    SkPoint pts[2];
    if (!srcPath.isLine(pts)) {
        return false;
    }

    SkRect bounds = *cullRect;
    outset_for_stroke(&bounds, rec);

    SkScalar dx = pts[1].x() - pts[0].x();
    SkScalar dy = pts[1].y() - pts[0].y();

    // just do horizontal lines for now (lazy)
    if (dy) {
        return false;
    }

    SkScalar minX = pts[0].fX;
    SkScalar maxX = pts[1].fX;

    if (maxX < bounds.fLeft || minX > bounds.fRight) {
        return false;
    }

    if (dx < 0) {
        SkTSwap(minX, maxX);
    }

    // Now we actually perform the chop, removing the excess to the left and
    // right of the bounds (keeping our new line "in phase" with the dash,
    // hence the (mod intervalLength).

    if (minX < bounds.fLeft) {
        minX = bounds.fLeft - SkScalarMod(bounds.fLeft - minX,
                                          intervalLength);
    }
    if (maxX > bounds.fRight) {
        maxX = bounds.fRight + SkScalarMod(maxX - bounds.fRight,
                                           intervalLength);
    }

    SkASSERT(maxX >= minX);
    if (dx < 0) {
        SkTSwap(minX, maxX);
    }
    pts[0].fX = minX;
    pts[1].fX = maxX;

    dstPath->moveTo(pts[0]);
    dstPath->lineTo(pts[1]);
    return true;
}

class SpecialLineRec {
public:
    bool init(const SkPath& src, SkPath* dst, SkStrokeRec* rec,
              int intervalCount, SkScalar intervalLength) {
        if (rec->isHairlineStyle() || !src.isLine(fPts)) {
            return false;
        }

        // can relax this in the future, if we handle square and round caps
        if (SkPaint::kButt_Cap != rec->getCap()) {
            return false;
        }

        SkScalar pathLength = SkPoint::Distance(fPts[0], fPts[1]);

        fTangent = fPts[1] - fPts[0];
        if (fTangent.isZero()) {
            return false;
        }

        fPathLength = pathLength;
        fTangent.scale(SkScalarInvert(pathLength));
        fTangent.rotateCCW(&fNormal);
        fNormal.scale(SkScalarHalf(rec->getWidth()));

        // now estimate how many quads will be added to the path
        //     resulting segments = pathLen * intervalCount / intervalLen
        //     resulting points = 4 * segments

        SkScalar ptCount = SkScalarMulDiv(pathLength,
                                          SkIntToScalar(intervalCount),
                                          intervalLength);
        int n = SkScalarCeilToInt(ptCount) << 2;
        dst->incReserve(n);

        // we will take care of the stroking
        rec->setFillStyle();
        return true;
    }

    void addSegment(SkScalar d0, SkScalar d1, SkPath* path) const {
        SkASSERT(d0 < fPathLength);
        // clamp the segment to our length
        if (d1 > fPathLength) {
            d1 = fPathLength;
        }

        SkScalar x0 = fPts[0].fX + SkScalarMul(fTangent.fX, d0);
        SkScalar x1 = fPts[0].fX + SkScalarMul(fTangent.fX, d1);
        SkScalar y0 = fPts[0].fY + SkScalarMul(fTangent.fY, d0);
        SkScalar y1 = fPts[0].fY + SkScalarMul(fTangent.fY, d1);

        SkPoint pts[4];
        pts[0].set(x0 + fNormal.fX, y0 + fNormal.fY);   // moveTo
        pts[1].set(x1 + fNormal.fX, y1 + fNormal.fY);   // lineTo
        pts[2].set(x1 - fNormal.fX, y1 - fNormal.fY);   // lineTo
        pts[3].set(x0 - fNormal.fX, y0 - fNormal.fY);   // lineTo

        path->addPoly(pts, SK_ARRAY_COUNT(pts), false);
    }

private:
    SkPoint fPts[2];
    SkVector fTangent;
    SkVector fNormal;
    SkScalar fPathLength;
};


bool SkDashPath::FilterDashPath(SkPath* dst, const SkPath& src, SkStrokeRec* rec,
                                const SkRect* cullRect, const SkScalar aIntervals[],
                                int32_t count, SkScalar initialDashLength, int32_t initialDashIndex,
                                SkScalar intervalLength) {

    // we do nothing if the src wants to be filled, or if our dashlength is 0
    if (rec->isFillStyle() || initialDashLength < 0) {
        return false;
    }

    const SkScalar* intervals = aIntervals;
    SkScalar        dashCount = 0;
    int             segCount = 0;

    SkPath cullPathStorage;
    const SkPath* srcPtr = &src;
    if (cull_path(src, *rec, cullRect, intervalLength, &cullPathStorage)) {
        srcPtr = &cullPathStorage;
    }

    SpecialLineRec lineRec;
    bool specialLine = lineRec.init(*srcPtr, dst, rec, count >> 1, intervalLength);

    SkPathMeasure   meas(*srcPtr, false);

    do {
        bool        skipFirstSegment = meas.isClosed();
        bool        addedSegment = false;
        SkScalar    length = meas.getLength();
        int         index = initialDashIndex;

        // Since the path length / dash length ratio may be arbitrarily large, we can exert
        // significant memory pressure while attempting to build the filtered path. To avoid this,
        // we simply give up dashing beyond a certain threshold.
        //
        // The original bug report (http://crbug.com/165432) is based on a path yielding more than
        // 90 million dash segments and crashing the memory allocator. A limit of 1 million
        // segments seems reasonable: at 2 verbs per segment * 9 bytes per verb, this caps the
        // maximum dash memory overhead at roughly 17MB per path.
        static const SkScalar kMaxDashCount = 1000000;
        dashCount += length * (count >> 1) / intervalLength;
        if (dashCount > kMaxDashCount) {
            dst->reset();
            return false;
        }

        // Using double precision to avoid looping indefinitely due to single precision rounding
        // (for extreme path_length/dash_length ratios). See test_infinite_dash() unittest.
        double  distance = 0;
        double  dlen = initialDashLength;

        while (distance < length) {
            SkASSERT(dlen >= 0);
            addedSegment = false;
            if (is_even(index) && dlen > 0 && !skipFirstSegment) {
                addedSegment = true;
                ++segCount;

                if (specialLine) {
                    lineRec.addSegment(SkDoubleToScalar(distance),
                                       SkDoubleToScalar(distance + dlen),
                                       dst);
                } else {
                    meas.getSegment(SkDoubleToScalar(distance),
                                    SkDoubleToScalar(distance + dlen),
                                    dst, true);
                }
            }
            distance += dlen;

            // clear this so we only respect it the first time around
            skipFirstSegment = false;

            // wrap around our intervals array if necessary
            index += 1;
            SkASSERT(index <= count);
            if (index == count) {
                index = 0;
            }

            // fetch our next dlen
            dlen = intervals[index];
        }

        // extend if we ended on a segment and we need to join up with the (skipped) initial segment
        if (meas.isClosed() && is_even(initialDashIndex) &&
            initialDashLength > 0) {
            meas.getSegment(0, initialDashLength, dst, !addedSegment);
            ++segCount;
        }
    } while (meas.nextContour());

    if (segCount > 1) {
        dst->setConvexity(SkPath::kConcave_Convexity);
    }

    return true;
}

bool SkDashPath::FilterDashPath(SkPath* dst, const SkPath& src, SkStrokeRec* rec,
                                const SkRect* cullRect, const SkPathEffect::DashInfo& info) {
    SkScalar initialDashLength = 0;
    int32_t initialDashIndex = 0;
    SkScalar intervalLength = 0;
    CalcDashParameters(info.fPhase, info.fIntervals, info.fCount,
                       &initialDashLength, &initialDashIndex, &intervalLength);
    return FilterDashPath(dst, src, rec, cullRect, info.fIntervals, info.fCount, initialDashLength,
                          initialDashIndex, intervalLength);
}
