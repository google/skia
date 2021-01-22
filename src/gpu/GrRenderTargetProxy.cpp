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
#include "src/gpu/GrOpsTask.h"
#include "src/gpu/GrRenderTarget.h"
#include "src/gpu/GrResourceProvider.h"
#include "src/gpu/GrSurface.h"
#include "src/gpu/GrTextureRenderTargetProxy.h"

#ifdef SK_DEBUG
#include "include/gpu/GrDirectContext.h"
#include "src/gpu/GrDirectContextPriv.h"
#endif

// Deferred version
// TODO: we can probably munge the 'desc' in both the wrapped and deferred
// cases to make the sampleConfig/numSamples stuff more rational.
GrRenderTargetProxy::GrRenderTargetProxy(const GrCaps& caps,
                                         const GrBackendFormat& format,
                                         SkISize dimensions,
                                         int sampleCount,
                                         SkBackingFit fit,
                                         SkBudgeted budgeted,
                                         GrProtected isProtected,
                                         GrInternalSurfaceFlags surfaceFlags,
                                         UseAllocator useAllocator)
        : INHERITED(format, dimensions, fit, budgeted, isProtected, surfaceFlags, useAllocator)
        , fSampleCnt(sampleCount)
        , fWrapsVkSecondaryCB(WrapsVkSecondaryCB::kNo) {}

// Lazy-callback version
GrRenderTargetProxy::GrRenderTargetProxy(LazyInstantiateCallback&& callback,
                                         const GrBackendFormat& format,
                                         SkISize dimensions,
                                         int sampleCount,
                                         SkBackingFit fit,
                                         SkBudgeted budgeted,
                                         GrProtected isProtected,
                                         GrInternalSurfaceFlags surfaceFlags,
                                         UseAllocator useAllocator,
                                         WrapsVkSecondaryCB wrapsVkSecondaryCB)
        : INHERITED(std::move(callback), format, dimensions, fit, budgeted, isProtected,
                    surfaceFlags, useAllocator)
        , fSampleCnt(sampleCount)
        , fWrapsVkSecondaryCB(wrapsVkSecondaryCB) {}

// Wrapped version
GrRenderTargetProxy::GrRenderTargetProxy(sk_sp<GrSurface> surf,
                                         UseAllocator useAllocator,
                                         WrapsVkSecondaryCB wrapsVkSecondaryCB)
        : INHERITED(std::move(surf), SkBackingFit::kExact, useAllocator)
        , fSampleCnt(fTarget->asRenderTarget()->numSamples())
        , fWrapsVkSecondaryCB(wrapsVkSecondaryCB) {
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
    if (!this->instantiateImpl(resourceProvider, fSampleCnt, GrRenderable::kYes, GrMipmapped::kNo,
                               nullptr)) {
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
    sk_sp<GrSurface> surface = this->createSurfaceImpl(resourceProvider, fSampleCnt,
                                                       GrRenderable::kYes, GrMipmapped::kNo);
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
    return GrSurface::ComputeSize(this->backendFormat(), this->dimensions(),
                                  colorSamplesPerPixel, GrMipmapped::kNo, !this->priv().isExact());
}

bool GrRenderTargetProxy::refsWrappedObjects() const {
    if (!this->isInstantiated()) {
        return false;
    }

    GrSurface* surface = this->peekSurface();
    return surface->resourcePriv().refsWrappedObjects();
}

GrSurfaceProxy::LazySurfaceDesc GrRenderTargetProxy::callbackDesc() const {
    // We only expect exactly sized lazy RT proxies.
    SkASSERT(!this->isFullyLazy());
    SkASSERT(this->isFunctionallyExact());
    return {
            this->dimensions(),
            SkBackingFit::kExact,
            GrRenderable::kYes,
            GrMipmapped::kNo,
            this->numSamples(),
            this->backendFormat(),
            this->isProtected(),
            this->isBudgeted(),
    };
}

#ifdef SK_DEBUG
void GrRenderTargetProxy::onValidateSurface(const GrSurface* surface) {
    // We do not check that surface->asTexture returns null since, when replaying DDLs we
    // can fulfill a renderTarget-only proxy w/ a textureRenderTarget.

    // Anything that is checked here should be duplicated in GrTextureRenderTargetProxy's version
    SkASSERT(surface->asRenderTarget());
    SkASSERT(surface->asRenderTarget()->numSamples() == this->numSamples());

    GrInternalSurfaceFlags proxyFlags = fSurfaceFlags;
    GrInternalSurfaceFlags surfaceFlags = surface->flags();
    if (proxyFlags & GrInternalSurfaceFlags::kGLRTFBOIDIs0 && this->numSamples() == 1) {
        // Ganesh never internally creates FBO0 proxies or surfaces so this must be a wrapped
        // proxy. In this case, with no MSAA, rendering to FBO0 is strictly more limited than
        // rendering to an arbitrary surface so we allow a non-FBO0 surface to be matched with
        // the proxy.
        surfaceFlags |= GrInternalSurfaceFlags::kGLRTFBOIDIs0;
    }
    SkASSERT(((int)proxyFlags & kGrInternalRenderTargetFlagsMask) ==
             ((int)surfaceFlags & kGrInternalRenderTargetFlagsMask));

    // We manually check the kVkRTSupportsInputAttachment since we only require it on the surface if
    // the proxy has it set. If the proxy doesn't have the flag it is legal for the surface to
    // have the flag.
    if (proxyFlags & GrInternalSurfaceFlags::kVkRTSupportsInputAttachment) {
        SkASSERT(surfaceFlags & GrInternalSurfaceFlags::kVkRTSupportsInputAttachment);
    }
}
#endif
