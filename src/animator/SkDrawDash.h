/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkDrawDash_DEFINED
#define SkDrawDash_DEFINED

#include "SkPaintPart.h"
#include "SkIntArray.h"

class SkDash : public SkDrawPathEffect {
    DECLARE_MEMBER_INFO(Dash);
    SkDash();
    virtual ~SkDash();
    SkPathEffect* getPathEffect() override;
private:
    SkTDScalarArray intervals;
    SkScalar phase;
};

#endif // SkDrawDash_DEFINED
