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
    SkImageInfo info = {
        width,
        height,
        kPMColor_SkColorType,
        kPremul_SkAlphaType,
    };
    result->setConfig(info);
    result->setPixelRef(SkNEW_ARGS(SkGrPixelRef, (info, texture)))->unref();
    return true;
}

bool SkImageFilterUtils::GetInputResultGPU(SkImageFilter* filter, SkImageFilter::Proxy* proxy,
                                           const SkBitmap& src, const SkMatrix& ctm,
                                           SkBitmap* result, SkIPoint* offset) {
    // Ensure that GrContext calls under filterImage and filterImageGPU below will see an identity
    // matrix with no clip and that the matrix, clip, and render target set before this function was
    // called are restored before we return to the caller.
    GrContext* context = src.getTexture()->getContext();
    GrContext::AutoWideOpenIdentityDraw awoid(context, NULL);
    if (!filter) {
        *result = src;
        return true;
    } else if (filter->canFilterImageGPU()) {
        return filter->filterImageGPU(proxy, src, ctm, result, offset);
    } else {
        if (filter->filterImage(proxy, src, ctm, result, offset)) {
            if (!result->getTexture()) {
                SkImageInfo info;
                if (!result->asImageInfo(&info)) {
                    return false;
                }
                GrTexture* resultTex = GrLockAndRefCachedBitmapTexture(context, *result, NULL);
                result->setPixelRef(new SkGrPixelRef(info, resultTex))->unref();
                GrUnlockAndUnrefCachedBitmapTexture(resultTex);
            }
            return true;
        } else {
            return false;
        }
    }
}
#endif
