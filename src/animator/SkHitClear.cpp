
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkHitClear.h"

#if SK_USE_CONDENSED_INFO == 0

const SkMemberInfo SkHitClear::fInfo[] = {
    SK_MEMBER_ARRAY(targets, Displayable)
};

#endif

DEFINE_GET_MEMBER(SkHitClear);

bool SkHitClear::enable(SkAnimateMaker&) {
    for (int tIndex = 0; tIndex < targets.count(); tIndex++) {
        SkDisplayable* target = targets[tIndex];
        target->clearBounder();
    }
    return true;
}

bool SkHitClear::hasEnable() const {
    return true;
}
