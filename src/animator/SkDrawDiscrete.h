/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkDrawDiscrete_DEFINED
#define SkDrawDiscrete_DEFINED

#include "SkPaintPart.h"

class SkDiscrete : public SkDrawPathEffect {
    DECLARE_MEMBER_INFO(Discrete);
    SkDiscrete();
    SkPathEffect* getPathEffect() override;
private:
    SkScalar deviation;
    SkScalar segLength;
};

#endif //SkDrawDiscrete_DEFINED
