
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkTextToPath_DEFINED
#define SkTextToPath_DEFINED

#include "SkDrawPath.h"
#include "SkMemberInfo.h"

class SkDrawPaint;
class SkDrawPath;
class SkText;

class SkTextToPath : public SkADrawable {
    DECLARE_MEMBER_INFO(TextToPath);
    SkTextToPath();
    bool draw(SkAnimateMaker& ) override;
    void onEndElement(SkAnimateMaker& ) override;
private:
    SkDrawPaint* paint;
    SkDrawPath* path;
    SkText* text;
};

#endif // SkTextToPath_DEFINED
