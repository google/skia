
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkPostParts_DEFINED
#define SkPostParts_DEFINED

#include "SkDisplayInput.h"

class SkPost;

class SkDataInput: public SkInput {
    DECLARE_MEMBER_INFO(DataInput);
    SkDataInput();
    bool add();
    void dirty() override;
    SkDisplayable* getParent() const override;
    void onEndElement(SkAnimateMaker& ) override;
    bool setParent(SkDisplayable* ) override;
protected:
    SkPost* fParent;
    typedef SkInput INHERITED;
    friend class SkPost;
};

#endif // SkPostParts_DEFINED
