/* libs/graphics/animator/SkPostParts.cpp
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

#include "SkPostParts.h"
#include "SkDisplayPost.h"

#if SK_USE_CONDENSED_INFO == 0

const SkMemberInfo SkDataInput::fInfo[] = {
    SK_MEMBER_INHERITED
};

#endif

DEFINE_GET_MEMBER(SkDataInput);

SkDataInput::SkDataInput() : fParent(NULL) {}

bool SkDataInput::add() {
    SkASSERT(name.size() > 0);
    const char* dataName = name.c_str();
    if (fInt != (int) SK_NaN32)
        fParent->fEvent.setS32(dataName, fInt);
    else if (SkScalarIsNaN(fFloat) == false)
        fParent->fEvent.setScalar(dataName, fFloat);
    else if (string.size() > 0) 
        fParent->fEvent.setString(dataName, string);
//  else
//      SkASSERT(0);
    return false;
}

void SkDataInput::dirty() {
    fParent->dirty();
}

SkDisplayable* SkDataInput::getParent() const {
    return fParent;
}

bool SkDataInput::setParent(SkDisplayable* displayable) {
    if (displayable->isPost() == false)
        return true;
    fParent = (SkPost*) displayable;
    return false;
}

void SkDataInput::onEndElement(SkAnimateMaker&) {
    add();
}

