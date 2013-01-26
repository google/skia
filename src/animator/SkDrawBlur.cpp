
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkDrawBlur.h"

#if SK_USE_CONDENSED_INFO == 0

const SkMemberInfo SkDrawBlur::fInfo[] = {
    SK_MEMBER(blurStyle, MaskFilterBlurStyle),
    SK_MEMBER(radius, Float)
};

#endif

DEFINE_GET_MEMBER(SkDrawBlur);

SkDrawBlur::SkDrawBlur() : radius(-1),
    blurStyle(SkBlurMaskFilter::kNormal_BlurStyle) {
}

SkMaskFilter* SkDrawBlur::getMaskFilter() {
    if (radius < 0)
        return NULL;
    return SkBlurMaskFilter::Create(radius, (SkBlurMaskFilter::BlurStyle) blurStyle);
}
