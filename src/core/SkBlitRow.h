/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkBlitRow_DEFINED
#define SkBlitRow_DEFINED

#include "include/core/SkBitmap.h"
#include "include/core/SkColor.h"

class SkBlitRow {
public:
    enum Flags32 {
        kGlobalAlpha_Flag32     = 1 << 0,
        kSrcPixelAlpha_Flag32   = 1 << 1
    };

    /** Function pointer that blends 32bit colors onto a 32bit destination.
        @param dst  array of dst 32bit colors
        @param src  array of src 32bit colors (w/ or w/o alpha)
        @param count number of colors to blend
        @param alpha global alpha to be applied to all src colors
     */
    typedef void (*Proc32)(uint32_t dst[], const SkPMColor src[], int count, U8CPU alpha);

    static Proc32 Factory32(unsigned flags32);

    /** Blend a single color onto a row of S32 pixels, writing the result
        into a row of D32 pixels. src and dst may be the same memory, but
        if they are not, they may not overlap.
     */
    static void Color32(SkPMColor dst[], const SkPMColor src[], int count, SkPMColor color);
};

#endif
