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
#include "GrRenderTargetContext.h"
#include "GrTexture.h"
#include "GrTextureProducer.h"
#include "SkAutoPixmapStorage.h"
#include "SkGr.h"
#include "SkImage_Gpu.h"
#include "SkImage_GpuYUVA.h"
#include "SkMipMap.h"
#include "SkYUVASizeInfo.h"
#include "effects/GrYUVtoRGBEffect.h"

SkImage_GpuYUVA::SkImage_GpuYUVA(sk_sp<GrContext> context, int width, int height, uint32_t uniqueID,
                                 SkYUVColorSpace colorSpace, sk_sp<GrTextureProxy> proxies[],
                                 int numProxies, const SkYUVAIndex yuvaIndices[4],
                                 GrSurfaceOrigin origin, sk_sp<SkColorSpace> imageColorSpace,
                                 SkBudgeted budgeted)
        : INHERITED(std::move(context), width, height, uniqueID,
                    // If an alpha channel is present we always switch to kPremul. This is because,
                    // although the planar data is always un-premul, the final interleaved RGB image
                    // is/would-be premul.
                    GetAlphaTypeFromYUVAIndices(yuvaIndices),
                    budgeted, imageColorSpace)
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

SkImage_GpuYUVA::~SkImage_GpuYUVA() {}

SkImageInfo SkImage_GpuYUVA::onImageInfo() const {
    // Note: this is the imageInfo for the flattened image, not the YUV planes
    return SkImageInfo::Make(this->width(), this->height(), kRGBA_8888_SkColorType,
                             fAlphaType, fColorSpace);
}

bool SkImage_GpuYUVA::setupMipmapsForPlanes() const {
    for (int i = 0; i < fNumProxies; ++i) {
        GrTextureProducer::CopyParams copyParams;
        int mipCount = SkMipMap::ComputeLevelCount(fProxies[i]->width(), fProxies[i]->height());
        if (mipCount && GrGpu::IsACopyNeededForMips(fContext->contextPriv().caps(),
                                                    fProxies[i].get(),
                                                    GrSamplerState::Filter::kMipMap,
                                                    &copyParams)) {
            auto mippedProxy = GrCopyBaseMipMapToTextureProxy(fContext.get(), fProxies[i].get());
            if (!mippedProxy) {
                return false;
            }
            fProxies[i] = mippedProxy;
        }
    }
    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////

sk_sp<GrTextureProxy> SkImage_GpuYUVA::asTextureProxyRef() const {
    if (!fRGBProxy) {
        const GrBackendFormat format =
            fContext->contextPriv().caps()->getBackendFormatFromColorType(kRGBA_8888_SkColorType);

        // Needs to create a render target in order to draw to it for the yuv->rgb conversion.
        sk_sp<GrRenderTargetContext> renderTargetContext(
            fContext->contextPriv().makeDeferredRenderTargetContext(
                format, SkBackingFit::kExact, this->width(), this->height(),
                kRGBA_8888_GrPixelConfig, fColorSpace, 1, GrMipMapped::kNo, fOrigin));
        if (!renderTargetContext) {
            return nullptr;
        }

        const SkRect rect = SkRect::MakeIWH(this->width(), this->height());
        if (!RenderYUVAToRGBA(fContext.get(), renderTargetContext.get(), rect, fYUVColorSpace,
                              fProxies, fYUVAIndices)) {
            return nullptr;
        }

        fRGBProxy = renderTargetContext->asTextureProxyRef();
    }

    return fRGBProxy;
}

sk_sp<GrTextureProxy> SkImage_GpuYUVA::asMippedTextureProxyRef() const {
    // if invalid or already has miplevels
    auto proxy = this->asTextureProxyRef();
    if (!proxy || GrMipMapped::kYes == fRGBProxy->mipMapped()) {
        return proxy;
    }

    // need to generate mips for the proxy
    if (auto mippedProxy = GrCopyBaseMipMapToTextureProxy(fContext.get(), proxy.get())) {
        fRGBProxy = mippedProxy;
        return mippedProxy;
    }

    // failed to generate mips
    return nullptr;
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
                                       numTextures, yuvaIndices, imageOrigin, imageColorSpace,
                                       SkBudgeted::kYes);
}

sk_sp<SkImage> SkImage::MakeFromYUVAPixmaps(
        GrContext* context, SkYUVColorSpace yuvColorSpace, const SkPixmap yuvaPixmaps[],
        const SkYUVAIndex yuvaIndices[4], SkISize imageSize, GrSurfaceOrigin imageOrigin,
        bool buildMips, bool limitToMaxTextureSize, sk_sp<SkColorSpace> imageColorSpace) {
    int numPixmaps;
    if (!SkYUVAIndex::AreValidIndices(yuvaIndices, &numPixmaps)) {
        return nullptr;
    }

    // Make proxies
    GrProxyProvider* proxyProvider = context->contextPriv().proxyProvider();
    sk_sp<GrTextureProxy> tempTextureProxies[4];
    for (int i = 0; i < numPixmaps; ++i) {
        const SkPixmap* pixmap = &yuvaPixmaps[i];
        SkAutoPixmapStorage resized;
        int maxTextureSize = context->contextPriv().caps()->maxTextureSize();
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
        if (buildMips) {
            SkBitmap bmp;
            bmp.installPixels(*pixmap);
            tempTextureProxies[i] = proxyProvider->createMipMapProxyFromBitmap(bmp);
        } else {
            if (SkImageInfoIsValid(pixmap->info())) {
                ATRACE_ANDROID_FRAMEWORK("Upload Texture [%ux%u]",
                                         pixmap->width(), pixmap->height());
                // We don't need a release proc on the data in pixmap since we know we are in a
                // GrContext that has a resource provider. Thus the createTextureProxy call will
                // immediately upload the data.
                sk_sp<SkImage> image = SkImage::MakeFromRaster(*pixmap, nullptr, nullptr);
                tempTextureProxies[i] =
                        proxyProvider->createTextureProxy(std::move(image), kNone_GrSurfaceFlags, 1,
                                                          SkBudgeted::kYes, SkBackingFit::kExact);
            }
        }

        if (!tempTextureProxies[i]) {
            return nullptr;
        }
    }

    return sk_make_sp<SkImage_GpuYUVA>(sk_ref_sp(context), imageSize.width(), imageSize.height(),
                                       kNeedNewImageUniqueID, yuvColorSpace, tempTextureProxies,
                                       numPixmaps, yuvaIndices, imageOrigin, imageColorSpace,
                                       SkBudgeted::kYes);
}


/////////////////////////////////////////////////////////////////////////////////////////////////
sk_sp<SkImage> SkImage_GpuYUVA::MakePromiseYUVATexture(GrContext* context,
                                                       SkYUVColorSpace yuvColorSpace,
                                                       const GrBackendFormat yuvaFormats[],
                                                       const SkISize yuvaSizes[],
                                                       const SkYUVAIndex yuvaIndices[4],
                                                       int imageWidth,
                                                       int imageHeight,
                                                       GrSurfaceOrigin imageOrigin,
                                                       sk_sp<SkColorSpace> imageColorSpace,
                                                       TextureFulfillProc textureFulfillProc,
                                                       TextureReleaseProc textureReleaseProc,
                                                       PromiseDoneProc promiseDoneProc,
                                                       TextureContext textureContexts[]) {
    // The contract here is that if 'promiseDoneProc' is passed in it should always be called,
    // even if creation of the SkImage fails.
    if (!promiseDoneProc) {
        return nullptr;
    }

    int numTextures;
    bool valid = SkYUVAIndex::AreValidIndices(yuvaIndices, &numTextures);

    // Set up promise helpers
    SkPromiseImageHelper promiseHelpers[4];
    for (int texIdx = 0; texIdx < numTextures; ++texIdx) {
        promiseHelpers[texIdx].set(textureFulfillProc, textureReleaseProc, promiseDoneProc,
                                   textureContexts[texIdx]);
    }

    if (!valid) {
        return nullptr;
    }

    if (!context) {
        return nullptr;
    }

    if (imageWidth <= 0 || imageWidth <= 0) {
        return nullptr;
    }

    if (!textureFulfillProc || !textureReleaseProc) {
        return nullptr;
    }

    SkAlphaType at = (-1 != yuvaIndices[SkYUVAIndex::kA_Index].fIndex) ? kPremul_SkAlphaType
                                                                       : kOpaque_SkAlphaType;
    SkImageInfo info = SkImageInfo::Make(imageWidth, imageHeight, kRGBA_8888_SkColorType,
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
    GrProxyProvider* proxyProvider = context->contextPriv().proxyProvider();
    sk_sp<GrTextureProxy> proxies[4];
    for (int texIdx = 0; texIdx < numTextures; ++texIdx) {
        // for each proxy we need to fill in
        struct {
            GrPixelConfig fConfig;
            SkPromiseImageHelper fPromiseHelper;
        } params;
        bool res = context->contextPriv().caps()->getYUVAConfigFromBackendFormat(
                       yuvaFormats[texIdx],
                       &params.fConfig);
        if (!res) {
            return nullptr;
        }
        params.fPromiseHelper = promiseHelpers[texIdx];

        GrProxyProvider::LazyInstantiateCallback lazyInstCallback =
            [params](GrResourceProvider* resourceProvider) mutable {
            if (!resourceProvider || !params.fPromiseHelper.isValid()) {
                if (params.fPromiseHelper.isValid()) {
                    params.fPromiseHelper.reset();
                }
                return sk_sp<GrTexture>();
            }

            return params.fPromiseHelper.getTexture(resourceProvider, params.fConfig);
        };
        GrSurfaceDesc desc;
        desc.fFlags = kNone_GrSurfaceFlags;
        desc.fWidth = yuvaSizes[texIdx].width();
        desc.fHeight = yuvaSizes[texIdx].height();
        desc.fConfig = params.fConfig;
        desc.fSampleCnt = 1;
        proxies[texIdx] = proxyProvider->createLazyProxy(
                            std::move(lazyInstCallback), yuvaFormats[texIdx], desc, imageOrigin,
                            GrMipMapped::kNo, GrInternalSurfaceFlags::kNone,
                            SkBackingFit::kExact, SkBudgeted::kNo,
                            GrSurfaceProxy::LazyInstantiationType::kUninstantiate);
        if (!proxies[texIdx]) {
            return nullptr;
        }
    }

    return sk_make_sp<SkImage_GpuYUVA>(sk_ref_sp(context), imageWidth, imageHeight,
                                       kNeedNewImageUniqueID, yuvColorSpace, proxies, numTextures,
                                       yuvaIndices, imageOrigin, std::move(imageColorSpace),
                                       SkBudgeted::kNo);
}

