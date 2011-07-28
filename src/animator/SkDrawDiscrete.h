
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkDrawDiscrete_DEFINED
#define SkDrawDiscrete_DEFINED

#include "SkPaintParts.h"

class SkDiscrete : public SkDrawPathEffect {
    DECLARE_MEMBER_INFO(Discrete);
    SkDiscrete();
    virtual SkPathEffect* getPathEffect();
private:
    SkScalar deviation;
    SkScalar segLength;
};

#endif //SkDrawDiscrete_DEFINED
