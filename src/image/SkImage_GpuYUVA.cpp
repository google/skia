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
#include "effects/GrYUVtoRGBEffect.h"

SkImage_GpuYUVA::SkImage_GpuYUVA(sk_sp<GrContext> context, uint32_t uniqueID,
                                 SkYUVColorSpace colorSpace, sk_sp<GrTextureProxy> proxies[],
                                 const SkYUVAIndex yuvaIndices[4], SkISize size,
                                 GrSurfaceOrigin origin, sk_sp<SkColorSpace> imageColorSpace,
                                 SkBudgeted budgeted)
        : INHERITED(std::move(context), size.width(), size.height(), uniqueID,
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

        // cast to non-const
        (sk_sp<GrTextureProxy>)(fRGBProxy) = renderTargetContext->asTextureProxyRef();
    }

    return fRGBProxy;
}

//////////////////////////////////////////////////////////////////////////////////////////////////

//*** bundle this into a helper function used by this and SkImage_Gpu?
sk_sp<SkImage> SkImage_GpuYUVA::MakeFromYUVATextures(GrContext* ctx,
                                                     SkYUVColorSpace colorSpace,
                                                     const GrBackendTexture yuvaTextures[],
                                                     SkYUVAIndex yuvaIndices[4],
                                                     SkISize size,
                                                     GrSurfaceOrigin origin,
                                                     sk_sp<SkColorSpace> imageColorSpace) {
    GrProxyProvider* proxyProvider = ctx->contextPriv().proxyProvider();

    // Right now this still only deals with YUV and NV12 formats. Assuming that YUV has different
    // textures for U and V planes, while NV12 uses same texture for U and V planes.
    bool nv12 = (yuvaIndices[SkYUVAIndex::kU_Index].fIndex ==
                 yuvaIndices[SkYUVAIndex::kV_Index].fIndex);

    // We need to make a copy of the input backend textures because we need to preserve the result
    // of validate_backend_texture.
    GrBackendTexture yuvaTexturesCopy[4];
    for (int i = 0; i < 4; ++i) {
        // Validate that the yuvaIndices refer to valid backend textures.
        const SkYUVAIndex& yuvaIndex = yuvaIndices[i];
        if (SkYUVAIndex::kA_Index == i && yuvaIndex.fIndex == -1) {
            // Meaning the A plane isn't passed in.
            continue;
        }
        if (yuvaIndex.fIndex == -1 || yuvaIndex.fIndex > 3) {
            // Y plane, U plane, and V plane must refer to image sources being passed in. There are
            // at most 4 image sources being passed in, could not have a index more than 3.
            return nullptr;
        }
        SkColorType ct = kUnknown_SkColorType;
        if (SkYUVAIndex::kY_Index == i || SkYUVAIndex::kA_Index == i) {
            // The Y and A planes are always kAlpha8 (for now)
            ct = kAlpha_8_SkColorType;
        } else {
            // The UV planes can either be interleaved or planar
            ct = nv12 ? kRGBA_8888_SkColorType : kAlpha_8_SkColorType;
        }

        if (!yuvaTexturesCopy[yuvaIndex.fIndex].isValid()) {
            yuvaTexturesCopy[yuvaIndex.fIndex] = yuvaTextures[yuvaIndex.fIndex];
            // TODO: Instead of using assumption about whether it is NV12 format to guess colorType,
            // actually use channel information here.
            if (!ValidateBackendTexture(ctx, yuvaTexturesCopy[yuvaIndex.fIndex],
                                        &yuvaTexturesCopy[yuvaIndex.fIndex].fConfig,
                                        ct, kUnpremul_SkAlphaType, nullptr)) {
                return nullptr;
            }
        }

        // TODO: Check that for each plane, the channel actually exist in the image source we are
        // reading from.
    }

    sk_sp<GrTextureProxy> tempTextureProxies[4]; // build from yuvaTextures
    for (int i = 0; i < 4; ++i) {
        // Fill in tempTextureProxies to avoid duplicate texture proxies.
        int textureIndex = yuvaIndices[i].fIndex;

        // Safely ignore since this means we are missing the A plane.
        if (textureIndex == -1) {
            SkASSERT(SkYUVAIndex::kA_Index == i);
            continue;
        }

        if (!tempTextureProxies[textureIndex]) {
            SkASSERT(yuvaTexturesCopy[textureIndex].isValid());
            tempTextureProxies[textureIndex] =
                proxyProvider->wrapBackendTexture(yuvaTexturesCopy[textureIndex], origin);
            if (!tempTextureProxies[textureIndex]) {
                return nullptr;
            }
        }
    }

    return sk_make_sp<SkImage_GpuYUVA>(sk_ref_sp(ctx), kNeedNewImageUniqueID, colorSpace,
                                       tempTextureProxies, yuvaIndices, size, origin,
                                       imageColorSpace, SkBudgeted::kYes);
}

/////////////////////////////////////////////////////////////////////////////////////////////////
sk_sp<SkImage> SkImage_GpuYUVA::MakePromiseYUVATexture(GrContext* context,
                                                       SkYUVColorSpace yuvColorSpace,
                                                       const GrBackendFormat yuvaFormats[],
                                                       const SkYUVAIndex yuvaIndices[4],
                                                       SkISize size,
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

    if (size.width() <= 0 || size.height() <= 0) {
        return nullptr;
    }

    if (!textureFulfillProc || !textureReleaseProc) {
        return nullptr;
    }

    SkAlphaType at = (-1 != yuvaIndices[SkYUVAIndex::kA_Index].fIndex) ? kPremul_SkAlphaType
                                                                       : kOpaque_SkAlphaType;
    SkImageInfo info = SkImageInfo::Make(size.width(), size.height(), kRGBA_8888_SkColorType,
                                         at, imageColorSpace);
    if (!SkImageInfoIsValid(info)) {
        return nullptr;
    }

    // Set up color types
    SkColorType texColorTypes[4] = { kUnknown_SkColorType, kUnknown_SkColorType,
                                     kUnknown_SkColorType, kUnknown_SkColorType };
    for (int yuvIndex = 0; yuvIndex < 4; ++yuvIndex) {
        int texIdx = yuvaIndices[yuvIndex].fIndex;
        if (texIdx < 0) {
            SkASSERT(SkYUVAIndex::kA_Index);
            continue;
        }
        if (kUnknown_SkColorType == texColorTypes[texIdx]) {
            texColorTypes[texIdx] = kAlpha_8_SkColorType;
        } else {
            texColorTypes[texIdx] = kRGBA_8888_SkColorType;
        }
    }
    // If UV is interleaved, then Y will have RGBA color type
    if (kRGBA_8888_SkColorType == texColorTypes[yuvaIndices[SkYUVAIndex::kU_Index].fIndex]) {
        texColorTypes[yuvaIndices[SkYUVAIndex::kY_Index].fIndex] = kRGBA_8888_SkColorType;
    }

    // Get lazy proxies
    GrProxyProvider* proxyProvider = context->contextPriv().proxyProvider();
    GrSurfaceDesc desc;
    desc.fFlags = kNone_GrSurfaceFlags;
    desc.fWidth = size.width();
    desc.fHeight = size.height();
    desc.fConfig = kUnknown_GrPixelConfig;  // We'll replace this for each proxy.
    desc.fSampleCnt = 1;
    sk_sp<GrTextureProxy> proxies[4];
    for (int texIdx = 0; texIdx < numTextures; ++texIdx) {
        // for each proxy we need to fill in
        struct {
            GrPixelConfig fConfig;
            SkPromiseImageHelper fPromiseHelper;
        } params;
        if (!context->contextPriv().caps()->getConfigFromBackendFormat(yuvaFormats[texIdx],
                                                                       texColorTypes[texIdx],
                                                                       &params.fConfig)) {
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
        desc.fConfig = params.fConfig;
        proxies[texIdx] = proxyProvider->createLazyProxy(
                            std::move(lazyInstCallback), desc, imageOrigin, GrMipMapped::kNo,
                            GrTextureType::k2D, GrInternalSurfaceFlags::kNone,
                            SkBackingFit::kExact, SkBudgeted::kNo,
                            GrSurfaceProxy::LazyInstantiationType::kUninstantiate);
        if (!proxies[texIdx]) {
            return nullptr;
        }
    }

    return sk_make_sp<SkImage_GpuYUVA>(sk_ref_sp(context), kNeedNewImageUniqueID, yuvColorSpace,
                                       proxies, yuvaIndices, size, imageOrigin,
                                       std::move(imageColorSpace), SkBudgeted::kNo);
}

