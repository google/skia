/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "ProxyUtils.h"
#include "GrContextPriv.h"
#include "GrDrawingManager.h"
#include "GrProxyProvider.h"

namespace sk_gpu_test {

sk_sp<GrTextureProxy> MakeTextureProxyFromData(GrContext* context, bool isRT, int width, int height,
                                               GrColorType ct, GrSRGBEncoded srgbEncoded,
                                               GrSurfaceOrigin origin, const void* data,
                                               size_t rowBytes) {
    GrSurfaceDesc desc;
    desc.fConfig = GrColorTypeToPixelConfig(ct, srgbEncoded);
    desc.fWidth = width;
    desc.fHeight = height;
    desc.fFlags = isRT ? kRenderTarget_GrSurfaceFlag : kNone_GrSurfaceFlags;
    auto proxy = context->contextPriv().proxyProvider()->createProxy(
            desc, origin, SkBackingFit::kExact, SkBudgeted::kYes);
    if (!proxy) {
        return nullptr;
    }
    auto sContext = context->contextPriv().makeWrappedSurfaceContext(proxy, nullptr);
    if (!sContext) {
        return nullptr;
    }
    if (!context->contextPriv().writeSurfacePixels(sContext.get(), 0, 0, width, height, ct, nullptr,
                                                   data, rowBytes)) {
        return nullptr;
    }
    return proxy;
}

}  // namespace sk_gpu_test
