
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkDisplayInclude_DEFINED
#define SkDisplayInclude_DEFINED

#include "SkDisplayable.h"
#include "SkMemberInfo.h"

class SkInclude : public SkDisplayable {
    DECLARE_MEMBER_INFO(Include);
    void onEndElement(SkAnimateMaker & ) override;
    bool enable(SkAnimateMaker & ) override;
    bool hasEnable() const override;
protected:
    SkString src;
};

#endif // SkDisplayInclude_DEFINED
