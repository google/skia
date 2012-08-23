
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkDisplayNumber_DEFINED
#define SkDisplayNumber_DEFINED

#include "SkDisplayable.h"
#include "SkMemberInfo.h"

class SkDisplayNumber : public SkDisplayable {
    DECLARE_DISPLAY_MEMBER_INFO(Number);
    virtual bool getProperty(int index, SkScriptValue* value) const;
private:
};

#endif // SkDisplayNumber_DEFINED
