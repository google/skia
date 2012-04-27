
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkDashPathEffect.h"
#include "SkBuffer.h"
#include "SkPathMeasure.h"

static inline int is_even(int x) {
    return (~x) << 31;
}

static SkScalar FindFirstInterval(const SkScalar intervals[], SkScalar phase,
                                  int32_t* index) {
    int i;

    for (i = 0; phase > intervals[i]; i++) {
        phase -= intervals[i];
    }
    *index = i;
    return intervals[i] - phase;
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

        fInitialDashLength = FindFirstInterval(intervals, phase, &fInitialDashIndex);

        SkASSERT(fInitialDashLength >= 0);
        SkASSERT(fInitialDashIndex >= 0 && fInitialDashIndex < fCount);
    } else {
        fInitialDashLength = -1;    // signal bad dash intervals
    }
}

SkDashPathEffect::~SkDashPathEffect() {
    sk_free(fIntervals);
}

bool SkDashPathEffect::filterPath(SkPath* dst, const SkPath& src,
                                  SkScalar* width) {
    // we do nothing if the src wants to be filled, or if our dashlength is 0
    if (*width < 0 || fInitialDashLength < 0) {
        return false;
    }

    SkPathMeasure   meas(src, false);
    const SkScalar* intervals = fIntervals;

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
                meas.getSegment(distance, distance + dlen, dst, true);
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
    buffer.write32(fCount);
    buffer.write32(fInitialDashIndex);
    buffer.writeScalar(fInitialDashLength);
    buffer.writeScalar(fIntervalLength);
    buffer.write32(fScaleToFit);
    buffer.writeMul4(fIntervals, fCount * sizeof(fIntervals[0]));
}

SkFlattenable* SkDashPathEffect::CreateProc(SkFlattenableReadBuffer& buffer) {
    return SkNEW_ARGS(SkDashPathEffect, (buffer));
}

SkDashPathEffect::SkDashPathEffect(SkFlattenableReadBuffer& buffer) {
    fCount = buffer.readS32();
    fInitialDashIndex = buffer.readS32();
    fInitialDashLength = buffer.readScalar();
    fIntervalLength = buffer.readScalar();
    fScaleToFit = (buffer.readS32() != 0);
    
    fIntervals = (SkScalar*)sk_malloc_throw(sizeof(SkScalar) * fCount);
    buffer.read(fIntervals, fCount * sizeof(fIntervals[0]));
}

///////////////////////////////////////////////////////////////////////////////

SK_DEFINE_FLATTENABLE_REGISTRAR(SkDashPathEffect)
