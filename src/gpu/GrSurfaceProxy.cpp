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

// Lazy-callback version
GrSurfaceProxy::GrSurfaceProxy(LazyInstantiateCallback&& callback, GrPixelConfig config)
        : fConfig(config)
        , fWidth(-1) // Width, height, and origin will be initialized upon lazy instantiation.
        , fHeight(-1)
        , fOrigin(kTopLeft_GrSurfaceOrigin)
        , fFit(SkBackingFit::kApprox)
        , fBudgeted(SkBudgeted::kYes)
        , fFlags(GrResourceProvider::kNoPendingIO_Flag)
        , fLazyInstantiateCallback(std::move(callback))
        , fNeedsClear(false)
        , fGpuMemorySize(kInvalidGpuMemorySize)
        , fLastOpList(nullptr) {
    // NOTE: the default fUniqueID ctor pulls a value from the same pool as the GrGpuResources.
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
    SkASSERT(!this->isPendingLazyInstantiation());
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
    SkASSERT(!this->isPendingLazyInstantiation());
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
    SkASSERT(!this->isPendingLazyInstantiation());
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
    SkASSERT(!this->isPendingLazyInstantiation());
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

sk_sp<GrSurfaceProxy> GrSurfaceProxy::MakeWrapped(sk_sp<GrSurface> surf, GrSurfaceOrigin origin) {
    if (!surf) {
        return nullptr;
    }

    if (surf->getUniqueKey().isValid()) {
        // The proxy may already be in the hash. Thus we need to look for it first before creating
        // new one.
        GrProxyProvider* provider = surf->getContext()->contextPriv().proxyProvider();
        sk_sp<GrSurfaceProxy> proxy = provider->findProxyByUniqueKey(surf->getUniqueKey(), origin);
        if (proxy) {
            return proxy;
        }
    }

    if (surf->asTexture()) {
        if (surf->asRenderTarget()) {
            return sk_sp<GrSurfaceProxy>(new GrTextureRenderTargetProxy(std::move(surf), origin));
        } else {
            return sk_sp<GrSurfaceProxy>(new GrTextureProxy(std::move(surf), origin));
        }
    } else {
        SkASSERT(surf->asRenderTarget());

        // Not texturable
        return sk_sp<GrSurfaceProxy>(new GrRenderTargetProxy(std::move(surf), origin));
    }
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

sk_sp<GrTextureProxy> GrSurfaceProxy::MakeDeferred(GrProxyProvider* proxyProvider,
                                                   const GrSurfaceDesc& desc,
                                                   SkBackingFit fit,
                                                   SkBudgeted budgeted,
                                                   uint32_t flags) {
    SkASSERT(0 == flags || GrResourceProvider::kNoPendingIO_Flag == flags);

    const GrCaps* caps = proxyProvider->caps();

    // TODO: move this logic into GrResourceProvider!
    // TODO: share this testing code with check_texture_creation_params
    if (!caps->isConfigTexturable(desc.fConfig)) {
        return nullptr;
    }

    bool willBeRT = SkToBool(desc.fFlags & kRenderTarget_GrSurfaceFlag);
    if (willBeRT && !caps->isConfigRenderable(desc.fConfig, desc.fSampleCnt > 0)) {
        return nullptr;
    }

    // We currently do not support multisampled textures
    if (!willBeRT && desc.fSampleCnt > 0) {
        return nullptr;
    }

    int maxSize;
    if (willBeRT) {
        maxSize = caps->maxRenderTargetSize();
    } else {
        maxSize = caps->maxTextureSize();
    }

    if (desc.fWidth > maxSize || desc.fHeight > maxSize || desc.fWidth <= 0 || desc.fHeight <= 0) {
        return nullptr;
    }

    GrSurfaceDesc copyDesc = desc;
    copyDesc.fSampleCnt = caps->getSampleCount(desc.fSampleCnt, desc.fConfig);

#ifdef SK_DISABLE_DEFERRED_PROXIES
    // Temporarily force instantiation for crbug.com/769760 and crbug.com/769898
    sk_sp<GrTexture> tex;

    if (SkBackingFit::kApprox == fit) {
        tex = resourceProvider->createApproxTexture(copyDesc, flags);
    } else {
        tex = resourceProvider->createTexture(copyDesc, budgeted, flags);
    }

    if (!tex) {
        return nullptr;
    }

    return GrSurfaceProxy::MakeWrapped(std::move(tex), copyDesc.fOrigin);
#else
    if (willBeRT) {
        // We know anything we instantiate later from this deferred path will be
        // both texturable and renderable
        return sk_sp<GrTextureProxy>(new GrTextureRenderTargetProxy(*caps, copyDesc, fit,
                                                                    budgeted, flags));
    }

    return sk_sp<GrTextureProxy>(new GrTextureProxy(copyDesc, fit, budgeted, nullptr, 0, flags));
#endif
}

sk_sp<GrTextureProxy> GrSurfaceProxy::MakeDeferred(GrProxyProvider* proxyProvider,
                                                   const GrSurfaceDesc& desc,
                                                   SkBudgeted budgeted,
                                                   const void* srcData,
                                                   size_t rowBytes) {
    if (srcData) {
        GrMipLevel mipLevel = { srcData, rowBytes };

        return proxyProvider->createTextureProxy(desc, budgeted, mipLevel);
    }

    return GrSurfaceProxy::MakeDeferred(proxyProvider, desc, SkBackingFit::kExact, budgeted);
}

sk_sp<GrTextureProxy> GrSurfaceProxy::MakeDeferredMipMap(GrProxyProvider* proxyProvider,
                                                         const GrSurfaceDesc& desc,
                                                         SkBudgeted budgeted) {
    // SkMipMap doesn't include the base level in the level count so we have to add 1
    int mipCount = SkMipMap::ComputeLevelCount(desc.fWidth, desc.fHeight) + 1;

    std::unique_ptr<GrMipLevel[]> texels(new GrMipLevel[mipCount]);

    // We don't want to upload any texel data
    for (int i = 0; i < mipCount; i++) {
        texels[i].fPixels = nullptr;
        texels[i].fRowBytes = 0;
    }

    return MakeDeferredMipMap(proxyProvider, desc, budgeted, texels.get(), mipCount);
}

sk_sp<GrTextureProxy> GrSurfaceProxy::MakeDeferredMipMap(
                                                    GrProxyProvider* proxyProvider,
                                                    const GrSurfaceDesc& desc,
                                                    SkBudgeted budgeted,
                                                    const GrMipLevel texels[],
                                                    int mipLevelCount,
                                                    SkDestinationSurfaceColorMode mipColorMode) {
    if (!mipLevelCount) {
        if (texels) {
            return nullptr;
        }
        return GrSurfaceProxy::MakeDeferred(proxyProvider, desc, budgeted, nullptr, 0);
    }
    if (!texels) {
        return nullptr;
    }

    if (1 == mipLevelCount) {
        return proxyProvider->createTextureProxy(desc, budgeted, texels[0]);
    }

#ifdef SK_DEBUG
    // There are only three states we want to be in when uploading data to a mipped surface.
    // 1) We have data to upload to all layers
    // 2) We are not uploading data to any layers
    // 3) We are only uploading data to the base layer
    // We check here to make sure we do not have any other state.
    bool firstLevelHasData = SkToBool(texels[0].fPixels);
    bool allOtherLevelsHaveData = true, allOtherLevelsLackData = true;
    for  (int i = 1; i < mipLevelCount; ++i) {
        if (texels[i].fPixels) {
            allOtherLevelsLackData = false;
        } else {
            allOtherLevelsHaveData = false;
        }
    }
    SkASSERT((firstLevelHasData && allOtherLevelsHaveData) || allOtherLevelsLackData);
#endif

    return proxyProvider->createTextureProxy(desc, budgeted, texels, mipLevelCount, mipColorMode);
}

sk_sp<GrTextureProxy> GrSurfaceProxy::MakeWrappedBackend(GrContext* context,
                                                         const GrBackendTexture& backendTex,
                                                         GrSurfaceOrigin origin) {
    sk_sp<GrTexture> tex(context->resourceProvider()->wrapBackendTexture(backendTex));
    return GrSurfaceProxy::MakeWrapped(std::move(tex), origin);
}

sk_sp<GrTextureProxy> GrSurfaceProxy::MakeLazy(LazyInstantiateCallback&& callback,
                                               Renderable renderable, GrPixelConfig config) {
    return sk_sp<GrTextureProxy>(Renderable::kYes == renderable ?
                                 new GrTextureRenderTargetProxy(std::move(callback), config) :
                                 new GrTextureProxy(std::move(callback), config));
}

int GrSurfaceProxy::worstCaseWidth() const {
    SkASSERT(!this->isPendingLazyInstantiation());
    if (fTarget) {
        return fTarget->width();
    }

    if (SkBackingFit::kExact == fFit) {
        return fWidth;
    }
    return SkTMax(GrResourceProvider::kMinScratchTextureSize, GrNextPow2(fWidth));
}

int GrSurfaceProxy::worstCaseHeight() const {
    SkASSERT(!this->isPendingLazyInstantiation());
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
    SkASSERT(!src->isPendingLazyInstantiation());
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
    SkASSERT(!src->isPendingLazyInstantiation());
    return Copy(context, src, mipMapped, SkIRect::MakeWH(src->width(), src->height()), budgeted);
}

sk_sp<GrSurfaceContext> GrSurfaceProxy::TestCopy(GrContext* context, const GrSurfaceDesc& dstDesc,
                                                 GrSurfaceProxy* srcProxy) {
    SkASSERT(!srcProxy->isPendingLazyInstantiation());
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
    SkASSERT(!fProxy->isPendingLazyInstantiation());
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

