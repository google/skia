/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkLocalMatrixImageFilter.h"
#include "SkReadBuffer.h"
#include "SkString.h"

SkImageFilter* SkLocalMatrixImageFilter::Create(const SkMatrix& localM, SkImageFilter* input) {
    if (!input) {
        return nullptr;
    }
    if (localM.getType() & (SkMatrix::kAffine_Mask | SkMatrix::kPerspective_Mask)) {
        return nullptr;
    }
    if (localM.isIdentity()) {
        return SkRef(input);
    }
    return new SkLocalMatrixImageFilter(localM, input);
}

SkLocalMatrixImageFilter::SkLocalMatrixImageFilter(const SkMatrix& localM, SkImageFilter* input)
  : INHERITED(1, &input), fLocalM(localM)
{}

SkFlattenable* SkLocalMatrixImageFilter::CreateProc(SkReadBuffer& buffer) {
    SK_IMAGEFILTER_UNFLATTEN_COMMON(common, 1);
    SkMatrix lm;
    buffer.readMatrix(&lm);
    return SkLocalMatrixImageFilter::Create(lm, common.getInput(0));
}

void SkLocalMatrixImageFilter::flatten(SkWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    buffer.writeMatrix(fLocalM);
}

bool SkLocalMatrixImageFilter::onFilterImage(Proxy* proxy, const SkBitmap& src, const Context& ctx,
                                             SkBitmap* result, SkIPoint* offset) const {
    Context localCtx(SkMatrix::Concat(ctx.ctm(), fLocalM), ctx.clipBounds(), ctx.cache());
    return this->filterInput(0, proxy, src, localCtx, result, offset);
}

bool SkLocalMatrixImageFilter::onFilterBounds(const SkIRect& src, const SkMatrix& matrix,
                                              SkIRect* dst) const {
    return this->getInput(0)->filterBounds(src, SkMatrix::Concat(matrix, fLocalM), dst);
}

#ifndef SK_IGNORE_TO_STRING
void SkLocalMatrixImageFilter::toString(SkString* str) const {
    str->append("SkLocalMatrixImageFilter: (");
    str->append(")");
}
#endif
