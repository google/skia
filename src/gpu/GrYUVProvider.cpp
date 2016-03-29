/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrContext.h"
#include "GrDrawContext.h"
#include "GrYUVProvider.h"
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

GrTexture* GrYUVProvider::refAsTexture(GrContext* ctx, const GrSurfaceDesc& desc, bool useCache) {
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

    GrSurfaceDesc rtDesc = desc;
    rtDesc.fFlags = rtDesc.fFlags | kRenderTarget_GrSurfaceFlag;

    SkAutoTUnref<GrTexture> result(ctx->textureProvider()->createTexture(rtDesc, SkBudgeted::kYes,
                                                                         nullptr, 0));
    if (!result) {
        return nullptr;
    }

    GrRenderTarget* renderTarget = result->asRenderTarget();
    SkASSERT(renderTarget);

    GrPaint paint;
    // We may be decoding an sRGB image, but the result of our linear math on the YUV planes
    // is already in sRGB in that case. Don't convert (which will make the image too bright).
    paint.setDisableOutputConversionToSRGB(true);
    SkAutoTUnref<const GrFragmentProcessor> yuvToRgbProcessor(
                                        GrYUVEffect::CreateYUVToRGB(yuvTextures[0],
                                                                    yuvTextures[1],
                                                                    yuvTextures[2],
                                                                    yuvInfo.fSizeInfo.fSizes,
                                                                    yuvInfo.fColorSpace));
    paint.addColorFragmentProcessor(yuvToRgbProcessor);
    paint.setPorterDuffXPFactory(SkXfermode::kSrc_Mode);
    const SkRect r = SkRect::MakeIWH(yuvInfo.fSizeInfo.fSizes[SkYUVSizeInfo::kY].fWidth,
            yuvInfo.fSizeInfo.fSizes[SkYUVSizeInfo::kY].fHeight);

    SkAutoTUnref<GrDrawContext> drawContext(ctx->drawContext(renderTarget));
    if (!drawContext) {
        return nullptr;
    }

    drawContext->drawRect(GrClip::WideOpen(), paint, SkMatrix::I(), r);

    return result.release();
}
