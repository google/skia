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
#include "src/gpu/GrImageInfo.h"
#include "src/gpu/GrProxyProvider.h"
#include "src/gpu/SkGr.h"
#include "tools/gpu/ProxyUtils.h"

namespace sk_gpu_test {

sk_sp<GrTextureProxy> MakeTextureProxyFromData(GrContext* context,
                                               GrRenderable renderable,
                                               GrSurfaceOrigin origin,
                                               const GrImageInfo& imageInfo,
                                               const void* data,
                                               size_t rowBytes) {
    if (context->priv().abandoned()) {
        return nullptr;
    }

    const GrCaps* caps = context->priv().caps();

    const GrBackendFormat format = caps->getDefaultBackendFormat(imageInfo.colorType(), renderable);
    if (!format.isValid()) {
        return nullptr;
    }

    sk_sp<GrTextureProxy> proxy;
    GrSurfaceDesc desc;
    desc.fConfig = GrColorTypeToPixelConfig(imageInfo.colorType());
    desc.fWidth = imageInfo.width();
    desc.fHeight = imageInfo.height();
    proxy = context->priv().proxyProvider()->createProxy(format, desc, renderable, 1, origin,
                                                         GrMipMapped::kNo, SkBackingFit::kExact,
                                                         SkBudgeted::kYes, GrProtected::kNo);
    if (!proxy) {
        return nullptr;
    }
    auto sContext = context->priv().makeWrappedSurfaceContext(proxy, imageInfo.colorType(),
                                                              imageInfo.alphaType(),
                                                              imageInfo.refColorSpace(), nullptr);
    if (!sContext) {
        return nullptr;
    }
    if (!sContext->writePixels(imageInfo, data, rowBytes, {0, 0}, context)) {
        return nullptr;
    }
    return proxy;
}

}  // namespace sk_gpu_test
