/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ganesh/GrSurfaceProxy.h"

#include "include/core/SkAlphaType.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkPoint.h"
#include "include/gpu/GpuTypes.h"
#include "include/gpu/GrRecordingContext.h"
#include "src/gpu/SkBackingFit.h"
#include "src/gpu/Swizzle.h"
#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/GrGpuResourcePriv.h"
#include "src/gpu/ganesh/GrImageInfo.h"
#include "src/gpu/ganesh/GrRecordingContextPriv.h"
#include "src/gpu/ganesh/GrRenderTargetProxy.h"
#include "src/gpu/ganesh/GrRenderTask.h"
#include "src/gpu/ganesh/GrResourceProvider.h"
#include "src/gpu/ganesh/GrSurface.h"
#include "src/gpu/ganesh/GrSurfaceProxyPriv.h"
#include "src/gpu/ganesh/GrSurfaceProxyView.h"
#include "src/gpu/ganesh/GrTexture.h"
#include "src/gpu/ganesh/GrTextureProxy.h"
#include "src/gpu/ganesh/SurfaceContext.h"
#include "src/gpu/ganesh/SurfaceFillContext.h"

#include <memory>

#ifdef SK_DEBUG
#include "include/gpu/GrDirectContext.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"

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

GrSurfaceProxy::LazyCallbackResult::LazyCallbackResult(sk_sp<GrSurface> surf,
                                                       bool releaseCallback,
                                                       LazyInstantiationKeyMode mode)
        : fSurface(std::move(surf)), fKeyMode(mode), fReleaseCallback(releaseCallback) {}
GrSurfaceProxy::LazyCallbackResult::LazyCallbackResult(sk_sp<GrTexture> tex)
        : LazyCallbackResult(sk_sp<GrSurface>(std::move(tex))) {}

// Deferred version
GrSurfaceProxy::GrSurfaceProxy(const GrBackendFormat& format,
                               SkISize dimensions,
                               SkBackingFit fit,
                               skgpu::Budgeted budgeted,
                               GrProtected isProtected,
                               GrInternalSurfaceFlags surfaceFlags,
                               UseAllocator useAllocator,
                               std::string_view label)
        : fSurfaceFlags(surfaceFlags)
        , fFormat(format)
        , fDimensions(dimensions)
        , fFit(fit)
        , fBudgeted(budgeted)
        , fUseAllocator(useAllocator)
        , fIsProtected(isProtected)
        , fLabel(label) {
    SkASSERT(fFormat.isValid());
    SkASSERT(is_valid_non_lazy(dimensions));
}

// Lazy-callback version
GrSurfaceProxy::GrSurfaceProxy(LazyInstantiateCallback&& callback,
                               const GrBackendFormat& format,
                               SkISize dimensions,
                               SkBackingFit fit,
                               skgpu::Budgeted budgeted,
                               GrProtected isProtected,
                               GrInternalSurfaceFlags surfaceFlags,
                               UseAllocator useAllocator,
                               std::string_view label)
        : fSurfaceFlags(surfaceFlags)
        , fFormat(format)
        , fDimensions(dimensions)
        , fFit(fit)
        , fBudgeted(budgeted)
        , fUseAllocator(useAllocator)
        , fLazyInstantiateCallback(std::move(callback))
        , fIsProtected(isProtected)
        , fLabel(label) {
    SkASSERT(fFormat.isValid());
    SkASSERT(fLazyInstantiateCallback);
    SkASSERT(is_valid_lazy(dimensions, fit));
}

// Wrapped version
GrSurfaceProxy::GrSurfaceProxy(sk_sp<GrSurface> surface,
                               SkBackingFit fit,
                               UseAllocator useAllocator)
        : fTarget(std::move(surface))
        , fSurfaceFlags(fTarget->flags())
        , fFormat(fTarget->backendFormat())
        , fDimensions(fTarget->dimensions())
        , fFit(fit)
        , fBudgeted(fTarget->resourcePriv().budgetedType() == GrBudgetedType::kBudgeted
                            ? skgpu::Budgeted::kYes
                            : skgpu::Budgeted::kNo)
        , fUseAllocator(useAllocator)
        , fUniqueID(fTarget->uniqueID())  // Note: converting from unique resource ID to a proxy ID!
        , fIsProtected(fTarget->isProtected() ? GrProtected::kYes : GrProtected::kNo)
        , fLabel(fTarget->getLabel()) {
    SkASSERT(fFormat.isValid());
}

GrSurfaceProxy::~GrSurfaceProxy() {
}

sk_sp<GrSurface> GrSurfaceProxy::createSurfaceImpl(GrResourceProvider* resourceProvider,
                                                   int sampleCnt,
                                                   GrRenderable renderable,
                                                   skgpu::Mipmapped mipmapped) const {
    SkASSERT(mipmapped == skgpu::Mipmapped::kNo || fFit == SkBackingFit::kExact);
    SkASSERT(!this->isLazy());
    SkASSERT(!fTarget);

    sk_sp<GrSurface> surface;
    if (SkBackingFit::kApprox == fFit) {
        surface = resourceProvider->createApproxTexture(fDimensions,
                                                        fFormat,
                                                        fFormat.textureType(),
                                                        renderable,
                                                        sampleCnt,
                                                        fIsProtected,
                                                        fLabel);
    } else {
        surface = resourceProvider->createTexture(fDimensions,
                                                  fFormat,
                                                  fFormat.textureType(),
                                                  renderable,
                                                  sampleCnt,
                                                  mipmapped,
                                                  fBudgeted,
                                                  fIsProtected,
                                                  fLabel);
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

    // In order to give DDL users some flexibility in the destination of there DDLs,
    // a DDL's target proxy can be more conservative (and thus require less memory)
    // than the actual GrSurface used to fulfill it.
    if (!this->isDDLTarget() && kInvalidGpuMemorySize != this->getRawGpuMemorySize_debugOnly()) {
        // TODO(11373): Can this check be exact?
        SkASSERT(fTarget->gpuMemorySize() <= this->getRawGpuMemorySize_debugOnly());
    }
#endif
}

bool GrSurfaceProxy::instantiateImpl(GrResourceProvider* resourceProvider,
                                     int sampleCnt,
                                     GrRenderable renderable,
                                     skgpu::Mipmapped mipmapped,
                                     const skgpu::UniqueKey* uniqueKey) {
    SkASSERT(!this->isLazy());
    if (fTarget) {
        if (uniqueKey && uniqueKey->isValid()) {
            SkASSERT(fTarget->getUniqueKey().isValid() && fTarget->getUniqueKey() == *uniqueKey);
        }
        return true;
    }

    sk_sp<GrSurface> surface = this->createSurfaceImpl(resourceProvider, sampleCnt, renderable,
                                                       mipmapped);
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

void GrSurfaceProxy::computeScratchKey(const GrCaps& caps, skgpu::ScratchKey* key) const {
    SkASSERT(!this->isFullyLazy());
    GrRenderable renderable = GrRenderable::kNo;
    int sampleCount = 1;
    if (const auto* rtp = this->asRenderTargetProxy()) {
        renderable = GrRenderable::kYes;
        sampleCount = rtp->numSamples();
    }

    const GrTextureProxy* tp = this->asTextureProxy();
    skgpu::Mipmapped mipmapped = skgpu::Mipmapped::kNo;
    if (tp) {
        mipmapped = tp->mipmapped();
    }

    GrTexture::ComputeScratchKey(caps, this->backendFormat(), this->backingStoreDimensions(),
                                 renderable, sampleCount, mipmapped, fIsProtected, key);
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
        SkASSERT(fTarget->getContext()->priv().matches(context));
    }
}
#endif

sk_sp<GrSurfaceProxy> GrSurfaceProxy::Copy(GrRecordingContext* rContext,
                                           sk_sp<GrSurfaceProxy> src,
                                           GrSurfaceOrigin origin,
                                           skgpu::Mipmapped mipmapped,
                                           SkIRect srcRect,
                                           SkBackingFit fit,
                                           skgpu::Budgeted budgeted,
                                           std::string_view label,
                                           RectsMustMatch rectsMustMatch,
                                           sk_sp<GrRenderTask>* outTask) {
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
        GrImageInfo info(GrColorType::kUnknown, kUnknown_SkAlphaType, nullptr, {width, height});
        auto dstContext = rContext->priv().makeSC(info,
                                                  format,
                                                  label,
                                                  fit,
                                                  origin,
                                                  GrRenderable::kNo,
                                                  1,
                                                  mipmapped,
                                                  src->isProtected(),
                                                  budgeted);
        sk_sp<GrRenderTask> copyTask;
        if (dstContext && (copyTask = dstContext->copy(src, srcRect, dstPoint))) {
            if (outTask) {
                *outTask = std::move(copyTask);
            }
            return dstContext->asSurfaceProxyRef();
        }
    }
    if (src->asTextureProxy()) {
        auto dstContext = rContext->priv().makeSFC(kUnknown_SkAlphaType,
                                                   nullptr,
                                                   {width, height},
                                                   fit,
                                                   format,
                                                   1,
                                                   mipmapped,
                                                   src->isProtected(),
                                                   skgpu::Swizzle::RGBA(),
                                                   skgpu::Swizzle::RGBA(),
                                                   origin,
                                                   budgeted,
                                                   label);
        GrSurfaceProxyView view(std::move(src), origin, skgpu::Swizzle::RGBA());
        if (dstContext && dstContext->blitTexture(std::move(view), srcRect, dstPoint)) {
            if (outTask) {
                *outTask = dstContext->refRenderTask();
            }
            return dstContext->asSurfaceProxyRef();
        }
    }
    // Can't use backend copies or draws.
    return nullptr;
}

sk_sp<GrSurfaceProxy> GrSurfaceProxy::Copy(GrRecordingContext* context,
                                           sk_sp<GrSurfaceProxy> src,
                                           GrSurfaceOrigin origin,
                                           skgpu::Mipmapped mipmapped,
                                           SkBackingFit fit,
                                           skgpu::Budgeted budgeted,
                                           std::string_view label,
                                           sk_sp<GrRenderTask>* outTask) {
    SkASSERT(!src->isFullyLazy());
    auto rect = SkIRect::MakeSize(src->dimensions());
    return Copy(context,
                std::move(src),
                origin,
                mipmapped,
                rect,
                fit,
                budgeted,
                label,
                RectsMustMatch::kNo,
                outTask);
}

#if defined(GR_TEST_UTILS)
int32_t GrSurfaceProxy::testingOnly_getBackingRefCnt() const {
    if (fTarget) {
        return fTarget->testingOnly_getRefCnt();
    }

    return -1; // no backing GrSurface
}

GrInternalSurfaceFlags GrSurfaceProxy::testingOnly_getFlags() const {
    return fSurfaceFlags;
}

SkString GrSurfaceProxy::dump() const {
    SkString tmp;

    tmp.appendf("proxyID: %d - surfaceID: %d",
                this->uniqueID().asUInt(),
                this->peekSurface() ? this->peekSurface()->uniqueID().asUInt()
                                    : -1);
    return tmp;
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
    if (const auto& uniqueKey = fProxy->getUniqueKey(); uniqueKey.isValid()) {
        // First try to reattach to a cached version if the proxy is uniquely keyed
        surface = resourceProvider->findByUniqueKey<GrSurface>(uniqueKey);
    }

    bool syncKey = true;
    bool releaseCallback = false;
    if (!surface) {
        auto result = fProxy->fLazyInstantiateCallback(resourceProvider, fProxy->callbackDesc());
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
            const skgpu::UniqueKey& key = texProxy->getUniqueKey();
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
    SkASSERTF(surface->backendFormat() == fFormat, "%s != %s",
              surface->backendFormat().toStr().c_str(), fFormat.toStr().c_str());

    this->onValidateSurface(surface);
}
#endif
