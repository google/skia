
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkAnimate_DEFINED
#define SkAnimate_DEFINED

#include "SkAnimateBase.h"
#include "SkDisplayType.h"
#include "SkIntArray.h"
#include "SkUtils.h"

class SkAnimate : public SkAnimateBase {
    DECLARE_MEMBER_INFO(Animate);
    SkAnimate();
    virtual ~SkAnimate();
    virtual int components();
#ifdef SK_DUMP_ENABLED
    virtual void dump(SkAnimateMaker* );
#endif
    virtual void onEndElement(SkAnimateMaker& maker);
protected:
    bool resolveCommon(SkAnimateMaker& );
    int fComponents;
private:
    typedef SkAnimateBase INHERITED;
};

#endif // SkAnimateField_DEFINED
