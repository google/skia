/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrSurfaceProxy.h"
#include "src/gpu/GrSurfaceProxyPriv.h"

#include "include/gpu/GrContext.h"
#include "include/private/GrRecordingContext.h"
#include "src/core/SkMathPriv.h"
#include "src/core/SkMipMap.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrClip.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrGpuResourcePriv.h"
#include "src/gpu/GrOpsTask.h"
#include "src/gpu/GrProxyProvider.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/GrStencilAttachment.h"
#include "src/gpu/GrSurfacePriv.h"
#include "src/gpu/GrTextureContext.h"
#include "src/gpu/GrTexturePriv.h"
#include "src/gpu/GrTextureRenderTargetProxy.h"

#ifdef SK_DEBUG
#include "src/gpu/GrRenderTarget.h"
#include "src/gpu/GrRenderTargetPriv.h"

static bool is_valid_lazy(const GrSurfaceDesc& desc, SkBackingFit fit) {
    // A "fully" lazy proxy's width and height are not known until instantiation time.
    // So fully lazy proxies are created with width and height < 0. Regular lazy proxies must be
    // created with positive widths and heights. The width and height are set to 0 only after a
    // failed instantiation. The former must be "approximate" fit while the latter can be either.
    return desc.fConfig != kUnknown_GrPixelConfig &&
           ((desc.fWidth < 0 && desc.fHeight < 0 && SkBackingFit::kApprox == fit) ||
            (desc.fWidth > 0 && desc.fHeight > 0));
}

static bool is_valid_non_lazy(const GrSurfaceDesc& desc) {
    return desc.fWidth > 0 && desc.fHeight > 0 && desc.fConfig != kUnknown_GrPixelConfig;
}
#endif

// Deferred version
GrSurfaceProxy::GrSurfaceProxy(const GrBackendFormat& format,
                               const GrSurfaceDesc& desc,
                               GrRenderable renderable,
                               GrSurfaceOrigin origin,
                               const GrSwizzle& textureSwizzle,
                               SkBackingFit fit,
                               SkBudgeted budgeted,
                               GrProtected isProtected,
                               GrInternalSurfaceFlags surfaceFlags,
                               UseAllocator useAllocator)
        : fSurfaceFlags(surfaceFlags)
        , fFormat(format)
        , fConfig(desc.fConfig)
        , fWidth(desc.fWidth)
        , fHeight(desc.fHeight)
        , fOrigin(origin)
        , fTextureSwizzle(textureSwizzle)
        , fFit(fit)
        , fBudgeted(budgeted)
        , fUseAllocator(useAllocator)
        , fIsProtected(isProtected)
        , fGpuMemorySize(kInvalidGpuMemorySize) {
    SkASSERT(fFormat.isValid());
    SkASSERT(is_valid_non_lazy(desc));
}

// Lazy-callback version
GrSurfaceProxy::GrSurfaceProxy(LazyInstantiateCallback&& callback,
                               const GrBackendFormat& format,
                               const GrSurfaceDesc& desc,
                               GrRenderable renderable,
                               GrSurfaceOrigin origin,
                               const GrSwizzle& textureSwizzle,
                               SkBackingFit fit,
                               SkBudgeted budgeted,
                               GrProtected isProtected,
                               GrInternalSurfaceFlags surfaceFlags,
                               UseAllocator useAllocator)
        : fSurfaceFlags(surfaceFlags)
        , fFormat(format)
        , fConfig(desc.fConfig)
        , fWidth(desc.fWidth)
        , fHeight(desc.fHeight)
        , fOrigin(origin)
        , fTextureSwizzle(textureSwizzle)
        , fFit(fit)
        , fBudgeted(budgeted)
        , fUseAllocator(useAllocator)
        , fLazyInstantiateCallback(std::move(callback))
        , fIsProtected(isProtected)
        , fGpuMemorySize(kInvalidGpuMemorySize) {
    SkASSERT(fFormat.isValid());
    SkASSERT(fLazyInstantiateCallback);
    SkASSERT(is_valid_lazy(desc, fit));
}

// Wrapped version
GrSurfaceProxy::GrSurfaceProxy(sk_sp<GrSurface> surface,
                               GrSurfaceOrigin origin,
                               const GrSwizzle& textureSwizzle,
                               SkBackingFit fit,
                               UseAllocator useAllocator)
        : fTarget(std::move(surface))
        , fSurfaceFlags(fTarget->surfacePriv().flags())
        , fFormat(fTarget->backendFormat())
        , fConfig(fTarget->config())
        , fWidth(fTarget->width())
        , fHeight(fTarget->height())
        , fOrigin(origin)
        , fTextureSwizzle(textureSwizzle)
        , fFit(fit)
        , fBudgeted(fTarget->resourcePriv().budgetedType() == GrBudgetedType::kBudgeted
                            ? SkBudgeted::kYes
                            : SkBudgeted::kNo)
        , fUseAllocator(useAllocator)
        , fUniqueID(fTarget->uniqueID())  // Note: converting from unique resource ID to a proxy ID!
        , fIsProtected(fTarget->isProtected() ? GrProtected::kYes : GrProtected::kNo)
        , fGpuMemorySize(kInvalidGpuMemorySize) {
    SkASSERT(fFormat.isValid());
}

GrSurfaceProxy::~GrSurfaceProxy() {
    // For this to be deleted the opsTask that held a ref on it (if there was one) must have been
    // deleted. Which would have cleared out this back pointer.
    SkASSERT(!fLastRenderTask);
}

bool GrSurfaceProxyPriv::AttachStencilIfNeeded(GrResourceProvider* resourceProvider,
                                               GrSurface* surface, int minStencilSampleCount) {
    if (minStencilSampleCount) {
        GrRenderTarget* rt = surface->asRenderTarget();
        if (!rt) {
            SkASSERT(0);
            return false;
        }

        if (!resourceProvider->attachStencilAttachment(rt, minStencilSampleCount)) {
            return false;
        }
    }

    return true;
}

sk_sp<GrSurface> GrSurfaceProxy::createSurfaceImpl(GrResourceProvider* resourceProvider,
                                                   int sampleCnt,
                                                   int minStencilSampleCount,
                                                   GrRenderable renderable,
                                                   GrMipMapped mipMapped) const {
    SkASSERT(mipMapped == GrMipMapped::kNo || fFit == SkBackingFit::kExact);
    SkASSERT(!this->isLazy());
    SkASSERT(!fTarget);
    GrSurfaceDesc desc;
    desc.fWidth = fWidth;
    desc.fHeight = fHeight;
    desc.fConfig = fConfig;

    sk_sp<GrSurface> surface;
    if (SkBackingFit::kApprox == fFit) {
        surface = resourceProvider->createApproxTexture(desc, fFormat, renderable, sampleCnt,
                                                        fIsProtected);
    } else {
        surface = resourceProvider->createTexture(desc, fFormat, renderable, sampleCnt, mipMapped,
                                                  fBudgeted, fIsProtected);
    }
    if (!surface) {
        return nullptr;
    }

    if (!GrSurfaceProxyPriv::AttachStencilIfNeeded(resourceProvider, surface.get(),
                                                   minStencilSampleCount)) {
        return nullptr;
    }

    return surface;
}

bool GrSurfaceProxy::canSkipResourceAllocator() const {
    if (fUseAllocator == UseAllocator::kNo) {
        // Usually an atlas or onFlush proxy
        return true;
    }

    auto peek = this->peekSurface();
    if (!peek) {
        return false;
    }
    // If this resource is already allocated and not recyclable then the resource allocator does
    // not need to do anything with it.
    return !peek->resourcePriv().getScratchKey().isValid();
}

void GrSurfaceProxy::assign(sk_sp<GrSurface> surface) {
    SkASSERT(!fTarget && surface);

    SkDEBUGCODE(this->validateSurface(surface.get());)

    fTarget = std::move(surface);

#ifdef SK_DEBUG
    if (this->asRenderTargetProxy()) {
        SkASSERT(fTarget->asRenderTarget());
        if (int minStencilSampleCount = this->asRenderTargetProxy()->numStencilSamples()) {
            auto* stencil = fTarget->asRenderTarget()->renderTargetPriv().getStencilAttachment();
            SkASSERT(stencil);
            SkASSERT(stencil->numSamples() >= minStencilSampleCount);
        }
    }

    if (kInvalidGpuMemorySize != this->getRawGpuMemorySize_debugOnly()) {
        SkASSERT(fTarget->gpuMemorySize() <= this->getRawGpuMemorySize_debugOnly());
    }
#endif
}

bool GrSurfaceProxy::instantiateImpl(GrResourceProvider* resourceProvider, int sampleCnt,
                                     int minStencilSampleCount, GrRenderable renderable,
                                     GrMipMapped mipMapped, const GrUniqueKey* uniqueKey) {
    SkASSERT(!this->isLazy());
    if (fTarget) {
        if (uniqueKey && uniqueKey->isValid()) {
            SkASSERT(fTarget->getUniqueKey().isValid() && fTarget->getUniqueKey() == *uniqueKey);
        }
        return GrSurfaceProxyPriv::AttachStencilIfNeeded(resourceProvider, fTarget.get(),
                                                         minStencilSampleCount);
    }

    sk_sp<GrSurface> surface = this->createSurfaceImpl(
            resourceProvider, sampleCnt, minStencilSampleCount, renderable, mipMapped);
    if (!surface) {
        return false;
    }

    // If there was an invalidation message pending for this key, we might have just processed it,
    // causing the key (stored on this proxy) to become invalid.
    if (uniqueKey && uniqueKey->isValid()) {
        resourceProvider->assignUniqueKeyToResource(*uniqueKey, surface.get());
    }

    this->assign(std::move(surface));

    return true;
}

void GrSurfaceProxy::deinstantiate() {
    SkASSERT(this->isInstantiated());
    fTarget = nullptr;
}

void GrSurfaceProxy::computeScratchKey(GrScratchKey* key) const {
    SkASSERT(!this->isFullyLazy());
    GrRenderable renderable = GrRenderable::kNo;
    int sampleCount = 1;
    if (const auto* rtp = this->asRenderTargetProxy()) {
        renderable = GrRenderable::kYes;
        sampleCount = rtp->numSamples();
    }

    const GrTextureProxy* tp = this->asTextureProxy();
    GrMipMapped mipMapped = GrMipMapped::kNo;
    if (tp) {
        mipMapped = tp->mipMapped();
    }

    int width = this->worstCaseWidth();
    int height = this->worstCaseHeight();

    GrTexturePriv::ComputeScratchKey(this->config(), width, height, renderable, sampleCount,
                                     mipMapped, fIsProtected, key);
}

void GrSurfaceProxy::setLastRenderTask(GrRenderTask* renderTask) {
#ifdef SK_DEBUG
    if (fLastRenderTask) {
        SkASSERT(fLastRenderTask->isClosed());
    }
#endif

    // Un-reffed
    fLastRenderTask = renderTask;
}

GrOpsTask* GrSurfaceProxy::getLastOpsTask() {
    return fLastRenderTask ? fLastRenderTask->asOpsTask() : nullptr;
}

int GrSurfaceProxy::worstCaseWidth() const {
    SkASSERT(!this->isFullyLazy());
    if (fTarget) {
        return fTarget->width();
    }

    if (SkBackingFit::kExact == fFit) {
        return fWidth;
    }
    return GrResourceProvider::MakeApprox(fWidth);
}

int GrSurfaceProxy::worstCaseHeight() const {
    SkASSERT(!this->isFullyLazy());
    if (fTarget) {
        return fTarget->height();
    }

    if (SkBackingFit::kExact == fFit) {
        return fHeight;
    }
    return GrResourceProvider::MakeApprox(fHeight);
}

#ifdef SK_DEBUG
void GrSurfaceProxy::validate(GrContext_Base* context) const {
    if (fTarget) {
        SkASSERT(fTarget->getContext() == context);
    }
}
#endif

sk_sp<GrTextureProxy> GrSurfaceProxy::Copy(GrRecordingContext* context,
                                           GrSurfaceProxy* src,
                                           GrColorType srcColorType,
                                           GrMipMapped mipMapped,
                                           SkIRect srcRect,
                                           SkBackingFit fit,
                                           SkBudgeted budgeted,
                                           RectsMustMatch rectsMustMatch) {
    SkASSERT(!src->isFullyLazy());
    GrProtected isProtected = src->isProtected() ? GrProtected::kYes : GrProtected::kNo;
    int width;
    int height;

    SkIPoint dstPoint;
    if (rectsMustMatch == RectsMustMatch::kYes) {
        width = src->width();
        height = src->height();
        dstPoint = {srcRect.fLeft, srcRect.fTop};
    } else {
        width = srcRect.width();
        height = srcRect.height();
        dstPoint = {0, 0};
    }

    if (!srcRect.intersect(SkIRect::MakeWH(src->width(), src->height()))) {
        return nullptr;
    }
    auto colorType = GrPixelConfigToColorType(src->config());
    if (src->backendFormat().textureType() != GrTextureType::kExternal) {
        auto dstContext = context->priv().makeDeferredTextureContext(
                fit, width, height, colorType, kUnknown_SkAlphaType, nullptr, mipMapped,
                src->origin(), budgeted, isProtected);
        if (!dstContext) {
            return nullptr;
        }
        if (dstContext->copy(src, srcRect, dstPoint)) {
            return dstContext->asTextureProxyRef();
        }
    }
    if (src->asTextureProxy()) {
        auto dstContext = context->priv().makeDeferredRenderTargetContext(
                fit, width, height, colorType, nullptr, 1, mipMapped, src->origin(), nullptr,
                budgeted);

        if (dstContext && dstContext->blitTexture(src->asTextureProxy(), srcColorType, srcRect,
                                                  dstPoint)) {
            return dstContext->asTextureProxyRef();
        }
    }
    // Can't use backend copies or draws.
    return nullptr;
}

sk_sp<GrTextureProxy> GrSurfaceProxy::Copy(GrRecordingContext* context, GrSurfaceProxy* src,
                                           GrColorType srcColorType, GrMipMapped mipMapped,
                                           SkBackingFit fit, SkBudgeted budgeted) {
    SkASSERT(!src->isFullyLazy());
    return Copy(context, src, srcColorType, mipMapped, SkIRect::MakeWH(src->width(), src->height()),
                fit, budgeted);
}

#if GR_TEST_UTILS
int32_t GrSurfaceProxy::testingOnly_getBackingRefCnt() const {
    if (fTarget) {
        return fTarget->testingOnly_getRefCnt();
    }

    return -1; // no backing GrSurface
}

GrInternalSurfaceFlags GrSurfaceProxy::testingOnly_getFlags() const {
    return fSurfaceFlags;
}
#endif

void GrSurfaceProxyPriv::exactify(bool allocatedCaseOnly) {
    SkASSERT(!fProxy->isFullyLazy());
    if (this->isExact()) {
        return;
    }

    SkASSERT(SkBackingFit::kApprox == fProxy->fFit);

    if (fProxy->fTarget) {
        // The kApprox but already instantiated case. Setting the proxy's width & height to
        // the instantiated width & height could have side-effects going forward, since we're
        // obliterating the area of interest information. This call (exactify) only used
        // when converting an SkSpecialImage to an SkImage so the proxy shouldn't be
        // used for additional draws.
        fProxy->fWidth = fProxy->fTarget->width();
        fProxy->fHeight = fProxy->fTarget->height();
        return;
    }

#ifndef SK_CRIPPLE_TEXTURE_REUSE
    // In the post-implicit-allocation world we can't convert this proxy to be exact fit
    // at this point. With explicit allocation switching this to exact will result in a
    // different allocation at flush time. With implicit allocation, allocation would occur
    // at draw time (rather than flush time) so this pathway was encountered less often (if
    // at all).
    if (allocatedCaseOnly) {
        return;
    }
#endif

    // The kApprox uninstantiated case. Making this proxy be exact should be okay.
    // It could mess things up if prior decisions were based on the approximate size.
    fProxy->fFit = SkBackingFit::kExact;
    // If fGpuMemorySize is used when caching specialImages for the image filter DAG. If it has
    // already been computed we want to leave it alone so that amount will be removed when
    // the special image goes away. If it hasn't been computed yet it might as well compute the
    // exact amount.
}

bool GrSurfaceProxyPriv::doLazyInstantiation(GrResourceProvider* resourceProvider) {
    SkASSERT(fProxy->isLazy());

    sk_sp<GrSurface> surface;
    if (fProxy->asTextureProxy() && fProxy->asTextureProxy()->getUniqueKey().isValid()) {
        // First try to reattach to a cached version if the proxy is uniquely keyed
        surface = resourceProvider->findByUniqueKey<GrSurface>(
                                                        fProxy->asTextureProxy()->getUniqueKey());
    }

    bool syncKey = true;
    bool releaseCallback = false;
    if (!surface) {
        auto result = fProxy->fLazyInstantiateCallback(resourceProvider);
        surface = std::move(result.fSurface);
        syncKey = result.fKeyMode == GrSurfaceProxy::LazyInstantiationKeyMode::kSynced;
        releaseCallback = surface && result.fReleaseCallback;
    }
    if (!surface) {
        fProxy->fWidth = 0;
        fProxy->fHeight = 0;
        return false;
    }

    if (fProxy->isFullyLazy()) {
        // This was a fully lazy proxy. We need to fill in the width & height. For partially
        // lazy proxies we must preserve the original width & height since that indicates
        // the content area.
        fProxy->fWidth = surface->width();
        fProxy->fHeight = surface->height();
    }

    SkASSERT(fProxy->fWidth <= surface->width());
    SkASSERT(fProxy->fHeight <= surface->height());

    auto rt = fProxy->asRenderTargetProxy();
    int minStencilSampleCount = rt ? rt->numSamples() : 0;

    if (!GrSurfaceProxyPriv::AttachStencilIfNeeded(
            resourceProvider, surface.get(), minStencilSampleCount)) {
        return false;
    }

    if (GrTextureProxy* texProxy = fProxy->asTextureProxy()) {
        texProxy->setTargetKeySync(syncKey);
        if (syncKey) {
            const GrUniqueKey& key = texProxy->getUniqueKey();
            if (key.isValid()) {
                if (!surface->asTexture()->getUniqueKey().isValid()) {
                    // If 'surface' is newly created, attach the unique key
                    resourceProvider->assignUniqueKeyToResource(key, surface.get());
                } else {
                    // otherwise we had better have reattached to a cached version
                    SkASSERT(surface->asTexture()->getUniqueKey() == key);
                }
            } else {
                SkASSERT(!surface->getUniqueKey().isValid());
            }
        }
    }

    this->assign(std::move(surface));
    if (releaseCallback) {
        fProxy->fLazyInstantiateCallback = nullptr;
    }

    return true;
}

#ifdef SK_DEBUG
void GrSurfaceProxy::validateSurface(const GrSurface* surface) {
    SkASSERT(surface->config() == fConfig);

    this->onValidateSurface(surface);
}
#endif
