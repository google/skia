/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/utils/SkDashPathPriv.h"

#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkPathBuilder.h"
#include "include/core/SkPathMeasure.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSpan.h"
#include "include/core/SkStrokeRec.h"
#include "include/core/SkTypes.h"
#include "include/private/base/SkAlign.h"
#include "include/private/base/SkFloatingPoint.h"
#include "include/private/base/SkTo.h"
#include "src/core/SkPathEffectBase.h"
#include "src/core/SkPointPriv.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>

static inline int is_even(int x) {
    return !(x & 1);
}

static SkScalar find_first_interval(SkSpan<const SkScalar> intervals, SkScalar phase,
                                    size_t* index) {
    for (size_t i = 0; i < intervals.size(); ++i) {
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

void SkDashPath::CalcDashParameters(SkScalar phase, SkSpan<const SkScalar> intervals,
                                    SkScalar* initialDashLength, size_t* initialDashIndex,
                                    SkScalar* intervalLength, SkScalar* adjustedPhase) {
    SkScalar len = 0;
    for (SkScalar interval : intervals) {
        len += interval;
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

    *initialDashLength = find_first_interval(intervals, phase, initialDashIndex);

    SkASSERT(*initialDashLength >= 0);
    SkASSERT(*initialDashIndex < intervals.size());
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

// If line is zero-length, bump out the end by a tiny amount
// to draw endcaps. The bump factor is sized so that
// SkPoint::Distance() computes a non-zero length.
// Offsets SK_ScalarNearlyZero or smaller create empty paths when Iter measures length.
// Large values are scaled by SK_ScalarNearlyZero so significant bits change.
static void adjust_zero_length_line(SkPoint pts[2]) {
    SkASSERT(pts[0] == pts[1]);
    pts[1].fX += std::max(1.001f, pts[1].fX) * SK_ScalarNearlyZero;
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
        using std::swap;
        swap(minXY, maxXY);
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
        using std::swap;
        swap(minXY, maxXY);
    }
    (&pts[0].fX)[xyOffset] = minXY;
    (&pts[1].fX)[xyOffset] = maxXY;

    if (minXY == maxXY) {
        adjust_zero_length_line(pts);
    }
    return true;
}

// Handles only lines and rects.
// If cull_path() returns true, builder is the new smaller path,
// otherwise builder may have been changed but you should ignore it.
static bool cull_path(const SkPath& srcPath, const SkStrokeRec& rec,
                      const SkRect* cullRect, SkScalar intervalLength, SkPathBuilder* builder) {
    if (!cullRect) {
        SkPoint pts[2];
        if (srcPath.isLine(pts) && pts[0] == pts[1]) {
            adjust_zero_length_line(pts);
            builder->moveTo(pts[0]);
            builder->lineTo(pts[1]);
            return true;
        }
        return false;
    }

    SkRect bounds;
    bounds = *cullRect;
    outset_for_stroke(&bounds, rec);

    {
        SkPoint pts[2];
        if (srcPath.isLine(pts)) {
            if (clip_line(pts, bounds, intervalLength, 0)) {
                builder->moveTo(pts[0]);
                builder->lineTo(pts[1]);
                return true;
            }
            return false;
        }
    }

    if (srcPath.isRect(nullptr)) {
        // We'll break the rect into four lines, culling each separately.
        SkPath::Iter iter(srcPath, false);

        std::optional<SkPath::IterRec> it = iter.next();
        SkASSERT(it.has_value() && it->fVerb == SkPathVerb::kMove);

        double accum = 0;  // Sum of unculled edge lengths to keep the phase correct.
                           // Intentionally a double to minimize the risk of overflow and drift.
        while ((it = iter.next()) && (it->fVerb == SkPathVerb::kLine)) {
            // Notice this vector v and accum work with the original unclipped length.
            SkVector v = it->fPoints[1] - it->fPoints[0];

            SkPoint pts[2] = {it->fPoints[0], it->fPoints[1]};
            if (clip_line(pts, bounds, intervalLength, std::fmod(accum, intervalLength))) {
                // pts[0] may have just been changed by clip_line().
                // If that's not where we ended the previous lineTo(), we need to moveTo() there.
                auto maybeLast = builder->getLastPt();
                if (!maybeLast || *maybeLast != pts[0]) {
                    builder->moveTo(pts[0]);
                }
                builder->lineTo(pts[1]);
            }

            // We either just traveled v.fX horizontally or v.fY vertically.
            SkASSERT(v.fX == 0 || v.fY == 0);
            accum += SkScalarAbs(v.fX + v.fY);
        }
        return !builder->isEmpty();
    }

    return false;
}

class SpecialLineRec {
public:
    bool init(const SkPath& src, SkPathBuilder* dst, SkStrokeRec* rec,
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
        fTangent.scale(sk_ieee_float_divide(1.0f, pathLength));
        if (!SkIsFinite(fTangent.fX, fTangent.fY)) {
            return false;
        }
        SkPointPriv::RotateCCW(fTangent, &fNormal);
        fNormal.scale(SkScalarHalf(rec->getWidth()));

        // now estimate how many quads will be added to the path
        //     resulting segments = pathLen * intervalCount / intervalLen
        //     resulting points = 4 * segments

        SkScalar ptCount = pathLength * intervalCount / (float)intervalLength;
        ptCount = std::min(ptCount, SkDashPath::kMaxDashCount);
        if (SkIsNaN(ptCount)) {
            return false;
        }
        int n = SkScalarCeilToInt(ptCount) << 2;
        dst->incReserve(n);

        // we will take care of the stroking
        rec->setFillStyle();
        return true;
    }

    void addSegment(SkScalar d0, SkScalar d1, SkPathBuilder* path) const {
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

        path->addPolygon(pts, false);
    }

private:
    SkPoint fPts[2];
    SkVector fTangent;
    SkVector fNormal;
    SkScalar fPathLength;
};


bool SkDashPath::InternalFilter(SkPathBuilder* dst, const SkPath& src, SkStrokeRec* rec,
                                const SkRect* cullRect, SkSpan<const SkScalar> aIntervals,
                                SkScalar initialDashLength, int32_t initialDashIndex,
                                SkScalar intervalLength, SkScalar startPhase,
                                StrokeRecApplication strokeRecApplication) {
    const size_t count = aIntervals.size();
    // we must always have an even number of intervals
    SkASSERT(is_even(count));

    // we do nothing if the src wants to be filled
    SkStrokeRec::Style style = rec->getStyle();
    if (SkStrokeRec::kFill_Style == style || SkStrokeRec::kStrokeAndFill_Style == style) {
        return false;
    }

    const SkScalar* intervals = aIntervals.data();
    SkScalar        dashCount = 0;

    SkPathBuilder builder;
    SkPath cullPathStorage;
    const SkPath* srcPtr = &src;
    if (cull_path(src, *rec, cullRect, intervalLength, &builder)) {
        // if rect is closed, starts in a dash, and ends in a dash, add the initial join
        // potentially a better fix is described here: skbug.com/40038693
        if (src.isRect(nullptr) && src.isLastContourClosed() && is_even(initialDashIndex)) {
            SkScalar pathLength = SkPathMeasure(src, false, rec->getResScale()).getLength();
            SkScalar endPhase = SkScalarMod(pathLength + startPhase, intervalLength);
            size_t index = 0;
            while (endPhase > intervals[index]) {
                endPhase -= intervals[index++];
                SkASSERT(index <= count);
                if (index == count) {
                    // We have run out of intervals. endPhase "should" never get to this point,
                    // but it could if the subtracts underflowed. Hence we will pin it as if it
                    // perfectly ran through the intervals.
                    // See crbug.com/875494 (and skbug.com/40039544)
                    endPhase = 0;
                    break;
                }
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
                builder.moveTo(midPoint - v);
                builder.lineTo(midPoint);
                v = midPoint - src.getPoint(next);
                // scale vector to make end of tiny right angle
                v *= kTinyOffset;
                builder.lineTo(midPoint - v);
            }
        }

        // If PathMeasure took a SkPathRaw, we could pass it the raw from src or the builder,
        // and not need to first 'detach' a path from the builder.
        cullPathStorage = builder.detach();
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
        size_t      index = initialDashIndex;

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
        }
    } while (meas.nextContour());

    return true;
}

bool SkDashPath::FilterDashPath(SkPathBuilder* dst, const SkPath& src, SkStrokeRec* rec,
                                const SkRect* cullRect, const SkPathEffectBase::DashInfo& info) {
    if (!ValidDashPath(info.fPhase, info.fIntervals)) {
        return false;
    }
    SkScalar initialDashLength = 0;
    size_t initialDashIndex = 0;
    SkScalar intervalLength = 0;
    CalcDashParameters(info.fPhase, info.fIntervals, &initialDashLength,
                       &initialDashIndex, &intervalLength);
    return InternalFilter(dst, src, rec, cullRect, info.fIntervals, initialDashLength,
                          initialDashIndex, intervalLength, info.fPhase);
}

bool SkDashPath::ValidDashPath(SkScalar phase, SkSpan<const SkScalar> intervals) {
    if (intervals.size() < 2 || !SkIsAlign2(intervals.size())) {
        return false;
    }
    SkScalar length = 0;
    for (SkScalar interval : intervals) {
        if (interval < 0) {
            return false;
        }
        length += interval;
    }
    // watch out for values that might make us go out of bounds
    return length > 0 && SkIsFinite(phase, length);
}
