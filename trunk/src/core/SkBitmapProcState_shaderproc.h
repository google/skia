
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkMathPriv.h"

#define SCALE_FILTER_NAME       MAKENAME(_filter_DX_shaderproc)

// Can't be static in the general case because some of these implementations
// will be defined and referenced in different object files.
void SCALE_FILTER_NAME(const SkBitmapProcState& s, int x, int y,
                       DSTTYPE* SK_RESTRICT colors, int count);

void SCALE_FILTER_NAME(const SkBitmapProcState& s, int x, int y,
                       DSTTYPE* SK_RESTRICT colors, int count) {
    SkASSERT((s.fInvType & ~(SkMatrix::kTranslate_Mask |
                             SkMatrix::kScale_Mask)) == 0);
    SkASSERT(s.fInvKy == 0);
    SkASSERT(count > 0 && colors != NULL);
    SkASSERT(s.fDoFilter);
    SkDEBUGCODE(CHECKSTATE(s);)

    const unsigned maxX = s.fBitmap->width() - 1;
    const SkFixed oneX = s.fFilterOneX;
    const SkFixed dx = s.fInvSx;
    SkFixed fx;
    const SRCTYPE* SK_RESTRICT row0;
    const SRCTYPE* SK_RESTRICT row1;
    unsigned subY;

    {
        SkPoint pt;
        s.fInvProc(*s.fInvMatrix, SkIntToScalar(x) + SK_ScalarHalf,
                   SkIntToScalar(y) + SK_ScalarHalf, &pt);
        SkFixed fy = SkScalarToFixed(pt.fY) - (s.fFilterOneY >> 1);
        const unsigned maxY = s.fBitmap->height() - 1;
        // compute our two Y values up front
        subY = TILEY_LOW_BITS(fy, maxY);
        int y0 = TILEY_PROCF(fy, maxY);
        int y1 = TILEY_PROCF((fy + s.fFilterOneY), maxY);

        const char* SK_RESTRICT srcAddr = (const char*)s.fBitmap->getPixels();
        unsigned rb = s.fBitmap->rowBytes();
        row0 = (const SRCTYPE*)(srcAddr + y0 * rb);
        row1 = (const SRCTYPE*)(srcAddr + y1 * rb);
        // now initialize fx
        fx = SkScalarToFixed(pt.fX) - (oneX >> 1);
    }

#ifdef PREAMBLE
    PREAMBLE(s);
#endif

    do {
        unsigned subX = TILEX_LOW_BITS(fx, maxX);
        unsigned x0 = TILEX_PROCF(fx, maxX);
        unsigned x1 = TILEX_PROCF((fx + oneX), maxX);

        FILTER_PROC(subX, subY,
                    SRC_TO_FILTER(row0[x0]),
                    SRC_TO_FILTER(row0[x1]),
                    SRC_TO_FILTER(row1[x0]),
                    SRC_TO_FILTER(row1[x1]),
                    colors);
        colors += 1;

        fx += dx;
    } while (--count != 0);

#ifdef POSTAMBLE
    POSTAMBLE(s);
#endif
}

///////////////////////////////////////////////////////////////////////////////

#undef TILEX_PROCF
#undef TILEY_PROCF
#undef TILEX_LOW_BITS
#undef TILEY_LOW_BITS
#undef MAKENAME
#undef SRCTYPE
#undef DSTTYPE
#undef CHECKSTATE
#undef SRC_TO_FILTER
#undef FILTER_TO_DST
#undef PREAMBLE
#undef POSTAMBLE

#undef SCALE_FILTER_NAME
