/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrRenderTargetProxy.h"

#include "src/core/SkMathPriv.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrGpuResourcePriv.h"
#include "src/gpu/GrRenderTargetOpList.h"
#include "src/gpu/GrRenderTargetPriv.h"
#include "src/gpu/GrResourceProvider.h"
#include "src/gpu/GrSurfacePriv.h"
#include "src/gpu/GrTextureRenderTargetProxy.h"

// Deferred version
// TODO: we can probably munge the 'desc' in both the wrapped and deferred
// cases to make the sampleConfig/numSamples stuff more rational.
GrRenderTargetProxy::GrRenderTargetProxy(const GrCaps& caps, const GrBackendFormat& format,
                                         const GrSurfaceDesc& desc, GrSurfaceOrigin origin,
                                         const GrSwizzle& textureSwizzle,
                                         const GrSwizzle& outputSwizzle, SkBackingFit fit,
                                         SkBudgeted budgeted, GrInternalSurfaceFlags surfaceFlags)
        : INHERITED(format, desc, origin, textureSwizzle, fit, budgeted, surfaceFlags)
        , fSampleCnt(desc.fSampleCnt)
        , fWrapsVkSecondaryCB(WrapsVkSecondaryCB::kNo)
        , fOutputSwizzle(outputSwizzle) {
}

// Lazy-callback version
GrRenderTargetProxy::GrRenderTargetProxy(LazyInstantiateCallback&& callback,
                                         LazyInstantiationType lazyType,
                                         const GrBackendFormat& format, const GrSurfaceDesc& desc,
                                         GrSurfaceOrigin origin, const GrSwizzle& textureSwizzle,
                                         const GrSwizzle& outputSwizzle, SkBackingFit fit,
                                         SkBudgeted budgeted, GrInternalSurfaceFlags surfaceFlags,
                                         WrapsVkSecondaryCB wrapsVkSecondaryCB)
        : INHERITED(std::move(callback), lazyType, format, desc, origin, textureSwizzle, fit,
                    budgeted, surfaceFlags)
        , fSampleCnt(desc.fSampleCnt)
        , fWrapsVkSecondaryCB(wrapsVkSecondaryCB)
        , fOutputSwizzle(outputSwizzle) {
    SkASSERT(SkToBool(kRenderTarget_GrSurfaceFlag & desc.fFlags));
}

// Wrapped version
GrRenderTargetProxy::GrRenderTargetProxy(sk_sp<GrSurface> surf, GrSurfaceOrigin origin,
                                         const GrSwizzle& textureSwizzle,
                                         const GrSwizzle& outputSwizzle,
                                         WrapsVkSecondaryCB wrapsVkSecondaryCB)
        : INHERITED(std::move(surf), origin, textureSwizzle, SkBackingFit::kExact)
        , fSampleCnt(fTarget->asRenderTarget()->numSamples())
        , fWrapsVkSecondaryCB(wrapsVkSecondaryCB)
        , fOutputSwizzle(outputSwizzle) {
}

int GrRenderTargetProxy::maxWindowRectangles(const GrCaps& caps) const {
    return this->glRTFBOIDIs0() ? 0 : caps.maxWindowRectangles();
}

bool GrRenderTargetProxy::instantiate(GrResourceProvider* resourceProvider) {
    if (LazyState::kNot != this->lazyInstantiationState()) {
        return false;
    }
    static constexpr GrSurfaceDescFlags kDescFlags = kRenderTarget_GrSurfaceFlag;

    if (!this->instantiateImpl(resourceProvider, fSampleCnt, fNumStencilSamples, kDescFlags,
                               GrMipMapped::kNo, nullptr)) {
        return false;
    }

    SkASSERT(this->peekRenderTarget());
    SkASSERT(!this->peekTexture());
    return true;
}

bool GrRenderTargetProxy::canChangeStencilAttachment() const {
    if (!fTarget) {
        // If we aren't instantiated, then we definitely are an internal render target. Ganesh is
        // free to change stencil attachments on internal render targets.
        return true;
    }
    return fTarget->asRenderTarget()->canAttemptStencilAttachment();
}

sk_sp<GrSurface> GrRenderTargetProxy::createSurface(GrResourceProvider* resourceProvider) const {
    static constexpr GrSurfaceDescFlags kDescFlags = kRenderTarget_GrSurfaceFlag;

    sk_sp<GrSurface> surface = this->createSurfaceImpl(
            resourceProvider, fSampleCnt, fNumStencilSamples, kDescFlags, GrMipMapped::kNo);
    if (!surface) {
        return nullptr;
    }
    SkASSERT(surface->asRenderTarget());
    SkASSERT(!surface->asTexture());
    return surface;
}

size_t GrRenderTargetProxy::onUninstantiatedGpuMemorySize() const {
    int colorSamplesPerPixel = this->numSamples();
    if (colorSamplesPerPixel > 1) {
        // Add one for the resolve buffer.
        ++colorSamplesPerPixel;
    }

    // TODO: do we have enough information to improve this worst case estimate?
    return GrSurface::ComputeSize(this->config(), this->width(), this->height(),
                                  colorSamplesPerPixel, GrMipMapped::kNo, !this->priv().isExact());
}

bool GrRenderTargetProxy::refsWrappedObjects() const {
    if (!this->isInstantiated()) {
        return false;
    }

    GrSurface* surface = this->peekSurface();
    return surface->resourcePriv().refsWrappedObjects();
}

#ifdef SK_DEBUG
void GrRenderTargetProxy::onValidateSurface(const GrSurface* surface) {
    // We do not check that surface->asTexture returns null since, when replaying DDLs we
    // can fulfill a renderTarget-only proxy w/ a textureRenderTarget.

    // Anything that is checked here should be duplicated in GrTextureRenderTargetProxy's version
    SkASSERT(surface->asRenderTarget());
    SkASSERT(surface->asRenderTarget()->numSamples() == this->numSamples());

    GrInternalSurfaceFlags proxyFlags = fSurfaceFlags;
    GrInternalSurfaceFlags surfaceFlags = surface->surfacePriv().flags();
    SkASSERT((proxyFlags & GrInternalSurfaceFlags::kRenderTargetMask) ==
             (surfaceFlags & GrInternalSurfaceFlags::kRenderTargetMask));
}
#endif
