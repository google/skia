
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkAnimateSet_DEFINED
#define SkAnimateSet_DEFINED

#include "SkAnimate.h"

class SkSet : public SkAnimate {
    DECLARE_MEMBER_INFO(Set);
    SkSet();
#ifdef SK_DUMP_ENABLED
    virtual void dump(SkAnimateMaker* );
#endif
    virtual void onEndElement(SkAnimateMaker& );
    virtual void refresh(SkAnimateMaker& );
private:
    typedef SkAnimate INHERITED;
};

#endif // SkAnimateSet_DEFINED
