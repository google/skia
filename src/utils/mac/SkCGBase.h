/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkCGBase_DEFINED
#define SkCGBase_DEFINED

#include "include/private/base/SkFeatures.h"

#if defined(SK_BUILD_FOR_MAC) || defined(SK_BUILD_FOR_IOS)

#include "include/core/SkScalar.h"

#ifdef SK_BUILD_FOR_MAC
#import <ApplicationServices/ApplicationServices.h>
#endif

#ifdef SK_BUILD_FOR_IOS
#include <CoreGraphics/CoreGraphics.h>
#endif

// Skia extensions for types in CGBase.h

static inline CGFloat SkScalarToCGFloat(SkScalar scalar) {
    return CGFLOAT_IS_DOUBLE ? SkScalarToDouble(scalar) : scalar;
}

static inline SkScalar SkScalarFromCGFloat(CGFloat cgFloat) {
    return CGFLOAT_IS_DOUBLE ? SkDoubleToScalar(cgFloat) : cgFloat;
}

static inline float SkFloatFromCGFloat(CGFloat cgFloat) {
    return CGFLOAT_IS_DOUBLE ? static_cast<float>(cgFloat) : cgFloat;
}

#endif
#endif //SkCGBase_DEFINED
