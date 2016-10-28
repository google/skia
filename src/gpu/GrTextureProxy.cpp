/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrTextureProxy.h"

#include "GrTextureProvider.h"

GrTextureProxy::GrTextureProxy(const GrSurfaceDesc& srcDesc, SkBackingFit fit, SkBudgeted budgeted,
                               const void* /*srcData*/, size_t /*rowBytes*/)
    : INHERITED(srcDesc, fit, budgeted) {
    // TODO: Handle 'srcData' here
}

GrTextureProxy::GrTextureProxy(sk_sp<GrTexture> tex)
    : INHERITED(std::move(tex), SkBackingFit::kExact) {
}

GrTexture* GrTextureProxy::instantiate(GrTextureProvider* texProvider) {
    if (fTarget) {
        return fTarget->asTexture();
    }

    if (SkBackingFit::kApprox == fFit) {
        fTarget = texProvider->createApproxTexture(fDesc);
    } else {
        fTarget = texProvider->createTexture(fDesc, fBudgeted);
    }

    return fTarget->asTexture();
}

sk_sp<GrTextureProxy> GrTextureProxy::Make(const GrSurfaceDesc& desc,
                                           SkBackingFit fit,
                                           SkBudgeted budgeted,
                                           const void* srcData,
                                           size_t rowBytes) {
    // TODO: handle 'srcData' (we could use the wrapped version if there is data)
    SkASSERT(!srcData && !rowBytes);
    return sk_sp<GrTextureProxy>(new GrTextureProxy(desc, fit, budgeted, srcData, rowBytes));
}

sk_sp<GrTextureProxy> GrTextureProxy::Make(sk_sp<GrTexture> tex) {
    return sk_sp<GrTextureProxy>(new GrTextureProxy(std::move(tex)));
}
