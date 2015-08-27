
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkDrawClip.h"
#include "SkAnimateMaker.h"
#include "SkCanvas.h"
#include "SkDrawRectangle.h"
#include "SkDrawPath.h"


#if SK_USE_CONDENSED_INFO == 0

const SkMemberInfo SkDrawClip::fInfo[] = {
    SK_MEMBER(path, Path),
    SK_MEMBER(rect, Rect)
};

#endif

DEFINE_GET_MEMBER(SkDrawClip);

SkDrawClip::SkDrawClip() : rect(nullptr), path(nullptr) {
}

bool SkDrawClip::draw(SkAnimateMaker& maker ) {
    if (rect != nullptr)
        maker.fCanvas->clipRect(rect->fRect);
    else {
        SkASSERT(path != nullptr);
        maker.fCanvas->clipPath(path->fPath);
    }
    return false;
}
