
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

class SkTextToPath : public SkDrawable {
    DECLARE_MEMBER_INFO(TextToPath);
    SkTextToPath();
    virtual bool draw(SkAnimateMaker& );
    virtual void onEndElement(SkAnimateMaker& );
private:
    SkDrawPaint* paint;
    SkDrawPath* path;
    SkText* text;
};

#endif // SkTextToPath_DEFINED

