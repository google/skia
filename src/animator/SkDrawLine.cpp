
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkDrawLine.h"
#include "SkAnimateMaker.h"
#include "SkCanvas.h"
#include "SkPaint.h"

#if SK_USE_CONDENSED_INFO == 0

const SkMemberInfo SkLine::fInfo[] = {
    SK_MEMBER(x1, Float),
    SK_MEMBER(x2, Float),
    SK_MEMBER(y1, Float),
    SK_MEMBER(y2, Float)
};

#endif

DEFINE_GET_MEMBER(SkLine);

SkLine::SkLine() : x1(0), x2(0), y1(0), y2(0) {
}

bool SkLine::draw(SkAnimateMaker& maker) {
    SkBoundableAuto boundable(this, maker);
    maker.fCanvas->drawLine(x1, y1, x2, y2, *maker.fPaint);
    return false;
}
