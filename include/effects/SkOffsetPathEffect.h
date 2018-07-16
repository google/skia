/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkOffsetPathEffect_DEFINED
#define SkOffsetPathEffect_DEFINED

#include "SkPathEffect.h"
#include "SkPaint.h"

class SK_API SkOffsetPathEffect {
public:
    /** radius must be > 0 to have an effect. It specifies the distance from each corner
        that should be "rounded".
    */
    static sk_sp<SkPathEffect> Make(SkScalar offset, SkPaint::Join, SkScalar miter);
};

#endif
