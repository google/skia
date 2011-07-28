
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
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


