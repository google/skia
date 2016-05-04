/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrTextureProxy.h"

#include "GrTextureProvider.h"
#include "GrGpuResourcePriv.h"

GrTextureProxy::GrTextureProxy(sk_sp<GrTexture> tex)
    : INHERITED(tex->desc(), SkBackingFit::kExact, tex->resourcePriv().isBudgeted())
    , fTexture(std::move(tex)) {
}

GrTexture* GrTextureProxy::instantiate(GrTextureProvider* texProvider) {
    if (fTexture) {
        return fTexture.get();
    }

    if (SkBackingFit::kApprox == fFit) {
        fTexture.reset(texProvider->createApproxTexture(fDesc));
    } else {
        fTexture.reset(texProvider->createTexture(fDesc, fBudgeted));
    }

    return fTexture.get();
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
