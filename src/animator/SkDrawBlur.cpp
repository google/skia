
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkDrawBlur.h"

#if SK_USE_CONDENSED_INFO == 0

const SkMemberInfo SkDrawBlur::fInfo[] = {
    SK_MEMBER(fBlurStyle, MaskFilterBlurStyle),
    SK_MEMBER(fSigma, Float)
};

#endif

DEFINE_GET_MEMBER(SkDrawBlur);

SkDrawBlur::SkDrawBlur()
    : fSigma(-1)
    , fBlurStyle(kNormal_SkBlurStyle) {
}

SkMaskFilter* SkDrawBlur::getMaskFilter() {
    if (fSigma <= 0) {
        return nullptr;
    }
    return SkBlurMaskFilter::Make((SkBlurStyle)fBlurStyle, fSigma).release();
}
