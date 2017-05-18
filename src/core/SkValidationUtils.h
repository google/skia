/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkValidationUtils_DEFINED
#define SkValidationUtils_DEFINED

#include "SkBitmap.h"
#include "SkBlendMode.h"
#include "SkXfermodePriv.h"

/** Returns true if coeff's value is in the SkXfermode::Coeff enum.
  */
static inline bool SkIsValidCoeff(SkXfermode::Coeff coeff) {
    return coeff >= 0 && coeff < SkXfermode::kCoeffCount;
}

/** Returns true if mode's value is in the SkBlendMode enum.
  */
static inline bool SkIsValidMode(SkBlendMode mode) {
    return (unsigned)mode <= (unsigned)SkBlendMode::kLastMode;
}

/** Returns true if the rect's dimensions are between 0 and SK_MaxS32
  */
static inline bool SkIsValidIRect(const SkIRect& rect) {
    return rect.width() >= 0 && rect.height() >= 0;
}

/** Returns true if the rect's dimensions are between 0 and SK_ScalarMax
  */
static inline bool SkIsValidRect(const SkRect& rect) {
    return (rect.fLeft <= rect.fRight) &&
           (rect.fTop <= rect.fBottom) &&
           SkScalarIsFinite(rect.width()) &&
           SkScalarIsFinite(rect.height());
}

#endif
