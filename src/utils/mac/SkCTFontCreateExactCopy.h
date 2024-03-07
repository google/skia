/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkCTFontCreateExactCopy_DEFINED
#define SkCTFontCreateExactCopy_DEFINED

#include "include/core/SkTypes.h"
#if defined(SK_BUILD_FOR_MAC) || defined(SK_BUILD_FOR_IOS)

#ifdef SK_BUILD_FOR_MAC
#import <ApplicationServices/ApplicationServices.h>
#endif

#ifdef SK_BUILD_FOR_IOS
#include <CoreGraphics/CoreGraphics.h>
#endif

#include "src/utils/mac/SkUniqueCFRef.h"

/*
 *  This function attempts to resize a CTFont without inadvertently changing
 *  unrelated properties, like the optical size, relative glyph metrics,
 *  or the underlying font data used.
 */
SkUniqueCFRef<CTFontRef> SkCTFontCreateExactCopy(CTFontRef baseFont, CGFloat textSize,
                                                 OpszVariation opsz);

#endif  // defined(SK_BUILD_FOR_MAC) || defined(SK_BUILD_FOR_IOS)
#endif  // SkCTFont_DEFINED
