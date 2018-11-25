/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkDashPathPriv.h"
#include "SkPathMeasure.h"
#include "SkPointPriv.h"
#include "SkStrokeRec.h"

static inline int is_even(int x) {
    return !(x & 1);
}

static SkScalar find_first_interval(const SkScalar intervals[], SkScalar phase,
                                    int32_t* index, int count) {
    for (int i = 0; i < count; ++i) {
        SkScalar gap = intervals[i];
        if (phase > gap || (phase == gap && gap)) {
            phase -= gap;
        } else {
            *index = i;
            return gap - phase;
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
}

static void outset_for_stroke(SkRect* rect, const SkStrokeRec& rec) {
    SkScalar radius = SkScalarHalf(rec.getWidth());
    if (0 == radius) {
        radius = SK_Scalar1;    // hairlines
    }
    if (SkPaint::kMiter_Join == rec.getJoin()) {
        radius *= rec.getMiter();
    }
    rect->outset(radius, radius);
}

#ifndef SK_SUPPORT_LEGACY_DASH_CULL_PATH
// If line is zero-length, bump out the end by a tiny amount
// to draw endcaps. The bump factor is sized so that
// SkPoint::Distance() computes a non-zero length.
// Offsets SK_ScalarNearlyZero or smaller create empty paths when Iter measures length.
// Large values are scaled by SK_ScalarNearlyZero so significant bits change.
static void adjust_zero_length_line(SkPoint pts[2]) {
    SkASSERT(pts[0] == pts[1]);
    pts[1].fX += SkTMax(1.001f, pts[1].fX) * SK_ScalarNearlyZero;
}

static bool clip_line(SkPoint pts[2], const SkRect& bounds, SkScalar intervalLength,
                      SkScalar priorPhase) {
    SkVector dxy = pts[1] - pts[0];

    // only horizontal or vertical lines
    if (dxy.fX && dxy.fY) {
        return false;
    }
    int xyOffset = SkToBool(dxy.fY);  // 0 to adjust horizontal, 1 to adjust vertical

    SkScalar minXY = (&pts[0].fX)[xyOffset];
    SkScalar maxXY = (&pts[1].fX)[xyOffset];
    bool swapped = maxXY < minXY;
    if (swapped) {
        SkTSwap(minXY, maxXY);
    }

    SkASSERT(minXY <= maxXY);
    SkScalar leftTop = (&bounds.fLeft)[xyOffset];
    SkScalar rightBottom = (&bounds.fRight)[xyOffset];
    if (maxXY < leftTop || minXY > rightBottom) {
        return false;
    }

    // Now we actually perform the chop, removing the excess to the left/top and
    // right/bottom of the bounds (keeping our new line "in phase" with the dash,
    // hence the (mod intervalLength).

    if (minXY < leftTop) {
        minXY = leftTop - SkScalarMod(leftTop - minXY, intervalLength);
        if (!swapped) {
            minXY -= priorPhase;  // for rectangles, adjust by prior phase
        }
    }
    if (maxXY > rightBottom) {
        maxXY = rightBottom + SkScalarMod(maxXY - rightBottom, intervalLength);
        if (swapped) {
            maxXY += priorPhase;  // for rectangles, adjust by prior phase
        }
    }

    SkASSERT(maxXY >= minXY);
    if (swapped) {
        SkTSwap(minXY, maxXY);
    }
    (&pts[0].fX)[xyOffset] = minXY;
    (&pts[1].fX)[xyOffset] = maxXY;

    if (minXY == maxXY) {
        adjust_zero_length_line(pts);
    }
    return true;
}

static bool contains_inclusive(const SkRect& rect, const SkPoint& pt) {
    return rect.fLeft <= pt.fX && pt.fX <= rect.fRight &&
            rect.fTop <= pt.fY && pt.fY <= rect.fBottom;
}

// Returns true is b is between a and c, that is: a <= b <= c, or a >= b >= c.
// Can perform this test with one branch by observing that, relative to b,
// the condition is true only if one side is positive and one side is negative.
// If the numbers are very small, the optimization may return the wrong result
// because the multiply may generate a zero where the simple compare does not.
// For this reason the assert does not fire when all three numbers are near zero.
static bool between(SkScalar a, SkScalar b, SkScalar c) {
    SkASSERT(((a <= b && b <= c) || (a >= b && b >= c)) == ((a - b) * (c - b) <= 0)
            || (SkScalarNearlyZero(a) && SkScalarNearlyZero(b) && SkScalarNearlyZero(c)));
    return (a - b) * (c - b) <= 0;
}
#endif

// Only handles lines for now. If returns true, dstPath is the new (smaller)
// path. If returns false, then dstPath parameter is ignored.
static bool cull_path(const SkPath& srcPath, const SkStrokeRec& rec,
                      const SkRect* cullRect, SkScalar intervalLength,
                      SkPath* dstPath) {
#ifdef SK_SUPPORT_LEGACY_DASH_CULL_PATH
    if (nullptr == cullRect) {
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

    if (dx < 0) {
        SkTSwap(minX, maxX);
    }

    SkASSERT(minX <= maxX);
    if (maxX < bounds.fLeft || minX > bounds.fRight) {
        return false;
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

    // If line is zero-length, bump out the end by a tiny amount
    // to draw endcaps. The bump factor is sized so that
    // SkPoint::Distance() computes a non-zero length.
    if (minX == maxX) {
        pts[1].fX += maxX * FLT_EPSILON * 32;  // 16 instead of 32 does not draw; length stays zero
    }
#else // !SK_SUPPORT_LEGACY_DASH_CULL_PATH
    SkPoint pts[4];
    if (nullptr == cullRect) {
        if (srcPath.isLine(pts) && pts[0] == pts[1]) {
            adjust_zero_length_line(pts);
        } else {
            return false;
        }
    } else {
        SkRect bounds;
        bool isLine = srcPath.isLine(pts);
        bool isRect = !isLine && srcPath.isRect(nullptr);
        if (!isLine && !isRect) {
            return false;
        }
        bounds = *cullRect;
        outset_for_stroke(&bounds, rec);
        if (isRect) {
            // break rect into four lines, and call each one separately
            SkPath::Iter iter(srcPath, false);
            SkAssertResult(SkPath::kMove_Verb == iter.next(pts));
            SkScalar priorLength = 0;
            while (SkPath::kLine_Verb == iter.next(pts)) {
                SkVector v = pts[1] - pts[0];
                // if line is entirely outside clip rect, skip it
                if (v.fX ? between(bounds.fTop, pts[0].fY, bounds.fBottom) :
                        between(bounds.fLeft, pts[0].fX, bounds.fRight)) {
                    bool skipMoveTo = contains_inclusive(bounds, pts[0]);
                    if (clip_line(pts, bounds, intervalLength,
                                  SkScalarMod(priorLength, intervalLength))) {
                        if (0 == priorLength || !skipMoveTo) {
                            dstPath->moveTo(pts[0]);
                        }
                        dstPath->lineTo(pts[1]);
                    }
                }
                // keep track of all prior lengths to set phase of next line
                priorLength += SkScalarAbs(v.fX ? v.fX : v.fY);
            }
            return !dstPath->isEmpty();
        }
        SkASSERT(isLine);
        if (!clip_line(pts, bounds, intervalLength, 0)) {
            return false;
        }
    }
#endif
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
        SkPointPriv::RotateCCW(fTangent, &fNormal);
        fNormal.scale(SkScalarHalf(rec->getWidth()));

        // now estimate how many quads will be added to the path
        //     resulting segments = pathLen * intervalCount / intervalLen
        //     resulting points = 4 * segments

        SkScalar ptCount = pathLength * intervalCount / (float)intervalLength;
        ptCount = SkTMin(ptCount, SkDashPath::kMaxDashCount);
        int n = SkScalarCeilToInt(ptCount) << 2;
        dst->incReserve(n);

        // we will take care of the stroking
        rec->setFillStyle();
        return true;
    }

    void addSegment(SkScalar d0, SkScalar d1, SkPath* path) const {
        SkASSERT(d0 <= fPathLength);
        // clamp the segment to our length
        if (d1 > fPathLength) {
            d1 = fPathLength;
        }

        SkScalar x0 = fPts[0].fX + fTangent.fX * d0;
        SkScalar x1 = fPts[0].fX + fTangent.fX * d1;
        SkScalar y0 = fPts[0].fY + fTangent.fY * d0;
        SkScalar y1 = fPts[0].fY + fTangent.fY * d1;

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


bool SkDashPath::InternalFilter(SkPath* dst, const SkPath& src, SkStrokeRec* rec,
                                const SkRect* cullRect, const SkScalar aIntervals[],
                                int32_t count, SkScalar initialDashLength, int32_t initialDashIndex,
                                SkScalar intervalLength,
                                StrokeRecApplication strokeRecApplication) {

    // we do nothing if the src wants to be filled
    SkStrokeRec::Style style = rec->getStyle();
    if (SkStrokeRec::kFill_Style == style || SkStrokeRec::kStrokeAndFill_Style == style) {
        return false;
    }

    const SkScalar* intervals = aIntervals;
    SkScalar        dashCount = 0;
    int             segCount = 0;

    SkPath cullPathStorage;
    const SkPath* srcPtr = &src;
    if (cull_path(src, *rec, cullRect, intervalLength, &cullPathStorage)) {
        // if rect is closed, starts in a dash, and ends in a dash, add the initial join
        // potentially a better fix is described here: bug.skia.org/7445
        if (src.isRect(nullptr) && src.isLastContourClosed() && is_even(initialDashIndex)) {
            SkScalar pathLength = SkPathMeasure(src, false, rec->getResScale()).getLength();
            SkScalar endPhase = SkScalarMod(pathLength + initialDashLength, intervalLength);
            int index = 0;
            while (endPhase > intervals[index]) {
                endPhase -= intervals[index++];
                SkASSERT(index <= count);
            }
            // if dash ends inside "on", or ends at beginning of "off"
            if (is_even(index) == (endPhase > 0)) {
                SkPoint midPoint = src.getPoint(0);
                // get vector at end of rect
                int last = src.countPoints() - 1;
                while (midPoint == src.getPoint(last)) {
                    --last;
                    SkASSERT(last >= 0);
                }
                // get vector at start of rect
                int next = 1;
                while (midPoint == src.getPoint(next)) {
                    ++next;
                    SkASSERT(next < last);
                }
                SkVector v = midPoint - src.getPoint(last);
                const SkScalar kTinyOffset = SK_ScalarNearlyZero;
                // scale vector to make start of tiny right angle
                v *= kTinyOffset;
                cullPathStorage.moveTo(midPoint - v);
                cullPathStorage.lineTo(midPoint);
                v = midPoint - src.getPoint(next);
                // scale vector to make end of tiny right angle
                v *= kTinyOffset;
                cullPathStorage.lineTo(midPoint - v);
            }
        }
        srcPtr = &cullPathStorage;
    }

    SpecialLineRec lineRec;
    bool specialLine = (StrokeRecApplication::kAllow == strokeRecApplication) &&
                       lineRec.init(*srcPtr, dst, rec, count >> 1, intervalLength);

    SkPathMeasure   meas(*srcPtr, false, rec->getResScale());

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
            if (is_even(index) && !skipFirstSegment) {
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
            initialDashLength >= 0) {
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
    if (!ValidDashPath(info.fPhase, info.fIntervals, info.fCount)) {
        return false;
    }
    SkScalar initialDashLength = 0;
    int32_t initialDashIndex = 0;
    SkScalar intervalLength = 0;
    CalcDashParameters(info.fPhase, info.fIntervals, info.fCount,
                       &initialDashLength, &initialDashIndex, &intervalLength);
    return InternalFilter(dst, src, rec, cullRect, info.fIntervals, info.fCount, initialDashLength,
                          initialDashIndex, intervalLength);
}

bool SkDashPath::ValidDashPath(SkScalar phase, const SkScalar intervals[], int32_t count) {
    if (count < 2 || !SkIsAlign2(count)) {
        return false;
    }
    SkScalar length = 0;
    for (int i = 0; i < count; i++) {
        if (intervals[i] < 0) {
            return false;
        }
        length += intervals[i];
    }
    // watch out for values that might make us go out of bounds
    return length > 0 && SkScalarIsFinite(phase) && SkScalarIsFinite(length);
}
