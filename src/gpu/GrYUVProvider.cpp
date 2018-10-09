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

sk_sp<SkCachedData> GrYUVProvider::getPlanes(SkYUVASizeInfo* size,
                                             SkYUVAIndex yuvaIndices[4],
                                             SkYUVColorSpace* colorSpace,
                                             const void* constPlanes[4]) {
    sk_sp<SkCachedData> data;
    SkYUVPlanesCache::Info yuvInfo;
    data.reset(SkYUVPlanesCache::FindAndRef(this->onGetID(), &yuvInfo));

    void* planes[4];

    if (data.get()) {
        planes[0] = (void*)data->data();
        planes[1] = (uint8_t*)planes[0] + (yuvInfo.fSizeInfo1.fWidthBytes[SkYUVASizeInfo::kY] *
                                           yuvInfo.fSizeInfo1.fSizes[SkYUVASizeInfo::kY].fHeight);
        planes[2] = (uint8_t*)planes[1] + (yuvInfo.fSizeInfo1.fWidthBytes[SkYUVASizeInfo::kU] *
                                           yuvInfo.fSizeInfo1.fSizes[SkYUVASizeInfo::kU].fHeight);
        planes[3] = nullptr;
    } else {
        // Fetch yuv plane sizes for memory allocation.
        if (!this->onQueryYUVA8(&yuvInfo.fSizeInfo1, yuvInfo.fYUVAIndices, &yuvInfo.fColorSpace1)) {
            return nullptr;
        }

        // Allocate the memory for YUVA
        size_t totalSize(0);
        for (int i = 0; i < 4; i++) {
            totalSize += yuvInfo.fSizeInfo1.fWidthBytes[i] * yuvInfo.fSizeInfo1.fSizes[i].fHeight;
        }
        data.reset(SkResourceCache::NewCachedData(totalSize));
        planes[0] = data->writable_data();
        planes[1] = (uint8_t*)planes[0] + (yuvInfo.fSizeInfo1.fWidthBytes[SkYUVASizeInfo::kY] *
                                           yuvInfo.fSizeInfo1.fSizes[SkYUVASizeInfo::kY].fHeight);
        planes[2] = (uint8_t*)planes[1] + (yuvInfo.fSizeInfo1.fWidthBytes[SkYUVASizeInfo::kU] *
                                           yuvInfo.fSizeInfo1.fSizes[SkYUVASizeInfo::kU].fHeight);
        planes[3] = nullptr;

        // Get the YUV planes.
        if (!this->onGetYUVA8Planes(yuvInfo.fSizeInfo1, yuvInfo.fYUVAIndices, planes)) {
            return nullptr;
        }

        // Decoding is done, cache the resulting YUV planes
        SkYUVPlanesCache::Add(this->onGetID(), data.get(), &yuvInfo);
    }

    *size = yuvInfo.fSizeInfo1;
    memcpy(yuvaIndices, yuvInfo.fYUVAIndices, sizeof(yuvInfo.fYUVAIndices));
    *colorSpace = yuvInfo.fColorSpace1;
    constPlanes[0] = planes[0];
    constPlanes[1] = planes[1];
    constPlanes[2] = planes[2];
    constPlanes[3] = planes[3];
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
    SkYUVASizeInfo yuvSizeInfo;
    SkYUVAIndex yuvaIndices[4];
    SkYUVColorSpace yuvColorSpace;
    const void* planes[3];

    sk_sp<SkCachedData> dataStorage = this->getPlanes(&yuvSizeInfo, yuvaIndices,
                                                      &yuvColorSpace, planes);
    if (!dataStorage) {
        return nullptr;
    }

    sk_sp<GrTextureProxy> yuvTextureProxies[3];
    for (int i = 0; i < 3; i++) {
        int componentWidth  = yuvSizeInfo.fSizes[i].fWidth;
        int componentHeight = yuvSizeInfo.fSizes[i].fHeight;
        // If the sizes of the components are not all the same we choose to create exact-match
        // textures for the smaller ones rather than add a texture domain to the draw.
        // TODO: revisit this decision to improve texture reuse?
        SkBackingFit fit =
                (componentWidth  != yuvSizeInfo.fSizes[SkYUVASizeInfo::kY].fWidth) ||
                (componentHeight != yuvSizeInfo.fSizes[SkYUVASizeInfo::kY].fHeight)
                    ? SkBackingFit::kExact : SkBackingFit::kApprox;

        SkImageInfo imageInfo = SkImageInfo::MakeA8(componentWidth, componentHeight);
        SkPixmap pixmap(imageInfo, planes[i], yuvSizeInfo.fWidthBytes[i]);
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

        SkASSERT(yuvTextureProxies[i]->width() == yuvSizeInfo.fSizes[i].fWidth);
        SkASSERT(yuvTextureProxies[i]->height() == yuvSizeInfo.fSizes[i].fHeight);
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
                                   yuvColorSpace, false);
    paint.addColorFragmentProcessor(std::move(yuvToRgbProcessor));

    // If the caller expects the pixels in a different color space than the one from the image,
    // apply a color conversion to do this.
    std::unique_ptr<GrFragmentProcessor> colorConversionProcessor =
            GrColorSpaceXformEffect::Make(srcColorSpace, kOpaque_SkAlphaType,
                                          dstColorSpace, kOpaque_SkAlphaType);
    if (colorConversionProcessor) {
        paint.addColorFragmentProcessor(std::move(colorConversionProcessor));
    }

    paint.setPorterDuffXPFactory(SkBlendMode::kSrc);
    const SkRect r = SkRect::MakeIWH(yuvSizeInfo.fSizes[SkYUVASizeInfo::kY].fWidth,
                                     yuvSizeInfo.fSizes[SkYUVASizeInfo::kY].fHeight);

    renderTargetContext->drawRect(GrNoClip(), std::move(paint), GrAA::kNo, SkMatrix::I(), r);

    return renderTargetContext->asTextureProxyRef();
}
