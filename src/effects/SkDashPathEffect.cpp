/* libs/graphics/effects/SkDashPathEffect.cpp
**
** Copyright 2006, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License"); 
** you may not use this file except in compliance with the License. 
** You may obtain a copy of the License at 
**
**     http://www.apache.org/licenses/LICENSE-2.0 
**
** Unless required by applicable law or agreed to in writing, software 
** distributed under the License is distributed on an "AS IS" BASIS, 
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
** See the License for the specific language governing permissions and 
** limitations under the License.
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

    if (len > 0) {  // we don't handle 0 length dash arrays
        if (phase < 0) {
            phase = -phase;
            if (phase > len) {
                phase = SkScalarMod(phase, len);
            }
            phase = len - phase;
        } else if (phase >= len) {
            phase = SkScalarMod(phase, len);
        }

        // got to watch out for values that might make us go out of bounds
        if (!SkScalarIsFinite(phase) || !SkScalarIsFinite(len)) {
            goto BAD_DASH;
        }

        SkASSERT(phase >= 0 && phase < len);
        fInitialDashLength = FindFirstInterval(intervals, phase, &fInitialDashIndex);

        SkASSERT(fInitialDashLength >= 0);
        SkASSERT(fInitialDashIndex >= 0 && fInitialDashIndex < fCount);
    } else {
        BAD_DASH:
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

void SkDashPathEffect::flatten(SkFlattenableWriteBuffer& buffer) {
    SkASSERT(fInitialDashLength >= 0);

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

static SkFlattenable::Registrar gReg("SkDashPathEffect",
                                     SkDashPathEffect::CreateProc);

