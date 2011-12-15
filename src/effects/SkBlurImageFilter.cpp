/*
 * Copyright 2011 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBlurImageFilter.h"

SkBlurImageFilter::SkBlurImageFilter(SkFlattenableReadBuffer& buffer)
  : INHERITED(buffer) {
    fSigma.fWidth = buffer.readScalar();
    fSigma.fHeight = buffer.readScalar();
}

SkBlurImageFilter::SkBlurImageFilter(SkScalar sigmaX, SkScalar sigmaY)
    : fSigma(SkSize::Make(sigmaX, sigmaY)) {
    SkASSERT(sigmaX >= 0 && sigmaY >= 0);
}

bool SkBlurImageFilter::asABlur(SkSize* sigma) const {
    *sigma = fSigma;
    return true;
}

void SkBlurImageFilter::flatten(SkFlattenableWriteBuffer& buffer) {
    this->INHERITED::flatten(buffer);
    buffer.writeScalar(fSigma.fWidth);
    buffer.writeScalar(fSigma.fHeight);
}

SK_DEFINE_FLATTENABLE_REGISTRAR(SkBlurImageFilter)
