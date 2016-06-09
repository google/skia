/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrContext.h"
#include "GrDrawContext.h"
#include "GrYUVProvider.h"
#include "effects/GrGammaEffect.h"
#include "effects/GrYUVEffect.h"

#include "SkCachedData.h"
#include "SkRefCnt.h"
#include "SkResourceCache.h"
#include "SkYUVPlanesCache.h"

namespace {
/**
 *  Helper class to manage the resources used for storing the YUV planar data. Depending on the
 *  useCache option, we may find (and lock) the data in our ResourceCache, or we may have allocated
 *  it in scratch storage.
 */
class YUVScoper {
public:
    bool init(GrYUVProvider*, SkYUVPlanesCache::Info*, void* planes[3], bool useCache);

private:
    // we only use one or the other of these
    SkAutoTUnref<SkCachedData>  fCachedData;
    SkAutoMalloc                fStorage;
};
}

bool YUVScoper::init(GrYUVProvider* provider, SkYUVPlanesCache::Info* yuvInfo, void* planes[3],
                     bool useCache) {
    if (useCache) {
        fCachedData.reset(SkYUVPlanesCache::FindAndRef(provider->onGetID(), yuvInfo));
    }

    if (fCachedData.get()) {
        planes[0] = (void*)fCachedData->data();
        planes[1] = (uint8_t*)planes[0] + (yuvInfo->fSizeInfo.fWidthBytes[SkYUVSizeInfo::kY] *
                                           yuvInfo->fSizeInfo.fSizes[SkYUVSizeInfo::kY].fHeight);
        planes[2] = (uint8_t*)planes[1] + (yuvInfo->fSizeInfo.fWidthBytes[SkYUVSizeInfo::kU] *
                                           yuvInfo->fSizeInfo.fSizes[SkYUVSizeInfo::kU].fHeight);
    } else {
        // Fetch yuv plane sizes for memory allocation.
        if (!provider->onQueryYUV8(&yuvInfo->fSizeInfo, &yuvInfo->fColorSpace)) {
            return false;
        }

        // Allocate the memory for YUV
        size_t totalSize(0);
        for (int i = 0; i < 3; i++) {
            totalSize += yuvInfo->fSizeInfo.fWidthBytes[i] * yuvInfo->fSizeInfo.fSizes[i].fHeight;
        }
        if (useCache) {
            fCachedData.reset(SkResourceCache::NewCachedData(totalSize));
            planes[0] = fCachedData->writable_data();
        } else {
            fStorage.reset(totalSize);
            planes[0] = fStorage.get();
        }
        planes[1] = (uint8_t*)planes[0] + (yuvInfo->fSizeInfo.fWidthBytes[SkYUVSizeInfo::kY] *
                                           yuvInfo->fSizeInfo.fSizes[SkYUVSizeInfo::kY].fHeight);
        planes[2] = (uint8_t*)planes[1] + (yuvInfo->fSizeInfo.fWidthBytes[SkYUVSizeInfo::kU] *
                                           yuvInfo->fSizeInfo.fSizes[SkYUVSizeInfo::kU].fHeight);

        // Get the YUV planes.
        if (!provider->onGetYUV8Planes(yuvInfo->fSizeInfo, planes)) {
            return false;
        }

        if (useCache) {
            // Decoding is done, cache the resulting YUV planes
            SkYUVPlanesCache::Add(provider->onGetID(), fCachedData, yuvInfo);
        }
    }
    return true;
}

sk_sp<GrTexture> GrYUVProvider::refAsTexture(GrContext* ctx,
                                             const GrSurfaceDesc& desc,
                                             bool useCache) {
    SkYUVPlanesCache::Info yuvInfo;
    void* planes[3];
    YUVScoper scoper;
    if (!scoper.init(this, &yuvInfo, planes, useCache)) {
        return nullptr;
    }

    GrSurfaceDesc yuvDesc;
    yuvDesc.fConfig = kAlpha_8_GrPixelConfig;
    SkAutoTUnref<GrTexture> yuvTextures[3];
    for (int i = 0; i < 3; i++) {
        yuvDesc.fWidth  = yuvInfo.fSizeInfo.fSizes[i].fWidth;
        yuvDesc.fHeight = yuvInfo.fSizeInfo.fSizes[i].fHeight;
        // TODO: why do we need this check?
        bool needsExactTexture =
                (yuvDesc.fWidth  != yuvInfo.fSizeInfo.fSizes[SkYUVSizeInfo::kY].fWidth) ||
                (yuvDesc.fHeight != yuvInfo.fSizeInfo.fSizes[SkYUVSizeInfo::kY].fHeight);
        if (needsExactTexture) {
            yuvTextures[i].reset(ctx->textureProvider()->createTexture(yuvDesc, SkBudgeted::kYes));
        } else {
            yuvTextures[i].reset(ctx->textureProvider()->createApproxTexture(yuvDesc));
        }
        if (!yuvTextures[i] ||
            !yuvTextures[i]->writePixels(0, 0, yuvDesc.fWidth, yuvDesc.fHeight, yuvDesc.fConfig,
                                         planes[i], yuvInfo.fSizeInfo.fWidthBytes[i])) {
                return nullptr;
            }
    }

    sk_sp<GrDrawContext> drawContext(ctx->newDrawContext(SkBackingFit::kExact,
                                                         desc.fWidth, desc.fHeight,
                                                         desc.fConfig, desc.fSampleCnt));
    if (!drawContext) {
        return nullptr;
    }

    GrPaint paint;
    sk_sp<GrFragmentProcessor> yuvToRgbProcessor(
        GrYUVEffect::MakeYUVToRGB(yuvTextures[0], yuvTextures[1], yuvTextures[2],
                                  yuvInfo.fSizeInfo.fSizes, yuvInfo.fColorSpace, false));
    paint.addColorFragmentProcessor(std::move(yuvToRgbProcessor));

    // If we're decoding an sRGB image, the result of our linear math on the YUV planes is already
    // in sRGB. (The encoding is just math on bytes, with no concept of color spaces.) So, we need
    // to output the results of that math directly to the buffer that we will then consider sRGB.
    // If we have sRGB write control, we can just tell the HW not to do the Linear -> sRGB step.
    // Otherwise, we do our shader math to go from YUV -> sRGB, manually convert sRGB -> Linear,
    // then let the HW convert Linear -> sRGB.
    if (GrPixelConfigIsSRGB(desc.fConfig)) {
        if (ctx->caps()->srgbWriteControl()) {
            paint.setDisableOutputConversionToSRGB(true);
        } else {
            paint.addColorFragmentProcessor(GrGammaEffect::Make(2.2f));
        }
    }

    paint.setPorterDuffXPFactory(SkXfermode::kSrc_Mode);
    const SkRect r = SkRect::MakeIWH(yuvInfo.fSizeInfo.fSizes[SkYUVSizeInfo::kY].fWidth,
            yuvInfo.fSizeInfo.fSizes[SkYUVSizeInfo::kY].fHeight);

    drawContext->drawRect(GrNoClip(), paint, SkMatrix::I(), r);

    return drawContext->asTexture();
}
