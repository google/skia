
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkDrawEmboss.h"

#if SK_USE_CONDENSED_INFO == 0

const SkMemberInfo SkDrawEmboss::fInfo[] = {
    SK_MEMBER(fAmbient, Float),
    SK_MEMBER_ARRAY(fDirection, Float),
    SK_MEMBER(fSigma, Float),
    SK_MEMBER(fSpecular, Float)
};

#endif

DEFINE_GET_MEMBER(SkDrawEmboss);

SkDrawEmboss::SkDrawEmboss() : fSigma(-1) {
    fDirection.setCount(3);
}

SkMaskFilter* SkDrawEmboss::getMaskFilter() {
    if (fSigma < 0 || fDirection.count() !=3)
        return nullptr;
    return SkBlurMaskFilter::MakeEmboss(fSigma, fDirection.begin(),
                                        fAmbient, fSpecular).release();
}
