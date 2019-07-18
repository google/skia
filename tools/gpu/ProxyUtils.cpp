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

sk_sp<GrTextureProxy> MakeTextureProxyFromData(GrContext* context,
                                               GrRenderable renderable,
                                               int width,
                                               int height,
                                               GrColorType colorType, SkAlphaType alphaType,
                                               GrSurfaceOrigin origin,
                                               const void* data, size_t rowBytes) {
    if (context->priv().abandoned()) {
        return nullptr;
    }

    const GrCaps* caps = context->priv().caps();

    const GrBackendFormat format = caps->getBackendFormatFromColorType(colorType);
    if (!format.isValid()) {
        return nullptr;
    }

    sk_sp<GrTextureProxy> proxy;
    if (kBottomLeft_GrSurfaceOrigin == origin) {
        // We (soon will) only support using kBottomLeft with wrapped textures.
        auto backendTex = context->createBackendTexture(
                width, height, format, SkColors::kTransparent, GrMipMapped::kNo, renderable,
                GrProtected::kNo);
        if (!backendTex.isValid()) {
            return nullptr;
        }

        // Adopt ownership so our caller doesn't have to worry about deleting the backend texture.
        if (GrRenderable::kYes == renderable) {
            proxy = context->priv().proxyProvider()->wrapRenderableBackendTexture(
                    backendTex, origin, 1, colorType, kAdopt_GrWrapOwnership, GrWrapCacheable::kNo,
                    nullptr, nullptr);
        } else {
            proxy = context->priv().proxyProvider()->wrapBackendTexture(
                    backendTex, origin, kAdopt_GrWrapOwnership, GrWrapCacheable::kNo, kRW_GrIOType);
        }

        if (!proxy) {
            context->deleteBackendTexture(backendTex);
            return nullptr;
        }

    } else {
        GrPixelConfig config = GrColorTypeToPixelConfig(colorType);
        if (!context->priv().caps()->isConfigTexturable(config)) {
            return nullptr;
        }

        GrSurfaceDesc desc;
        desc.fConfig = config;
        desc.fWidth = width;
        desc.fHeight = height;
        proxy = context->priv().proxyProvider()->createProxy(
                format, desc, renderable, origin, SkBackingFit::kExact, SkBudgeted::kYes);
        if (!proxy) {
            return nullptr;
        }
    }

    auto sContext = context->priv().makeWrappedSurfaceContext(proxy, colorType, alphaType, nullptr);
    if (!sContext) {
        return nullptr;
    }
    if (!sContext->writePixels({colorType, alphaType, nullptr, width, height}, data, rowBytes,
                               {0, 0}, context)) {
        return nullptr;
    }
    return proxy;
}

}  // namespace sk_gpu_test
