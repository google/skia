
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkTextOnPath_DEFINED
#define SkTextOnPath_DEFINED

#include "SkBoundable.h"
#include "SkMemberInfo.h"

class SkDrawPath;
class SkText;

class SkTextOnPath : public SkBoundable {
    DECLARE_MEMBER_INFO(TextOnPath);
    SkTextOnPath();
    bool draw(SkAnimateMaker& ) override;
private:
    SkScalar offset;
    SkDrawPath* path;
    SkText* text;
    typedef SkBoundable INHERITED;
};

#endif // SkTextOnPath_DEFINED
