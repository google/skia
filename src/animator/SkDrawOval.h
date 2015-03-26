
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkDrawOval_DEFINED
#define SkDrawOval_DEFINED

#include "SkDrawRectangle.h"

class SkOval : public SkDrawRect {
    DECLARE_MEMBER_INFO(Oval);
    bool draw(SkAnimateMaker& ) override;
private:
    typedef SkDrawRect INHERITED;
};

#endif // SkDrawOval_DEFINED
