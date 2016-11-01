/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrTextureProxy.h"

#include "GrContext.h"
#include "GrTextureProvider.h"

GrTextureProxy::GrTextureProxy(GrContext* context, 
                               const GrSurfaceDesc& srcDesc, SkBackingFit fit, SkBudgeted budgeted,
                               const void* /*srcData*/, size_t /*rowBytes*/)
    : INHERITED(srcDesc, fit, budgeted)
    , fContext(context) {
    // TODO: Handle 'srcData' here
}

GrTextureProxy::GrTextureProxy(sk_sp<GrTexture> tex)
    : INHERITED(std::move(tex), SkBackingFit::kExact) {
    fContext = fTarget->getContext();
}

GrTexture* GrTextureProxy::instantiate() {
    if (fTarget) {
        return fTarget->asTexture();
    }

    if (SkBackingFit::kApprox == fFit) {
        fTarget = fContext->textureProvider()->createApproxTexture(fDesc);
    } else {
        fTarget = fContext->textureProvider()->createTexture(fDesc, fBudgeted);
    }

#ifdef SK_DEBUG
    if (kInvalidGpuMemorySize != fGpuMemorySize) {
        SkASSERT(fTarget->gpuMemorySize() == fGpuMemorySize);
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

//    if (this->texturePriv().hasMipMaps()) {
        // We don't have to worry about the mipmaps being a different size than
        // we'd expect because we never change fDesc.fWidth/fHeight.
        textureSize += textureSize/3;
//    }

    SkASSERT(!SkToBool(fDesc.fFlags & kRenderTarget_GrSurfaceFlag));
    SkASSERT(textureSize <= GrSurface::WorstCaseSize(fDesc));

    return textureSize;
}

sk_sp<GrTextureProxy> GrTextureProxy::Make(GrContext* context,
                                           const GrSurfaceDesc& desc,
                                           SkBackingFit fit,
                                           SkBudgeted budgeted,
                                           const void* srcData,
                                           size_t rowBytes) {
    // TODO: handle 'srcData' (we could use the wrapped version if there is data)
    SkASSERT(!srcData && !rowBytes);
    return sk_sp<GrTextureProxy>(new GrTextureProxy(context, desc, fit, budgeted,
                                                    srcData, rowBytes));
}

sk_sp<GrTextureProxy> GrTextureProxy::Make(sk_sp<GrTexture> tex) {
    return sk_sp<GrTextureProxy>(new GrTextureProxy(std::move(tex)));
}
