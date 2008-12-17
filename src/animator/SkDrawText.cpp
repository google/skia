/* libs/graphics/animator/SkDrawText.cpp
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

#include "SkDrawText.h"
#include "SkAnimateMaker.h"
#include "SkCanvas.h"
#include "SkPaint.h"

enum SkText_Properties {
    SK_PROPERTY(length)
};

#if SK_USE_CONDENSED_INFO == 0

const SkMemberInfo SkText::fInfo[] = {
    SK_MEMBER_PROPERTY(length, Int),
    SK_MEMBER(text, String),
    SK_MEMBER(x, Float),
    SK_MEMBER(y, Float)
};

#endif

DEFINE_GET_MEMBER(SkText);

SkText::SkText() : x(0), y(0) {
}

SkText::~SkText() {
}

bool SkText::draw(SkAnimateMaker& maker) {
    SkBoundableAuto boundable(this, maker);
    maker.fCanvas->drawText(text.c_str(), text.size(), x, y, *maker.fPaint);
    return false;
}

#ifdef SK_DUMP_ENABLED
void SkText::dump(SkAnimateMaker* maker) {
    INHERITED::dump(maker);
}
#endif

bool SkText::getProperty(int index, SkScriptValue* value) const {
    SkASSERT(index == SK_PROPERTY(length));
    value->fType = SkType_Int;
    value->fOperand.fS32 = (int32_t) text.size();
    return true;
}

