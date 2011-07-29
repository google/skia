
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkAnimateSet.h"
#include "SkAnimateMaker.h"
#include "SkAnimateProperties.h"
#include "SkParse.h"

#if SK_USE_CONDENSED_INFO == 0

const SkMemberInfo SkSet::fInfo[] = {
    SK_MEMBER(begin, MSec),
    SK_MEMBER(dur, MSec),
    SK_MEMBER_PROPERTY(dynamic, Boolean),
    SK_MEMBER(field, String),
//  SK_MEMBER(formula, DynamicString),
    SK_MEMBER(lval, DynamicString),
//  SK_MEMBER_PROPERTY(reset, Boolean),
    SK_MEMBER_PROPERTY(step, Int),
    SK_MEMBER(target, DynamicString),
    SK_MEMBER(to, DynamicString)
};

#endif

DEFINE_GET_MEMBER(SkSet);

SkSet::SkSet() {
    dur = 1; 
}

#ifdef SK_DUMP_ENABLED
void SkSet::dump(SkAnimateMaker* maker) {
    INHERITED::dump(maker);
    if (dur != 1) {
#ifdef SK_CAN_USE_FLOAT
        SkDebugf("dur=\"%g\" ", SkScalarToFloat(SkScalarDiv(dur,1000)));
#else
        SkDebugf("dur=\"%x\" ", SkScalarDiv(dur,1000));
#endif
    }
    //don't want double />\n's
    SkDebugf("/>\n");

}
#endif

void SkSet::refresh(SkAnimateMaker& maker) {
    fFieldInfo->setValue(maker, &fValues, 0, fFieldInfo->fCount, NULL, 
        fFieldInfo->getType(), to);
}

void SkSet::onEndElement(SkAnimateMaker& maker) {
    if (resolveCommon(maker) == false)
        return;
    if (fFieldInfo == NULL) {
        maker.setErrorCode(SkDisplayXMLParserError::kFieldNotInTarget);
        return;
    }
    fReset = dur != 1;
    SkDisplayTypes outType = fFieldInfo->getType();
    int comps = outType == SkType_String || outType == SkType_DynamicString ? 1 :
        fFieldInfo->getSize((const SkDisplayable*) fTarget) / sizeof(int);
    if (fValues.getType() == SkType_Unknown) {
        fValues.setType(outType);
        fValues.setCount(comps);
        if (outType == SkType_String || outType == SkType_DynamicString)
            fValues[0].fString = SkNEW(SkString);
        else
            memset(fValues.begin(), 0, fValues.count() * sizeof(fValues.begin()[0]));
    } else {
        SkASSERT(fValues.getType() == outType);
        if (fFieldInfo->fType == SkType_Array)
            comps = fValues.count();
        else {
            SkASSERT(fValues.count() == comps);
        }
    }
    if (formula.size() > 0) {
        comps = 1;
        outType = SkType_MSec;
    }
    fFieldInfo->setValue(maker, &fValues, fFieldOffset, comps, this, outType, formula.size() > 0 ? formula : to);
    fComponents = fValues.count();
}
