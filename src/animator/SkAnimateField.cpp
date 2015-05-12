
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkAnimate.h"
#include "SkAnimateMaker.h"
#include "SkADrawable.h"
#include "SkParse.h"

#if SK_USE_CONDENSED_INFO == 0

const SkMemberInfo SkAnimate::fInfo[] = {
    SK_MEMBER_INHERITED
};

#endif

DEFINE_GET_MEMBER(SkAnimate);

SkAnimate::SkAnimate() : fComponents(0) {
}

SkAnimate::~SkAnimate() {
}

int SkAnimate::components() {
    return fComponents;
}

#ifdef SK_DUMP_ENABLED
void SkAnimate::dump(SkAnimateMaker* maker) {
    INHERITED::dump(maker); //from animateBase
    //SkSet inherits from this class
    if (getType() != SkType_Set) {
        if (fMirror)
            SkDebugf("mirror=\"true\" ");
        if (fReset)
            SkDebugf("reset=\"true\" ");
        SkDebugf("dur=\"%g\" ", dur * 0.001);
        if (repeat != SK_Scalar1)
            SkDebugf("repeat=\"%g\" ", SkScalarToFloat(repeat));
        //if (fHasValues)
        //    SkDebugf("values=\"%s\" ", values);
        if (blend.count() != 1 || blend[0] != SK_Scalar1) {
            SkDebugf("blend=\"[");
            bool firstElem = true;
            for (int i = 0; i < blend.count(); i++) {
                if (!firstElem)
                    SkDebugf(",");
                firstElem = false;
                SkDebugf("%g", SkScalarToFloat(blend[i]));
            }
            SkDebugf("]\" ");
        }
        SkDebugf("/>\n");//i assume that if it IS, we will do it separately
    }
}
#endif

bool SkAnimate::resolveCommon(SkAnimateMaker& maker) {
    if (fTarget == NULL) // if NULL, recall onEndElement after apply closes and sets target to scope
        return false;
    INHERITED::onEndElement(maker);
    return maker.hasError() == false;
}

void SkAnimate::onEndElement(SkAnimateMaker& maker) {
    bool resolved = resolveCommon(maker);
    if (resolved && fFieldInfo == NULL) {
        maker.setErrorNoun(field);
        maker.setErrorCode(SkDisplayXMLParserError::kFieldNotInTarget);
    }
    if (resolved == false || fFieldInfo == NULL)
        return;
    SkDisplayTypes outType = fFieldInfo->getType();
    if (fHasValues) {
        SkASSERT(to.size() > 0);
        fFieldInfo->setValue(maker, &fValues, 0, 0, NULL, outType, to);
        SkASSERT(0);
        // !!! this needs to set fComponents
        return;
    }
    fComponents = fFieldInfo->getCount();
    if (fFieldInfo->fType == SkType_Array) {
        SkTypedArray* array = (SkTypedArray*) fFieldInfo->memberData(fTarget);
        int count = array->count();
        if (count > 0)
            fComponents = count;
    }
    if (outType == SkType_ARGB) {
        fComponents <<= 2;  // four color components
        outType = SkType_Float;
    }
    fValues.setType(outType);
    if (formula.size() > 0){
        fComponents = 1;
        from.set("0");
        to.set("dur");
        outType = SkType_MSec;
    }
    int max = fComponents * 2;
    fValues.setCount(max);
    memset(fValues.begin(), 0, max * sizeof(fValues.begin()[0]));
    fFieldInfo->setValue(maker, &fValues, fFieldOffset, max, this, outType, from);
    fFieldInfo->setValue(maker, &fValues, fComponents + fFieldOffset, max, this, outType, to);
}
