
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkDisplayNumber.h"

enum SkDisplayNumber_Properties {
    SK_PROPERTY(MAX_VALUE),
    SK_PROPERTY(MIN_VALUE),
    SK_PROPERTY(NEGATIVE_INFINITY),
    SK_PROPERTY(NaN),
    SK_PROPERTY(POSITIVE_INFINITY)
};

#if SK_USE_CONDENSED_INFO == 0

const SkMemberInfo SkDisplayNumber::fInfo[] = {
    SK_MEMBER_PROPERTY(MAX_VALUE, Float),
    SK_MEMBER_PROPERTY(MIN_VALUE, Float),
    SK_MEMBER_PROPERTY(NEGATIVE_INFINITY, Float),
    SK_MEMBER_PROPERTY(NaN, Float),
    SK_MEMBER_PROPERTY(POSITIVE_INFINITY, Float)
};

#endif

DEFINE_GET_MEMBER(SkDisplayNumber);

#if defined _WIN32
#pragma warning ( push )
// we are intentionally causing an overflow here
//      (warning C4756: overflow in constant arithmetic)
#pragma warning ( disable : 4756 )
#endif

bool SkDisplayNumber::getProperty(int index, SkScriptValue* value) const {
    SkScalar constant;
    switch (index) {
        case SK_PROPERTY(MAX_VALUE):
            constant = SK_ScalarMax;
        break;
        case SK_PROPERTY(MIN_VALUE):
            constant = SK_ScalarMin;
        break;
        case SK_PROPERTY(NEGATIVE_INFINITY):
            constant = -SK_ScalarInfinity;
        break;
        case SK_PROPERTY(NaN):
            constant = SK_ScalarNaN;
        break;
        case SK_PROPERTY(POSITIVE_INFINITY):
            constant = SK_ScalarInfinity;
        break;
        default:
            SkASSERT(0);
            return false;
    }
    value->fOperand.fScalar = constant;
    value->fType = SkType_Float;
    return true;
}

#if defined _WIN32
#pragma warning ( pop )
#endif
