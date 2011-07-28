
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkDrawFull_DEFINED
#define SkDrawFull_DEFINED

#include "SkBoundable.h"

class SkFull : public SkBoundable {
    DECLARE_EMPTY_MEMBER_INFO(Full);
    virtual bool draw(SkAnimateMaker& );
private:
    typedef SkBoundable INHERITED;
};

#endif // SkDrawFull_DEFINED
