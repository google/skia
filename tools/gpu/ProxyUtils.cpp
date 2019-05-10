/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/GrBackendSurface.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrDrawingManager.h"
#include "src/gpu/GrGpu.h"
#include "src/gpu/GrProxyProvider.h"
#include "src/gpu/SkGr.h"
#include "tools/gpu/ProxyUtils.h"

namespace sk_gpu_test {

sk_sp<GrTextureProxy> MakeTextureProxyFromData(GrContext* context, GrRenderable renderable,
                                               int width, int height,
                                               GrColorType colorType, GrSRGBEncoded srgbEncoded,
                                               GrSurfaceOrigin origin,
                                               const void* data, size_t rowBytes) {
    if (context->priv().abandoned()) {
        return nullptr;
    }

    const GrCaps* caps = context->priv().caps();

    const GrBackendFormat format = caps->getBackendFormatFromGrColorType(colorType, srgbEncoded);
    if (!format.isValid()) {
        return nullptr;
    }

    sk_sp<GrTextureProxy> proxy;
    if (kBottomLeft_GrSurfaceOrigin == origin) {
        // We (soon will) only support using kBottomLeft with wrapped textures.
        auto backendTex = context->priv().getGpu()->createTestingOnlyBackendTexture(
                width, height, format, GrMipMapped::kNo, renderable);
        if (!backendTex.isValid()) {
            return nullptr;
        }

        // Adopt ownership so our caller doesn't have to worry about deleting the backend texture.
        if (GrRenderable::kYes == renderable) {
            proxy = context->priv().proxyProvider()->wrapRenderableBackendTexture(
                    backendTex, origin, 1, kAdopt_GrWrapOwnership, GrWrapCacheable::kNo, nullptr,
                    nullptr);
        } else {
            proxy = context->priv().proxyProvider()->wrapBackendTexture(
                    backendTex, origin, kAdopt_GrWrapOwnership, GrWrapCacheable::kNo, kRW_GrIOType);
        }

        if (!proxy) {
            context->priv().getGpu()->deleteTestingOnlyBackendTexture(backendTex);
            return nullptr;
        }

    } else {
        GrPixelConfig config = GrColorTypeToPixelConfig(colorType, srgbEncoded);
        if (!context->priv().caps()->isConfigTexturable(config)) {
            return nullptr;
        }

        GrSurfaceDesc desc;
        desc.fConfig = config;
        desc.fWidth = width;
        desc.fHeight = height;
        desc.fFlags = GrRenderable::kYes == renderable ? kRenderTarget_GrSurfaceFlag
                                                       : kNone_GrSurfaceFlags;
        proxy = context->priv().proxyProvider()->createProxy(
                format, desc, origin, SkBackingFit::kExact, SkBudgeted::kYes);
        if (!proxy) {
            return nullptr;
        }
    }

    auto sContext = context->priv().makeWrappedSurfaceContext(proxy, nullptr);
    if (!sContext) {
        return nullptr;
    }
    if (!context->priv().writeSurfacePixels(sContext.get(), 0, 0, width, height, colorType,
                                            nullptr, data, rowBytes)) {
        return nullptr;
    }
    return proxy;
}

}  // namespace sk_gpu_test
