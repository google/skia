/* libs/graphics/animator/SkDrawTextBox.cpp
**
** Copyright 2006, Google Inc.
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

#include "SkDrawTextBox.h"
#include "SkAnimateMaker.h"
#include "SkCanvas.h"
#include "SkPaint.h"

enum SkDrawTextBox_Properties {
    foo = 100,
    SK_PROPERTY(spacingAlign),
    SK_PROPERTY(mode)
};


#if SK_USE_CONDENSED_INFO == 0

const SkMemberInfo SkDrawTextBox::fInfo[] = {
    SK_MEMBER_INHERITED,
    SK_MEMBER(mode, TextBoxMode),
    SK_MEMBER_ALIAS(spacingAdd, fSpacingAdd, Float),
    SK_MEMBER(spacingAlign, TextBoxAlign),
    SK_MEMBER_ALIAS(spacingMul, fSpacingMul, Float),
    SK_MEMBER_ALIAS(text, fText, String)
};

#endif

DEFINE_GET_MEMBER(SkDrawTextBox);

SkDrawTextBox::SkDrawTextBox()
{
    fSpacingMul     = SK_Scalar1;
    fSpacingAdd     = 0;
    spacingAlign    = SkTextBox::kStart_SpacingAlign;
    mode            = SkTextBox::kLineBreak_Mode;
}

#ifdef SK_DUMP_ENABLED
void SkDrawTextBox::dump(SkAnimateMaker* maker)
{
    dumpBase(maker);
    dumpAttrs(maker);
    if (mode == 0) 
        SkDebugf("mode=\"oneLine\" ");
    if (spacingAlign == 1)
        SkDebugf("spacingAlign=\"center\" ");
    else if (spacingAlign == 2)
        SkDebugf("spacingAlign=\"end\" ");
    SkDebugf("/>\n");
}
#endif

bool SkDrawTextBox::getProperty(int index, SkScriptValue* value) const
{
    return this->INHERITED::getProperty(index, value);
}

bool SkDrawTextBox::setProperty(int index, SkScriptValue& scriptValue)
{
    return this->INHERITED::setProperty(index, scriptValue);
}

bool SkDrawTextBox::draw(SkAnimateMaker& maker)
{
    SkTextBox   box;
    box.setMode((SkTextBox::Mode) mode);
    box.setSpacingAlign((SkTextBox::SpacingAlign) spacingAlign);
    box.setBox(fRect);
    box.setSpacing(fSpacingMul, fSpacingAdd);
    SkBoundableAuto boundable(this, maker);
    box.draw(maker.fCanvas, fText.c_str(), fText.size(), *maker.fPaint);
    return false;
}


