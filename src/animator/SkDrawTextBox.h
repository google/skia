
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkDrawTextBox_DEFINED
#define SkDrawTextBox_DEFINED

#include "SkDrawRectangle.h"
#include "SkTextBox.h"

class SkDrawTextBox : public SkDrawRect {
    DECLARE_DRAW_MEMBER_INFO(TextBox);
    SkDrawTextBox();

    // overrides
    bool draw(SkAnimateMaker& ) override;
#ifdef SK_DUMP_ENABLED
    void dump(SkAnimateMaker* ) override;
#endif
    bool getProperty(int index, SkScriptValue* value) const override;
    bool setProperty(int index, SkScriptValue& ) override;

private:
    SkString fText;
    SkScalar fSpacingMul;
    SkScalar fSpacingAdd;
    int /*SkTextBox::Mode*/  mode;
    int /*SkTextBox::SpacingAlign*/ spacingAlign;

    typedef SkDrawRect INHERITED;
};

#endif // SkDrawTextBox_DEFINED
