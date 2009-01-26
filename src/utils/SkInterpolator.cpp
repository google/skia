/*
 * Copyright (C) 2006-2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "SkInterpolator.h"
#include "SkMath.h"
#include "SkTSearch.h"

SkInterpolatorBase::SkInterpolatorBase() {
    fStorage    = NULL;
    fTimes      = NULL;
    SkDEBUGCODE(fTimesArray = NULL;)
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
        fStorage = NULL;
        fTimes = NULL;
        SkDEBUGCODE(fTimesArray = NULL);
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

    SkScalar t = SkScalarDiv((SkScalar)(time - prevTime),
                             (SkScalar)(nextTime - prevTime));
    return blend ?
            SkUnitCubicInterp(t, blend[0], blend[1], blend[2], blend[3]) : t;
}

SkInterpolatorBase::Result SkInterpolatorBase::timeToT(SkMSec time, SkScalar* T,
                                        int* indexPtr, SkBool* exactPtr) const {
    SkASSERT(fFrameCount > 0);
    Result  result = kNormal_Result;
    if (fRepeat != SK_Scalar1) {
        SkMSec startTime = 0, endTime = 0;  // initialize to avoid warning
        this->getDuration(&startTime, &endTime);
        SkMSec totalTime = endTime - startTime;
        SkMSec offsetTime = time - startTime;
        endTime = SkScalarMulFloor(fRepeat, totalTime);
        if (offsetTime >= endTime) {
            SkScalar fraction = SkScalarFraction(fRepeat);
            offsetTime = fraction == 0 && fRepeat > 0 ? totalTime :
                SkScalarMulFloor(fraction, totalTime);
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
    fValues = NULL;
    SkDEBUGCODE(fScalarsArray = NULL;)
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
#ifdef SK_SCALAR_IS_FLOAT
    0.33333333f, 0.33333333f, 0.66666667f, 0.66666667f
#else
    SK_Fixed1Third, SK_Fixed1Third, SK_Fixed2Third, SK_Fixed2Third
#endif
};

bool SkInterpolator::setKeyFrame(int index, SkMSec time,
                            const SkScalar values[], const SkScalar blend[4]) {
    SkASSERT(values != NULL);
    
    if (blend == NULL) {
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
    SkBool exact;
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

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#ifdef SK_DEBUG

#ifdef SK_SUPPORT_UNITTEST
    static SkScalar* iset(SkScalar array[3], int a, int b, int c) {
        array[0] = SkIntToScalar(a);
        array[1] = SkIntToScalar(b);
        array[2] = SkIntToScalar(c);
        return array;
    }
#endif

void SkInterpolator::UnitTest() {
#ifdef SK_SUPPORT_UNITTEST
    SkInterpolator  inter(3, 2);
    SkScalar        v1[3], v2[3], v[3], vv[3];
    Result          result;

    inter.setKeyFrame(0, 100, iset(v1, 10, 20, 30), 0);
    inter.setKeyFrame(1, 200, iset(v2, 110, 220, 330));

    result = inter.timeToValues(0, v);
    SkASSERT(result == kFreezeStart_Result);
    SkASSERT(memcmp(v, v1, sizeof(v)) == 0);

    result = inter.timeToValues(99, v);
    SkASSERT(result == kFreezeStart_Result);
    SkASSERT(memcmp(v, v1, sizeof(v)) == 0);

    result = inter.timeToValues(100, v);
    SkASSERT(result == kNormal_Result);
    SkASSERT(memcmp(v, v1, sizeof(v)) == 0);

    result = inter.timeToValues(200, v);
    SkASSERT(result == kNormal_Result);
    SkASSERT(memcmp(v, v2, sizeof(v)) == 0);

    result = inter.timeToValues(201, v);
    SkASSERT(result == kFreezeEnd_Result);
    SkASSERT(memcmp(v, v2, sizeof(v)) == 0);

    result = inter.timeToValues(150, v);
    SkASSERT(result == kNormal_Result);
    SkASSERT(memcmp(v, iset(vv, 60, 120, 180), sizeof(v)) == 0);

    result = inter.timeToValues(125, v);
    SkASSERT(result == kNormal_Result);
    result = inter.timeToValues(175, v);
    SkASSERT(result == kNormal_Result);
#endif
}

#endif

