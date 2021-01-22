/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkCGGeometry_DEFINED
#define SkCGGeometry_DEFINED

#include "include/core/SkTypes.h"
#if defined(SK_BUILD_FOR_MAC) || defined(SK_BUILD_FOR_IOS)

#ifdef SK_BUILD_FOR_MAC
#import <ApplicationServices/ApplicationServices.h>
#endif

#ifdef SK_BUILD_FOR_IOS
#include <CoreGraphics/CoreGraphics.h>
#endif

// Skia extensions for types in CGGeometry.h

// Inline versions of these CGRect helpers.
// The CG versions require making a call and a copy of the CGRect on the stack.

static inline bool SkCGRectIsEmpty(const CGRect& rect) {
    return rect.size.width <= 0 || rect.size.height <= 0;
}

static inline CGFloat SkCGRectGetMinX(const CGRect& rect) {
    return rect.origin.x;
}

static inline CGFloat SkCGRectGetMaxX(const CGRect& rect) {
    return rect.origin.x + rect.size.width;
}

static inline CGFloat SkCGRectGetMinY(const CGRect& rect) {
    return rect.origin.y;
}

static inline CGFloat SkCGRectGetMaxY(const CGRect& rect) {
    return rect.origin.y + rect.size.height;
}

static inline CGFloat SkCGRectGetWidth(const CGRect& rect) {
    return rect.size.width;
}

#endif
#endif //SkCGGeometry_DEFINED
