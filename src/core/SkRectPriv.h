/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkRectPriv_DEFINED
#define SkRectPriv_DEFINED

#include "SkRect.h"

class SkRectPriv {
public:
    // Returns true iff width and height are positive. Catches inverted, empty, and overflowing
    // (way too big) rects. This is used by clients that want a non-empty rect that they can also
    // actually use its computed width/height.
    //
    static bool PositiveDimensions(const SkIRect& r) {
        return r.width() > 0 && r.height() > 0;
    }
};

#endif
