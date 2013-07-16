/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkColorFilter.h"

#include "SkFlattenableBuffers.h"
#include "SkShader.h"
#include "SkUnPreMultiply.h"
#include "SkString.h"

SK_DEFINE_INST_COUNT(SkColorFilter)

bool SkColorFilter::asColorMode(SkColor* color, SkXfermode::Mode* mode) const {
    return false;
}

bool SkColorFilter::asColorMatrix(SkScalar matrix[20]) const {
    return false;
}

bool SkColorFilter::asComponentTable(SkBitmap*) const {
    return false;
}

void SkColorFilter::filterSpan16(const uint16_t s[], int count, uint16_t d[]) const {
    SkASSERT(this->getFlags() & SkColorFilter::kHasFilter16_Flag);
    SkDEBUGFAIL("missing implementation of SkColorFilter::filterSpan16");

    if (d != s) {
        memcpy(d, s, count * sizeof(uint16_t));
    }
}

SkColor SkColorFilter::filterColor(SkColor c) const {
    SkPMColor dst, src = SkPreMultiplyColor(c);
    this->filterSpan(&src, 1, &dst);
    return SkUnPreMultiply::PMColorToColor(dst);
}

GrEffectRef* SkColorFilter::asNewEffect(GrContext*) const {
    return NULL;
}
