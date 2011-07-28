
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkDisplayScreenplay_DEFINED
#define SkDisplayScreenplay_DEFINED

#include "SkDisplayable.h"
#include "SkMemberInfo.h"

class SkDisplayScreenplay : public SkDisplayable {
    DECLARE_DISPLAY_MEMBER_INFO(Screenplay);
    SkMSec time;
};

#endif // SkDisplayScreenplay_DEFINED
