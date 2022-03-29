/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrDynamicAtlas.h"

#include "src/core/SkIPoint16.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrOnFlushResourceProvider.h"
#include "src/gpu/GrProxyProvider.h"
#include "src/gpu/GrRectanizerPow2.h"
#include "src/gpu/GrRectanizerSkyline.h"
#include "src/gpu/GrRenderTarget.h"
#include "src/gpu/GrResourceProvider.h"
#include "src/gpu/GrSurfaceProxyPriv.h"
#include "src/gpu/GrSurfaceProxyView.h"

sk_sp<GrTextureProxy>
GrDynamicAtlas::MakeLazyAtlasProxy(LazyInstantiateAtlasCallback&& callback,
                                   GrColorType colorType,
                                   InternalMultisample internalMultisample,
                                   const GrCaps& caps,
                                   GrSurfaceProxy::UseAllocator useAllocator) {
    GrBackendFormat format = caps.getDefaultBackendFormat(colorType, GrRenderable::kYes);

    int sampleCount = 1;
    if (InternalMultisample::kYes == internalMultisample) {
        sampleCount = caps.internalMultisampleCount(format);
    }

    sk_sp<GrTextureProxy> proxy = GrProxyProvider::MakeFullyLazyProxy(std::move(callback),
                                                                      format,
                                                                      GrRenderable::kYes,
                                                                      sampleCount,
                                                                      GrProtected::kNo,
                                                                      caps,
                                                                      useAllocator);

    return proxy;
}

GrDynamicAtlas::GrDynamicAtlas(GrColorType colorType,
                               InternalMultisample internalMultisample,
                               SkISize initialSize,
                               int maxAtlasSize,
                               const GrCaps& caps,
                               RectanizerAlgorithm algorithm) :
    fColorType(colorType),
    fInternalMultisample(internalMultisample),
    fDynamicRectanizer(initialSize, maxAtlasSize, algorithm) {
    SkASSERT(fDynamicRectanizer.maxAtlasSize() <= caps.maxTextureSize());
    this->reset(initialSize, caps);
}

GrDynamicAtlas::~GrDynamicAtlas() {}

void GrDynamicAtlas::reset(SkISize initialSize, const GrCaps& caps) {
    fDynamicRectanizer.reset(initialSize);
    fTextureProxy = MakeLazyAtlasProxy(
        [this](GrResourceProvider* resourceProvider, const LazyAtlasDesc& desc) {
            if (!fBackingTexture) {
                fBackingTexture =
                    resourceProvider->createTexture(fTextureProxy->backingStoreDimensions(),
                                                    desc.fFormat,
                                                    desc.fTextureType,
                                                    desc.fRenderable,
                                                    desc.fSampleCnt,
                                                    desc.fMipmapped,
                                                    desc.fBudgeted,
                                                    desc.fProtected);
            }
            return GrSurfaceProxy::LazyCallbackResult(fBackingTexture);
        },
        fColorType,
        fInternalMultisample,
        caps,
        GrSurfaceProxy::UseAllocator::kNo);
    fBackingTexture = nullptr;
}

GrSurfaceProxyView GrDynamicAtlas::readView(const GrCaps& caps) const {
    return {fTextureProxy,
            kTextureOrigin,
            caps.getReadSwizzle(fTextureProxy->backendFormat(), fColorType)};
}

GrSurfaceProxyView GrDynamicAtlas::writeView(const GrCaps& caps) const {
    return {fTextureProxy,
            kTextureOrigin,
            caps.getWriteSwizzle(fTextureProxy->backendFormat(), fColorType)};
}

bool GrDynamicAtlas::addRect(int width, int height, SkIPoint16* location) {
    return fDynamicRectanizer.addRect(width, height, location);
}

void GrDynamicAtlas::instantiate(GrOnFlushResourceProvider* onFlushRP,
                                 sk_sp<GrTexture> backingTexture) {
    SkASSERT(!this->isInstantiated()); // This method should only be called once.
    // Caller should have cropped any paths to the destination render target instead of asking for
    // an atlas larger than maxRenderTargetSize.
    SkASSERT(std::max(fDynamicRectanizer.height(), fDynamicRectanizer.width()) <=
             fDynamicRectanizer.maxAtlasSize());
    SkASSERT(fDynamicRectanizer.maxAtlasSize() <= onFlushRP->caps()->maxRenderTargetSize());

    if (fTextureProxy->isFullyLazy()) {
        // Finalize the content size of our proxy. The GPU can potentially make optimizations if it
        // knows we only intend to write out a smaller sub-rectangle of the backing texture.
        fTextureProxy->priv().setLazyDimensions(fDynamicRectanizer.drawBounds());
    }
    SkASSERT(fTextureProxy->dimensions() == fDynamicRectanizer.drawBounds());

    if (backingTexture) {
#ifdef SK_DEBUG
        auto backingRT = backingTexture->asRenderTarget();
        SkASSERT(backingRT);
        SkASSERT(backingRT->backendFormat() == fTextureProxy->backendFormat());
        SkASSERT(backingRT->numSamples() == fTextureProxy->asRenderTargetProxy()->numSamples());
        SkASSERT(backingRT->dimensions() == fTextureProxy->backingStoreDimensions());
#endif
        fBackingTexture = std::move(backingTexture);
    }
    onFlushRP->instatiateProxy(fTextureProxy.get());
}
