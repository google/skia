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
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrClip.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrGpuResourcePriv.h"
#include "src/gpu/GrOpList.h"
#include "src/gpu/GrProxyProvider.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/GrSurfaceContext.h"
#include "src/gpu/GrSurfaceContextPriv.h"
#include "src/gpu/GrSurfacePriv.h"
#include "src/gpu/GrTexturePriv.h"
#include "src/gpu/GrTextureRenderTargetProxy.h"

#include "src/core/SkMathPriv.h"
#include "src/core/SkMipMap.h"

#ifdef SK_DEBUG
#include "include/gpu/GrRenderTarget.h"
#include "src/gpu/GrRenderTargetPriv.h"

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
                               GrSurfaceOrigin origin, const GrSwizzle& textureSwizzle,
                               SkBackingFit fit, SkBudgeted budgeted,
                               GrInternalSurfaceFlags surfaceFlags)
        : fSurfaceFlags(surfaceFlags)
        , fFormat(format)
        , fConfig(desc.fConfig)
        , fWidth(desc.fWidth)
        , fHeight(desc.fHeight)
        , fOrigin(origin)
        , fTextureSwizzle(textureSwizzle)
        , fFit(fit)
        , fBudgeted(budgeted)
        , fLazyInstantiateCallback(std::move(callback))
        , fLazyInstantiationType(lazyType)
        , fNeedsClear(SkToBool(desc.fFlags & kPerformInitialClear_GrSurfaceFlag))
        , fIsProtected(desc.fIsProtected)
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
GrSurfaceProxy::GrSurfaceProxy(sk_sp<GrSurface> surface, GrSurfaceOrigin origin,
                               const GrSwizzle& textureSwizzle, SkBackingFit fit)
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
        , fUniqueID(fTarget->uniqueID())  // Note: converting from unique resource ID to a proxy ID!
        , fNeedsClear(false)
        , fIsProtected(fTarget->isProtected() ? GrProtected::kYes : GrProtected::kNo)
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
                                                   GrMipMapped mipMapped) const {
    SkASSERT(GrSurfaceProxy::LazyState::kNot == this->lazyInstantiationState());
    SkASSERT(!fTarget);
    GrSurfaceDesc desc;
    desc.fFlags = descFlags;
    if (fNeedsClear) {
        desc.fFlags |= kPerformInitialClear_GrSurfaceFlag;
    }
    desc.fWidth = fWidth;
    desc.fHeight = fHeight;
    desc.fIsProtected = fIsProtected;
    desc.fConfig = fConfig;
    desc.fSampleCnt = sampleCnt;

    // The explicit resource allocator requires that any resources it pulls out of the
    // cache have no pending IO.
    GrResourceProvider::Flags resourceProviderFlags = GrResourceProvider::Flags::kNoPendingIO;

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

    if (!GrSurfaceProxyPriv::AttachStencilIfNeeded(resourceProvider, surface.get(),
                                                   needsStencil)) {
        return nullptr;
    }

    return surface;
}

bool GrSurfaceProxy::canSkipResourceAllocator() const {
    if (this->ignoredByResourceAllocator()) {
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
                                     GrMipMapped mipMapped, const GrUniqueKey* uniqueKey) {
    SkASSERT(LazyState::kNot == this->lazyInstantiationState());
    if (fTarget) {
        if (uniqueKey && uniqueKey->isValid()) {
            SkASSERT(fTarget->getUniqueKey().isValid() && fTarget->getUniqueKey() == *uniqueKey);
        }
        return GrSurfaceProxyPriv::AttachStencilIfNeeded(resourceProvider, fTarget.get(),
                                                         needsStencil);
    }

    sk_sp<GrSurface> surface = this->createSurfaceImpl(resourceProvider, sampleCnt, needsStencil,
                                                       descFlags, mipMapped);
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
    SkASSERT(LazyState::kFully != this->lazyInstantiationState());
    const GrRenderTargetProxy* rtp = this->asRenderTargetProxy();
    int sampleCount = 1;
    if (rtp) {
        sampleCount = rtp->numSamples();
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
}
#endif

sk_sp<GrTextureProxy> GrSurfaceProxy::Copy(GrRecordingContext* context,
                                           GrSurfaceProxy* src,
                                           GrMipMapped mipMapped,
                                           SkIRect srcRect,
                                           SkBackingFit fit,
                                           SkBudgeted budgeted,
                                           RectsMustMatch rectsMustMatch) {
    SkASSERT(LazyState::kFully != src->lazyInstantiationState());
    GrSurfaceDesc dstDesc;
    dstDesc.fIsProtected = src->isProtected() ? GrProtected::kYes : GrProtected::kNo;
    dstDesc.fConfig = src->config();

    SkIPoint dstPoint;
    if (rectsMustMatch == RectsMustMatch::kYes) {
        dstDesc.fWidth = src->width();
        dstDesc.fHeight = src->height();
        dstPoint = {srcRect.fLeft, srcRect.fTop};
    } else {
        dstDesc.fWidth = srcRect.width();
        dstDesc.fHeight = srcRect.height();
        dstPoint = {0, 0};
    }

    if (!srcRect.intersect(SkIRect::MakeWH(src->width(), src->height()))) {
        return nullptr;
    }

    if (src->backendFormat().textureType() != GrTextureType::kExternal) {
        sk_sp<GrSurfaceContext> dstContext(context->priv().makeDeferredSurfaceContext(
                src->backendFormat().makeTexture2D(), dstDesc, src->origin(), mipMapped, fit,
                budgeted, kUnknown_SkAlphaType));
        if (!dstContext) {
            return nullptr;
        }
        if (dstContext->copy(src, srcRect, dstPoint)) {
            return dstContext->asTextureProxyRef();
        }
    }
    if (src->asTextureProxy()) {
        GrBackendFormat format = src->backendFormat().makeTexture2D();
        if (!format.isValid()) {
            return nullptr;
        }

        sk_sp<GrRenderTargetContext> dstContext = context->priv().makeDeferredRenderTargetContext(
                format, fit, dstDesc.fWidth, dstDesc.fHeight, dstDesc.fConfig, nullptr, 1,
                mipMapped, src->origin(), nullptr, budgeted);

        if (dstContext && dstContext->blitTexture(src->asTextureProxy(), srcRect, dstPoint)) {
            return dstContext->asTextureProxyRef();
        }
    }
    // Can't use backend copies or draws.
    return nullptr;
}

sk_sp<GrTextureProxy> GrSurfaceProxy::Copy(GrRecordingContext* context, GrSurfaceProxy* src,
                                           GrMipMapped mipMapped, SkBackingFit fit,
                                           SkBudgeted budgeted) {
    SkASSERT(LazyState::kFully != src->lazyInstantiationState());
    return Copy(context, src, mipMapped, SkIRect::MakeWH(src->width(), src->height()), fit,
                budgeted);
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

    bool syncKey = true;
    if (!surface) {
        auto result = fProxy->fLazyInstantiateCallback(resourceProvider);
        surface = std::move(result.fSurface);
        syncKey = result.fKeyMode == GrSurfaceProxy::LazyInstantiationKeyMode::kSynced;
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

    SkASSERT(fProxy->fWidth <= surface->width());
    SkASSERT(fProxy->fHeight <= surface->height());

    bool needsStencil = fProxy->asRenderTargetProxy()
                                        ? fProxy->asRenderTargetProxy()->needsStencil()
                                        : false;

    if (!GrSurfaceProxyPriv::AttachStencilIfNeeded(resourceProvider, surface.get(), needsStencil)) {
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
    return true;
}

#ifdef SK_DEBUG
void GrSurfaceProxy::validateSurface(const GrSurface* surface) {
    SkASSERT(surface->config() == fConfig);

    this->onValidateSurface(surface);
}
#endif
