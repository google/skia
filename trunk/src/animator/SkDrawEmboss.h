
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkDrawEmboss_DEFINED
#define SkDrawEmboss_DEFINED

#include "SkDrawBlur.h"

class SkDrawEmboss : public SkDrawMaskFilter {
    DECLARE_DRAW_MEMBER_INFO(Emboss);
    SkDrawEmboss();
    virtual SkMaskFilter* getMaskFilter();
protected:
    SkTDScalarArray direction;
    SkScalar radius, ambient, specular;
};

#endif // SkDrawEmboss_DEFINED

