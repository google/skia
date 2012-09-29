
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkDisplayScreenplay.h"

#if SK_USE_CONDENSED_INFO == 0

const SkMemberInfo SkDisplayScreenplay::fInfo[] = {
    SK_MEMBER(time, MSec)
};

#endif

DEFINE_GET_MEMBER(SkDisplayScreenplay);


