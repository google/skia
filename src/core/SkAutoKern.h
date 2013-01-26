
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkAutoKern_DEFINED
#define SkAutoKern_DEFINED

#include "SkGlyph.h"

#define SkAutoKern_AdjustF(prev, next)    (((next) - (prev) + 32) >> 6 << 16)
#define SkAutoKern_AdjustS(prev, next)    SkIntToScalar(((next) - (prev) + 32) >> 6)

/* this is a helper class to perform auto-kerning
 * the adjust() method returns a SkFixed corresponding
 * to a +1/0/-1 pixel adjustment
 */

class SkAutoKern {
public:
    SkAutoKern() : fPrevRsbDelta(0) {}

    SkFixed  adjust(const SkGlyph&  glyph)
    {
//        if (SkAbs32(glyph.fLsbDelta) > 47 || SkAbs32(glyph.fRsbDelta) > 47)
//            printf("------- %d> L %d R %d\n", glyph.f_GlyphID, glyph.fLsbDelta, glyph.fRsbDelta);

#if 0
        int  distort = fPrevRsbDelta - glyph.fLsbDelta;

        fPrevRsbDelta = glyph.fRsbDelta;

        if (distort >= 32)
            return -SK_Fixed1;
        else if (distort < -32)
            return +SK_Fixed1;
        else
            return 0;
#else
        SkFixed adjust = SkAutoKern_AdjustF(fPrevRsbDelta, glyph.fLsbDelta);
        fPrevRsbDelta = glyph.fRsbDelta;
        return adjust;
#endif
    }
private:
    int   fPrevRsbDelta;
};

#endif
