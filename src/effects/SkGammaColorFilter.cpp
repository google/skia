/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkGammaColorFilter.h"

#include "SkReadBuffer.h"
#include "SkString.h"

#if SK_SUPPORT_GPU
#include "effects/GrGammaEffect.h"
#endif

void SkGammaColorFilter::filterSpan(const SkPMColor src[], int count,
                                    SkPMColor dst[]) const {
    // Gamma-correcting bytes to bytes is pretty questionable.
    SkASSERT(0);
    for (int i = 0; i < count; ++i) {
        SkPMColor c = src[i];

        // TODO: implement cpu gamma correction?
        dst[i] = c;
    }
}

sk_sp<SkColorFilter> SkGammaColorFilter::Make(SkScalar gamma) {
    return sk_sp<SkColorFilter>(new SkGammaColorFilter(gamma));
}

SkGammaColorFilter::SkGammaColorFilter(SkScalar gamma) : fGamma(gamma) {}

sk_sp<SkFlattenable> SkGammaColorFilter::CreateProc(SkReadBuffer& buffer) {
    SkScalar gamma = buffer.readScalar();

    return Make(gamma);
}

void SkGammaColorFilter::flatten(SkWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    buffer.writeScalar(fGamma);
}

#ifndef SK_IGNORE_TO_STRING
void SkGammaColorFilter::toString(SkString* str) const {
    str->appendf("SkGammaColorFilter (%.2f)", fGamma);
}
#endif

#if SK_SUPPORT_GPU
sk_sp<GrFragmentProcessor> SkGammaColorFilter::asFragmentProcessor(GrContext*) const {
    return GrGammaEffect::Make(fGamma);
}
#endif
