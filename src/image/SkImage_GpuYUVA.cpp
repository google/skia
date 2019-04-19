/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <cstddef>
#include <cstring>
#include <type_traits>

#include "GrClip.h"
#include "GrContext.h"
#include "GrContextPriv.h"
#include "GrGpu.h"
#include "GrRecordingContext.h"
#include "GrRecordingContextPriv.h"
#include "GrRenderTargetContext.h"
#include "GrTexture.h"
#include "GrTextureProducer.h"
#include "SkAutoPixmapStorage.h"
#include "SkGr.h"
#include "SkImage_Gpu.h"
#include "SkImage_GpuYUVA.h"
#include "SkMipMap.h"
#include "SkScopeExit.h"
#include "SkYUVASizeInfo.h"
#include "effects/GrYUVtoRGBEffect.h"

static constexpr auto kAssumedColorType = kRGBA_8888_SkColorType;

SkImage_GpuYUVA::SkImage_GpuYUVA(sk_sp<GrContext> context, int width, int height, uint32_t uniqueID,
                                 SkYUVColorSpace colorSpace, sk_sp<GrTextureProxy> proxies[],
                                 int numProxies, const SkYUVAIndex yuvaIndices[4],
                                 GrSurfaceOrigin origin, sk_sp<SkColorSpace> imageColorSpace)
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

    for (int i = 0; i < fNumProxies; ++i) {
        fProxies[i] = image->fProxies[i];  // we ref in this case, not move
    }
    memcpy(fYUVAIndices, image->fYUVAIndices, 4 * sizeof(SkYUVAIndex));
}

SkImage_GpuYUVA::~SkImage_GpuYUVA() {}

bool SkImage_GpuYUVA::setupMipmapsForPlanes(GrRecordingContext* context) const {
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
            auto mippedProxy = GrCopyBaseMipMapToTextureProxy(context, fProxies[i].get());
            if (!mippedProxy) {
                return false;
            }
            fProxies[i] = mippedProxy;
        }
    }
    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////

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

    const GrBackendFormat format =
        fContext->priv().caps()->getBackendFormatFromColorType(kAssumedColorType);

    // Needs to create a render target in order to draw to it for the yuv->rgb conversion.
    sk_sp<GrRenderTargetContext> renderTargetContext(
            context->priv().makeDeferredRenderTargetContext(
                    format, SkBackingFit::kExact, this->width(), this->height(),
                    kRGBA_8888_GrPixelConfig, this->refColorSpace(), 1, GrMipMapped::kNo, fOrigin));
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
    if (auto mippedProxy = GrCopyBaseMipMapToTextureProxy(context, proxy.get())) {
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

    return sk_make_sp<SkImage_GpuYUVA>(sk_ref_sp(ctx), imageSize.width(), imageSize.height(),
                                       kNeedNewImageUniqueID, colorSpace, tempTextureProxies,
                                       numTextures, yuvaIndices, imageOrigin, imageColorSpace);
}

sk_sp<SkImage> SkImage::MakeFromYUVAPixmaps(
        GrContext* context, SkYUVColorSpace yuvColorSpace, const SkPixmap yuvaPixmaps[],
        const SkYUVAIndex yuvaIndices[4], SkISize imageSize, GrSurfaceOrigin imageOrigin,
        bool buildMips, bool limitToMaxTextureSize, sk_sp<SkColorSpace> imageColorSpace) {
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
    }

    return sk_make_sp<SkImage_GpuYUVA>(sk_ref_sp(context), imageSize.width(), imageSize.height(),
                                       kNeedNewImageUniqueID, yuvColorSpace, tempTextureProxies,
                                       numPixmaps, yuvaIndices, imageOrigin, imageColorSpace);
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
    for (int texIdx = 0; texIdx < numTextures; ++texIdx) {
        GrPixelConfig config =
                context->priv().caps()->getYUVAConfigFromBackendFormat(yuvaFormats[texIdx]);
        if (config == kUnknown_GrPixelConfig) {
            return nullptr;
        }
        proxies[texIdx] = MakePromiseImageLazyProxy(
                context, yuvaSizes[texIdx].width(), yuvaSizes[texIdx].height(), imageOrigin, config,
                yuvaFormats[texIdx], GrMipMapped::kNo, textureFulfillProc, textureReleaseProc,
                promiseDoneProc, textureContexts[texIdx], version);
        ++proxiesCreated;
        if (!proxies[texIdx]) {
            return nullptr;
        }
    }

    return sk_make_sp<SkImage_GpuYUVA>(sk_ref_sp(context), imageWidth, imageHeight,
                                       kNeedNewImageUniqueID, yuvColorSpace, proxies, numTextures,
                                       yuvaIndices, imageOrigin, std::move(imageColorSpace));
}
