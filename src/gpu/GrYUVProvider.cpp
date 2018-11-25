/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrYUVProvider.h"
#include "GrClip.h"
#include "GrContext.h"
#include "GrContextPriv.h"
#include "GrRenderTargetContext.h"
#include "GrTextureProxy.h"
#include "SkAutoMalloc.h"
#include "SkCachedData.h"
#include "SkRefCnt.h"
#include "SkResourceCache.h"
#include "SkYUVPlanesCache.h"
#include "effects/GrNonlinearColorSpaceXformEffect.h"
#include "effects/GrSRGBEffect.h"
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

sk_sp<GrTextureProxy> GrYUVProvider::refAsTextureProxy(GrContext* ctx, const GrSurfaceDesc& desc,
                                                       const SkColorSpace* srcColorSpace,
                                                       const SkColorSpace* dstColorSpace) {
    SkYUVPlanesCache::Info yuvInfo;
    void* planes[3];

    sk_sp<SkCachedData>  dataStorage = init_provider(this, &yuvInfo, planes);
    if (!dataStorage) {
        return nullptr;
    }

    GrSurfaceDesc yuvDesc;
    yuvDesc.fOrigin = kTopLeft_GrSurfaceOrigin;
    yuvDesc.fConfig = kAlpha_8_GrPixelConfig;
    sk_sp<GrSurfaceContext> yuvTextureContexts[3];
    for (int i = 0; i < 3; i++) {
        yuvDesc.fWidth  = yuvInfo.fSizeInfo.fSizes[i].fWidth;
        yuvDesc.fHeight = yuvInfo.fSizeInfo.fSizes[i].fHeight;
        // TODO: why do we need this check?
        SkBackingFit fit =
                (yuvDesc.fWidth  != yuvInfo.fSizeInfo.fSizes[SkYUVSizeInfo::kY].fWidth) ||
                (yuvDesc.fHeight != yuvInfo.fSizeInfo.fSizes[SkYUVSizeInfo::kY].fHeight)
                    ? SkBackingFit::kExact : SkBackingFit::kApprox;

        yuvTextureContexts[i] = ctx->contextPriv().makeDeferredSurfaceContext(yuvDesc,
                                                                              GrMipMapped::kNo,
                                                                              fit,
                                                                              SkBudgeted::kYes);
        if (!yuvTextureContexts[i]) {
            return nullptr;
        }

        const SkImageInfo ii = SkImageInfo::MakeA8(yuvDesc.fWidth, yuvDesc.fHeight);
        if (!yuvTextureContexts[i]->writePixels(ii, planes[i],
                                                yuvInfo.fSizeInfo.fWidthBytes[i], 0, 0)) {
            return nullptr;
        }
    }

    // We never want to perform color-space conversion during the decode
    // TODO: investigate preallocating mip maps here
    sk_sp<GrRenderTargetContext> renderTargetContext(ctx->makeDeferredRenderTargetContext(
                                                                    SkBackingFit::kExact,
                                                                    desc.fWidth, desc.fHeight,
                                                                    desc.fConfig, nullptr,
                                                                    desc.fSampleCnt,
                                                                    GrMipMapped::kNo,
                                                                    kTopLeft_GrSurfaceOrigin));
    if (!renderTargetContext) {
        return nullptr;
    }

    GrPaint paint;
    auto yuvToRgbProcessor =
            GrYUVtoRGBEffect::Make(yuvTextureContexts[0]->asTextureProxyRef(),
                                   yuvTextureContexts[1]->asTextureProxyRef(),
                                   yuvTextureContexts[2]->asTextureProxyRef(),
                                   yuvInfo.fSizeInfo.fSizes, yuvInfo.fColorSpace, false);
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
            paint.addColorFragmentProcessor(GrSRGBEffect::Make(GrSRGBEffect::Mode::kSRGBToLinear,
                                                               GrSRGBEffect::Alpha::kOpaque));
        }
    }

    // If the caller expects the pixels in a different color space than the one from the image,
    // apply a color conversion to do this.
    std::unique_ptr<GrFragmentProcessor> colorConversionProcessor =
            GrNonlinearColorSpaceXformEffect::Make(srcColorSpace, dstColorSpace);
    if (colorConversionProcessor) {
        paint.addColorFragmentProcessor(std::move(colorConversionProcessor));
    }

    paint.setPorterDuffXPFactory(SkBlendMode::kSrc);
    const SkRect r = SkRect::MakeIWH(yuvInfo.fSizeInfo.fSizes[SkYUVSizeInfo::kY].fWidth,
                                     yuvInfo.fSizeInfo.fSizes[SkYUVSizeInfo::kY].fHeight);

    renderTargetContext->drawRect(GrNoClip(), std::move(paint), GrAA::kNo, SkMatrix::I(), r);

    return renderTargetContext->asTextureProxyRef();
}
