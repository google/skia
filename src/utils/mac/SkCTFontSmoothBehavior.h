/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkCTFontSmoothBehavior_DEFINED
#define SkCTFontSmoothBehavior_DEFINED

#include "include/core/SkTypes.h"
#if defined(SK_BUILD_FOR_MAC) || defined(SK_BUILD_FOR_IOS)

enum class SkCTFontSmoothBehavior {
    none, // SmoothFonts produces no effect.
    some, // SmoothFonts produces some effect, but not subpixel coverage.
    subpixel, // SmoothFonts produces some effect and provides subpixel coverage.
};

SkCTFontSmoothBehavior SkCTFontGetSmoothBehavior();

#endif
#endif // SkCTFontSmoothBehavior_DEFINED
