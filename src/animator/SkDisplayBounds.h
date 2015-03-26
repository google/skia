
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkDisplayBounds_DEFINED
#define SkDisplayBounds_DEFINED

#include "SkDrawRectangle.h"

class SkDisplayBounds : public SkDrawRect {
    DECLARE_DISPLAY_MEMBER_INFO(Bounds);
    SkDisplayBounds();
    bool draw(SkAnimateMaker& ) override;
private:
    SkBool inval;
    typedef SkDrawRect INHERITED;
};

#endif // SkDisplayBounds_DEFINED
