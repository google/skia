/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPMColor_DEFINED
#define SkPMColor_DEFINED

#include "include/core/SkColor.h"
#include "include/private/base/SkAPI.h"

#include <cstdint>

/** Returns a SkPMColor value from already premultiplied 8-bit component values.

    @param a  amount of alpha, from fully transparent (0) to fully opaque (255)
    @param r  amount of red, from no red (0) to full red (255)
    @param g  amount of green, from no green (0) to full green (255)
    @param b  amount of blue, from no blue (0) to full blue (255)
    @return   premultiplied color
*/
SK_API SkPMColor SkPMColorSetARGB(SkAlpha a, uint8_t r, uint8_t g, uint8_t b);

/** Returns alpha component of premultiplied color. */
SK_API SkAlpha SkPMColorGetA(SkPMColor);

/** Returns red component of premultiplied color. */
SK_API uint8_t SkPMColorGetR(SkPMColor);

/** Returns green component of premultiplied color. */
SK_API uint8_t SkPMColorGetG(SkPMColor);

/** Returns blue component of premultiplied color. */
SK_API uint8_t SkPMColorGetB(SkPMColor);

#endif
