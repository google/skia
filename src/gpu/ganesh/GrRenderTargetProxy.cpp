/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ganesh/GrRenderTargetProxy.h"

#include "include/core/SkSize.h"
#include "include/gpu/GpuTypes.h"
#include "include/gpu/ganesh/GrBackendSurface.h"
#include "include/gpu/ganesh/GrTypes.h"
#include "include/private/base/SkTo.h"
#include "src/gpu/SkBackingFit.h"
#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/GrGpuResourcePriv.h"
#include "src/gpu/ganesh/GrRenderTarget.h"
#include "src/gpu/ganesh/GrSurface.h"
#include "src/gpu/ganesh/GrSurfaceProxyPriv.h"

#include <utility>

#ifdef SK_DEBUG
#include "include/gpu/ganesh/GrDirectContext.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#endif

// Deferred version
// TODO: we can probably munge the 'desc' in both the wrapped and deferred
// cases to make the sampleConfig/numSamples stuff more rational.
GrRenderTargetProxy::GrRenderTargetProxy(const GrCaps& caps,
                                         const GrBackendFormat& format,
                                         SkISize dimensions,
                                         int sampleCount,
                                         SkBackingFit fit,
                                         skgpu::Budgeted budgeted,
                                         GrProtected isProtected,
                                         GrInternalSurfaceFlags surfaceFlags,
                                         UseAllocator useAllocator,
                                         std::string_view label)
        : INHERITED(
                  format, dimensions, fit, budgeted, isProtected, surfaceFlags, useAllocator, label)
        , fSampleCnt(sampleCount)
        , fWrapsVkSecondaryCB(WrapsVkSecondaryCB::kNo) {}

// Lazy-callback version
GrRenderTargetProxy::GrRenderTargetProxy(LazyInstantiateCallback&& callback,
                                         const GrBackendFormat& format,
                                         SkISize dimensions,
                                         int sampleCount,
                                         SkBackingFit fit,
                                         skgpu::Budgeted budgeted,
                                         GrProtected isProtected,
                                         GrInternalSurfaceFlags surfaceFlags,
                                         UseAllocator useAllocator,
                                         WrapsVkSecondaryCB wrapsVkSecondaryCB,
                                         std::string_view label)
        : INHERITED(std::move(callback),
                    format,
                    dimensions,
                    fit,
                    budgeted,
                    isProtected,
                    surfaceFlags,
                    useAllocator,
                    label)
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
    if (!this->instantiateImpl(resourceProvider, fSampleCnt, GrRenderable::kYes,
                               skgpu::Mipmapped::kNo, nullptr)) {
        return false;
    }

    SkASSERT(this->peekRenderTarget());
    SkASSERT(!this->peekTexture());
    return true;
}

bool GrRenderTargetProxy::canUseStencil(const GrCaps& caps) const {
    if (caps.avoidStencilBuffers() || this->wrapsVkSecondaryCB()) {
        return false;
    }
    if (!this->isInstantiated()) {
        if (this->isLazy() && this->backendFormat().backend() == GrBackendApi::kOpenGL) {
            // It's possible for wrapped GL render targets to not have stencil. We don't currently
            // have an exact way of knowing whether the target will be able to use stencil, so we do
            // the best we can: if a lazy GL proxy doesn't have a texture, then it might be a
            // wrapped target without stencil, so we conservatively block stencil.
            // FIXME: skbug.com/40043036: GrSurfaceCharacterization needs a "canUseStencil" flag.
            return SkToBool(this->asTextureProxy());
        } else {
            // Otherwise the target will definitely not be wrapped. Ganesh is free to attach
            // stencils on internal render targets.
            return true;
        }
    }
    // Just ask the actual target if we can use stencil.
    GrRenderTarget* rt = this->peekRenderTarget();
    // The dmsaa attachment (if any) always supports stencil. The real question is whether the
    // non-dmsaa attachment supports stencil.
    bool useMSAASurface = rt->numSamples() > 1;
    return rt->getStencilAttachment(useMSAASurface) ||
           rt->canAttemptStencilAttachment(useMSAASurface);
}

sk_sp<GrSurface> GrRenderTargetProxy::createSurface(GrResourceProvider* resourceProvider) const {
    sk_sp<GrSurface> surface = this->createSurfaceImpl(resourceProvider, fSampleCnt,
                                                       GrRenderable::kYes, skgpu::Mipmapped::kNo);
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
                                  colorSamplesPerPixel, skgpu::Mipmapped::kNo,
                                  !this->priv().isExact());
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
            skgpu::Mipmapped::kNo,
            this->numSamples(),
            this->backendFormat(),
            GrTextureType::kNone,
            this->isProtected(),
            this->isBudgeted(),
            this->getLabel(),
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
