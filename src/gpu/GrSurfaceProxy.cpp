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
#include "src/gpu/GrRenderTargetContext.h"
#include "src/gpu/GrStencilAttachment.h"
#include "src/gpu/GrSurfacePriv.h"
#include "src/gpu/GrTexturePriv.h"
#include "src/gpu/GrTextureRenderTargetProxy.h"

#ifdef SK_DEBUG
#include "src/gpu/GrRenderTarget.h"
#include "src/gpu/GrRenderTargetPriv.h"

static bool is_valid_lazy(const SkISize& dimensions, SkBackingFit fit) {
    // A "fully" lazy proxy's width and height are not known until instantiation time.
    // So fully lazy proxies are created with width and height < 0. Regular lazy proxies must be
    // created with positive widths and heights. The width and height are set to 0 only after a
    // failed instantiation. The former must be "approximate" fit while the latter can be either.
    return ((dimensions.fWidth < 0 && dimensions.fHeight < 0 && SkBackingFit::kApprox == fit) ||
            (dimensions.fWidth > 0 && dimensions.fHeight > 0));
}

static bool is_valid_non_lazy(SkISize dimensions) {
    return dimensions.fWidth > 0 && dimensions.fHeight > 0;
}
#endif

// Deferred version
GrSurfaceProxy::GrSurfaceProxy(const GrBackendFormat& format,
                               SkISize dimensions,
                               SkBackingFit fit,
                               SkBudgeted budgeted,
                               GrProtected isProtected,
                               GrInternalSurfaceFlags surfaceFlags,
                               UseAllocator useAllocator)
        : fSurfaceFlags(surfaceFlags)
        , fFormat(format)
        , fDimensions(dimensions)
        , fFit(fit)
        , fBudgeted(budgeted)
        , fUseAllocator(useAllocator)
        , fIsProtected(isProtected)
        , fGpuMemorySize(kInvalidGpuMemorySize) {
    SkASSERT(fFormat.isValid());
    SkASSERT(is_valid_non_lazy(dimensions));
}

// Lazy-callback version
GrSurfaceProxy::GrSurfaceProxy(LazyInstantiateCallback&& callback,
                               const GrBackendFormat& format,
                               SkISize dimensions,
                               SkBackingFit fit,
                               SkBudgeted budgeted,
                               GrProtected isProtected,
                               GrInternalSurfaceFlags surfaceFlags,
                               UseAllocator useAllocator)
        : fSurfaceFlags(surfaceFlags)
        , fFormat(format)
        , fDimensions(dimensions)
        , fFit(fit)
        , fBudgeted(budgeted)
        , fUseAllocator(useAllocator)
        , fLazyInstantiateCallback(std::move(callback))
        , fIsProtected(isProtected)
        , fGpuMemorySize(kInvalidGpuMemorySize) {
    SkASSERT(fFormat.isValid());
    SkASSERT(fLazyInstantiateCallback);
    SkASSERT(is_valid_lazy(dimensions, fit));
}

// Wrapped version
GrSurfaceProxy::GrSurfaceProxy(sk_sp<GrSurface> surface,
                               SkBackingFit fit,
                               UseAllocator useAllocator)
        : fTarget(std::move(surface))
        , fSurfaceFlags(fTarget->surfacePriv().flags())
        , fFormat(fTarget->backendFormat())
        , fDimensions(fTarget->dimensions())
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

sk_sp<GrSurface> GrSurfaceProxy::createSurfaceImpl(GrResourceProvider* resourceProvider,
                                                   int sampleCnt,
                                                   GrRenderable renderable,
                                                   GrMipMapped mipMapped) const {
    SkASSERT(mipMapped == GrMipMapped::kNo || fFit == SkBackingFit::kExact);
    SkASSERT(!this->isLazy());
    SkASSERT(!fTarget);

    sk_sp<GrSurface> surface;
    if (SkBackingFit::kApprox == fFit) {
        surface = resourceProvider->createApproxTexture(fDimensions, fFormat, renderable, sampleCnt,
                                                        fIsProtected);
    } else {
        surface = resourceProvider->createTexture(fDimensions, fFormat, renderable, sampleCnt,
                                                  mipMapped, fBudgeted, fIsProtected);
    }
    if (!surface) {
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
    }

    if (kInvalidGpuMemorySize != this->getRawGpuMemorySize_debugOnly()) {
        SkASSERT(fTarget->gpuMemorySize() <= this->getRawGpuMemorySize_debugOnly());
    }
#endif
}

bool GrSurfaceProxy::instantiateImpl(GrResourceProvider* resourceProvider, int sampleCnt,
                                     GrRenderable renderable, GrMipMapped mipMapped,
                                     const GrUniqueKey* uniqueKey) {
    SkASSERT(!this->isLazy());
    if (fTarget) {
        if (uniqueKey && uniqueKey->isValid()) {
            SkASSERT(fTarget->getUniqueKey().isValid() && fTarget->getUniqueKey() == *uniqueKey);
        }
        return true;
    }

    sk_sp<GrSurface> surface = this->createSurfaceImpl(resourceProvider, sampleCnt, renderable,
                                                       mipMapped);
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

void GrSurfaceProxy::computeScratchKey(const GrCaps& caps, GrScratchKey* key) const {
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

    GrTexturePriv::ComputeScratchKey(caps, this->backendFormat(), this->backingStoreDimensions(),
                                     renderable, sampleCount, mipMapped, fIsProtected, key);
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

SkISize GrSurfaceProxy::backingStoreDimensions() const {
    SkASSERT(!this->isFullyLazy());
    if (fTarget) {
        return fTarget->dimensions();
    }

    if (SkBackingFit::kExact == fFit) {
        return fDimensions;
    }
    return GrResourceProvider::MakeApprox(fDimensions);
}

bool GrSurfaceProxy::isFunctionallyExact() const {
    SkASSERT(!this->isFullyLazy());
    return fFit == SkBackingFit::kExact ||
           fDimensions == GrResourceProvider::MakeApprox(fDimensions);
}

bool GrSurfaceProxy::isFormatCompressed(const GrCaps* caps) const {
    return caps->isFormatCompressed(this->backendFormat());
}

#ifdef SK_DEBUG
void GrSurfaceProxy::validate(GrContext_Base* context) const {
    if (fTarget) {
        SkASSERT(fTarget->getContext() == context);
    }
}
#endif

GrSurfaceProxyView GrSurfaceProxy::Copy(GrRecordingContext* context,
                                        GrSurfaceProxy* src,
                                        GrSurfaceOrigin origin,
                                        GrColorType srcColorType,
                                        GrMipMapped mipMapped,
                                        SkIRect srcRect,
                                        SkBackingFit fit,
                                        SkBudgeted budgeted,
                                        RectsMustMatch rectsMustMatch) {
    SkASSERT(!src->isFullyLazy());
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

    if (!srcRect.intersect(SkIRect::MakeSize(src->dimensions()))) {
        return {};
    }
    auto format = src->backendFormat().makeTexture2D();
    SkASSERT(format.isValid());

    if (src->backendFormat().textureType() != GrTextureType::kExternal) {
        auto dstContext = GrSurfaceContext::Make(context, {width, height}, format,
                                                 GrRenderable::kNo, 1, mipMapped,
                                                 src->isProtected(), origin, srcColorType,
                                                 kUnknown_SkAlphaType, nullptr, fit, budgeted);
        if (dstContext && dstContext->copy(src, origin, srcRect, dstPoint)) {
            return dstContext->readSurfaceView();
        }
    }
    if (src->asTextureProxy()) {
        auto dstContext = GrRenderTargetContext::Make(context, srcColorType, nullptr, fit,
                                                      {width, height}, format, 1,
                                                      mipMapped, src->isProtected(), origin,
                                                      budgeted, nullptr);
        GrSwizzle swizzle = context->priv().caps()->getReadSwizzle(src->backendFormat(),
                                                                   srcColorType);
        GrSurfaceProxyView view(sk_ref_sp(src), origin, swizzle);
        if (dstContext && dstContext->blitTexture(std::move(view), srcRect, dstPoint)) {
            return dstContext->readSurfaceView();
        }
    }
    // Can't use backend copies or draws.
    return {};
}

GrSurfaceProxyView GrSurfaceProxy::Copy(GrRecordingContext* context, GrSurfaceProxy* src,
                                        GrSurfaceOrigin origin, GrColorType srcColorType,
                                        GrMipMapped mipMapped, SkBackingFit fit,
                                        SkBudgeted budgeted) {
    SkASSERT(!src->isFullyLazy());
    return Copy(context, src, origin, srcColorType, mipMapped, SkIRect::MakeSize(src->dimensions()),
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
        fProxy->fDimensions = fProxy->fTarget->dimensions();
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
        fProxy->fDimensions.setEmpty();
        return false;
    }

    if (fProxy->isFullyLazy()) {
        // This was a fully lazy proxy. We need to fill in the width & height. For partially
        // lazy proxies we must preserve the original width & height since that indicates
        // the content area.
        fProxy->fDimensions = surface->dimensions();
    }

    SkASSERT(fProxy->width() <= surface->width());
    SkASSERT(fProxy->height() <= surface->height());

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
    SkASSERT(surface->backendFormat() == fFormat);

    this->onValidateSurface(surface);
}
#endif
