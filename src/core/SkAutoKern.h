/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkAutoKern_DEFINED
#define SkAutoKern_DEFINED

#include "SkGlyph.h"

 // Track previous right side bearing and subtract from the left side bearing of the glyph passed
 // in. The output are the values -1/0/+1 pixel adjustments.
 //
 // The side bearing values are start in an int8_t format with 2.6 fixed-point encoding.
class SkAutoKern {
public:
    SkScalar  adjust(const SkGlyph&  glyph) {
        SkScalar adjust = AdjustKerning(fPrevRsbDelta, glyph.fLsbDelta);
        fPrevRsbDelta = glyph.fRsbDelta;
        return adjust;
    }

    static SkScalar AdjustKerning(int rsbDelta, int lsbDelta) {
        const int half = 32;
        return SkIntToScalar((lsbDelta - rsbDelta + half) >> 6);
    }

private:
    int fPrevRsbDelta{0};
};

#endif
