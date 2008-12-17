/* libs/graphics/animator/SkDisplayNumber.cpp
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
