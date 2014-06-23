/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef sk_tool_utils_DEFINED
#define sk_tool_utils_DEFINED

#include "SkBitmap.h"
#include "SkCanvas.h"
#include "SkImageInfo.h"
#include "SkPaint.h"
#include "SkTypeface.h"

namespace sk_tool_utils {

    extern bool gEnablePortableTypeface;

    const char* colortype_name(SkColorType);

    /**
     * Sets the paint to use a platform-independent text renderer.
     */
    void set_portable_typeface(SkPaint* paint, SkTypeface::Style style = SkTypeface::kNormal);

    /**
     *  Call canvas->writePixels() by using the pixels from bitmap, but with an info that claims
     *  the pixels are colorType + alphaType
     */
    void write_pixels(SkCanvas*, const SkBitmap&, int x, int y, SkColorType, SkAlphaType);

}  // namespace sk_tool_utils

#endif  // sk_tool_utils_DEFINED
