
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkDrawPoint.h"
#include "SkAnimateMaker.h"
#include "SkCanvas.h"
#include "SkPaint.h"

#if SK_USE_CONDENSED_INFO == 0

const SkMemberInfo Sk_Point::fInfo[] = {
    SK_MEMBER_ALIAS(x, fPoint.fX, Float),
    SK_MEMBER_ALIAS(y, fPoint.fY, Float)
};

#endif

DEFINE_NO_VIRTUALS_GET_MEMBER(Sk_Point);

#if SK_USE_CONDENSED_INFO == 0

const SkMemberInfo SkDrawPoint::fInfo[] = {
    SK_MEMBER_ALIAS(x, fPoint.fX, Float),
    SK_MEMBER_ALIAS(y, fPoint.fY, Float)
};

#endif

DEFINE_GET_MEMBER(SkDrawPoint);

SkDrawPoint::SkDrawPoint() {
    fPoint.set(0, 0);
}

void SkDrawPoint::getBounds(SkRect* rect ) {
    rect->fLeft = rect->fRight = fPoint.fX;
    rect->fTop = rect->fBottom = fPoint.fY;
}


