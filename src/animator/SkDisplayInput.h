
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkDisplayInput_DEFINED
#define SkDisplayInput_DEFINED

#include "SkDisplayable.h"
#include "SkMemberInfo.h"

class SkInput : public SkDisplayable {
    DECLARE_MEMBER_INFO(Input);
    SkInput();
    SkDisplayable* contains(const SkString& ) override;
    bool getProperty(int index, SkScriptValue* value) const override;
    bool enable(SkAnimateMaker & ) override;
    bool hasEnable() const override;
protected:
    SkString name;
    int32_t  fInt;
    SkScalar fFloat;
    SkString string;
private:
    friend class SkDisplayEvent;
    friend class SkPost;
};

#endif // SkDisplayInput_DEFINED
