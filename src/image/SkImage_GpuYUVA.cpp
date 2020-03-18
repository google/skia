/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <cstddef>
#include <cstring>
#include <type_traits>

#include "include/core/SkYUVASizeInfo.h"
#include "include/gpu/GrContext.h"
#include "include/private/GrRecordingContext.h"
#include "src/core/SkAutoPixmapStorage.h"
#include "src/core/SkMipMap.h"
#include "src/core/SkScopeExit.h"
#include "src/gpu/GrBitmapTextureMaker.h"
#include "src/gpu/GrClip.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrGpu.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/GrRenderTargetContext.h"
#include "src/gpu/GrTexture.h"
#include "src/gpu/GrTextureProducer.h"
#include "src/gpu/SkGr.h"
#include "src/gpu/effects/GrYUVtoRGBEffect.h"
#include "src/image/SkImage_Gpu.h"
#include "src/image/SkImage_GpuYUVA.h"

static constexpr auto kAssumedColorType = kRGBA_8888_SkColorType;

SkImage_GpuYUVA::SkImage_GpuYUVA(sk_sp<GrContext> context, SkISize size, uint32_t uniqueID,
                                 SkYUVColorSpace colorSpace, GrSurfaceProxyView views[],
                                 GrColorType proxyColorTypes[], int numViews,
                                 const SkYUVAIndex yuvaIndices[4], GrSurfaceOrigin origin,
                                 sk_sp<SkColorSpace> imageColorSpace)
        : INHERITED(std::move(context), size, uniqueID, kAssumedColorType,
                    // If an alpha channel is present we always switch to kPremul. This is because,
                    // although the planar data is always un-premul, the final interleaved RGB image
                    // is/would-be premul.
                    GetAlphaTypeFromYUVAIndices(yuvaIndices), std::move(imageColorSpace))
        , fNumViews(numViews)
        , fYUVColorSpace(colorSpace)
        , fOrigin(origin) {
    // The caller should have done this work, just verifying
    SkDEBUGCODE(int textureCount;)
    SkASSERT(SkYUVAIndex::AreValidIndices(yuvaIndices, &textureCount));
    SkASSERT(textureCount == fNumViews);

    for (int i = 0; i < numViews; ++i) {
        fViews[i] = std::move(views[i]);
        fProxyColorTypes[i] = proxyColorTypes[i];
    }
    memcpy(fYUVAIndices, yuvaIndices, 4 * sizeof(SkYUVAIndex));
}

// For onMakeColorSpace()
SkImage_GpuYUVA::SkImage_GpuYUVA(const SkImage_GpuYUVA* image, sk_sp<SkColorSpace> targetCS)
        : INHERITED(image->fContext, image->dimensions(), kNeedNewImageUniqueID, kAssumedColorType,
                    // If an alpha channel is present we always switch to kPremul. This is because,
                    // although the planar data is always un-premul, the final interleaved RGB image
                    // is/would-be premul.
                    GetAlphaTypeFromYUVAIndices(image->fYUVAIndices), std::move(targetCS))
        , fNumViews(image->fNumViews)
        , fYUVColorSpace(image->fYUVColorSpace)
        , fOrigin(image->fOrigin)
        // Since null fFromColorSpace means no GrColorSpaceXform, we turn a null
        // image->refColorSpace() into an explicit SRGB.
        , fFromColorSpace(image->colorSpace() ? image->refColorSpace() : SkColorSpace::MakeSRGB()) {
    // The caller should have done this work, just verifying
    SkDEBUGCODE(int textureCount;)
    SkASSERT(SkYUVAIndex::AreValidIndices(image->fYUVAIndices, &textureCount));
    SkASSERT(textureCount == fNumViews);

    if (image->fRGBView.proxy()) {
        fRGBView = image->fRGBView;  // we ref in this case, not move
    } else {
        for (int i = 0; i < fNumViews; ++i) {
            fViews[i] = image->fViews[i];  // we ref in this case, not move
            fProxyColorTypes[i] = image->fProxyColorTypes[i];
        }
    }
    memcpy(fYUVAIndices, image->fYUVAIndices, 4 * sizeof(SkYUVAIndex));
}

bool SkImage_GpuYUVA::setupMipmapsForPlanes(GrRecordingContext* context) const {
    // We shouldn't get here if the planes were already flattened to RGBA.
    SkASSERT(fViews[0].proxy() && !fRGBView.proxy());
    if (!context || !fContext->priv().matches(context)) {
        return false;
    }

    for (int i = 0; i < fNumViews; ++i) {
        int mipCount = SkMipMap::ComputeLevelCount(fViews[i].proxy()->width(),
                                                   fViews[i].proxy()->height());
        if (mipCount && GrGpu::IsACopyNeededForMips(fContext->priv().caps(),
                                                    fViews[i].asTextureProxy(),
                                                    GrSamplerState::Filter::kMipMap)) {
            auto mippedView = GrCopyBaseMipMapToTextureProxy(context, fViews[i].asTextureProxy(),
                                                             fOrigin, fProxyColorTypes[i]);
            if (!mippedView.proxy()) {
                return false;
            }
            fViews[i] = std::move(mippedView);
        }
    }
    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////

GrSemaphoresSubmitted SkImage_GpuYUVA::onFlush(GrContext* context, const GrFlushInfo& info) {
    if (!context || !fContext->priv().matches(context) || fContext->abandoned()) {
        return GrSemaphoresSubmitted::kNo;
    }

    GrSurfaceProxy* proxies[4] = {fViews[0].proxy(), fViews[1].proxy(), fViews[2].proxy(),
                                  fViews[3].proxy()};
    int numProxies = fNumViews;
    if (fRGBView.proxy()) {
        // Either we've already flushed the flattening draw or the flattening is unflushed. In the
        // latter case it should still be ok to just pass fRGBView proxy because it in turn depends
        // on the planar proxies and will cause all of their work to flush as well.
        proxies[0] = fRGBView.proxy();
        numProxies = 1;
    }
    return context->priv().flushSurfaces(proxies, numProxies, info);
}

GrTextureProxy* SkImage_GpuYUVA::peekProxy() const { return fRGBView.asTextureProxy(); }

void SkImage_GpuYUVA::flattenToRGB(GrRecordingContext* context) const {
    if (fRGBView.proxy()) {
        return;
    }

    if (!context || !fContext->priv().matches(context)) {
        return;
    }

    // Needs to create a render target in order to draw to it for the yuv->rgb conversion.
    auto renderTargetContext = GrRenderTargetContext::Make(
            context, GrColorType::kRGBA_8888, this->refColorSpace(), SkBackingFit::kExact,
            this->dimensions(), 1, GrMipMapped::kNo, GrProtected::kNo, fOrigin);
    if (!renderTargetContext) {
        return;
    }

    sk_sp<GrColorSpaceXform> colorSpaceXform;
    if (fFromColorSpace) {
        colorSpaceXform = GrColorSpaceXform::Make(fFromColorSpace.get(), this->alphaType(),
                                                  this->colorSpace(), this->alphaType());
    }
    const SkRect rect = SkRect::MakeIWH(this->width(), this->height());
    if (!RenderYUVAToRGBA(fContext.get(), renderTargetContext.get(), rect, fYUVColorSpace,
                          std::move(colorSpaceXform), fViews, fYUVAIndices)) {
        return;
    }

    fRGBView = renderTargetContext->readSurfaceView();
    SkASSERT(fRGBView.origin() == fOrigin);
    SkASSERT(fRGBView.swizzle() == GrSwizzle());
    for (auto& v : fViews) {
        v.reset();
    }
}

GrSurfaceProxyView SkImage_GpuYUVA::refMippedView(GrRecordingContext* context) const {
    // if invalid or already has miplevels
    this->flattenToRGB(context);
    if (!fRGBView || fRGBView.asTextureProxy()->mipMapped() == GrMipMapped::kYes) {
        return fRGBView;
    }

    // need to generate mips for the proxy
    GrColorType srcColorType = SkColorTypeToGrColorType(this->colorType());
    GrSurfaceProxyView mippedView = GrCopyBaseMipMapToTextureProxy(context, fRGBView.proxy(),
                                                                   fRGBView.origin(), srcColorType);
    if (mippedView) {
        fRGBView = std::move(mippedView);
        return fRGBView;
    }

    // failed to generate mips
    return {};
}

const GrSurfaceProxyView* SkImage_GpuYUVA::view(GrRecordingContext* context) const {
    this->flattenToRGB(context);
    if (!fRGBView.proxy()) {
        return nullptr;
    }
    return &fRGBView;
}

//////////////////////////////////////////////////////////////////////////////////////////////////

sk_sp<SkImage> SkImage_GpuYUVA::onMakeColorTypeAndColorSpace(
        GrRecordingContext*, SkColorType, sk_sp<SkColorSpace> targetCS) const {
    // We explicitly ignore color type changes, for now.

    // we may need a mutex here but for now we expect usage to be in a single thread
    if (fOnMakeColorSpaceTarget &&
        SkColorSpace::Equals(targetCS.get(), fOnMakeColorSpaceTarget.get())) {
        return fOnMakeColorSpaceResult;
    }
    sk_sp<SkImage> result = sk_sp<SkImage>(new SkImage_GpuYUVA(this, targetCS));
    if (result) {
        fOnMakeColorSpaceTarget = targetCS;
        fOnMakeColorSpaceResult = result;
    }
    return result;
}

sk_sp<SkImage> SkImage_GpuYUVA::onReinterpretColorSpace(sk_sp<SkColorSpace> newCS) const {
    return sk_make_sp<SkImage_GpuYUVA>(fContext, this->dimensions(), kNeedNewImageUniqueID,
                                       fYUVColorSpace, fViews, fProxyColorTypes, fNumViews,
                                       fYUVAIndices, fOrigin, std::move(newCS));
}

//////////////////////////////////////////////////////////////////////////////////////////////////

sk_sp<SkImage> SkImage::MakeFromYUVATextures(GrContext* ctx,
                                             SkYUVColorSpace colorSpace,
                                             const GrBackendTexture yuvaTextures[],
                                             const SkYUVAIndex yuvaIndices[4],
                                             SkISize imageSize,
                                             GrSurfaceOrigin imageOrigin,
                                             sk_sp<SkColorSpace> imageColorSpace) {
    int numTextures;
    if (!SkYUVAIndex::AreValidIndices(yuvaIndices, &numTextures)) {
        return nullptr;
    }

    GrSurfaceProxyView tempViews[4];
    if (!SkImage_GpuBase::MakeTempTextureProxies(ctx, yuvaTextures, numTextures, yuvaIndices,
                                                 imageOrigin, tempViews)) {
        return nullptr;
    }
    GrColorType proxyColorTypes[4];
    for (int i = 0; i < numTextures; ++i) {
        proxyColorTypes[i] = ctx->priv().caps()->getYUVAColorTypeFromBackendFormat(
                yuvaTextures[i].getBackendFormat(), yuvaIndices[3].fIndex == i);
    }

    return sk_make_sp<SkImage_GpuYUVA>(sk_ref_sp(ctx), imageSize, kNeedNewImageUniqueID, colorSpace,
                                       tempViews, proxyColorTypes, numTextures, yuvaIndices,
                                       imageOrigin, imageColorSpace);
}

sk_sp<SkImage> SkImage::MakeFromYUVAPixmaps(GrContext* context, SkYUVColorSpace yuvColorSpace,
                                            const SkPixmap yuvaPixmaps[],
                                            const SkYUVAIndex yuvaIndices[4], SkISize imageSize,
                                            GrSurfaceOrigin imageOrigin, bool buildMips,
                                            bool limitToMaxTextureSize,
                                            sk_sp<SkColorSpace> imageColorSpace) {
    if (!context) {
        return nullptr;  // until we impl this for raster backend
    }

    int numPixmaps;
    if (!SkYUVAIndex::AreValidIndices(yuvaIndices, &numPixmaps)) {
        return nullptr;
    }

    if (!context->priv().caps()->mipMapSupport()) {
        buildMips = false;
    }

    // Make proxies
    GrSurfaceProxyView tempViews[4];
    GrColorType proxyColorTypes[4];
    for (int i = 0; i < numPixmaps; ++i) {
        const SkPixmap* pixmap = &yuvaPixmaps[i];
        SkAutoPixmapStorage resized;
        int maxTextureSize = context->priv().caps()->maxTextureSize();
        int maxDim = std::max(yuvaPixmaps[i].width(), yuvaPixmaps[i].height());
        if (limitToMaxTextureSize && maxDim > maxTextureSize) {
            float scale = static_cast<float>(maxTextureSize) / maxDim;
            int newWidth = std::min(static_cast<int>(yuvaPixmaps[i].width() * scale), maxTextureSize);
            int newHeight =
                    std::min(static_cast<int>(yuvaPixmaps[i].height() * scale), maxTextureSize);
            SkImageInfo info = yuvaPixmaps[i].info().makeWH(newWidth, newHeight);
            if (!resized.tryAlloc(info) ||
                !yuvaPixmaps[i].scalePixels(resized, kLow_SkFilterQuality)) {
                return nullptr;
            }
            pixmap = &resized;
        }
        // Turn the pixmap into a GrTextureProxy
        SkBitmap bmp;
        bmp.installPixels(*pixmap);
        GrBitmapTextureMaker bitmapMaker(context, bmp, GrImageTexGenPolicy::kNew_Uncached_Budgeted);
        GrMipMapped mipMapped = buildMips ? GrMipMapped::kYes : GrMipMapped::kNo;
        GrSurfaceProxyView view;
        tempViews[i] = bitmapMaker.view(mipMapped);
        if (!tempViews[i]) {
            return nullptr;
        }
        proxyColorTypes[i] = bitmapMaker.colorType();
    }

    return sk_make_sp<SkImage_GpuYUVA>(sk_ref_sp(context), imageSize, kNeedNewImageUniqueID,
                                       yuvColorSpace, tempViews, proxyColorTypes, numPixmaps,
                                       yuvaIndices, imageOrigin, imageColorSpace);
}

/////////////////////////////////////////////////////////////////////////////////////////////////
sk_sp<SkImage> SkImage_GpuYUVA::MakePromiseYUVATexture(
        GrContext* context,
        SkYUVColorSpace yuvColorSpace,
        const GrBackendFormat yuvaFormats[],
        const SkISize yuvaSizes[],
        const SkYUVAIndex yuvaIndices[4],
        int imageWidth,
        int imageHeight,
        GrSurfaceOrigin imageOrigin,
        sk_sp<SkColorSpace> imageColorSpace,
        PromiseImageTextureFulfillProc textureFulfillProc,
        PromiseImageTextureReleaseProc textureReleaseProc,
        PromiseImageTextureDoneProc promiseDoneProc,
        PromiseImageTextureContext textureContexts[],
        PromiseImageApiVersion version) {
    int numTextures;
    bool valid = SkYUVAIndex::AreValidIndices(yuvaIndices, &numTextures);

    // The contract here is that if 'promiseDoneProc' is passed in it should always be called,
    // even if creation of the SkImage fails. Once we call MakePromiseImageLazyProxy it takes
    // responsibility for calling the done proc.
    if (!promiseDoneProc) {
        return nullptr;
    }
    int proxiesCreated = 0;
    SkScopeExit callDone([promiseDoneProc, textureContexts, numTextures, &proxiesCreated]() {
        for (int i = proxiesCreated; i < numTextures; ++i) {
            promiseDoneProc(textureContexts[i]);
        }
    });

    if (!valid) {
        return nullptr;
    }

    if (!context) {
        return nullptr;
    }

    if (imageWidth <= 0 || imageHeight <= 0) {
        return nullptr;
    }

    SkAlphaType at = (-1 != yuvaIndices[SkYUVAIndex::kA_Index].fIndex) ? kPremul_SkAlphaType
                                                                       : kOpaque_SkAlphaType;
    SkImageInfo info =
            SkImageInfo::Make(imageWidth, imageHeight, kAssumedColorType, at, imageColorSpace);
    if (!SkImageInfoIsValid(info)) {
        return nullptr;
    }

    // verify sizes with expected texture count
    for (int i = 0; i < numTextures; ++i) {
        if (yuvaSizes[i].isEmpty()) {
            return nullptr;
        }
    }
    for (int i = numTextures; i < SkYUVASizeInfo::kMaxCount; ++i) {
        if (!yuvaSizes[i].isEmpty()) {
            return nullptr;
        }
    }

    // Get lazy proxies
    GrSurfaceProxyView views[4];
    GrColorType proxyColorTypes[4];
    for (int texIdx = 0; texIdx < numTextures; ++texIdx) {
        GrColorType colorType = context->priv().caps()->getYUVAColorTypeFromBackendFormat(
                yuvaFormats[texIdx], yuvaIndices[3].fIndex == texIdx);
        if (GrColorType::kUnknown == colorType) {
            return nullptr;
        }

        auto proxy = MakePromiseImageLazyProxy(
                context, yuvaSizes[texIdx].width(), yuvaSizes[texIdx].height(), colorType,
                yuvaFormats[texIdx], GrMipMapped::kNo, textureFulfillProc, textureReleaseProc,
                promiseDoneProc, textureContexts[texIdx], version);
        ++proxiesCreated;
        if (!proxy) {
            return nullptr;
        }
        GrSwizzle swizzle = context->priv().caps()->getReadSwizzle(proxy->backendFormat(),
                                                                   colorType);
        views[texIdx] = GrSurfaceProxyView(std::move(proxy), imageOrigin, swizzle);
        proxyColorTypes[texIdx] = colorType;
    }

    return sk_make_sp<SkImage_GpuYUVA>(sk_ref_sp(context), SkISize{imageWidth, imageHeight},
                                       kNeedNewImageUniqueID, yuvColorSpace, views,
                                       proxyColorTypes, numTextures, yuvaIndices, imageOrigin,
                                       std::move(imageColorSpace));
}
