
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
    void onEndElement(SkAnimateMaker & ) SK_OVERRIDE;
    bool enable(SkAnimateMaker & ) SK_OVERRIDE;
    bool hasEnable() const SK_OVERRIDE;
protected:
    SkString src;
};

#endif // SkDisplayInclude_DEFINED
