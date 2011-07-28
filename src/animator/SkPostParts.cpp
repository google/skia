
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
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

