/*
 * Copyright 2012 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkOffsetImageFilter.h"
#include "SkBitmap.h"
#include "SkMatrix.h"
#include "SkFlattenableBuffers.h"

bool SkOffsetImageFilter::onFilterImage(Proxy* proxy, const SkBitmap& source,
                                        const SkMatrix& matrix,
                                        SkBitmap* result,
                                        SkIPoint* loc) {
    SkBitmap src = this->getInputResult(0, proxy, source, matrix, loc);
    SkVector vec;
    matrix.mapVectors(&vec, &fOffset, 1);

    loc->fX += SkScalarRoundToInt(vec.fX);
    loc->fY += SkScalarRoundToInt(vec.fY);
    *result = src;
    return true;
}

bool SkOffsetImageFilter::onFilterBounds(const SkIRect& src, const SkMatrix& ctm,
                                         SkIRect* dst) {
    SkVector vec;
    ctm.mapVectors(&vec, &fOffset, 1);

    *dst = src;
    dst->offset(SkScalarRoundToInt(vec.fX), SkScalarRoundToInt(vec.fY));
    return true;
}

void SkOffsetImageFilter::flatten(SkFlattenableWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    buffer.writePoint(fOffset);
}

SkOffsetImageFilter::SkOffsetImageFilter(SkScalar dx, SkScalar dy,
                                         SkImageFilter* input) : INHERITED(input) {
    fOffset.set(dx, dy);
}

SkOffsetImageFilter::SkOffsetImageFilter(SkFlattenableReadBuffer& buffer) : INHERITED(buffer) {
    buffer.readPoint(&fOffset);
}
