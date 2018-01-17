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
#include "GrSurfaceContext.h"
#include "GrTexturePriv.h"
#include "GrTextureRenderTargetProxy.h"

#include "SkMathPriv.h"
#include "SkMipMap.h"

#ifdef SK_DEBUG
static bool is_valid_fully_lazy(const GrSurfaceDesc& desc, SkBackingFit fit) {
    return desc.fWidth <= 0 &&
           desc.fHeight <= 0 &&
           desc.fConfig != kUnknown_GrPixelConfig &&
           desc.fSampleCnt == 0 &&
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
GrSurfaceProxy::GrSurfaceProxy(LazyInstantiateCallback&& callback, const GrSurfaceDesc& desc,
                               SkBackingFit fit, SkBudgeted budgeted, uint32_t flags)
        : fConfig(desc.fConfig)
        , fWidth(desc.fWidth)
        , fHeight(desc.fHeight)
        , fOrigin(desc.fOrigin)
        , fFit(fit)
        , fBudgeted(budgeted)
        , fFlags(flags)
        , fLazyInstantiateCallback(std::move(callback))
        , fNeedsClear(SkToBool(desc.fFlags & kPerformInitialClear_GrSurfaceFlag))
        , fGpuMemorySize(kInvalidGpuMemorySize)
        , fLastOpList(nullptr) {
    // NOTE: the default fUniqueID ctor pulls a value from the same pool as the GrGpuResources.
    if (fLazyInstantiateCallback) {
        SkASSERT(is_valid_fully_lazy(desc, fit) || is_valid_partially_lazy(desc));
    } else {
        SkASSERT(is_valid_non_lazy(desc));
    }

}

// Wrapped version
GrSurfaceProxy::GrSurfaceProxy(sk_sp<GrSurface> surface, GrSurfaceOrigin origin, SkBackingFit fit)
        : INHERITED(std::move(surface))
        , fConfig(fTarget->config())
        , fWidth(fTarget->width())
        , fHeight(fTarget->height())
        , fOrigin(origin)
        , fFit(fit)
        , fBudgeted(fTarget->resourcePriv().isBudgeted())
        , fFlags(0)
        , fUniqueID(fTarget->uniqueID())  // Note: converting from unique resource ID to a proxy ID!
        , fNeedsClear(false)
        , fGpuMemorySize(kInvalidGpuMemorySize)
        , fLastOpList(nullptr) {
}

GrSurfaceProxy::~GrSurfaceProxy() {
    if (fLazyInstantiateCallback) {
        // We have an uninstantiated lazy proxy. Call fLazyInstantiateCallback with a nullptr for
        // the GrResourceProvider to signal the callback should clean itself up.
        this->fLazyInstantiateCallback(nullptr, nullptr);
    }
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

sk_sp<GrSurface> GrSurfaceProxy::createSurfaceImpl(
                                                GrResourceProvider* resourceProvider,
                                                int sampleCnt, bool needsStencil,
                                                GrSurfaceFlags flags, GrMipMapped mipMapped,
                                                SkDestinationSurfaceColorMode mipColorMode) const {
    SkASSERT(GrSurfaceProxy::LazyState::kNot == this->lazyInstantiationState());
    SkASSERT(GrMipMapped::kNo == mipMapped);
    GrSurfaceDesc desc;
    desc.fFlags = flags;
    if (fNeedsClear) {
        desc.fFlags |= kPerformInitialClear_GrSurfaceFlag;
    }
    desc.fOrigin = fOrigin;
    desc.fWidth = fWidth;
    desc.fHeight = fHeight;
    desc.fConfig = fConfig;
    desc.fSampleCnt = sampleCnt;

    sk_sp<GrSurface> surface;
    if (SkBackingFit::kApprox == fFit) {
        surface.reset(resourceProvider->createApproxTexture(desc, fFlags).release());
    } else {
        surface.reset(resourceProvider->createTexture(desc, fBudgeted, fFlags).release());
    }
    if (!surface) {
        return nullptr;
    }

    surface->asTexture()->texturePriv().setMipColorMode(mipColorMode);

    if (!GrSurfaceProxyPriv::AttachStencilIfNeeded(resourceProvider, surface.get(), needsStencil)) {
        return nullptr;
    }

    return surface;
}

void GrSurfaceProxy::assign(sk_sp<GrSurface> surface) {
    SkASSERT(LazyState::kNot == this->lazyInstantiationState());
    SkASSERT(!fTarget && surface);
    fTarget = surface.release();
    this->INHERITED::transferRefs();

#ifdef SK_DEBUG
    if (kInvalidGpuMemorySize != this->getRawGpuMemorySize_debugOnly()) {
        SkASSERT(fTarget->gpuMemorySize() <= this->getRawGpuMemorySize_debugOnly());
    }
#endif
}

bool GrSurfaceProxy::instantiateImpl(GrResourceProvider* resourceProvider, int sampleCnt,
                                     bool needsStencil, GrSurfaceFlags flags, GrMipMapped mipMapped,
                                     SkDestinationSurfaceColorMode mipColorMode,
                                     const GrUniqueKey* uniqueKey) {
    SkASSERT(LazyState::kNot == this->lazyInstantiationState());
    if (fTarget) {
        if (uniqueKey) {
            SkASSERT(fTarget->getUniqueKey() == *uniqueKey);
        }
        return GrSurfaceProxyPriv::AttachStencilIfNeeded(resourceProvider, fTarget, needsStencil);
    }

    sk_sp<GrSurface> surface = this->createSurfaceImpl(resourceProvider, sampleCnt, needsStencil,
                                                       flags, mipMapped, mipColorMode);
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

void GrSurfaceProxy::computeScratchKey(GrScratchKey* key) const {
    SkASSERT(LazyState::kFully != this->lazyInstantiationState());
    const GrRenderTargetProxy* rtp = this->asRenderTargetProxy();
    int sampleCount = 0;
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

sk_sp<GrTextureProxy> GrSurfaceProxy::MakeWrapped(sk_sp<GrTexture> tex, GrSurfaceOrigin origin) {
    if (!tex) {
        return nullptr;
    }

    if (tex->getUniqueKey().isValid()) {
        // The proxy may already be in the hash. Thus we need to look for it first before creating
        // new one.
        GrProxyProvider* provider = tex->getContext()->contextPriv().proxyProvider();
        sk_sp<GrTextureProxy> proxy = provider->findProxyByUniqueKey(tex->getUniqueKey(), origin);
        if (proxy) {
            return proxy;
        }
    }

    if (tex->asRenderTarget()) {
        return sk_sp<GrTextureProxy>(new GrTextureRenderTargetProxy(std::move(tex), origin));
    } else {
        return sk_sp<GrTextureProxy>(new GrTextureProxy(std::move(tex), origin));
    }
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
void GrSurfaceProxy::validate(GrContext* context) const {
    if (fTarget) {
        SkASSERT(fTarget->getContext() == context);
    }

    INHERITED::validate();
}
#endif

sk_sp<GrTextureProxy> GrSurfaceProxy::Copy(GrContext* context,
                                           GrSurfaceProxy* src,
                                           GrMipMapped mipMapped,
                                           SkIRect srcRect,
                                           SkBudgeted budgeted) {
    SkASSERT(LazyState::kFully != src->lazyInstantiationState());
    if (!srcRect.intersect(SkIRect::MakeWH(src->width(), src->height()))) {
        return nullptr;
    }

    GrSurfaceDesc dstDesc;
    dstDesc.fOrigin = src->origin();
    dstDesc.fWidth = srcRect.width();
    dstDesc.fHeight = srcRect.height();
    dstDesc.fConfig = src->config();

    sk_sp<GrSurfaceContext> dstContext(context->contextPriv().makeDeferredSurfaceContext(
                                                                            dstDesc,
                                                                            mipMapped,
                                                                            SkBackingFit::kExact,
                                                                            budgeted));
    if (!dstContext) {
        return nullptr;
    }

    if (!dstContext->copy(src, srcRect, SkIPoint::Make(0, 0))) {
        return nullptr;
    }

    return dstContext->asTextureProxyRef();
}

sk_sp<GrTextureProxy> GrSurfaceProxy::Copy(GrContext* context, GrSurfaceProxy* src,
                                           GrMipMapped mipMapped, SkBudgeted budgeted) {
    SkASSERT(LazyState::kFully != src->lazyInstantiationState());
    return Copy(context, src, mipMapped, SkIRect::MakeWH(src->width(), src->height()), budgeted);
}

sk_sp<GrSurfaceContext> GrSurfaceProxy::TestCopy(GrContext* context, const GrSurfaceDesc& dstDesc,
                                                 GrSurfaceProxy* srcProxy) {
    SkASSERT(LazyState::kFully != srcProxy->lazyInstantiationState());
    sk_sp<GrSurfaceContext> dstContext(context->contextPriv().makeDeferredSurfaceContext(
                                                                            dstDesc,
                                                                            GrMipMapped::kNo,
                                                                            SkBackingFit::kExact,
                                                                            SkBudgeted::kYes));
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

void GrSurfaceProxyPriv::doLazyInstantiation(GrResourceProvider* resourceProvider) {
    SkASSERT(fProxy->fLazyInstantiateCallback);
    SkASSERT(!fProxy->fTarget);

    sk_sp<GrTexture> texture = fProxy->fLazyInstantiateCallback(resourceProvider, &fProxy->fOrigin);

    // Indicate we are no longer pending lazy instantiation.
    fProxy->fLazyInstantiateCallback = nullptr;

    if (!texture) {
        fProxy->fWidth = 0;
        fProxy->fHeight = 0;
        fProxy->fOrigin = kTopLeft_GrSurfaceOrigin;
        return;
    }

    fProxy->fWidth = texture->width();
    fProxy->fHeight = texture->height();

    SkASSERT(texture->config() == fProxy->fConfig);
    SkDEBUGCODE(fProxy->validateLazyTexture(texture.get());)
    this->assign(std::move(texture));
}

