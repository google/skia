/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkUtils.h"

// declare functions externally to suppress warnings.
void MAKENAME(_nofilter_DX)(const SkBitmapProcState& s,
                            const uint32_t* SK_RESTRICT xy,
                            int count, SkPMColor* SK_RESTRICT colors);

void MAKENAME(_nofilter_DX)(const SkBitmapProcState& s,
                            const uint32_t* SK_RESTRICT xy,
                            int count, SkPMColor* SK_RESTRICT colors) {
    SkASSERT(count > 0 && colors != nullptr);
    SkASSERT(s.fInvType <= (SkMatrix::kTranslate_Mask | SkMatrix::kScale_Mask));
    SkASSERT(kNone_SkFilterQuality == s.fFilterQuality);
    SkDEBUGCODE(CHECKSTATE(s);)

#ifdef PREAMBLE
    PREAMBLE(s);
#endif
    const SRCTYPE* SK_RESTRICT srcAddr = (const SRCTYPE*)s.fPixmap.addr();

    // buffer is y32, x16, x16, x16, x16, x16
    // bump srcAddr to the proper row, since we're told Y never changes
    SkASSERT((unsigned)xy[0] < (unsigned)s.fPixmap.height());
    srcAddr = (const SRCTYPE*)((const char*)srcAddr +
                                                xy[0] * s.fPixmap.rowBytes());
    xy += 1;

    SRCTYPE src;

    if (1 == s.fPixmap.width()) {
        src = srcAddr[0];
        SkPMColor dstValue = RETURNDST(src);
        sk_memset32(colors, dstValue, count);
    } else {
        int i;
        for (i = (count >> 2); i > 0; --i) {
            uint32_t xx0 = *xy++;
            uint32_t xx1 = *xy++;
            SRCTYPE x0 = srcAddr[UNPACK_PRIMARY_SHORT(xx0)];
            SRCTYPE x1 = srcAddr[UNPACK_SECONDARY_SHORT(xx0)];
            SRCTYPE x2 = srcAddr[UNPACK_PRIMARY_SHORT(xx1)];
            SRCTYPE x3 = srcAddr[UNPACK_SECONDARY_SHORT(xx1)];

            *colors++ = RETURNDST(x0);
            *colors++ = RETURNDST(x1);
            *colors++ = RETURNDST(x2);
            *colors++ = RETURNDST(x3);
        }
        const uint16_t* SK_RESTRICT xx = (const uint16_t*)(xy);
        for (i = (count & 3); i > 0; --i) {
            SkASSERT(*xx < (unsigned)s.fPixmap.width());
            src = srcAddr[*xx++]; *colors++ = RETURNDST(src);
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

#undef MAKENAME
#undef SRCTYPE
#undef CHECKSTATE
#undef RETURNDST
#undef SRC_TO_FILTER
#undef FILTER_TO_DST

#ifdef PREAMBLE
    #undef PREAMBLE
#endif

#undef FILTER_PROC_TYPE
#undef GET_FILTER_TABLE
#undef GET_FILTER_ROW
#undef GET_FILTER_ROW_PROC
#undef GET_FILTER_PROC
