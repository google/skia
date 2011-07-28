
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkOperandInterpolator.h"
#include "SkScript.h"

SkOperandInterpolator::SkOperandInterpolator() {
    INHERITED::reset(0, 0);
    fType = SkType_Unknown;
}

SkOperandInterpolator::SkOperandInterpolator(int elemCount, int frameCount, 
                                             SkDisplayTypes type)
{
    this->reset(elemCount, frameCount, type);
}

void SkOperandInterpolator::reset(int elemCount, int frameCount, SkDisplayTypes type)
{
//  SkASSERT(type == SkType_String || type == SkType_Float || type == SkType_Int ||
//      type == SkType_Displayable || type == SkType_Drawable);
    INHERITED::reset(elemCount, frameCount);
    fType = type;
    fStorage = sk_malloc_throw((sizeof(SkOperand) * elemCount + sizeof(SkTimeCode)) * frameCount);
    fTimes = (SkTimeCode*) fStorage;
    fValues = (SkOperand*) ((char*) fStorage + sizeof(SkTimeCode) * frameCount);
#ifdef SK_DEBUG
    fTimesArray = (SkTimeCode(*)[10]) fTimes;
    fValuesArray = (SkOperand(*)[10]) fValues;
#endif
}

bool SkOperandInterpolator::setKeyFrame(int index, SkMSec time, const SkOperand values[], SkScalar blend)
{
    SkASSERT(values != NULL);
    blend = SkScalarPin(blend, 0, SK_Scalar1);

    bool success = ~index == SkTSearch<SkMSec>(&fTimes->fTime, index, time, sizeof(SkTimeCode));
    SkASSERT(success);
    if (success) {
        SkTimeCode* timeCode = &fTimes[index];
        timeCode->fTime = time;
        timeCode->fBlend[0] = SK_Scalar1 - blend;
        timeCode->fBlend[1] = 0;
        timeCode->fBlend[2] = 0;
        timeCode->fBlend[3] = SK_Scalar1 - blend;
        SkOperand* dst = &fValues[fElemCount * index];
        memcpy(dst, values, fElemCount * sizeof(SkOperand));
    }
    return success;
}

SkInterpolatorBase::Result SkOperandInterpolator::timeToValues(SkMSec time, SkOperand values[]) const
{
    SkScalar T;
    int index;
    SkBool exact;
    Result result = timeToT(time, &T, &index, &exact);
    if (values)
    {
        const SkOperand* nextSrc = &fValues[index * fElemCount];

        if (exact)
            memcpy(values, nextSrc, fElemCount * sizeof(SkScalar));
        else
        {
            SkASSERT(index > 0);

            const SkOperand* prevSrc = nextSrc - fElemCount;

            if (fType == SkType_Float || fType == SkType_3D_Point) {
                for (int i = fElemCount - 1; i >= 0; --i)
                    values[i].fScalar = SkScalarInterp(prevSrc[i].fScalar, nextSrc[i].fScalar, T);
            } else if (fType == SkType_Int || fType == SkType_MSec) {
                for (int i = fElemCount - 1; i >= 0; --i) {
                    int32_t a = prevSrc[i].fS32;
                    int32_t b = nextSrc[i].fS32;
                    values[i].fS32 = a + SkScalarRound((b - a) * T);
                }
            } else
                memcpy(values, prevSrc, sizeof(SkOperand) * fElemCount);
        }
    }
    return result;
}

///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////

#ifdef SK_DEBUG

#ifdef SK_SUPPORT_UNITTEST
    static SkOperand* iset(SkOperand array[3], int a, int b, int c)
    {
        array[0].fScalar = SkIntToScalar(a);
        array[1].fScalar = SkIntToScalar(b);
        array[2].fScalar = SkIntToScalar(c);
        return array;
    }
#endif

void SkOperandInterpolator::UnitTest()
{
#ifdef SK_SUPPORT_UNITTEST
    SkOperandInterpolator   inter(3, 2, SkType_Float);
    SkOperand       v1[3], v2[3], v[3], vv[3];
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


