
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkDashPathEffect.h"
#include "SkFlattenableBuffers.h"
#include "SkPathMeasure.h"

static inline int is_even(int x) {
    return (~x) << 31;
}

static SkScalar FindFirstInterval(const SkScalar intervals[], SkScalar phase,
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

SkDashPathEffect::SkDashPathEffect(const SkScalar intervals[], int count,
                                   SkScalar phase, bool scaleToFit)
        : fScaleToFit(scaleToFit) {
    SkASSERT(intervals);
    SkASSERT(count > 1 && SkAlign2(count) == count);

    fIntervals = (SkScalar*)sk_malloc_throw(sizeof(SkScalar) * count);
    fCount = count;

    SkScalar len = 0;
    for (int i = 0; i < count; i++) {
        SkASSERT(intervals[i] >= 0);
        fIntervals[i] = intervals[i];
        len += intervals[i];
    }
    fIntervalLength = len;

    // watch out for values that might make us go out of bounds
    if ((len > 0) && SkScalarIsFinite(phase) && SkScalarIsFinite(len)) {

        // Adjust phase to be between 0 and len, "flipping" phase if negative.
        // e.g., if len is 100, then phase of -20 (or -120) is equivalent to 80
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
        SkASSERT(phase >= 0 && phase < len);

        fInitialDashLength = FindFirstInterval(intervals, phase,
                                               &fInitialDashIndex, count);

        SkASSERT(fInitialDashLength >= 0);
        SkASSERT(fInitialDashIndex >= 0 && fInitialDashIndex < fCount);
    } else {
        fInitialDashLength = -1;    // signal bad dash intervals
    }
}

SkDashPathEffect::~SkDashPathEffect() {
    sk_free(fIntervals);
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

bool SkDashPathEffect::filterPath(SkPath* dst, const SkPath& src,
                              SkStrokeRec* rec, const SkRect* cullRect) const {
    // we do nothing if the src wants to be filled, or if our dashlength is 0
    if (rec->isFillStyle() || fInitialDashLength < 0) {
        return false;
    }

    const SkScalar* intervals = fIntervals;
    SkScalar        dashCount = 0;

    SkPath cullPathStorage;
    const SkPath* srcPtr = &src;
    if (cull_path(src, *rec, cullRect, fIntervalLength, &cullPathStorage)) {
        srcPtr = &cullPathStorage;
    }

    SpecialLineRec lineRec;
    bool specialLine = lineRec.init(*srcPtr, dst, rec, fCount >> 1, fIntervalLength);

    SkPathMeasure   meas(*srcPtr, false);

    do {
        bool        skipFirstSegment = meas.isClosed();
        bool        addedSegment = false;
        SkScalar    length = meas.getLength();
        int         index = fInitialDashIndex;
        SkScalar    scale = SK_Scalar1;

        // Since the path length / dash length ratio may be arbitrarily large, we can exert
        // significant memory pressure while attempting to build the filtered path. To avoid this,
        // we simply give up dashing beyond a certain threshold.
        //
        // The original bug report (http://crbug.com/165432) is based on a path yielding more than
        // 90 million dash segments and crashing the memory allocator. A limit of 1 million
        // segments seems reasonable: at 2 verbs per segment * 9 bytes per verb, this caps the
        // maximum dash memory overhead at roughly 17MB per path.
        static const SkScalar kMaxDashCount = 1000000;
        dashCount += length * (fCount >> 1) / fIntervalLength;
        if (dashCount > kMaxDashCount) {
            dst->reset();
            return false;
        }

        if (fScaleToFit) {
            if (fIntervalLength >= length) {
                scale = SkScalarDiv(length, fIntervalLength);
            } else {
                SkScalar div = SkScalarDiv(length, fIntervalLength);
                int n = SkScalarFloor(div);
                scale = SkScalarDiv(length, n * fIntervalLength);
            }
        }

        // Using double precision to avoid looping indefinitely due to single precision rounding
        // (for extreme path_length/dash_length ratios). See test_infinite_dash() unittest.
        double  distance = 0;
        double  dlen = SkScalarMul(fInitialDashLength, scale);

        while (distance < length) {
            SkASSERT(dlen >= 0);
            addedSegment = false;
            if (is_even(index) && dlen > 0 && !skipFirstSegment) {
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
            SkASSERT(index <= fCount);
            if (index == fCount) {
                index = 0;
            }

            // fetch our next dlen
            dlen = SkScalarMul(intervals[index], scale);
        }

        // extend if we ended on a segment and we need to join up with the (skipped) initial segment
        if (meas.isClosed() && is_even(fInitialDashIndex) &&
                fInitialDashLength > 0) {
            meas.getSegment(0, SkScalarMul(fInitialDashLength, scale), dst, !addedSegment);
        }
    } while (meas.nextContour());

    return true;
}

// Currently asPoints is more restrictive then it needs to be. In the future
// we need to:
//      allow kRound_Cap capping (could allow rotations in the matrix with this)
//      allow paths to be returned
bool SkDashPathEffect::asPoints(PointData* results,
                                const SkPath& src,
                                const SkStrokeRec& rec,
                                const SkMatrix& matrix,
                                const SkRect* cullRect) const {
    // width < 0 -> fill && width == 0 -> hairline so requiring width > 0 rules both out
    if (fInitialDashLength < 0 || 0 >= rec.getWidth()) {
        return false;
    }

    // TODO: this next test could be eased up. We could allow any number of
    // intervals as long as all the ons match and all the offs match.
    // Additionally, they do not necessarily need to be integers.
    // We cannot allow arbitrary intervals since we want the returned points
    // to be uniformly sized.
    if (fCount != 2 ||
        !SkScalarNearlyEqual(fIntervals[0], fIntervals[1]) ||
        !SkScalarIsInt(fIntervals[0]) ||
        !SkScalarIsInt(fIntervals[1])) {
        return false;
    }

    // TODO: this next test could be eased up. The rescaling should not impact
    // the equality of the ons & offs. However, we would need to remove the
    // integer intervals restriction first
    if (fScaleToFit) {
        return false;
    }

    SkPoint pts[2];

    if (!src.isLine(pts)) {
        return false;
    }

    // TODO: this test could be eased up to allow circles
    if (SkPaint::kButt_Cap != rec.getCap()) {
        return false;
    }

    // TODO: this test could be eased up for circles. Rotations could be allowed.
    if (!matrix.rectStaysRect()) {
        return false;
    }

    SkScalar        length = SkPoint::Distance(pts[1], pts[0]);

    SkVector tangent = pts[1] - pts[0];
    if (tangent.isZero()) {
        return false;
    }

    tangent.scale(SkScalarInvert(length));

    // TODO: make this test for horizontal & vertical lines more robust
    bool isXAxis = true;
    if (SK_Scalar1 == tangent.fX || -SK_Scalar1 == tangent.fX) {
        results->fSize.set(SkScalarHalf(fIntervals[0]), SkScalarHalf(rec.getWidth()));
    } else if (SK_Scalar1 == tangent.fY || -SK_Scalar1 == tangent.fY) {
        results->fSize.set(SkScalarHalf(rec.getWidth()), SkScalarHalf(fIntervals[0]));
        isXAxis = false;
    } else if (SkPaint::kRound_Cap != rec.getCap()) {
        // Angled lines don't have axis-aligned boxes.
        return false;
    }

    if (NULL != results) {
        results->fFlags = 0;
        SkScalar clampedInitialDashLength = SkMinScalar(length, fInitialDashLength);

        if (SkPaint::kRound_Cap == rec.getCap()) {
            results->fFlags |= PointData::kCircles_PointFlag;
        }

        results->fNumPoints = 0;
        SkScalar len2 = length;
        if (clampedInitialDashLength > 0 || 0 == fInitialDashIndex) {
            SkASSERT(len2 >= clampedInitialDashLength);
            if (0 == fInitialDashIndex) {
                if (clampedInitialDashLength > 0) {
                    if (clampedInitialDashLength >= fIntervals[0]) {
                        ++results->fNumPoints;  // partial first dash
                    }
                    len2 -= clampedInitialDashLength;
                }
                len2 -= fIntervals[1];  // also skip first space
                if (len2 < 0) {
                    len2 = 0;
                }
            } else {
                len2 -= clampedInitialDashLength; // skip initial partial empty
            }
        }
        int numMidPoints = SkScalarFloorToInt(SkScalarDiv(len2, fIntervalLength));
        results->fNumPoints += numMidPoints;
        len2 -= numMidPoints * fIntervalLength;
        bool partialLast = false;
        if (len2 > 0) {
            if (len2 < fIntervals[0]) {
                partialLast = true;
            } else {
                ++numMidPoints;
                ++results->fNumPoints;
            }
        }

        results->fPoints = new SkPoint[results->fNumPoints];

        SkScalar    distance = 0;
        int         curPt = 0;

        if (clampedInitialDashLength > 0 || 0 == fInitialDashIndex) {
            SkASSERT(clampedInitialDashLength <= length);

            if (0 == fInitialDashIndex) {
                if (clampedInitialDashLength > 0) {
                    // partial first block
                    SkASSERT(SkPaint::kRound_Cap != rec.getCap()); // can't handle partial circles
                    SkScalar x = pts[0].fX + SkScalarMul(tangent.fX, SkScalarHalf(clampedInitialDashLength));
                    SkScalar y = pts[0].fY + SkScalarMul(tangent.fY, SkScalarHalf(clampedInitialDashLength));
                    SkScalar halfWidth, halfHeight;
                    if (isXAxis) {
                        halfWidth = SkScalarHalf(clampedInitialDashLength);
                        halfHeight = SkScalarHalf(rec.getWidth());
                    } else {
                        halfWidth = SkScalarHalf(rec.getWidth());
                        halfHeight = SkScalarHalf(clampedInitialDashLength);
                    }
                    if (clampedInitialDashLength < fIntervals[0]) {
                        // This one will not be like the others
                        results->fFirst.addRect(x - halfWidth, y - halfHeight,
                                                x + halfWidth, y + halfHeight);
                    } else {
                        SkASSERT(curPt < results->fNumPoints);
                        results->fPoints[curPt].set(x, y);
                        ++curPt;
                    }

                    distance += clampedInitialDashLength;
                }

                distance += fIntervals[1];  // skip over the next blank block too
            } else {
                distance += clampedInitialDashLength;
            }
        }

        if (0 != numMidPoints) {
            distance += SkScalarHalf(fIntervals[0]);

            for (int i = 0; i < numMidPoints; ++i) {
                SkScalar x = pts[0].fX + SkScalarMul(tangent.fX, distance);
                SkScalar y = pts[0].fY + SkScalarMul(tangent.fY, distance);

                SkASSERT(curPt < results->fNumPoints);
                results->fPoints[curPt].set(x, y);
                ++curPt;

                distance += fIntervalLength;
            }

            distance -= SkScalarHalf(fIntervals[0]);
        }

        if (partialLast) {
            // partial final block
            SkASSERT(SkPaint::kRound_Cap != rec.getCap()); // can't handle partial circles
            SkScalar temp = length - distance;
            SkASSERT(temp < fIntervals[0]);
            SkScalar x = pts[0].fX + SkScalarMul(tangent.fX, distance + SkScalarHalf(temp));
            SkScalar y = pts[0].fY + SkScalarMul(tangent.fY, distance + SkScalarHalf(temp));
            SkScalar halfWidth, halfHeight;
            if (isXAxis) {
                halfWidth = SkScalarHalf(temp);
                halfHeight = SkScalarHalf(rec.getWidth());
            } else {
                halfWidth = SkScalarHalf(rec.getWidth());
                halfHeight = SkScalarHalf(temp);
            }
            results->fLast.addRect(x - halfWidth, y - halfHeight,
                                   x + halfWidth, y + halfHeight);
        }

        SkASSERT(curPt == results->fNumPoints);
    }

    return true;
}

SkFlattenable::Factory SkDashPathEffect::getFactory() {
    return fInitialDashLength < 0 ? NULL : CreateProc;
}

void SkDashPathEffect::flatten(SkFlattenableWriteBuffer& buffer) const {
    SkASSERT(fInitialDashLength >= 0);

    this->INHERITED::flatten(buffer);
    buffer.writeInt(fInitialDashIndex);
    buffer.writeScalar(fInitialDashLength);
    buffer.writeScalar(fIntervalLength);
    buffer.writeBool(fScaleToFit);
    buffer.writeScalarArray(fIntervals, fCount);
}

SkFlattenable* SkDashPathEffect::CreateProc(SkFlattenableReadBuffer& buffer) {
    return SkNEW_ARGS(SkDashPathEffect, (buffer));
}

SkDashPathEffect::SkDashPathEffect(SkFlattenableReadBuffer& buffer) : INHERITED(buffer) {
    fInitialDashIndex = buffer.readInt();
    fInitialDashLength = buffer.readScalar();
    fIntervalLength = buffer.readScalar();
    fScaleToFit = buffer.readBool();

    fCount = buffer.getArrayCount();
    fIntervals = (SkScalar*)sk_malloc_throw(sizeof(SkScalar) * fCount);
    buffer.readScalarArray(fIntervals);
}
