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
#include "include/gpu/GrTexture.h"
#include "include/private/GrRecordingContext.h"
#include "src/core/SkAutoPixmapStorage.h"
#include "src/core/SkMipMap.h"
#include "src/core/SkScopeExit.h"
#include "src/gpu/GrClip.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrGpu.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/GrRenderTargetContext.h"
#include "src/gpu/GrTextureProducer.h"
#include "src/gpu/SkGr.h"
#include "src/gpu/effects/GrYUVtoRGBEffect.h"
#include "src/image/SkImage_Gpu.h"
#include "src/image/SkImage_GpuYUVA.h"

static constexpr auto kAssumedColorType = kRGBA_8888_SkColorType;

SkImage_GpuYUVA::SkImage_GpuYUVA(sk_sp<GrContext> context, int width, int height, uint32_t uniqueID,
                                 SkYUVColorSpace colorSpace, sk_sp<GrTextureProxy> proxies[],
                                 GrColorType proxyColorTypes[], int numProxies,
                                 const SkYUVAIndex yuvaIndices[4], GrSurfaceOrigin origin,
                                 sk_sp<SkColorSpace> imageColorSpace)
        : INHERITED(std::move(context), width, height, uniqueID, kAssumedColorType,
                    // If an alpha channel is present we always switch to kPremul. This is because,
                    // although the planar data is always un-premul, the final interleaved RGB image
                    // is/would-be premul.
                    GetAlphaTypeFromYUVAIndices(yuvaIndices), std::move(imageColorSpace))
        , fNumProxies(numProxies)
        , fYUVColorSpace(colorSpace)
        , fOrigin(origin) {
    // The caller should have done this work, just verifying
    SkDEBUGCODE(int textureCount;)
    SkASSERT(SkYUVAIndex::AreValidIndices(yuvaIndices, &textureCount));
    SkASSERT(textureCount == fNumProxies);

    for (int i = 0; i < numProxies; ++i) {
        fProxies[i] = std::move(proxies[i]);
        fProxyColorTypes[i] = proxyColorTypes[i];
    }
    memcpy(fYUVAIndices, yuvaIndices, 4*sizeof(SkYUVAIndex));
}

// For onMakeColorSpace()
SkImage_GpuYUVA::SkImage_GpuYUVA(const SkImage_GpuYUVA* image, sk_sp<SkColorSpace> targetCS)
        : INHERITED(image->fContext, image->width(), image->height(), kNeedNewImageUniqueID,
                    kAssumedColorType,
                    // If an alpha channel is present we always switch to kPremul. This is because,
                    // although the planar data is always un-premul, the final interleaved RGB image
                    // is/would-be premul.
                    GetAlphaTypeFromYUVAIndices(image->fYUVAIndices), std::move(targetCS))
        , fNumProxies(image->fNumProxies)
        , fYUVColorSpace(image->fYUVColorSpace)
        , fOrigin(image->fOrigin)
        // Since null fFromColorSpace means no GrColorSpaceXform, we turn a null
        // image->refColorSpace() into an explicit SRGB.
        , fFromColorSpace(image->colorSpace() ? image->refColorSpace() : SkColorSpace::MakeSRGB()) {
    // The caller should have done this work, just verifying
    SkDEBUGCODE(int textureCount;)
        SkASSERT(SkYUVAIndex::AreValidIndices(image->fYUVAIndices, &textureCount));
    SkASSERT(textureCount == fNumProxies);

    if (image->fRGBProxy) {
        fRGBProxy = image->fRGBProxy;  // we ref in this case, not move
    } else {
        for (int i = 0; i < fNumProxies; ++i) {
            fProxies[i] = image->fProxies[i];  // we ref in this case, not move
            fProxyColorTypes[i] = image->fProxyColorTypes[i];
        }
    }
    memcpy(fYUVAIndices, image->fYUVAIndices, 4 * sizeof(SkYUVAIndex));
}

SkImage_GpuYUVA::~SkImage_GpuYUVA() {}

bool SkImage_GpuYUVA::setupMipmapsForPlanes(GrRecordingContext* context) const {
    // We shouldn't get here if the planes were already flattened to RGBA.
    SkASSERT(fProxies[0] && !fRGBProxy);
    if (!context || !fContext->priv().matches(context)) {
        return false;
    }

    for (int i = 0; i < fNumProxies; ++i) {
        GrTextureProducer::CopyParams copyParams;
        int mipCount = SkMipMap::ComputeLevelCount(fProxies[i]->width(), fProxies[i]->height());
        if (mipCount && GrGpu::IsACopyNeededForMips(fContext->priv().caps(),
                                                    fProxies[i].get(),
                                                    GrSamplerState::Filter::kMipMap,
                                                    &copyParams)) {
            auto mippedProxy = GrCopyBaseMipMapToTextureProxy(context, fProxies[i].get(),
                                                              fProxyColorTypes[i]);
            if (!mippedProxy) {
                return false;
            }
            fProxies[i] = mippedProxy;
        }
    }
    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////

GrSemaphoresSubmitted SkImage_GpuYUVA::onFlush(GrContext* context, const GrFlushInfo& info) {
    if (!context || !fContext->priv().matches(context) || fContext->abandoned()) {
        return GrSemaphoresSubmitted::kNo;
    }

    GrSurfaceProxy* proxies[4] = {fProxies[0].get(), fProxies[1].get(),
                                  fProxies[2].get(), fProxies[3].get()};
    int numProxies = fNumProxies;
    if (fRGBProxy) {
        // Either we've already flushed the flattening draw or the flattening is unflushed. In the
        // latter case it should still be ok to just pass fRGBProxy because it in turn depends on
        // the planar proxies and will cause all of their work to flush as well.
        proxies[0] = fRGBProxy.get();
        numProxies = 1;
    }
    return context->priv().flushSurfaces(proxies, numProxies, info);
}

GrTextureProxy* SkImage_GpuYUVA::peekProxy() const {
    return fRGBProxy.get();
}

sk_sp<GrTextureProxy> SkImage_GpuYUVA::asTextureProxyRef(GrRecordingContext* context) const {
    if (fRGBProxy) {
        return fRGBProxy;
    }

    if (!context || !fContext->priv().matches(context)) {
        return nullptr;
    }

    // Needs to create a render target in order to draw to it for the yuv->rgb conversion.
    auto renderTargetContext = context->priv().makeDeferredRenderTargetContext(
            SkBackingFit::kExact, this->width(), this->height(), GrColorType::kRGBA_8888,
            this->refColorSpace(), 1, GrMipMapped::kNo, fOrigin);
    if (!renderTargetContext) {
        return nullptr;
    }

    sk_sp<GrColorSpaceXform> colorSpaceXform;
    if (fFromColorSpace) {
        colorSpaceXform = GrColorSpaceXform::Make(fFromColorSpace.get(), this->alphaType(),
                                                  this->colorSpace(), this->alphaType());
    }
    const SkRect rect = SkRect::MakeIWH(this->width(), this->height());
    if (!RenderYUVAToRGBA(fContext.get(), renderTargetContext.get(), rect, fYUVColorSpace,
                          std::move(colorSpaceXform), fProxies, fYUVAIndices)) {
        return nullptr;
    }

    fRGBProxy = renderTargetContext->asTextureProxyRef();
    for (auto& p : fProxies) {
        p.reset();
    }
    return fRGBProxy;
}

sk_sp<GrTextureProxy> SkImage_GpuYUVA::asMippedTextureProxyRef(GrRecordingContext* context) const {
    if (!context || !fContext->priv().matches(context)) {
        return nullptr;
    }

    // if invalid or already has miplevels
    auto proxy = this->asTextureProxyRef(context);
    if (!proxy || GrMipMapped::kYes == fRGBProxy->mipMapped()) {
        return proxy;
    }

    // need to generate mips for the proxy
    GrColorType srcColorType = SkColorTypeToGrColorType(this->colorType());
    if (auto mippedProxy = GrCopyBaseMipMapToTextureProxy(context, proxy.get(), srcColorType)) {
        fRGBProxy = mippedProxy;
        return mippedProxy;
    }

    // failed to generate mips
    return nullptr;
}

//////////////////////////////////////////////////////////////////////////////////////////////////

sk_sp<SkImage> SkImage_GpuYUVA::onMakeColorTypeAndColorSpace(GrRecordingContext*,
                                                             SkColorType,
                                                             sk_sp<SkColorSpace> targetCS) const {
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
    return sk_make_sp<SkImage_GpuYUVA>(fContext, this->width(), this->height(),
                                       kNeedNewImageUniqueID, fYUVColorSpace, fProxies,
                                       fProxyColorTypes, fNumProxies, fYUVAIndices, fOrigin,
                                       std::move(newCS));
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

    sk_sp<GrTextureProxy> tempTextureProxies[4];
    if (!SkImage_GpuBase::MakeTempTextureProxies(ctx, yuvaTextures, numTextures, yuvaIndices,
                                                 imageOrigin, tempTextureProxies)) {
        return nullptr;
    }
    GrColorType proxyColorTypes[4];
    for (int i = 0; i < numTextures; ++i) {
        proxyColorTypes[i] = ctx->priv().caps()->getYUVAColorTypeFromBackendFormat(
                yuvaTextures[i].getBackendFormat(), yuvaIndices[3].fIndex == i);
    }

    return sk_make_sp<SkImage_GpuYUVA>(sk_ref_sp(ctx), imageSize.width(), imageSize.height(),
                                       kNeedNewImageUniqueID, colorSpace, tempTextureProxies,
                                       proxyColorTypes, numTextures, yuvaIndices, imageOrigin,
                                       imageColorSpace);
}

sk_sp<SkImage> SkImage::MakeFromYUVAPixmaps(
        GrContext* context, SkYUVColorSpace yuvColorSpace, const SkPixmap yuvaPixmaps[],
        const SkYUVAIndex yuvaIndices[4], SkISize imageSize, GrSurfaceOrigin imageOrigin,
        bool buildMips, bool limitToMaxTextureSize, sk_sp<SkColorSpace> imageColorSpace) {
    if (!context) {
        return nullptr; // until we impl this for raster backend
    }

    int numPixmaps;
    if (!SkYUVAIndex::AreValidIndices(yuvaIndices, &numPixmaps)) {
        return nullptr;
    }

    if (!context->priv().caps()->mipMapSupport()) {
        buildMips = false;
    }

    // Make proxies
    GrProxyProvider* proxyProvider = context->priv().proxyProvider();
    sk_sp<GrTextureProxy> tempTextureProxies[4];
    GrColorType proxyColorTypes[4];
    for (int i = 0; i < numPixmaps; ++i) {
        const SkPixmap* pixmap = &yuvaPixmaps[i];
        SkAutoPixmapStorage resized;
        int maxTextureSize = context->priv().caps()->maxTextureSize();
        int maxDim = SkTMax(yuvaPixmaps[i].width(), yuvaPixmaps[i].height());
        if (limitToMaxTextureSize && maxDim > maxTextureSize) {
            float scale = static_cast<float>(maxTextureSize) / maxDim;
            int newWidth = SkTMin(static_cast<int>(yuvaPixmaps[i].width() * scale),
                                  maxTextureSize);
            int newHeight = SkTMin(static_cast<int>(yuvaPixmaps[i].height() * scale),
                                   maxTextureSize);
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
        GrMipMapped mipMapped = buildMips ? GrMipMapped::kYes : GrMipMapped::kNo;
        tempTextureProxies[i] = proxyProvider->createProxyFromBitmap(bmp, mipMapped);
        if (!tempTextureProxies[i]) {
            return nullptr;
        }
        proxyColorTypes[i] = SkColorTypeToGrColorType(bmp.colorType());
    }

    return sk_make_sp<SkImage_GpuYUVA>(sk_ref_sp(context), imageSize.width(), imageSize.height(),
                                       kNeedNewImageUniqueID, yuvColorSpace, tempTextureProxies,
                                       proxyColorTypes, numPixmaps, yuvaIndices, imageOrigin,
                                       imageColorSpace);
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
    SkImageInfo info = SkImageInfo::Make(imageWidth, imageHeight, kAssumedColorType,
                                         at, imageColorSpace);
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
    sk_sp<GrTextureProxy> proxies[4];
    GrColorType proxyColorTypes[4];
    for (int texIdx = 0; texIdx < numTextures; ++texIdx) {
        GrColorType colorType = context->priv().caps()->getYUVAColorTypeFromBackendFormat(
                                                                yuvaFormats[texIdx],
                                                                yuvaIndices[3].fIndex == texIdx);
        if (GrColorType::kUnknown == colorType) {
            return nullptr;
        }

        proxies[texIdx] = MakePromiseImageLazyProxy(
                context, yuvaSizes[texIdx].width(), yuvaSizes[texIdx].height(), imageOrigin,
                colorType, yuvaFormats[texIdx], GrMipMapped::kNo, textureFulfillProc,
                textureReleaseProc, promiseDoneProc, textureContexts[texIdx], version);
        ++proxiesCreated;
        if (!proxies[texIdx]) {
            return nullptr;
        }
        proxyColorTypes[texIdx] = colorType;
    }

    return sk_make_sp<SkImage_GpuYUVA>(sk_ref_sp(context), imageWidth, imageHeight,
                                       kNeedNewImageUniqueID, yuvColorSpace, proxies,
                                       proxyColorTypes, numTextures, yuvaIndices, imageOrigin,
                                       std::move(imageColorSpace));
}
