/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkSingleInputImageFilter.h"
#include "SkBitmap.h"
#include "SkFlattenableBuffers.h"
#include "SkMatrix.h"
#if SK_SUPPORT_GPU
#include "GrTexture.h"
#include "SkGr.h"
#include "SkGrPixelRef.h"
#endif

SkSingleInputImageFilter::SkSingleInputImageFilter(SkImageFilter* input) : INHERITED(input) {
}

SkSingleInputImageFilter::~SkSingleInputImageFilter() {
}

SkSingleInputImageFilter::SkSingleInputImageFilter(SkFlattenableReadBuffer& rb)
    : INHERITED(rb) {
}

void SkSingleInputImageFilter::flatten(SkFlattenableWriteBuffer& wb) const {
    this->INHERITED::flatten(wb);
}

SkBitmap SkSingleInputImageFilter::getInputResult(Proxy* proxy,
                                                  const SkBitmap& src,
                                                  const SkMatrix& ctm,
                                                  SkIPoint* offset) {
    return this->INHERITED::getInputResult(0, proxy, src, ctm, offset);
}

#if SK_SUPPORT_GPU
// FIXME:  generalize and move to base class
GrTexture* SkSingleInputImageFilter::getInputResultAsTexture(Proxy* proxy,
                                                             GrTexture* src,
                                                             const SkRect& rect) {
    GrTexture* resultTex = NULL;
    SkImageFilter* input = getInput(0);
    if (!input) {
        resultTex = src;
    } else if (input->canFilterImageGPU()) {
        // onFilterImageGPU() already refs the result, so just return it here.
        return input->onFilterImageGPU(proxy, src, rect);
    } else {
        SkBitmap srcBitmap, result;
        srcBitmap.setConfig(SkBitmap::kARGB_8888_Config, src->width(), src->height());
        srcBitmap.setPixelRef(new SkGrPixelRef(src))->unref();
        SkIPoint offset;
        if (input->filterImage(proxy, srcBitmap, SkMatrix(), &result, &offset)) {
            if (result.getTexture()) {
                resultTex = (GrTexture*) result.getTexture();
            } else {
                resultTex = GrLockCachedBitmapTexture(src->getContext(), result, NULL);
                SkSafeRef(resultTex);
                GrUnlockCachedBitmapTexture(resultTex);
                return resultTex;
            }
        } else {
            resultTex = src;
        }
    }
    SkSafeRef(resultTex);
    return resultTex;
}
#endif
