/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrSurfaceProxy.h"
#include "GrSurfaceProxyPriv.h"

#include "GrCaps.h"
#include "GrContext.h"
#include "GrContextPriv.h"
#include "GrGpuResourcePriv.h"
#include "GrOpList.h"
#include "GrProxyProvider.h"
#include "GrRecordingContext.h"
#include "GrRecordingContextPriv.h"
#include "GrSurfaceContext.h"
#include "GrSurfacePriv.h"
#include "GrTexturePriv.h"
#include "GrTextureRenderTargetProxy.h"

#include "SkMathPriv.h"
#include "SkMipMap.h"

#ifdef SK_DEBUG
#include "GrRenderTarget.h"
#include "GrRenderTargetPriv.h"

static bool is_valid_fully_lazy(const GrSurfaceDesc& desc, SkBackingFit fit) {
    return desc.fWidth <= 0 &&
           desc.fHeight <= 0 &&
           desc.fConfig != kUnknown_GrPixelConfig &&
           desc.fSampleCnt == 1 &&
           SkBackingFit::kApprox == fit;
}

static bool is_valid_partially_lazy(const GrSurfaceDesc& desc) {
    return ((desc.fWidth > 0 && desc.fHeight > 0) ||
            (desc.fWidth <= 0 && desc.fHeight <= 0))  &&
           desc.fConfig != kUnknown_GrPixelConfig;
}

static bool is_valid_non_lazy(const GrSurfaceDesc& desc) {
    return desc.fWidth > 0 &&
           desc.fHeight > 0 &&
           desc.fConfig != kUnknown_GrPixelConfig;
}
#endif

// Lazy-callback version
GrSurfaceProxy::GrSurfaceProxy(LazyInstantiateCallback&& callback, LazyInstantiationType lazyType,
                               const GrBackendFormat& format, const GrSurfaceDesc& desc,
                               GrSurfaceOrigin origin, SkBackingFit fit,
                               SkBudgeted budgeted, GrInternalSurfaceFlags surfaceFlags)
        : fSurfaceFlags(surfaceFlags)
        , fFormat(format)
        , fConfig(desc.fConfig)
        , fWidth(desc.fWidth)
        , fHeight(desc.fHeight)
        , fOrigin(origin)
        , fFit(fit)
        , fBudgeted(budgeted)
        , fLazyInstantiateCallback(std::move(callback))
        , fLazyInstantiationType(lazyType)
        , fNeedsClear(SkToBool(desc.fFlags & kPerformInitialClear_GrSurfaceFlag))
        , fGpuMemorySize(kInvalidGpuMemorySize)
        , fLastOpList(nullptr) {
    SkASSERT(fFormat.isValid());
    // NOTE: the default fUniqueID ctor pulls a value from the same pool as the GrGpuResources.
    if (fLazyInstantiateCallback) {
        SkASSERT(is_valid_fully_lazy(desc, fit) || is_valid_partially_lazy(desc));
    } else {
        SkASSERT(is_valid_non_lazy(desc));
    }

    if (GrPixelConfigIsCompressed(desc.fConfig)) {
        SkASSERT(!SkToBool(desc.fFlags & kRenderTarget_GrSurfaceFlag));
        fSurfaceFlags |= GrInternalSurfaceFlags::kReadOnly;
    }
}

// Wrapped version
GrSurfaceProxy::GrSurfaceProxy(sk_sp<GrSurface> surface, GrSurfaceOrigin origin, SkBackingFit fit)
        : INHERITED(std::move(surface))
        , fSurfaceFlags(fTarget->surfacePriv().flags())
        , fFormat(fTarget->backendFormat())
        , fConfig(fTarget->config())
        , fWidth(fTarget->width())
        , fHeight(fTarget->height())
        , fOrigin(origin)
        , fFit(fit)
        , fBudgeted(fTarget->resourcePriv().budgetedType() == GrBudgetedType::kBudgeted
                            ? SkBudgeted::kYes
                            : SkBudgeted::kNo)
        , fUniqueID(fTarget->uniqueID())  // Note: converting from unique resource ID to a proxy ID!
        , fNeedsClear(false)
        , fGpuMemorySize(kInvalidGpuMemorySize)
        , fLastOpList(nullptr) {
    SkASSERT(fFormat.isValid());
}

GrSurfaceProxy::~GrSurfaceProxy() {
    // For this to be deleted the opList that held a ref on it (if there was one) must have been
    // deleted. Which would have cleared out this back pointer.
    SkASSERT(!fLastOpList);
}

bool GrSurfaceProxyPriv::AttachStencilIfNeeded(GrResourceProvider* resourceProvider,
                                               GrSurface* surface, bool needsStencil) {
    if (needsStencil) {
        GrRenderTarget* rt = surface->asRenderTarget();
        if (!rt) {
            SkASSERT(0);
            return false;
        }

        if (!resourceProvider->attachStencilAttachment(rt)) {
            return false;
        }
    }

    return true;
}

sk_sp<GrSurface> GrSurfaceProxy::createSurfaceImpl(GrResourceProvider* resourceProvider,
                                                   int sampleCnt, bool needsStencil,
                                                   GrSurfaceDescFlags descFlags,
                                                   GrMipMapped mipMapped,
                                                   bool forceNoPendingIO) const {
    SkASSERT(GrSurfaceProxy::LazyState::kNot == this->lazyInstantiationState());
    SkASSERT(!fTarget);
    GrSurfaceDesc desc;
    desc.fFlags = descFlags;
    if (fNeedsClear) {
        desc.fFlags |= kPerformInitialClear_GrSurfaceFlag;
    }
    desc.fWidth = fWidth;
    desc.fHeight = fHeight;
    desc.fConfig = fConfig;
    desc.fSampleCnt = sampleCnt;

    GrResourceProvider::Flags resourceProviderFlags = GrResourceProvider::Flags::kNone;
    if ((fSurfaceFlags & GrInternalSurfaceFlags::kNoPendingIO) || forceNoPendingIO) {
        // The explicit resource allocator requires that any resources it pulls out of the
        // cache have no pending IO.
        resourceProviderFlags = GrResourceProvider::Flags::kNoPendingIO;
    }

    sk_sp<GrSurface> surface;
    if (GrMipMapped::kYes == mipMapped) {
        SkASSERT(SkBackingFit::kExact == fFit);

        // SkMipMap doesn't include the base level in the level count so we have to add 1
        int mipCount = SkMipMap::ComputeLevelCount(desc.fWidth, desc.fHeight) + 1;
        // We should have caught the case where mipCount == 1 when making the proxy and instead
        // created a non-mipmapped proxy.
        SkASSERT(mipCount > 1);
        std::unique_ptr<GrMipLevel[]> texels(new GrMipLevel[mipCount]);

        // We don't want to upload any texel data
        for (int i = 0; i < mipCount; i++) {
            texels[i].fPixels = nullptr;
            texels[i].fRowBytes = 0;
        }

        surface = resourceProvider->createTexture(desc, fBudgeted, texels.get(), mipCount);
        if (surface) {
            SkASSERT(surface->asTexture());
            SkASSERT(GrMipMapped::kYes == surface->asTexture()->texturePriv().mipMapped());
        }
    } else {
        if (SkBackingFit::kApprox == fFit) {
            surface = resourceProvider->createApproxTexture(desc, resourceProviderFlags);
        } else {
            surface = resourceProvider->createTexture(desc, fBudgeted, resourceProviderFlags);
        }
    }
    if (!surface) {
        return nullptr;
    }

    if (!GrSurfaceProxyPriv::AttachStencilIfNeeded(resourceProvider, surface.get(), needsStencil)) {
        return nullptr;
    }

    return surface;
}

bool GrSurfaceProxy::canSkipResourceAllocator() const {
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

    fTarget = surface.release();

    this->INHERITED::transferRefs();

#ifdef SK_DEBUG
    if (this->asRenderTargetProxy()) {
        SkASSERT(fTarget->asRenderTarget());
        if (this->asRenderTargetProxy()->needsStencil()) {
           SkASSERT(fTarget->asRenderTarget()->renderTargetPriv().getStencilAttachment());
        }
    }

    if (kInvalidGpuMemorySize != this->getRawGpuMemorySize_debugOnly()) {
        SkASSERT(fTarget->gpuMemorySize() <= this->getRawGpuMemorySize_debugOnly());
    }
#endif
}

bool GrSurfaceProxy::instantiateImpl(GrResourceProvider* resourceProvider, int sampleCnt,
                                     bool needsStencil, GrSurfaceDescFlags descFlags,
                                     GrMipMapped mipMapped, const GrUniqueKey* uniqueKey,
                                     bool dontForceNoPendingIO) {
    SkASSERT(LazyState::kNot == this->lazyInstantiationState());
    if (fTarget) {
        if (uniqueKey && uniqueKey->isValid()) {
            SkASSERT(fTarget->getUniqueKey().isValid() && fTarget->getUniqueKey() == *uniqueKey);
        }
        return GrSurfaceProxyPriv::AttachStencilIfNeeded(resourceProvider, fTarget, needsStencil);
    }

    sk_sp<GrSurface> surface = this->createSurfaceImpl(resourceProvider, sampleCnt, needsStencil,
                                                       descFlags, mipMapped, !dontForceNoPendingIO);
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

    this->release();
}

void GrSurfaceProxy::computeScratchKey(GrScratchKey* key) const {
    SkASSERT(LazyState::kFully != this->lazyInstantiationState());
    const GrRenderTargetProxy* rtp = this->asRenderTargetProxy();
    int sampleCount = 1;
    if (rtp) {
        sampleCount = rtp->numStencilSamples();
    }

    const GrTextureProxy* tp = this->asTextureProxy();
    GrMipMapped mipMapped = GrMipMapped::kNo;
    if (tp) {
        mipMapped = tp->mipMapped();
    }

    int width = this->worstCaseWidth();
    int height = this->worstCaseHeight();

    GrTexturePriv::ComputeScratchKey(this->config(), width, height, SkToBool(rtp), sampleCount,
                                     mipMapped, key);
}

void GrSurfaceProxy::setLastOpList(GrOpList* opList) {
#ifdef SK_DEBUG
    if (fLastOpList) {
        SkASSERT(fLastOpList->isClosed());
    }
#endif

    // Un-reffed
    fLastOpList = opList;
}

GrRenderTargetOpList* GrSurfaceProxy::getLastRenderTargetOpList() {
    return fLastOpList ? fLastOpList->asRenderTargetOpList() : nullptr;
}

GrTextureOpList* GrSurfaceProxy::getLastTextureOpList() {
    return fLastOpList ? fLastOpList->asTextureOpList() : nullptr;
}

int GrSurfaceProxy::worstCaseWidth() const {
    SkASSERT(LazyState::kFully != this->lazyInstantiationState());
    if (fTarget) {
        return fTarget->width();
    }

    if (SkBackingFit::kExact == fFit) {
        return fWidth;
    }
    return SkTMax(GrResourceProvider::kMinScratchTextureSize, GrNextPow2(fWidth));
}

int GrSurfaceProxy::worstCaseHeight() const {
    SkASSERT(LazyState::kFully != this->lazyInstantiationState());
    if (fTarget) {
        return fTarget->height();
    }

    if (SkBackingFit::kExact == fFit) {
        return fHeight;
    }
    return SkTMax(GrResourceProvider::kMinScratchTextureSize, GrNextPow2(fHeight));
}

#ifdef SK_DEBUG
void GrSurfaceProxy::validate(GrContext_Base* context) const {
    if (fTarget) {
        SkASSERT(fTarget->getContext() == context);
    }

    INHERITED::validate();
}
#endif

sk_sp<GrTextureProxy> GrSurfaceProxy::Copy(GrRecordingContext* context,
                                           GrSurfaceProxy* src,
                                           GrMipMapped mipMapped,
                                           SkIRect srcRect,
                                           SkBackingFit fit,
                                           SkBudgeted budgeted) {
    SkASSERT(LazyState::kFully != src->lazyInstantiationState());
    if (!srcRect.intersect(SkIRect::MakeWH(src->width(), src->height()))) {
        return nullptr;
    }

    GrSurfaceDesc dstDesc;
    dstDesc.fWidth = srcRect.width();
    dstDesc.fHeight = srcRect.height();
    dstDesc.fConfig = src->config();

    GrBackendFormat format = src->backendFormat().makeTexture2D();
    if (!format.isValid()) {
        return nullptr;
    }

    sk_sp<GrSurfaceContext> dstContext(context->priv().makeDeferredSurfaceContext(
            format, dstDesc, src->origin(), mipMapped, fit, budgeted));
    if (!dstContext) {
        return nullptr;
    }

    if (!dstContext->copy(src, srcRect, SkIPoint::Make(0, 0))) {
        return nullptr;
    }

    return dstContext->asTextureProxyRef();
}

sk_sp<GrTextureProxy> GrSurfaceProxy::Copy(GrRecordingContext* context, GrSurfaceProxy* src,
                                           GrMipMapped mipMapped, SkBackingFit fit,
                                           SkBudgeted budgeted) {
    SkASSERT(LazyState::kFully != src->lazyInstantiationState());
    return Copy(context, src, mipMapped, SkIRect::MakeWH(src->width(), src->height()), fit,
                budgeted);
}

sk_sp<GrSurfaceContext> GrSurfaceProxy::TestCopy(GrRecordingContext* context,
                                                 const GrSurfaceDesc& dstDesc,
                                                 GrSurfaceOrigin origin, GrSurfaceProxy* srcProxy) {
    SkASSERT(LazyState::kFully != srcProxy->lazyInstantiationState());

    GrBackendFormat format = srcProxy->backendFormat().makeTexture2D();
    if (!format.isValid()) {
        return nullptr;
    }

    sk_sp<GrSurfaceContext> dstContext(context->priv().makeDeferredSurfaceContext(
            format, dstDesc, origin, GrMipMapped::kNo, SkBackingFit::kExact, SkBudgeted::kYes));
    if (!dstContext) {
        return nullptr;
    }

    if (!dstContext->copy(srcProxy)) {
        return nullptr;
    }

    return dstContext;
}

void GrSurfaceProxyPriv::exactify() {
    SkASSERT(GrSurfaceProxy::LazyState::kFully != fProxy->lazyInstantiationState());
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

    // The kApprox uninstantiated case. Making this proxy be exact should be okay.
    // It could mess things up if prior decisions were based on the approximate size.
    fProxy->fFit = SkBackingFit::kExact;
    // If fGpuMemorySize is used when caching specialImages for the image filter DAG. If it has
    // already been computed we want to leave it alone so that amount will be removed when
    // the special image goes away. If it hasn't been computed yet it might as well compute the
    // exact amount.
}

bool GrSurfaceProxyPriv::doLazyInstantiation(GrResourceProvider* resourceProvider) {
    SkASSERT(GrSurfaceProxy::LazyState::kNot != fProxy->lazyInstantiationState());

    sk_sp<GrSurface> surface;
    if (fProxy->asTextureProxy() && fProxy->asTextureProxy()->getUniqueKey().isValid()) {
        // First try to reattach to a cached version if the proxy is uniquely keyed
        surface = resourceProvider->findByUniqueKey<GrSurface>(
                                                        fProxy->asTextureProxy()->getUniqueKey());
    }

    if (!surface) {
        surface = fProxy->fLazyInstantiateCallback(resourceProvider);
    }
    if (GrSurfaceProxy::LazyInstantiationType::kSingleUse == fProxy->fLazyInstantiationType) {
        fProxy->fLazyInstantiateCallback = nullptr;
    }
    if (!surface) {
        fProxy->fWidth = 0;
        fProxy->fHeight = 0;
        return false;
    }

    if (fProxy->fWidth <= 0 || fProxy->fHeight <= 0) {
        // This was a fully lazy proxy. We need to fill in the width & height. For partially
        // lazy proxies we must preserve the original width & height since that indicates
        // the content area.
        SkASSERT(fProxy->fWidth <= 0 && fProxy->fHeight <= 0);
        fProxy->fWidth = surface->width();
        fProxy->fHeight = surface->height();
    }

    bool needsStencil = fProxy->asRenderTargetProxy()
                                        ? fProxy->asRenderTargetProxy()->needsStencil()
                                        : false;

    if (!GrSurfaceProxyPriv::AttachStencilIfNeeded(resourceProvider, surface.get(), needsStencil)) {
        return false;
    }

    if (GrTextureProxy* texProxy = fProxy->asTextureProxy()) {
        const GrUniqueKey& key = texProxy->getUniqueKey();
        if (key.isValid()) {
            if (!surface->asTexture()->getUniqueKey().isValid()) {
                // If 'surface' is newly created, attach the unique key
                resourceProvider->assignUniqueKeyToResource(key, surface.get());
            } else {
                // otherwise we had better have reattached to a cached version
                SkASSERT(surface->asTexture()->getUniqueKey() == key);
            }
        }
    }

    this->assign(std::move(surface));
    return true;
}

#ifdef SK_DEBUG
void GrSurfaceProxy::validateSurface(const GrSurface* surface) {
    SkASSERT(surface->config() == fConfig);

    // Assert the flags are the same except for kNoPendingIO which is not passed onto the GrSurface.
    GrInternalSurfaceFlags proxyFlags = fSurfaceFlags & ~GrInternalSurfaceFlags::kNoPendingIO;
    GrInternalSurfaceFlags surfaceFlags = surface->surfacePriv().flags();
    SkASSERT((proxyFlags & GrInternalSurfaceFlags::kSurfaceMask) ==
             (surfaceFlags & GrInternalSurfaceFlags::kSurfaceMask));
    this->onValidateSurface(surface);
}
#endif
