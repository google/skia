/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkBlitRow_DEFINED
#define SkBlitRow_DEFINED

#include "SkBitmap.h"
#include "SkColor.h"

class SkBlitRow {
public:
    enum Flags16 {
        //! If set, the alpha parameter will be != 255
        kGlobalAlpha_Flag   = 0x01,
        //! If set, the src colors may have alpha != 255
        kSrcPixelAlpha_Flag = 0x02,
        //! If set, the resulting 16bit colors should be dithered
        kDither_Flag        = 0x04
    };

    /** Function pointer that reads a scanline of src SkPMColors, and writes
        a corresponding scanline of 16bit colors (specific format based on the
        config passed to the Factory.

        The x,y params provide the dithering phase for the start of the scanline

        @param alpha A global alpha to be applied to all of the src colors
        @param x The x coordinate of the beginning of the scanline
        @param y THe y coordinate of the scanline
     */
    typedef void (*Proc16)(uint16_t dst[], const SkPMColor src[], int count,
                           U8CPU alpha, int x, int y);

    static Proc16 Factory16(unsigned flags);

    /**
     *  Function pointer that blends a single src color onto a scaline of dst colors.
     *
     *  The x,y params provide the dithering phase for the start of the scanline
     */
    typedef void (*ColorProc16)(uint16_t dst[], SkPMColor src, int count, int x, int y);

    // Note : we ignore the kGlobalAlpha_Flag setting, but do respect kSrcPixelAlpha_Flag
    static ColorProc16 ColorFactory16(unsigned flags);

    ///////////// D32 version

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

    /** These static functions are called by the Factory and Factory32
        functions, and should return either NULL, or a
        platform-specific function-ptr to be used in place of the
        system default.
     */

    static Proc32 PlatformProcs32(unsigned flags);

private:
    enum {
        kFlags16_Mask = 7,
        kFlags32_Mask = 3
    };
};

#endif
