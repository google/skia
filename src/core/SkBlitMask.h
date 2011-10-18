/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkBlitMask_DEFINED
#define SkBlitMask_DEFINED

#include "SkBitmap.h"
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
    typedef void (*Proc)(void* dst, size_t dstRB,
                         const void* mask, size_t maskRB,
                         SkColor color, int width, int height);

    /**
     *  Public entry-point to return a blitmask function ptr.
     *  May return NULL if config or format are not supported.
     */
    static Proc Factory(SkBitmap::Config dstConfig, SkMask::Format, SkColor);

    /**
     *  Return either platform specific optimized blitmask function-ptr,
     *  or NULL if no optimized routine is available.
     */
    static Proc PlatformProcs(SkBitmap::Config dstConfig, SkMask::Format, SkColor);
};

#endif
