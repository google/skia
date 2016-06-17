/*
 * Copyright 2008 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkFixed.h"
#include "SkInterpolator.h"
#include "SkMath.h"
#include "SkTSearch.h"

SkInterpolatorBase::SkInterpolatorBase() {
    fStorage    = nullptr;
    fTimes      = nullptr;
    SkDEBUGCODE(fTimesArray = nullptr;)
}

SkInterpolatorBase::~SkInterpolatorBase() {
    if (fStorage) {
        sk_free(fStorage);
    }
}

void SkInterpolatorBase::reset(int elemCount, int frameCount) {
    fFlags = 0;
    fElemCount = SkToU8(elemCount);
    fFrameCount = SkToS16(frameCount);
    fRepeat = SK_Scalar1;
    if (fStorage) {
        sk_free(fStorage);
        fStorage = nullptr;
        fTimes = nullptr;
        SkDEBUGCODE(fTimesArray = nullptr);
    }
}

/*  Each value[] run is formated as:
        <time (in msec)>
        <blend>
        <data[fElemCount]>

    Totaling fElemCount+2 entries per keyframe
*/

bool SkInterpolatorBase::getDuration(SkMSec* startTime, SkMSec* endTime) const {
    if (fFrameCount == 0) {
        return false;
    }

    if (startTime) {
        *startTime = fTimes[0].fTime;
    }
    if (endTime) {
        *endTime = fTimes[fFrameCount - 1].fTime;
    }
    return true;
}

SkScalar SkInterpolatorBase::ComputeRelativeT(SkMSec time, SkMSec prevTime,
                                  SkMSec nextTime, const SkScalar blend[4]) {
    SkASSERT(time > prevTime && time < nextTime);

    SkScalar t = (SkScalar)(time - prevTime) / (SkScalar)(nextTime - prevTime);
    return blend ?
            SkUnitCubicInterp(t, blend[0], blend[1], blend[2], blend[3]) : t;
}

SkInterpolatorBase::Result SkInterpolatorBase::timeToT(SkMSec time, SkScalar* T,
                                        int* indexPtr, bool* exactPtr) const {
    SkASSERT(fFrameCount > 0);
    Result  result = kNormal_Result;
    if (fRepeat != SK_Scalar1) {
        SkMSec startTime = 0, endTime = 0;  // initialize to avoid warning
        this->getDuration(&startTime, &endTime);
        SkMSec totalTime = endTime - startTime;
        SkMSec offsetTime = time - startTime;
        endTime = SkScalarFloorToInt(fRepeat * totalTime);
        if (offsetTime >= endTime) {
            SkScalar fraction = SkScalarFraction(fRepeat);
            offsetTime = fraction == 0 && fRepeat > 0 ? totalTime :
                (SkMSec) SkScalarFloorToInt(fraction * totalTime);
            result = kFreezeEnd_Result;
        } else {
            int mirror = fFlags & kMirror;
            offsetTime = offsetTime % (totalTime << mirror);
            if (offsetTime > totalTime) { // can only be true if fMirror is true
                offsetTime = (totalTime << 1) - offsetTime;
            }
        }
        time = offsetTime + startTime;
    }

    int index = SkTSearch<SkMSec>(&fTimes[0].fTime, fFrameCount, time,
                                  sizeof(SkTimeCode));

    bool    exact = true;

    if (index < 0) {
        index = ~index;
        if (index == 0) {
            result = kFreezeStart_Result;
        } else if (index == fFrameCount) {
            if (fFlags & kReset) {
                index = 0;
            } else {
                index -= 1;
            }
            result = kFreezeEnd_Result;
        } else {
            exact = false;
        }
    }
    SkASSERT(index < fFrameCount);
    const SkTimeCode* nextTime = &fTimes[index];
    SkMSec   nextT = nextTime[0].fTime;
    if (exact) {
        *T = 0;
    } else {
        SkMSec prevT = nextTime[-1].fTime;
        *T = ComputeRelativeT(time, prevT, nextT, nextTime[-1].fBlend);
    }
    *indexPtr = index;
    *exactPtr = exact;
    return result;
}


SkInterpolator::SkInterpolator() {
    INHERITED::reset(0, 0);
    fValues = nullptr;
    SkDEBUGCODE(fScalarsArray = nullptr;)
}

SkInterpolator::SkInterpolator(int elemCount, int frameCount) {
    SkASSERT(elemCount > 0);
    this->reset(elemCount, frameCount);
}

void SkInterpolator::reset(int elemCount, int frameCount) {
    INHERITED::reset(elemCount, frameCount);
    fStorage = sk_malloc_throw((sizeof(SkScalar) * elemCount +
                                sizeof(SkTimeCode)) * frameCount);
    fTimes = (SkTimeCode*) fStorage;
    fValues = (SkScalar*) ((char*) fStorage + sizeof(SkTimeCode) * frameCount);
#ifdef SK_DEBUG
    fTimesArray = (SkTimeCode(*)[10]) fTimes;
    fScalarsArray = (SkScalar(*)[10]) fValues;
#endif
}

#define SK_Fixed1Third      (SK_Fixed1/3)
#define SK_Fixed2Third      (SK_Fixed1*2/3)

static const SkScalar gIdentityBlend[4] = {
    0.33333333f, 0.33333333f, 0.66666667f, 0.66666667f
};

bool SkInterpolator::setKeyFrame(int index, SkMSec time,
                            const SkScalar values[], const SkScalar blend[4]) {
    SkASSERT(values != nullptr);

    if (blend == nullptr) {
        blend = gIdentityBlend;
    }

    bool success = ~index == SkTSearch<SkMSec>(&fTimes->fTime, index, time,
                                               sizeof(SkTimeCode));
    SkASSERT(success);
    if (success) {
        SkTimeCode* timeCode = &fTimes[index];
        timeCode->fTime = time;
        memcpy(timeCode->fBlend, blend, sizeof(timeCode->fBlend));
        SkScalar* dst = &fValues[fElemCount * index];
        memcpy(dst, values, fElemCount * sizeof(SkScalar));
    }
    return success;
}

SkInterpolator::Result SkInterpolator::timeToValues(SkMSec time,
                                                    SkScalar values[]) const {
    SkScalar T;
    int index;
    bool exact;
    Result result = timeToT(time, &T, &index, &exact);
    if (values) {
        const SkScalar* nextSrc = &fValues[index * fElemCount];

        if (exact) {
            memcpy(values, nextSrc, fElemCount * sizeof(SkScalar));
        } else {
            SkASSERT(index > 0);

            const SkScalar* prevSrc = nextSrc - fElemCount;

            for (int i = fElemCount - 1; i >= 0; --i) {
                values[i] = SkScalarInterp(prevSrc[i], nextSrc[i], T);
            }
        }
    }
    return result;
}

///////////////////////////////////////////////////////////////////////////////

typedef int Dot14;
#define Dot14_ONE       (1 << 14)
#define Dot14_HALF      (1 << 13)

#define Dot14ToFloat(x) ((x) / 16384.f)

static inline Dot14 Dot14Mul(Dot14 a, Dot14 b) {
    return (a * b + Dot14_HALF) >> 14;
}

static inline Dot14 eval_cubic(Dot14 t, Dot14 A, Dot14 B, Dot14 C) {
    return Dot14Mul(Dot14Mul(Dot14Mul(C, t) + B, t) + A, t);
}

static inline Dot14 pin_and_convert(SkScalar x) {
    if (x <= 0) {
        return 0;
    }
    if (x >= SK_Scalar1) {
        return Dot14_ONE;
    }
    return SkScalarToFixed(x) >> 2;
}

SkScalar SkUnitCubicInterp(SkScalar value, SkScalar bx, SkScalar by,
                           SkScalar cx, SkScalar cy) {
    // pin to the unit-square, and convert to 2.14
    Dot14 x = pin_and_convert(value);

    if (x == 0) return 0;
    if (x == Dot14_ONE) return SK_Scalar1;

    Dot14 b = pin_and_convert(bx);
    Dot14 c = pin_and_convert(cx);

    // Now compute our coefficients from the control points
    //  t   -> 3b
    //  t^2 -> 3c - 6b
    //  t^3 -> 3b - 3c + 1
    Dot14 A = 3*b;
    Dot14 B = 3*(c - 2*b);
    Dot14 C = 3*(b - c) + Dot14_ONE;

    // Now search for a t value given x
    Dot14   t = Dot14_HALF;
    Dot14   dt = Dot14_HALF;
    for (int i = 0; i < 13; i++) {
        dt >>= 1;
        Dot14 guess = eval_cubic(t, A, B, C);
        if (x < guess) {
            t -= dt;
        } else {
            t += dt;
        }
    }

    // Now we have t, so compute the coeff for Y and evaluate
    b = pin_and_convert(by);
    c = pin_and_convert(cy);
    A = 3*b;
    B = 3*(c - 2*b);
    C = 3*(b - c) + Dot14_ONE;
    return SkFixedToScalar(eval_cubic(t, A, B, C) << 2);
}
