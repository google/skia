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

SkSingleInputImageFilter::SkSingleInputImageFilter(SkImageFilter* input) : fInput(input) {
    SkSafeRef(fInput);
}

SkSingleInputImageFilter::~SkSingleInputImageFilter() {
    SkSafeUnref(fInput);
}

SkSingleInputImageFilter::SkSingleInputImageFilter(SkFlattenableReadBuffer& rb) {
    if (rb.readBool()) {
        fInput = rb.readFlattenableT<SkImageFilter>();
    } else {
        fInput = NULL;
    }
}

void SkSingleInputImageFilter::flatten(SkFlattenableWriteBuffer& wb) const {
    wb.writeBool(NULL != fInput);
    if (NULL != fInput) {
        wb.writeFlattenable(fInput);
    }
}

SkBitmap SkSingleInputImageFilter::getInputResult(Proxy* proxy,
                                                  const SkBitmap& src,
                                                  const SkMatrix& ctm,
                                                  SkIPoint* offset) {
    SkBitmap result;
    if (fInput && fInput->filterImage(proxy, src, ctm, &result, offset)) {
        return result;
    } else {
        return src;
    }
}

#if SK_SUPPORT_GPU
GrTexture* SkSingleInputImageFilter::getInputResultAsTexture(GrTexture* src,
                                                             const SkRect& rect) {
    GrTexture* resultTex;
    if (!fInput) {
        resultTex = src;
    } else if (fInput->canFilterImageGPU()) {
        // onFilterImageGPU() already refs the result, so just return it here.
        return fInput->onFilterImageGPU(src, rect);
    } else {
        SkBitmap srcBitmap, result;
        srcBitmap.setConfig(SkBitmap::kARGB_8888_Config, src->width(), src->height());
        srcBitmap.setPixelRef(new SkGrPixelRef(src))->unref();
        SkIPoint offset;
        if (fInput->filterImage(NULL, srcBitmap, SkMatrix(), &result, &offset)) {
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

SK_DEFINE_FLATTENABLE_REGISTRAR(SkSingleInputImageFilter)
