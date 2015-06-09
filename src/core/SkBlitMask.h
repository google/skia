/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkBlitMask_DEFINED
#define SkBlitMask_DEFINED

#include "SkBitmap.h"
#include "SkColor.h"
#include "SkMask.h"

class SkBlitMask {
public:
    /**
     *  Returns true if the device config and mask format were supported.
     *  else return false (nothing was drawn)
     */
    static bool BlitColor(const SkBitmap& device, const SkMask& mask,
                          const SkIRect& clip, SkColor color);

    /**
     *  Function pointer that blits the mask into a device (dst) colorized
     *  by color. The number of pixels to blit is specified by width and height,
     *  but each scanline is offset by dstRB (rowbytes) and srcRB respectively.
     */
    typedef void (*ColorProc)(void* dst, size_t dstRB,
                              const void* mask, size_t maskRB,
                              SkColor color, int width, int height);

    /**
     *  Function pointer that blits a row of mask(lcd16) into a row of dst
     *  colorized by a single color. The number of pixels to blit is specified
     *  by width.
     */
    typedef void (*BlitLCD16RowProc)(SkPMColor dst[], const uint16_t src[],
                                     SkColor color, int width,
                                     SkPMColor opaqueDst);

    /**
     *  Function pointer that blits a row of src colors through a row of a mask
     *  onto a row of dst colors. The RowFactory that returns this function ptr
     *  will have been told the formats for the mask and the dst.
     */
    typedef void (*RowProc)(void* dst, const void* mask,
                            const SkPMColor* src, int width);

    /**
     *  Public entry-point to return a blitmask ColorProc.
     *  May return NULL if config or format are not supported.
     */
    static ColorProc ColorFactory(SkColorType, SkMask::Format, SkColor);

    /**
     *  Return either platform specific optimized blitmask ColorProc,
     *  or NULL if no optimized routine is available.
     */
    static ColorProc PlatformColorProcs(SkColorType, SkMask::Format, SkColor);

    /**
     *  Public entry-point to return a blitcolor BlitLCD16RowProc.
     */
    static BlitLCD16RowProc BlitLCD16RowFactory(bool isOpaque);

    /**
     *  Return either platform specific optimized blitcolor BlitLCD16RowProc,
     *  or NULL if no optimized routine is available.
     */
    static BlitLCD16RowProc PlatformBlitRowProcs16(bool isOpaque);

    enum RowFlags {
        kSrcIsOpaque_RowFlag    = 1 << 0
    };

    /**
     *  Public entry-point to return a blitmask RowProc.
     *  May return NULL if config or format are not supported.
     */
    static RowProc RowFactory(SkColorType, SkMask::Format, RowFlags);

    /**
     *  Return either platform specific optimized blitmask RowProc,
     *  or NULL if no optimized routine is available.
     */
    static RowProc PlatformRowProcs(SkColorType, SkMask::Format, RowFlags);
};

#endif
