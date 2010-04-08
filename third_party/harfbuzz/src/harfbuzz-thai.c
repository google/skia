/*
 * Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies)
 *
 * This is part of HarfBuzz, an OpenType Layout engine library.
 *
 * Permission is hereby granted, without written agreement and without
 * license or royalty fees, to use, copy, modify, and distribute this
 * software and its documentation for any purpose, provided that the
 * above copyright notice and the following two paragraphs appear in
 * all copies of this software.
 *
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE TO ANY PARTY FOR
 * DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES
 * ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN
 * IF THE COPYRIGHT HOLDER HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 *
 * THE COPYRIGHT HOLDER SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING,
 * BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE PROVIDED HEREUNDER IS
 * ON AN "AS IS" BASIS, AND THE COPYRIGHT HOLDER HAS NO OBLIGATION TO
 * PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
 */

#include "harfbuzz-shaper.h"
#include "harfbuzz-shaper-private.h"
#include "harfbuzz-external.h"

#include <assert.h>

static void thaiWordBreaks(const HB_UChar16 *string, hb_uint32 len, HB_CharAttributes *attributes)
{
    typedef int (*th_brk_def)(const char*, int[], int);
    static void *thaiCodec = 0;
    static th_brk_def th_brk = 0;
    char *cstr = 0;
    int brp[128];
    int *break_positions = brp;
    hb_uint32 numbreaks;
    hb_uint32 i;

    if (!thaiCodec)
        thaiCodec = HB_TextCodecForMib(2259);

    /* load libthai dynamically */
    if (!th_brk && thaiCodec) {
        th_brk = (th_brk_def)HB_Library_Resolve("thai", "th_brk");
        if (!th_brk)
            thaiCodec = 0;
    }

    if (!th_brk)
        return;

    cstr = HB_TextCodec_ConvertFromUnicode(thaiCodec, string, len, 0);
    if (!cstr)
        return;

    break_positions = brp;
    numbreaks = th_brk(cstr, break_positions, 128);
    if (numbreaks > 128) {
        break_positions = (int *)malloc(numbreaks * sizeof(int));
        numbreaks = th_brk(cstr, break_positions, numbreaks);
    }

    for (i = 0; i < len; ++i)
        attributes[i].lineBreakType = HB_NoBreak;

    for (i = 0; i < numbreaks; ++i) {
        if (break_positions[i] > 0)
            attributes[break_positions[i]-1].lineBreakType = HB_Break;
    }

    if (break_positions != brp)
        free(break_positions);

    HB_TextCodec_FreeResult(cstr);
}


void HB_ThaiAttributes(HB_Script script, const HB_UChar16 *text, hb_uint32 from, hb_uint32 len, HB_CharAttributes *attributes)
{
    assert(script == HB_Script_Thai);
    attributes += from;
    thaiWordBreaks(text + from, len, attributes);
}

