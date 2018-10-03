/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrBackendSurface.h"
#include "GrContext.h"
#include "GrContextPriv.h"
#include "GrTextureAdjuster.h"
#include "SkImage_Gpu.h"
#include "SkImage_GpuShared.h"

namespace SkImage_GpuShared {

bool ValidateBackendTexture(GrContext* ctx, const GrBackendTexture& tex,
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

sk_sp<GrTextureProxy> AsTextureProxyRef(GrContext* context,
                                        const GrSamplerState& params,
                                        SkColorSpace* dstColorSpace,
                                        sk_sp<SkColorSpace>* texColorSpace,
                                        SkScalar scaleAdjust[2],
                                        GrContext* imageContext,
                                        const SkImage_Base* image,
                                        const SkAlphaType imageAlphaType,
                                        SkColorSpace* imageColorSpace) {
    if (context->uniqueID() != imageContext->uniqueID()) {
        SkASSERT(0);
        return nullptr;
    }

    GrTextureAdjuster adjuster(imageContext, image->asTextureProxyRef(), imageAlphaType,
                               image->uniqueID(), imageColorSpace);
    return adjuster.refTextureProxyForParams(params, dstColorSpace, texColorSpace, scaleAdjust);
}

sk_sp<SkImage> OnMakeSubset(const SkIRect& subset, sk_sp<GrContext> imageContext,
                            const SkImage_Base* image, const SkAlphaType imageAlphaType,
                            sk_sp<SkColorSpace> imageColorSpace, const SkBudgeted budgeted){
    sk_sp<GrSurfaceProxy> proxy = image->asTextureProxyRef();

    GrSurfaceDesc desc;
    desc.fWidth = subset.width();
    desc.fHeight = subset.height();
    desc.fConfig = proxy->config();

    sk_sp<GrSurfaceContext> sContext(imageContext->contextPriv().makeDeferredSurfaceContext(
        desc, proxy->origin(), GrMipMapped::kNo, SkBackingFit::kExact, budgeted));
    if (!sContext) {
        return nullptr;
    }

    if (!sContext->copy(proxy.get(), subset, SkIPoint::Make(0, 0))) {
        return nullptr;
    }

    // MDB: this call is okay bc we know 'sContext' was kExact
    return sk_make_sp<SkImage_Gpu>(imageContext, kNeedNewImageUniqueID,
                                   imageAlphaType, sContext->asTextureProxyRef(),
                                   imageColorSpace, budgeted);
}


}
