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
                                 SkYUVAIndex yuvaIndices[4], SkISize size, GrSurfaceOrigin origin,
                                 sk_sp<SkColorSpace> imageColorSpace, SkBudgeted budgeted)
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
        // TODO: Modify the fragment processor to sample from different channel
        // instead of taking nv12 bool.
        bool nv12 = (fYUVAIndices[SkYUVAIndex::kU_Index].fIndex ==
                     fYUVAIndices[SkYUVAIndex::kV_Index].fIndex);
        // TODO: modify the YUVtoRGBEffect to do premul if fImageAlphaType is kPremul_AlphaType
        paint.addColorFragmentProcessor(GrYUVtoRGBEffect::Make(std::move(yProxy), std::move(uProxy),
                                                               std::move(vProxy), fYUVColorSpace,
                                                               nv12));

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
        }
    }
    if (!tempTextureProxies[yuvaIndices[SkYUVAIndex::kY_Index].fIndex] ||
        !tempTextureProxies[yuvaIndices[SkYUVAIndex::kU_Index].fIndex] ||
        !tempTextureProxies[yuvaIndices[SkYUVAIndex::kV_Index].fIndex]) {
        return nullptr;
    }

    return sk_make_sp<SkImage_GpuYUVA>(sk_ref_sp(ctx), kNeedNewImageUniqueID, colorSpace,
                                       tempTextureProxies, yuvaIndices, size, origin,
                                       imageColorSpace, SkBudgeted::kYes);
}

/////////////////////////////////////////////////////////////////////////////////////////////////
#if 0
sk_sp<SkImage> SkImage_Gpu::MakePromiseYUVATexture(GrContext* context,
                                                   SkYUVColorSpace yuvColorSpace,
                                                   const GrBackendFormat yuvaFormats[],
                                                   const SkYUVAIndex yuvaIndices[4],
                                                   int imageWidth,
                                                   int imageHeight,
                                                   GrSurfaceOrigin imageOrigin,
                                                   sk_sp<SkColorSpace> imageColorSpace,
                                                   TextureFulfillProc textureFulfillProc,
                                                   TextureReleaseProc textureReleaseProc,
                                                   PromiseDoneProc promiseDoneProc,
                                                   TextureContext textureContexts[]) {
    // DDL TODO: we need to create a SkImage_GpuYUVA here! This implementation just
    // returns the Y-plane.
    if (!context) {
        return nullptr;
    }

    if (imageWidth <= 0 || imageHeight <= 0) {
        return nullptr;
    }

    if (!textureFulfillProc || !textureReleaseProc || !promiseDoneProc) {
        return nullptr;
    }

    SkImageInfo info = SkImageInfo::Make(imageWidth, imageHeight, kRGBA_8888_SkColorType,
                                         kPremul_SkAlphaType, imageColorSpace);
    if (!SkImageInfoIsValid(info)) {
        return nullptr;
    }

    GrProxyProvider* proxyProvider = context->contextPriv().proxyProvider();

    // Determine which of the slots in yuvaFormats and textureContexts are actually used
    bool slotUsed[4] = { false, false, false, false };
    for (int i = 0; i < 4; ++i) {
        if (yuvaIndices[i].fIndex < 0) {
            SkASSERT(SkYUVAIndex::kA_Index == i); // We had better have YUV channels
            continue;
        }

        SkASSERT(yuvaIndices[i].fIndex < 4);
        slotUsed[yuvaIndices[i].fIndex] = true;
    }

    // Temporarily work around an MSVC compiler bug. Copying the arrays directly into the lambda
    // doesn't work on some older tool chains
    struct {
        GrPixelConfig fConfigs[4] = { kUnknown_GrPixelConfig, kUnknown_GrPixelConfig,
            kUnknown_GrPixelConfig, kUnknown_GrPixelConfig };
        PromiseImageHelper fPromiseHelpers[4];
        SkYUVAIndex fLocalIndices[4];
    } params;


    for (int i = 0; i < 4; ++i) {
        params.fLocalIndices[i] = yuvaIndices[i];

        if (slotUsed[i]) {
            params.fPromiseHelpers[i].set(textureFulfillProc, textureReleaseProc,
                                          promiseDoneProc, textureContexts[i]);

            // DDL TODO: This (the kAlpha_8) only works for non-NV12 YUV textures
            if (!context->contextPriv().caps()->getConfigFromBackendFormat(yuvaFormats[i],
                                                                           kAlpha_8_SkColorType,
                                                                           &params.fConfigs[i])) {
                return nullptr;
            }
        }
    }

    GrSurfaceDesc desc;
    desc.fFlags = kNone_GrSurfaceFlags;
    desc.fWidth = imageWidth;
    desc.fHeight = imageHeight;
    // Hack since we're just returning the Y-plane
    desc.fConfig = params.fConfigs[params.fLocalIndices[SkYUVAIndex::kY_Index].fIndex];
    desc.fSampleCnt = 1;


    sk_sp<GrTextureProxy> proxy = proxyProvider->createLazyProxy(
        [params](GrResourceProvider* resourceProvider) mutable {
        if (!resourceProvider) {
            for (int i = 0; i < 4; ++i) {
                if (params.fPromiseHelpers[i].isValid()) {
                    params.fPromiseHelpers[i].reset();
                }
            }
            return sk_sp<GrTexture>();
        }

        // We need to collect the YUVA planes as backend textures (vs. GrTextures) to
        // feed into the SkImage_GpuYUVA factory.
        GrBackendTexture yuvaTextures[4];
        for (int i = 0; i < 4; ++i) {
            if (params.fPromiseHelpers[i].isValid()) {
                sk_sp<GrTexture> tmp = params.fPromiseHelpers[i].getTexture(
                    resourceProvider, params.fConfigs[i]);
                if (!tmp) {
                    return sk_sp<GrTexture>();
                }
                yuvaTextures[i] = tmp->getBackendTexture();
            }
        }

#if 1
        // For the time being, simply return the Y-plane. The reason for this is that
        // this lazy proxy is instantiated at flush time, after the sort, therefore
        // we cannot be introducing a new opList (in order to render the YUV texture).
        int yIndex = params.fLocalIndices[SkYUVAIndex::kY_Index].fIndex;
        return params.fPromiseHelpers[yIndex].getTexture(resourceProvider,
                                                         params.fConfigs[yIndex]);
#else
        GrGpu* gpu = resourceProvider->priv().gpu();
        GrContext* context = gpu->getContext();

        sk_sp<SkImage> tmp = SkImage_Gpu::MakeFromYUVATexturesCopyImpl(context,
                                                                       yuvColorSpace,
                                                                       yuvaTextures,
                                                                       localIndices,
                                                                       imageSize,
                                                                       imageOrigin,
                                                                       imageColorSpace);
        if (!tmp) {
            return sk_sp<GrTexture>();
        }
        return sk_ref_sp<GrTexture>(tmp->getTexture());
#endif
    },
        desc, imageOrigin, GrMipMapped::kNo, GrTextureType::k2D,
        GrInternalSurfaceFlags::kNone, SkBackingFit::kExact, SkBudgeted::kNo,
        GrSurfaceProxy::LazyInstantiationType::kUninstantiate);

    if (!proxy) {
        return nullptr;
    }

    return sk_make_sp<SkImage_Gpu>(sk_ref_sp(context), kNeedNewImageUniqueID, kPremul_SkAlphaType,
                                   std::move(proxy), std::move(imageColorSpace), SkBudgeted::kNo);
}
#endif
