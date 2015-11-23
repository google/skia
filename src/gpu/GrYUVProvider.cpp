/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrContext.h"
#include "GrDrawContext.h"
#include "GrYUVProvider.h"
#include "effects/GrYUVtoRGBEffect.h"

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
        planes[1] = (uint8_t*)planes[0] + yuvInfo->fSizeInMemory[0];
        planes[2] = (uint8_t*)planes[1] + yuvInfo->fSizeInMemory[1];
    } else {
        // Fetch yuv plane sizes for memory allocation. Here, width and height can be
        // rounded up to JPEG block size and be larger than the image's width and height.
        if (!provider->onGetYUVSizes(yuvInfo->fSize)) {
            return false;
        }

        // Allocate the memory for YUV
        size_t totalSize(0);
        for (int i = 0; i < GrYUVProvider::kPlaneCount; ++i) {
            yuvInfo->fRowBytes[i] = yuvInfo->fSize[i].fWidth; // we assume snug fit: rb == width
            yuvInfo->fSizeInMemory[i] = yuvInfo->fRowBytes[i] * yuvInfo->fSize[i].fHeight;
            totalSize += yuvInfo->fSizeInMemory[i];
        }
        if (useCache) {
            fCachedData.reset(SkResourceCache::NewCachedData(totalSize));
            planes[0] = fCachedData->writable_data();
        } else {
            fStorage.reset(totalSize);
            planes[0] = fStorage.get();
        }
        planes[1] = (uint8_t*)planes[0] + yuvInfo->fSizeInMemory[0];
        planes[2] = (uint8_t*)planes[1] + yuvInfo->fSizeInMemory[1];

        // Get the YUV planes and update plane sizes to actual image size
        if (!provider->onGetYUVPlanes(yuvInfo->fSize, planes, yuvInfo->fRowBytes,
                                      &yuvInfo->fColorSpace)) {
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
    for (int i = 0; i < 3; ++i) {
        yuvDesc.fWidth  = yuvInfo.fSize[i].fWidth;
        yuvDesc.fHeight = yuvInfo.fSize[i].fHeight;
        // TODO: why do we need this check?
        bool needsExactTexture = (yuvDesc.fWidth  != yuvInfo.fSize[0].fWidth) ||
                                 (yuvDesc.fHeight != yuvInfo.fSize[0].fHeight);
        if (needsExactTexture) {
            yuvTextures[i].reset(ctx->textureProvider()->createTexture(yuvDesc, true));
        } else {
            yuvTextures[i].reset(ctx->textureProvider()->createApproxTexture(yuvDesc));
        }
        if (!yuvTextures[i] ||
            !yuvTextures[i]->writePixels(0, 0, yuvDesc.fWidth, yuvDesc.fHeight,
                                         yuvDesc.fConfig, planes[i], yuvInfo.fRowBytes[i])) {
                return nullptr;
            }
    }

    GrSurfaceDesc rtDesc = desc;
    rtDesc.fFlags = rtDesc.fFlags | kRenderTarget_GrSurfaceFlag;

    SkAutoTUnref<GrTexture> result(ctx->textureProvider()->createTexture(rtDesc, true, nullptr, 0));
    if (!result) {
        return nullptr;
    }

    GrRenderTarget* renderTarget = result->asRenderTarget();
    SkASSERT(renderTarget);

    GrPaint paint;
    SkAutoTUnref<GrFragmentProcessor> yuvToRgbProcessor(
                                        GrYUVtoRGBEffect::Create(yuvTextures[0],
                                                                 yuvTextures[1],
                                                                 yuvTextures[2],
                                                                 yuvInfo.fSize,
                                                                 yuvInfo.fColorSpace));
    paint.addColorFragmentProcessor(yuvToRgbProcessor);
    paint.setPorterDuffXPFactory(SkXfermode::kSrc_Mode);
    const SkRect r = SkRect::MakeIWH(yuvInfo.fSize[0].fWidth, yuvInfo.fSize[0].fHeight);

    SkAutoTUnref<GrDrawContext> drawContext(ctx->drawContext(renderTarget));
    if (!drawContext) {
        return nullptr;
    }

    drawContext->drawRect(GrClip::WideOpen(), paint, SkMatrix::I(), r);

    return result.detach();
}

