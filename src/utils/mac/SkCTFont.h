/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkCTFont_DEFINED
#define SkCTFont_DEFINED

#include "include/core/SkTypes.h"
#if defined(SK_BUILD_FOR_MAC) || defined(SK_BUILD_FOR_IOS)

#ifdef SK_BUILD_FOR_MAC
#import <ApplicationServices/ApplicationServices.h>
#endif

#ifdef SK_BUILD_FOR_IOS
#include <CoreGraphics/CoreGraphics.h>
#endif

enum class SkCTFontSmoothBehavior {
    none, // SmoothFonts produces no effect.
    some, // SmoothFonts produces some effect, but not subpixel coverage.
    subpixel, // SmoothFonts produces some effect and provides subpixel coverage.
};

SkCTFontSmoothBehavior SkCTFontGetSmoothBehavior();

using SkCTFontWeightMapping = const CGFloat[11];

/** Returns the [-1, 1] CTFontDescriptor weights for the
 *  <0, 100, 200, 300, 400, 500, 600, 700, 800, 900, 1000> CSS weights.
 *
 *  It is assumed that the values will be interpolated linearly between these points.
 *  NSFontWeightXXX were added in 10.11, appear in 10.10, but do not appear in 10.9.
 *  The actual values appear to be stable, but they may change without notice.
 *  These values are valid for system fonts only.
 */
SkCTFontWeightMapping& SkCTFontGetNSFontWeightMapping();

/** Returns the [-1, 1] CTFontDescriptor weights for the
 *  <0, 100, 200, 300, 400, 500, 600, 700, 800, 900, 1000> CSS weights.
 *
 *  It is assumed that the values will be interpolated linearly between these points.
 *  The actual values appear to be stable, but they may change without notice.
 *  These values are valid for fonts created from data only.
 */
SkCTFontWeightMapping& SkCTFontGetDataFontWeightMapping();

#endif
#endif // SkCTFontSmoothBehavior_DEFINED
