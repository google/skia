/* libs/graphics/animator/SkDisplayInput.cpp
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
