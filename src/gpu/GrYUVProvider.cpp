/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrYUVProvider.h"
#include "GrClip.h"
#include "GrColorSpaceXform.h"
#include "GrContext.h"
#include "GrContextPriv.h"
#include "GrProxyProvider.h"
#include "GrRenderTargetContext.h"
#include "GrTextureProxy.h"
#include "SkAutoMalloc.h"
#include "SkCachedData.h"
#include "SkRefCnt.h"
#include "SkResourceCache.h"
#include "SkYUVPlanesCache.h"
#include "effects/GrYUVtoRGBEffect.h"

sk_sp<SkCachedData> init_provider(GrYUVProvider* provider, SkYUVPlanesCache::Info* yuvInfo,
                                  void* planes[3]) {
    sk_sp<SkCachedData> data;
    data.reset(SkYUVPlanesCache::FindAndRef(provider->onGetID(), yuvInfo));

    if (data.get()) {
        planes[0] = (void*)data->data();
        planes[1] = (uint8_t*)planes[0] + (yuvInfo->fSizeInfo.fWidthBytes[SkYUVSizeInfo::kY] *
                                           yuvInfo->fSizeInfo.fSizes[SkYUVSizeInfo::kY].fHeight);
        planes[2] = (uint8_t*)planes[1] + (yuvInfo->fSizeInfo.fWidthBytes[SkYUVSizeInfo::kU] *
                                           yuvInfo->fSizeInfo.fSizes[SkYUVSizeInfo::kU].fHeight);
    } else {
        // Fetch yuv plane sizes for memory allocation.
        if (!provider->onQueryYUV8(&yuvInfo->fSizeInfo, &yuvInfo->fColorSpace)) {
            return nullptr;
        }

        // Allocate the memory for YUV
        size_t totalSize(0);
        for (int i = 0; i < 3; i++) {
            totalSize += yuvInfo->fSizeInfo.fWidthBytes[i] * yuvInfo->fSizeInfo.fSizes[i].fHeight;
        }
        data.reset(SkResourceCache::NewCachedData(totalSize));
        planes[0] = data->writable_data();
        planes[1] = (uint8_t*)planes[0] + (yuvInfo->fSizeInfo.fWidthBytes[SkYUVSizeInfo::kY] *
                                           yuvInfo->fSizeInfo.fSizes[SkYUVSizeInfo::kY].fHeight);
        planes[2] = (uint8_t*)planes[1] + (yuvInfo->fSizeInfo.fWidthBytes[SkYUVSizeInfo::kU] *
                                           yuvInfo->fSizeInfo.fSizes[SkYUVSizeInfo::kU].fHeight);

        // Get the YUV planes.
        if (!provider->onGetYUV8Planes(yuvInfo->fSizeInfo, planes)) {
            return nullptr;
        }

        // Decoding is done, cache the resulting YUV planes
        SkYUVPlanesCache::Add(provider->onGetID(), data.get(), yuvInfo);
    }
    return data;
}

void GrYUVProvider::YUVGen_DataReleaseProc(const void*, void* data) {
    SkCachedData* cachedData = static_cast<SkCachedData*>(data);
    SkASSERT(cachedData);
    cachedData->unref();
}

sk_sp<GrTextureProxy> GrYUVProvider::refAsTextureProxy(GrContext* ctx, const GrSurfaceDesc& desc,
                                                       SkColorSpace* srcColorSpace,
                                                       SkColorSpace* dstColorSpace) {
    SkYUVPlanesCache::Info yuvInfo;
    void* planes[3];

    sk_sp<SkCachedData>  dataStorage = init_provider(this, &yuvInfo, planes);
    if (!dataStorage) {
        return nullptr;
    }

    sk_sp<GrTextureProxy> yuvTextureProxies[3];
    for (int i = 0; i < 3; i++) {
        int componentWidth  = yuvInfo.fSizeInfo.fSizes[i].fWidth;
        int componentHeight = yuvInfo.fSizeInfo.fSizes[i].fHeight;
        // If the sizes of the components are not all the same we choose to create exact-match
        // textures for the smaller onces rather than add a texture domain to the draw.
        // TODO: revisit this decision to imporve texture reuse?
        SkBackingFit fit =
                (componentWidth  != yuvInfo.fSizeInfo.fSizes[SkYUVSizeInfo::kY].fWidth) ||
                (componentHeight != yuvInfo.fSizeInfo.fSizes[SkYUVSizeInfo::kY].fHeight)
                    ? SkBackingFit::kExact : SkBackingFit::kApprox;

        SkImageInfo imageInfo = SkImageInfo::MakeA8(componentWidth, componentHeight);
        SkPixmap pixmap(imageInfo, planes[i], yuvInfo.fSizeInfo.fWidthBytes[i]);
        SkCachedData* dataStoragePtr = dataStorage.get();
        // We grab a ref to cached yuv data. When the SkImage we create below goes away it will call
        // the YUVGen_DataReleaseProc which will release this ref.
        // DDL TODO: Currently we end up creating a lazy proxy that will hold onto a ref to the
        // SkImage in its lambda. This means that we'll keep the ref on the YUV data around for the
        // life time of the proxy and not just upload. For non-DDL draws we should look into
        // releasing this SkImage after uploads (by deleting the lambda after instantiation).
        dataStoragePtr->ref();
        sk_sp<SkImage> yuvImage = SkImage::MakeFromRaster(pixmap, YUVGen_DataReleaseProc,
                                                          dataStoragePtr);

        auto proxyProvider = ctx->contextPriv().proxyProvider();
        yuvTextureProxies[i] = proxyProvider->createTextureProxy(yuvImage, kNone_GrSurfaceFlags,
                                                                 1, SkBudgeted::kYes, fit);

        SkASSERT(yuvTextureProxies[i]->width() == yuvInfo.fSizeInfo.fSizes[i].fWidth);
        SkASSERT(yuvTextureProxies[i]->height() == yuvInfo.fSizeInfo.fSizes[i].fHeight);
    }

    // TODO: investigate preallocating mip maps here
    sk_sp<GrRenderTargetContext> renderTargetContext(
        ctx->contextPriv().makeDeferredRenderTargetContext(
            SkBackingFit::kExact, desc.fWidth, desc.fHeight, desc.fConfig, nullptr,
            desc.fSampleCnt, GrMipMapped::kNo, kTopLeft_GrSurfaceOrigin));
    if (!renderTargetContext) {
        return nullptr;
    }

    GrPaint paint;
    auto yuvToRgbProcessor =
            GrYUVtoRGBEffect::Make(std::move(yuvTextureProxies[0]),
                                   std::move(yuvTextureProxies[1]),
                                   std::move(yuvTextureProxies[2]),
                                   yuvInfo.fColorSpace, false);
    paint.addColorFragmentProcessor(std::move(yuvToRgbProcessor));

    // If the caller expects the pixels in a different color space than the one from the image,
    // apply a color conversion to do this.
    std::unique_ptr<GrFragmentProcessor> colorConversionProcessor =
            GrColorSpaceXformEffect::Make(srcColorSpace, dstColorSpace);
    if (colorConversionProcessor) {
        paint.addColorFragmentProcessor(std::move(colorConversionProcessor));
    }

    paint.setPorterDuffXPFactory(SkBlendMode::kSrc);
    const SkRect r = SkRect::MakeIWH(yuvInfo.fSizeInfo.fSizes[SkYUVSizeInfo::kY].fWidth,
                                     yuvInfo.fSizeInfo.fSizes[SkYUVSizeInfo::kY].fHeight);

    renderTargetContext->drawRect(GrNoClip(), std::move(paint), GrAA::kNo, SkMatrix::I(), r);

    return renderTargetContext->asTextureProxyRef();
}
