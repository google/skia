/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkBlitMask_DEFINED
#define SkBlitMask_DEFINED

#include "SkColor.h"
#include "SkMask.h"
#include "SkPixmap.h"

class SkBlitMask {
public:
    /**
     *  Returns true if the device config and mask format were supported.
     *  else return false (nothing was drawn)
     */
    static bool BlitColor(const SkPixmap& device, const SkMask& mask,
                          const SkIRect& clip, SkColor color);

    /**
     *  Function pointer that blits a row of mask(lcd16) into a row of dst
     *  colorized by a single color. The number of pixels to blit is specified
     *  by width.
     */
    typedef void (*BlitLCD16RowProc)(SkPMColor dst[], const uint16_t src[],
                                     SkColor color, int width,
                                     SkPMColor opaqueDst);

    /**
     *  Public entry-point to return a blitcolor BlitLCD16RowProc.
     */
    static BlitLCD16RowProc BlitLCD16RowFactory(bool isOpaque);

    /**
     *  Return either platform specific optimized blitcolor BlitLCD16RowProc,
     *  or nullptr if no optimized routine is available.
     */
    static BlitLCD16RowProc PlatformBlitRowProcs16(bool isOpaque);
};

#endif
