/*
 * Copyright 2013 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkMatrix.h"

#if SK_SUPPORT_GPU
#include "GrTexture.h"
#include "SkImageFilterUtils.h"
#include "SkBitmap.h"
#include "SkGrPixelRef.h"
#include "SkGr.h"

bool SkImageFilterUtils::WrapTexture(GrTexture* texture, int width, int height, SkBitmap* result) {
    result->setConfig(SkBitmap::kARGB_8888_Config, width, height);
    result->setPixelRef(SkNEW_ARGS(SkGrPixelRef, (texture)))->unref();
    return true;
}

bool SkImageFilterUtils::GetInputResultGPU(SkImageFilter* filter, SkImageFilter::Proxy* proxy,
                                           const SkBitmap& src, const SkMatrix& ctm,
                                           SkBitmap* result, SkIPoint* offset) {
    if (!filter) {
        *result = src;
        return true;
    } else if (filter->canFilterImageGPU()) {
        return filter->filterImageGPU(proxy, src, ctm, result, offset);
    } else {
        if (filter->filterImage(proxy, src, ctm, result, offset)) {
            if (!result->getTexture()) {
                GrContext* context = ((GrTexture *) src.getTexture())->getContext();
                GrTexture* resultTex = GrLockAndRefCachedBitmapTexture(context,
                    *result, NULL);
                result->setPixelRef(new SkGrPixelRef(resultTex))->unref();
                GrUnlockAndUnrefCachedBitmapTexture(resultTex);
            }
            return true;
        } else {
            return false;
        }
    }
}
#endif
