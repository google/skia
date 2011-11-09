/*
 * Copyright 2011 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBlurImageFilter.h"

SkBlurImageFilter::SkBlurImageFilter(SkScalar sigmaX, SkScalar sigmaY)
    : fSigma(SkSize::Make(sigmaX, sigmaY)) {
    SkASSERT(sigmaX >= 0 && sigmaY >= 0);
}

bool SkBlurImageFilter::asABlur(SkSize* sigma) const {
    *sigma = fSigma;
    return true;
}
