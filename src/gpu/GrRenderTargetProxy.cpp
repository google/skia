/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrRenderTargetProxy.h"

#include "include/gpu/GrContext.h"
#include "src/core/SkMathPriv.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrGpuResourcePriv.h"
#include "src/gpu/GrOpsTask.h"
#include "src/gpu/GrRenderTargetPriv.h"
#include "src/gpu/GrResourceProvider.h"
#include "src/gpu/GrSurfacePriv.h"
#include "src/gpu/GrTextureRenderTargetProxy.h"

// Deferred version
// TODO: we can probably munge the 'desc' in both the wrapped and deferred
// cases to make the sampleConfig/numSamples stuff more rational.
GrRenderTargetProxy::GrRenderTargetProxy(const GrCaps& caps,
                                         const GrBackendFormat& format,
                                         const GrSurfaceDesc& desc,
                                         int sampleCount,
                                         GrSurfaceOrigin origin,
                                         const GrSwizzle& textureSwizzle,
                                         const GrSwizzle& outputSwizzle,
                                         SkBackingFit fit,
                                         SkBudgeted budgeted,
                                         GrProtected isProtected,
                                         GrInternalSurfaceFlags surfaceFlags,
                                         UseAllocator useAllocator)
        : INHERITED(format, desc, GrRenderable::kYes, origin, textureSwizzle, fit, budgeted,
                    isProtected, surfaceFlags, useAllocator)
        , fSampleCnt(sampleCount)
        , fWrapsVkSecondaryCB(WrapsVkSecondaryCB::kNo)
        , fOutputSwizzle(outputSwizzle) {}

// Lazy-callback version
GrRenderTargetProxy::GrRenderTargetProxy(LazyInstantiateCallback&& callback,
                                         const GrBackendFormat& format,
                                         const GrSurfaceDesc& desc,
                                         int sampleCount,
                                         GrSurfaceOrigin origin,
                                         const GrSwizzle& textureSwizzle,
                                         const GrSwizzle& outputSwizzle,
                                         SkBackingFit fit,
                                         SkBudgeted budgeted,
                                         GrProtected isProtected,
                                         GrInternalSurfaceFlags surfaceFlags,
                                         UseAllocator useAllocator,
                                         WrapsVkSecondaryCB wrapsVkSecondaryCB)
        : INHERITED(std::move(callback), format, desc, GrRenderable::kYes, origin, textureSwizzle,
                    fit, budgeted, isProtected, surfaceFlags, useAllocator)
        , fSampleCnt(sampleCount)
        , fWrapsVkSecondaryCB(wrapsVkSecondaryCB)
        , fOutputSwizzle(outputSwizzle) {}

// Wrapped version
GrRenderTargetProxy::GrRenderTargetProxy(sk_sp<GrSurface> surf,
                                         GrSurfaceOrigin origin,
                                         const GrSwizzle& textureSwizzle,
                                         const GrSwizzle& outputSwizzle,
                                         UseAllocator useAllocator,
                                         WrapsVkSecondaryCB wrapsVkSecondaryCB)
        : INHERITED(std::move(surf), origin, textureSwizzle, SkBackingFit::kExact, useAllocator)
        , fSampleCnt(fTarget->asRenderTarget()->numSamples())
        , fWrapsVkSecondaryCB(wrapsVkSecondaryCB)
        , fOutputSwizzle(outputSwizzle) {
    // The kRequiresManualMSAAResolve flag better not be set if we are not multisampled or if
    // MSAA resolve should happen automatically.
    //
    // From the other side, we don't know enough about the wrapped surface to assert when
    // kRequiresManualMSAAResolve *should* be set. e.g., The caller might be wrapping a backend
    // texture as a render target at this point but we wouldn't know it.
    SkASSERT(!(this->numSamples() <= 1 ||
               fTarget->getContext()->priv().caps()->msaaResolvesAutomatically()) ||
             !this->requiresManualMSAAResolve());
}

int GrRenderTargetProxy::maxWindowRectangles(const GrCaps& caps) const {
    return this->glRTFBOIDIs0() ? 0 : caps.maxWindowRectangles();
}

bool GrRenderTargetProxy::instantiate(GrResourceProvider* resourceProvider) {
    if (this->isLazy()) {
        return false;
    }
    if (!this->instantiateImpl(resourceProvider, fSampleCnt, fNumStencilSamples, GrRenderable::kYes,
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
    sk_sp<GrSurface> surface = this->createSurfaceImpl(
            resourceProvider, fSampleCnt, fNumStencilSamples, GrRenderable::kYes, GrMipMapped::kNo);
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
    SkASSERT(((int)proxyFlags & kGrInternalRenderTargetFlagsMask) ==
             ((int)surfaceFlags & kGrInternalRenderTargetFlagsMask));
}
#endif
