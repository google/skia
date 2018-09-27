/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <cstddef>
#include <cstring>
#include <type_traits>

#include "GrAHardwareBufferImageGenerator.h"
#include "GrBackendSurface.h"
#include "GrBackendTextureImageGenerator.h"
#include "GrBitmapTextureMaker.h"
#include "GrCaps.h"
#include "GrColorSpaceXform.h"
#include "GrContext.h"
#include "GrContextPriv.h"
#include "GrGpu.h"
#include "GrImageTextureMaker.h"
#include "GrProxyProvider.h"
#include "GrRenderTargetContext.h"
#include "GrResourceProvider.h"
#include "GrSemaphore.h"
#include "GrSurfacePriv.h"
#include "GrTexture.h"
#include "GrTextureAdjuster.h"
#include "GrTexturePriv.h"
#include "GrTextureProxy.h"
#include "GrTextureProxyPriv.h"
#include "SkAutoPixmapStorage.h"
#include "SkBitmapCache.h"
#include "SkCanvas.h"
#include "SkGr.h"
#include "SkImageInfoPriv.h"
#include "SkImage_GpuYUV.h"
#include "SkMipMap.h"
#include "SkPixelRef.h"
#include "SkReadPixelsRec.h"
#include "SkTraceEvent.h"
#include "effects/GrYUVtoRGBEffect.h"
#include "gl/GrGLTexture.h"

SkImage_GpuYUV::SkImage_GpuYUV(sk_sp<GrContext> context, uint32_t uniqueID, SkAlphaType at,
                               SkYUVColorSpace colorSpace, SkTDArray<sk_sp<GrTextureProxy>> proxies,
                               SkYUVAIndex yuvaIndices[4], SkISize size,
                               sk_sp<SkColorSpace> imageColorSpace, SkBudgeted budgeted)
        : INHERITED(proxies[0]->worstCaseWidth(), proxies[0]->worstCaseHeight(), uniqueID)
        , fContext(std::move(context))
        , fProxies(std::move(proxies))
        , fSize(size)
        , fAlphaType(at)
        , fBudgeted(budgeted)
        , fColorSpace(colorSpace)
        , fImageColorSpace(std::move(imageColorSpace)) {
    memcpy(fYUVAIndices, yuvaIndices, 4*sizeof(SkYUVAIndex));
}

SkImage_GpuYUV::~SkImage_GpuYUV() {}

SkImageInfo SkImage_GpuYUV::onImageInfo() const {
    return SkImageInfo::Make(fSize.width(), fSize.height(), this->onColorType(), fAlphaType,
                             fImageColorSpace);
}

//////////////////////////////////////////////////////////////////////////////////////////////////
// need shared defs with SkImage_Gpu

static bool validate_backend_texture(GrContext* ctx, const GrBackendTexture& tex,
                                     GrPixelConfig* config, SkColorType ct, SkAlphaType at,
                                     sk_sp<SkColorSpace> cs) {
    if (!tex.isValid()) {
        return false;
    }
    // TODO: Create a SkImageColorInfo struct for color, alpha, and color space so we don't need to
    // create a fake image info here.
    SkImageInfo info = SkImageInfo::Make(1, 1, ct, at, cs);
    if (!SkImageInfoIsValid(info)) {
        return false;
    }

    return ctx->contextPriv().caps()->validateBackendTexture(tex, ct, config);
}

static void apply_premul(const SkImageInfo& info, void* pixels, size_t rowBytes) {
    switch (info.colorType()) {
    case kRGBA_8888_SkColorType:
    case kBGRA_8888_SkColorType:
        break;
    default:
        return; // nothing to do
    }

    // SkColor is not necesarily RGBA or BGRA, but it is one of them on little-endian,
    // and in either case, the alpha-byte is always in the same place, so we can safely call
    // SkPreMultiplyColor()
    //
    SkColor* row = (SkColor*)pixels;
    for (int y = 0; y < info.height(); ++y) {
        for (int x = 0; x < info.width(); ++x) {
            row[x] = SkPreMultiplyColor(row[x]);
        }
        row = (SkColor*)((char*)(row)+rowBytes);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////

bool SkImage_GpuYUV::onReadPixels(const SkImageInfo& dstInfo, void* dstPixels, size_t dstRB,
                                  int srcX, int srcY, CachingHint) const {
    if (!fContext->contextPriv().resourceProvider()) {
        // DDL TODO: buffer up the readback so it occurs when the DDL is drawn?
        return false;
    }

    if (!SkImageInfoValidConversion(dstInfo, this->onImageInfo())) {
        return false;
    }

    SkReadPixelsRec rec(dstInfo, dstPixels, dstRB, srcX, srcY);
    if (!rec.trim(this->width(), this->height())) {
        return false;
    }

    //*** render the planes into one proxy (put this into a helper routine)
    sk_sp<GrTextureProxy> yProxy = fProxies[fYUVAIndices[0].fIndex];
    sk_sp<GrTextureProxy> uProxy = fProxies[fYUVAIndices[1].fIndex];
    sk_sp<GrTextureProxy> vProxy = fProxies[fYUVAIndices[2].fIndex];

    if (!yProxy || !uProxy || !vProxy) {
        return false;
    }

    GrPaint paint;
    paint.setPorterDuffXPFactory(SkBlendMode::kSrc);
    // TODO: Modify the fragment processor to sample from different channel instead of taking nv12
    // bool.
    bool nv12 = (fYUVAIndices[1].fIndex == fYUVAIndices[2].fIndex);
    paint.addColorFragmentProcessor(GrYUVtoRGBEffect::Make(std::move(yProxy), std::move(uProxy),
                                                           std::move(vProxy), fColorSpace, nv12));

    const SkRect rect = SkRect::MakeIWH(this->width(), this->height());

    // Needs to create a render target in order to draw to it for the yuv->rgb conversion.
    sk_sp<GrRenderTargetContext> renderTargetContext(
        fContext->contextPriv().makeDeferredRenderTargetContext(
            SkBackingFit::kExact, this->width(), this->height(), kRGBA_8888_GrPixelConfig,
            std::move(fImageColorSpace), 1, GrMipMapped::kNo, yProxy->origin()));
    if (!renderTargetContext) {
        return nullptr;
    }
    renderTargetContext->drawRect(GrNoClip(), std::move(paint), GrAA::kNo, SkMatrix::I(), rect);
    sk_sp<GrSurfaceProxy> proxy(renderTargetContext->asSurfaceProxy());
    if (!proxy) {
        return nullptr;
    }

    // DDL TODO: in the promise image version we must not flush here
    fContext->contextPriv().flushSurfaceWrites(renderTargetContext->asSurfaceProxy());


    // TODO: this seems to duplicate code in GrTextureContext::onReadPixels and
    // GrRenderTargetContext::onReadPixels
    uint32_t flags = 0;
    if (kUnpremul_SkAlphaType == rec.fInfo.alphaType() && kPremul_SkAlphaType == fAlphaType) {
        // let the GPU perform this transformation for us
        flags = GrContextPriv::kUnpremul_PixelOpsFlag;
    }

    sk_sp<GrSurfaceContext> sContext = fContext->contextPriv().makeWrappedSurfaceContext(
        proxy, fImageColorSpace);
    if (!sContext) {
        return false;
    }

    if (!sContext->readPixels(rec.fInfo, rec.fPixels, rec.fRowBytes, rec.fX, rec.fY, flags)) {
        return false;
    }

    // do we have to manually fix-up the alpha channel?
    //      src         dst
    //      unpremul    premul      fix manually
    //      premul      unpremul    done by kUnpremul_PixelOpsFlag
    // all other combos need to change.
    //
    // Should this be handled by Ganesh? todo:?
    //
    if (kPremul_SkAlphaType == rec.fInfo.alphaType() && kUnpremul_SkAlphaType == fAlphaType) {
        apply_premul(rec.fInfo, rec.fPixels, rec.fRowBytes);
    }
    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////

//*** bundle this into a helper function used by this and SkImage_Gpu?
sk_sp<SkImage> SkImage_GpuYUV::MakeFromYUVATextures(GrContext* ctx,
                                                    SkYUVColorSpace colorSpace,
                                                    const GrBackendTexture yuvaTextures[],
                                                    SkYUVAIndex yuvaIndices[4],
                                                    SkISize size,
                                                    GrSurfaceOrigin origin,
                                                    sk_sp<SkColorSpace> imageColorSpace) {
    GrProxyProvider* proxyProvider = ctx->contextPriv().proxyProvider();

    // Right now this still only deals with YUV and NV12 formats. Assuming that YUV has different
    // textures for U and V planes, while NV12 uses same texture for U and V planes.
    bool nv12 = (yuvaIndices[1].fIndex == yuvaIndices[2].fIndex);
    auto ct = nv12 ? kRGBA_8888_SkColorType : kAlpha_8_SkColorType;

    // We need to make a copy of the input backend textures because we need to preserve the result
    // of validate_backend_texture.
    GrBackendTexture yuvaTexturesCopy[4];

    int textureProxyCount = 0;
    for (int i = 0; i < 4; ++i) {
        // Validate that the yuvaIndices refer to valid backend textures.
        const SkYUVAIndex& yuvaIndex = yuvaIndices[i];
        if (i == 3 && yuvaIndex.fIndex == -1) {
            // Meaning the A plane isn't passed in.
            continue;
        }
        if (yuvaIndex.fIndex == -1 || yuvaIndex.fIndex > 3) {
            // Y plane, U plane, and V plane must refer to image sources being passed in. There are
            // at most 4 image sources being passed in, could not have a index more than 3.
            return nullptr;
        }
        if (!yuvaTexturesCopy[yuvaIndex.fIndex].isValid()) {
            yuvaTexturesCopy[yuvaIndex.fIndex] = yuvaTextures[yuvaIndex.fIndex];
            // TODO: Instead of using assumption about whether it is NV12 format to guess colorType,
            // actually use channel information here.
            if (!validate_backend_texture(ctx, yuvaTexturesCopy[i], &yuvaTexturesCopy[i].fConfig,
                                          ct, kPremul_SkAlphaType, nullptr)) {
                return nullptr;
            }
        }
        if (yuvaIndex.fIndex > textureProxyCount) {
            textureProxyCount = yuvaIndex.fIndex;
        }

        // TODO: Check that for each plane, the channel actually exist in the image source we are
        // reading from.
    }

    SkTDArray<sk_sp<GrTextureProxy>> tempTextureProxies; // build from yuvaTextures
    tempTextureProxies.setCount(textureProxyCount);
    SkAlphaType at = kOpaque_SkAlphaType;
    for (int i = 0; i < 4; ++i) {
        // Fill in tempTextureProxies to avoid duplicate texture proxies.
        int textureIndex = yuvaIndices[i].fIndex;

        // Safely ignore since this means we are missing the A plane.
        if (textureIndex == -1) {
            SkASSERT(3 == i);
            continue;
        }
        if (3 == i) {
            at = kUnpremul_SkAlphaType; // not sure if this is right
        }

        if (!tempTextureProxies[textureIndex]) {
            SkASSERT(yuvaTexturesCopy[textureIndex].isValid());
            tempTextureProxies[textureIndex] =
                proxyProvider->wrapBackendTexture(yuvaTexturesCopy[textureIndex], origin);
        }
    }
    sk_sp<GrTextureProxy> yProxy = tempTextureProxies[yuvaIndices[0].fIndex];
    sk_sp<GrTextureProxy> uProxy = tempTextureProxies[yuvaIndices[1].fIndex];
    sk_sp<GrTextureProxy> vProxy = tempTextureProxies[yuvaIndices[2].fIndex];

    if (!yProxy || !uProxy || !vProxy) {
        return nullptr;
    }

    return sk_make_sp<SkImage_GpuYUV>(sk_ref_sp(ctx), kNeedNewImageUniqueID, at, colorSpace,
                                      tempTextureProxies, yuvaIndices, size,
                                      imageColorSpace, SkBudgeted::kYes);
}

