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
                               SkTDArray<sk_sp<GrTextureProxy>> proxies, SkYUVAIndex yuvaIndices[4],
                               SkISize size, sk_sp<SkColorSpace> colorSpace, SkBudgeted budgeted)
        : INHERITED(proxies[0]->worstCaseWidth(), proxies[0]->worstCaseHeight(), uniqueID)
        , fContext(std::move(context))
        , fProxies(std::move(proxies))
        , fSize(size)
        , fAlphaType(at)
        , fBudgeted(budgeted)
        , fColorSpace(std::move(colorSpace)) {
    memcpy(fYUVAIndices, yuvaIndices, 4*sizeof(SkYUVAIndex));
}

SkImage_GpuYUV::~SkImage_GpuYUV() {}

// need shared def
static bool validate_backend_texture(GrContext* ctx, const GrBackendTexture& tex, GrPixelConfig* config,
                              SkColorType ct, SkAlphaType at, sk_sp<SkColorSpace> cs) {
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

    return sk_make_sp<SkImage_GpuYUV>(sk_ref_sp(ctx), kNeedNewImageUniqueID, at,
                                      tempTextureProxies, yuvaIndices, size,
                                      imageColorSpace, SkBudgeted::kYes);
}

