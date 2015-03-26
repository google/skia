
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
    int components() override;
#ifdef SK_DUMP_ENABLED
    void dump(SkAnimateMaker* ) override;
#endif
    void onEndElement(SkAnimateMaker& maker) override;
protected:
    bool resolveCommon(SkAnimateMaker& );
    int fComponents;
private:
    typedef SkAnimateBase INHERITED;
};

#endif // SkAnimateField_DEFINED
