
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkDrawDiscrete.h"
#include "SkAnimateMaker.h"
#include "SkPaint.h"
#include "SkDiscretePathEffect.h"

#if SK_USE_CONDENSED_INFO == 0

const SkMemberInfo SkDiscrete::fInfo[] = {
    SK_MEMBER(deviation, Float),
    SK_MEMBER(segLength, Float)
};

#endif

DEFINE_GET_MEMBER(SkDiscrete);

SkDiscrete::SkDiscrete() : deviation(0), segLength(0) {
}

SkPathEffect* SkDiscrete::getPathEffect() {
    if (deviation <= 0 || segLength <= 0)
        return nullptr;
    else
        return SkDiscretePathEffect::Create(segLength, deviation);
}
