/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "ProxyUtils.h"
#include "GrBackendSurface.h"
#include "GrContextPriv.h"
#include "GrDrawingManager.h"
#include "GrGpu.h"
#include "GrProxyProvider.h"

namespace sk_gpu_test {

sk_sp<GrTextureProxy> MakeTextureProxyFromData(GrContext* context, bool isRT, int width, int height,
                                               GrColorType colorType, GrSRGBEncoded srgbEncoded,
                                               GrSurfaceOrigin origin, const void* data,
                                               size_t rowBytes) {
    if (context->abandoned()) {
        return nullptr;
    }

    sk_sp<GrTextureProxy> proxy;
    if (kBottomLeft_GrSurfaceOrigin == origin) {
        // We (soon will) only support using kBottomLeft with wrapped textures.
        auto backendTex = context->contextPriv().getGpu()->createTestingOnlyBackendTexture(
                nullptr, width, height, colorType, isRT, GrMipMapped::kNo);
        if (!backendTex.isValid()) {
            return nullptr;
        }
        // Adopt ownership so our caller doesn't have to worry about deleting the backend texture.
        if (isRT) {
            proxy = context->contextPriv().proxyProvider()->wrapRenderableBackendTexture(
                    backendTex, origin, 1, kAdopt_GrWrapOwnership, GrWrapCacheable::kNo);
        } else {
            proxy = context->contextPriv().proxyProvider()->wrapBackendTexture(
                    backendTex, origin, kAdopt_GrWrapOwnership, GrWrapCacheable::kNo, kRW_GrIOType);
        }

        if (!proxy) {
            context->contextPriv().getGpu()->deleteTestingOnlyBackendTexture(backendTex);
            return nullptr;
        }

    } else {
        GrPixelConfig config = GrColorTypeToPixelConfig(colorType, srgbEncoded);
        if (!context->contextPriv().caps()->isConfigTexturable(config)) {
            return nullptr;
        }

        const GrBackendFormat format =
                context->contextPriv().caps()->getBackendFormatFromGrColorType(colorType,
                                                                               srgbEncoded);
        if (!format.isValid()) {
            return nullptr;
        }

        GrSurfaceDesc desc;
        desc.fConfig = config;
        desc.fWidth = width;
        desc.fHeight = height;
        desc.fFlags = isRT ? kRenderTarget_GrSurfaceFlag : kNone_GrSurfaceFlags;
        proxy = context->contextPriv().proxyProvider()->createProxy(
                format, desc, origin, SkBackingFit::kExact, SkBudgeted::kYes);
        if (!proxy) {
            return nullptr;
        }
    }
    auto sContext = context->contextPriv().makeWrappedSurfaceContext(proxy, nullptr);
    if (!sContext) {
        return nullptr;
    }
    if (!context->contextPriv().writeSurfacePixels(sContext.get(), 0, 0, width, height, colorType,
                                                   nullptr, data, rowBytes)) {
        return nullptr;
    }
    return proxy;
}

}  // namespace sk_gpu_test
