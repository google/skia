
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

class SpecialLineRec {
public:
    bool init(const SkPath& src, SkPath* dst, SkStrokeRec* rec,
              SkScalar pathLength,
              int intervalCount, SkScalar intervalLength) {
        if (rec->isHairlineStyle() || !src.isLine(fPts)) {
            return false;
        }

        // can relax this in the future, if we handle square and round caps
        if (SkPaint::kButt_Cap != rec->getCap()) {
            return false;
        }

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
                                  SkStrokeRec* rec) {
    // we do nothing if the src wants to be filled, or if our dashlength is 0
    if (rec->isFillStyle() || fInitialDashLength < 0) {
        return false;
    }

    SkPathMeasure   meas(src, false);
    const SkScalar* intervals = fIntervals;

    SpecialLineRec lineRec;
    const bool specialLine = lineRec.init(src, dst, rec, meas.getLength(),
                                          fCount >> 1, fIntervalLength);

    do {
        bool        skipFirstSegment = meas.isClosed();
        bool        addedSegment = false;
        SkScalar    length = meas.getLength();
        int         index = fInitialDashIndex;
        SkScalar    scale = SK_Scalar1;

        if (fScaleToFit) {
            if (fIntervalLength >= length) {
                scale = SkScalarDiv(length, fIntervalLength);
            } else {
                SkScalar div = SkScalarDiv(length, fIntervalLength);
                int n = SkScalarFloor(div);
                scale = SkScalarDiv(length, n * fIntervalLength);
            }
        }

        SkScalar    distance = 0;
        SkScalar    dlen = SkScalarMul(fInitialDashLength, scale);

        while (distance < length) {
            SkASSERT(dlen >= 0);
            addedSegment = false;
            if (is_even(index) && dlen > 0 && !skipFirstSegment) {
                addedSegment = true;

                if (specialLine) {
                    lineRec.addSegment(distance, distance + dlen, dst);
                } else {
                    meas.getSegment(distance, distance + dlen, dst, true);
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

///////////////////////////////////////////////////////////////////////////////

SK_DEFINE_FLATTENABLE_REGISTRAR(SkDashPathEffect)
