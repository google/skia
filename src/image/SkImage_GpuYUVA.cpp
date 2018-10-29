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
#include "GrRenderTargetContext.h"
#include "GrTexture.h"
#include "SkImage_Gpu.h"
#include "SkImage_GpuYUVA.h"
#include "SkYUVASizeInfo.h"
#include "effects/GrYUVtoRGBEffect.h"

SkImage_GpuYUVA::SkImage_GpuYUVA(sk_sp<GrContext> context, int width, int height, uint32_t uniqueID,
                                 SkYUVColorSpace colorSpace, sk_sp<GrTextureProxy> proxies[],
                                 const SkYUVAIndex yuvaIndices[4],  GrSurfaceOrigin origin,
                                 sk_sp<SkColorSpace> imageColorSpace, SkBudgeted budgeted)
        : INHERITED(std::move(context), width, height, uniqueID,
                    // If an alpha channel is present we always switch to kPremul. This is because,
                    // although the planar data is always un-premul, the final interleaved RGB image
                    // is/would-be premul.
                    -1 != yuvaIndices[SkYUVAIndex::kA_Index].fIndex ? kPremul_SkAlphaType
                                                                    : kOpaque_SkAlphaType,
                    budgeted, imageColorSpace)
        , fYUVColorSpace(colorSpace)
        , fOrigin(origin) {
    int maxIndex = yuvaIndices[0].fIndex;
    for (int i = 1; i < 4; ++i) {
        if (yuvaIndices[i].fIndex > maxIndex) {
            maxIndex = yuvaIndices[i].fIndex;
        }
    }
    for (int i = 0; i <= maxIndex; ++i) {
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

//////////////////////////////////////////////////////////////////////////////////////////////////

sk_sp<GrTextureProxy> SkImage_GpuYUVA::asTextureProxyRef() const {
    if (!fRGBProxy) {
        sk_sp<GrTextureProxy> yProxy = fProxies[fYUVAIndices[SkYUVAIndex::kY_Index].fIndex];
        sk_sp<GrTextureProxy> uProxy = fProxies[fYUVAIndices[SkYUVAIndex::kU_Index].fIndex];
        sk_sp<GrTextureProxy> vProxy = fProxies[fYUVAIndices[SkYUVAIndex::kV_Index].fIndex];

        if (!yProxy || !uProxy || !vProxy) {
            return nullptr;
        }

        GrPaint paint;
        paint.setPorterDuffXPFactory(SkBlendMode::kSrc);

        // TODO: modify the YUVtoRGBEffect to do premul if fImageAlphaType is kPremul_AlphaType
        paint.addColorFragmentProcessor(GrYUVtoRGBEffect::Make(fProxies, fYUVAIndices,
                                                               fYUVColorSpace));

        const SkRect rect = SkRect::MakeIWH(this->width(), this->height());

        // Needs to create a render target in order to draw to it for the yuv->rgb conversion.
        sk_sp<GrRenderTargetContext> renderTargetContext(
            fContext->contextPriv().makeDeferredRenderTargetContext(
                SkBackingFit::kExact, this->width(), this->height(), kRGBA_8888_GrPixelConfig,
                std::move(fColorSpace), 1, GrMipMapped::kNo, fOrigin));
        if (!renderTargetContext) {
            return nullptr;
        }
        renderTargetContext->drawRect(GrNoClip(), std::move(paint), GrAA::kNo, SkMatrix::I(), rect);

        if (!renderTargetContext->asSurfaceProxy()) {
            return nullptr;
        }

        // DDL TODO: in the promise image version we must not flush here
        fContext->contextPriv().flushSurfaceWrites(renderTargetContext->asSurfaceProxy());

        fRGBProxy = renderTargetContext->asTextureProxyRef();
    }

    return fRGBProxy;
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
                            std::move(lazyInstCallback), desc, imageOrigin, GrMipMapped::kNo,
                            GrTextureType::k2D, GrInternalSurfaceFlags::kNone,
                            SkBackingFit::kExact, SkBudgeted::kNo,
                            GrSurfaceProxy::LazyInstantiationType::kUninstantiate);
        if (!proxies[texIdx]) {
            return nullptr;
        }
    }

    return sk_make_sp<SkImage_GpuYUVA>(sk_ref_sp(context), imageWidth, imageHeight,
                                       kNeedNewImageUniqueID, yuvColorSpace, proxies, yuvaIndices,
                                       imageOrigin, std::move(imageColorSpace), SkBudgeted::kNo);
}

