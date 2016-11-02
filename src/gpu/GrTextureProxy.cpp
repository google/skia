/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrTextureProxy.h"

#include "GrTextureProvider.h"

GrTextureProxy::GrTextureProxy(const GrSurfaceDesc& srcDesc, SkBackingFit fit, SkBudgeted budgeted,
                               const void* srcData, size_t /*rowBytes*/)
    : INHERITED(srcDesc, fit, budgeted) {
    SkASSERT(!srcData);   // currently handled in Make()
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

#ifdef SK_DEBUG
    if (kInvalidGpuMemorySize != fGpuMemorySize) {
        SkASSERT(fTarget->gpuMemorySize() <= fGpuMemorySize);
    }
#endif

    return fTarget->asTexture();
}

size_t GrTextureProxy::onGpuMemorySize() const {
    size_t textureSize;

    if (GrPixelConfigIsCompressed(fDesc.fConfig)) {
        textureSize = GrCompressedFormatDataSize(fDesc.fConfig, fDesc.fWidth, fDesc.fHeight);
    } else {
        textureSize = (size_t) fDesc.fWidth * fDesc.fHeight * GrBytesPerPixel(fDesc.fConfig);
    }

    // TODO: add tracking of mipmap state to improve the estimate
    textureSize += textureSize/3;

    SkASSERT(!SkToBool(fDesc.fFlags & kRenderTarget_GrSurfaceFlag));
    SkASSERT(textureSize <= GrSurface::WorstCaseSize(fDesc));

    return textureSize;
}

sk_sp<GrTextureProxy> GrTextureProxy::Make(GrTextureProvider* texProvider,
                                           const GrSurfaceDesc& desc,
                                           SkBackingFit fit,
                                           SkBudgeted budgeted,
                                           const void* srcData,
                                           size_t rowBytes) {
    if (srcData) {
        // If we have srcData, for now, we create a wrapped GrTextureProxy
        sk_sp<GrTexture> tex(texProvider->createTexture(desc, budgeted, srcData, rowBytes));
        return GrTextureProxy::Make(std::move(tex));
    }

    return sk_sp<GrTextureProxy>(new GrTextureProxy(desc, fit, budgeted, nullptr, 0));
}

sk_sp<GrTextureProxy> GrTextureProxy::Make(sk_sp<GrTexture> tex) {
    return sk_sp<GrTextureProxy>(new GrTextureProxy(std::move(tex)));
}
