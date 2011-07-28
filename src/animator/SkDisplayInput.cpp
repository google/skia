
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkDisplayInput.h"

enum SkInput_Properties {
    SK_PROPERTY(initialized)
};

#if SK_USE_CONDENSED_INFO == 0

const SkMemberInfo SkInput::fInfo[] = {
    SK_MEMBER_ALIAS(float, fFloat, Float),
    SK_MEMBER_PROPERTY(initialized, Boolean),
    SK_MEMBER_ALIAS(int, fInt, Int),
    SK_MEMBER(name, String),
    SK_MEMBER(string, String)
};

#endif

DEFINE_GET_MEMBER(SkInput);

SkInput::SkInput() : fInt((int) SK_NaN32), fFloat(SK_ScalarNaN) {}

SkDisplayable* SkInput::contains(const SkString& string) {
    return string.equals(name) ? this : NULL;
}

bool SkInput::enable(SkAnimateMaker & ) {
    return true;
}

bool SkInput::getProperty(int index, SkScriptValue* value) const {
    switch (index) {
        case SK_PROPERTY(initialized):
            value->fType = SkType_Boolean;
            value->fOperand.fS32 = fInt != (int) SK_NaN32 ||
                SkScalarIsNaN(fFloat) == false || string.size() > 0;
            break;
        default:
            return false;
    }
    return true;
}
 
bool SkInput::hasEnable() const {
    return true;
}
