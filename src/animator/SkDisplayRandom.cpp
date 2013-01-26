
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkDisplayRandom.h"
#include "SkInterpolator.h"

enum SkDisplayRandom_Properties {
    SK_PROPERTY(random),
    SK_PROPERTY(seed)
};

#if SK_USE_CONDENSED_INFO == 0

const SkMemberInfo SkDisplayRandom::fInfo[] = {
    SK_MEMBER(blend, Float),
    SK_MEMBER(max, Float),
    SK_MEMBER(min, Float),
    SK_MEMBER_DYNAMIC_PROPERTY(random, Float),
    SK_MEMBER_PROPERTY(seed, Int)
};

#endif

DEFINE_GET_MEMBER(SkDisplayRandom);

SkDisplayRandom::SkDisplayRandom() : blend(0), min(0), max(SK_Scalar1) {
}

#ifdef SK_DUMP_ENABLED
void SkDisplayRandom::dump(SkAnimateMaker* maker) {
    dumpBase(maker);
    SkDebugf("min=\"%g\" ", SkScalarToFloat(min));
    SkDebugf("max=\"%g\" ", SkScalarToFloat(max));
    SkDebugf("blend=\"%g\" ", SkScalarToFloat(blend));
    SkDebugf("/>\n");
}
#endif

bool SkDisplayRandom::getProperty(int index, SkScriptValue* value) const {
    switch(index) {
        case SK_PROPERTY(random): {
            SkScalar random = fRandom.nextUScalar1();
            SkScalar relativeT = SkUnitCubicInterp(random, SK_Scalar1 - blend, 0, 0, SK_Scalar1 - blend);
            value->fOperand.fScalar = min + SkScalarMul(max - min, relativeT);
            value->fType = SkType_Float;
            return true;
        }
        default:
            SkASSERT(0);
    }
    return false;
}

bool SkDisplayRandom::setProperty(int index, SkScriptValue& value) {
    SkASSERT(index == SK_PROPERTY(seed));
    SkASSERT(value.fType == SkType_Int);
    fRandom.setSeed(value.fOperand.fS32);
    return true;
}
