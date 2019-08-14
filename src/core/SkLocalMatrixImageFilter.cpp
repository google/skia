/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkString.h"
#include "src/core/SkLocalMatrixImageFilter.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkSpecialImage.h"

sk_sp<SkImageFilter> SkLocalMatrixImageFilter::Make(const SkMatrix& localM,
                                                    sk_sp<SkImageFilter> input) {
    if (!input) {
        return nullptr;
    }
    if (localM.isIdentity()) {
        return input;
    }
    if (!as_IFB(input)->canHandleComplexCTM() && !localM.isScaleTranslate()) {
        // Nothing we can do at this point
        return nullptr;
    }
    return sk_sp<SkImageFilter>(new SkLocalMatrixImageFilter(localM, input));
}

SkLocalMatrixImageFilter::SkLocalMatrixImageFilter(const SkMatrix& localM,
                                                   sk_sp<SkImageFilter> input)
    : INHERITED(&input, 1, nullptr)
    , fLocalM(localM) {
}

sk_sp<SkFlattenable> SkLocalMatrixImageFilter::CreateProc(SkReadBuffer& buffer) {
    SK_IMAGEFILTER_UNFLATTEN_COMMON(common, 1);
    SkMatrix lm;
    buffer.readMatrix(&lm);
    return SkLocalMatrixImageFilter::Make(lm, common.getInput(0));
}

void SkLocalMatrixImageFilter::flatten(SkWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    buffer.writeMatrix(fLocalM);
}

sk_sp<SkSpecialImage> SkLocalMatrixImageFilter::onFilterImage(const Context& ctx,
                                                              SkIPoint* offset) const {
    Context localCtx(SkMatrix::Concat(ctx.ctm(), fLocalM), ctx.clipBounds(), ctx.cache(),
                     ctx.colorType(), ctx.colorSpace(), ctx.sourceImage());
    return this->filterInput(0, localCtx, offset);
}

SkIRect SkLocalMatrixImageFilter::onFilterBounds(const SkIRect& src, const SkMatrix& ctm,
                                                 MapDirection dir, const SkIRect* inputRect) const {
    return this->getInput(0)->filterBounds(src, SkMatrix::Concat(ctm, fLocalM), dir, inputRect);
}
