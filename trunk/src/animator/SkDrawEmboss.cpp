
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkDrawEmboss.h"

#if SK_USE_CONDENSED_INFO == 0

const SkMemberInfo SkDrawEmboss::fInfo[] = {
    SK_MEMBER(ambient, Float),
    SK_MEMBER_ARRAY(direction, Float),
    SK_MEMBER(radius, Float),
    SK_MEMBER(specular, Float)
};

#endif

DEFINE_GET_MEMBER(SkDrawEmboss);

SkDrawEmboss::SkDrawEmboss() : radius(-1) {
    direction.setCount(3);
}

SkMaskFilter* SkDrawEmboss::getMaskFilter() {
    if (radius < 0 || direction.count() !=3)
        return NULL;
    return SkBlurMaskFilter::CreateEmboss(direction.begin(), ambient, specular, radius);
}

