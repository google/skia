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
#include "GrResourceProvider.h"
#include "GrSurfaceContext.h"
#include "GrTexturePriv.h"
#include "GrTextureRenderTargetProxy.h"

#include "SkMathPriv.h"

GrSurfaceProxy::GrSurfaceProxy(sk_sp<GrSurface> surface, SkBackingFit fit)
    : INHERITED(std::move(surface))
    , fDesc(fTarget->desc())
    , fFit(fit)
    , fBudgeted(fTarget->resourcePriv().isBudgeted())
    , fFlags(0)
    , fUniqueID(fTarget->uniqueID()) // Note: converting from unique resource ID to a proxy ID!
    , fGpuMemorySize(kInvalidGpuMemorySize)
    , fLastOpList(nullptr) {
}

GrSurfaceProxy::~GrSurfaceProxy() {
    if (fLastOpList) {
        fLastOpList->clearTarget();
    }
    SkSafeUnref(fLastOpList);
}

GrSurface* GrSurfaceProxy::instantiate(GrResourceProvider* resourceProvider) {
    if (fTarget) {
        return fTarget;
    }

    if (SkBackingFit::kApprox == fFit) {
        fTarget = resourceProvider->createApproxTexture(fDesc, fFlags);
    } else {
        fTarget = resourceProvider->createTexture(fDesc, fBudgeted, fFlags).release();
    }
    if (!fTarget) {
        return nullptr;
    }

    fTarget->asTexture()->texturePriv().setMipColorMode(fMipColorMode);
    this->INHERITED::transferRefs();

#ifdef SK_DEBUG
    if (kInvalidGpuMemorySize != this->getRawGpuMemorySize_debugOnly()) {
        SkASSERT(fTarget->gpuMemorySize() <= this->getRawGpuMemorySize_debugOnly());
    }
#endif

    return fTarget;
}

int GrSurfaceProxy::worstCaseWidth(const GrCaps& caps) const {
    if (fTarget) {
        return fTarget->width();
    }

    if (SkBackingFit::kExact == fFit) {
        return fDesc.fWidth;
    }

    if (caps.reuseScratchTextures() || fDesc.fFlags & kRenderTarget_GrSurfaceFlag) {
        return SkTMax(GrResourceProvider::kMinScratchTextureSize, GrNextPow2(fDesc.fWidth));
    }

    return fDesc.fWidth;
}

int GrSurfaceProxy::worstCaseHeight(const GrCaps& caps) const {
    if (fTarget) {
        return fTarget->height();
    }

    if (SkBackingFit::kExact == fFit) {
        return fDesc.fHeight;
    }

    if (caps.reuseScratchTextures() || fDesc.fFlags & kRenderTarget_GrSurfaceFlag) {
        return SkTMax(GrResourceProvider::kMinScratchTextureSize, GrNextPow2(fDesc.fHeight));
    }

    return fDesc.fHeight;
}

void GrSurfaceProxy::setLastOpList(GrOpList* opList) {
    if (fLastOpList) {
        // The non-MDB world never closes so we can't check this condition
#ifdef ENABLE_MDB
        SkASSERT(fLastOpList->isClosed());
#endif
        fLastOpList->clearTarget();
    }

    SkRefCnt_SafeAssign(fLastOpList, opList);
}

GrRenderTargetOpList* GrSurfaceProxy::getLastRenderTargetOpList() {
    return fLastOpList ? fLastOpList->asRenderTargetOpList() : nullptr;
}

GrTextureOpList* GrSurfaceProxy::getLastTextureOpList() {
    return fLastOpList ? fLastOpList->asTextureOpList() : nullptr;
}

sk_sp<GrSurfaceProxy> GrSurfaceProxy::MakeWrapped(sk_sp<GrSurface> surf) {
    if (!surf) {
        return nullptr;
    }

    if (surf->asTexture()) {
        if (surf->asRenderTarget()) {
            return sk_sp<GrSurfaceProxy>(new GrTextureRenderTargetProxy(std::move(surf)));
        } else {
            return sk_sp<GrSurfaceProxy>(new GrTextureProxy(std::move(surf)));
        }
    } else {
        SkASSERT(surf->asRenderTarget());

        // Not texturable
        return sk_sp<GrSurfaceProxy>(new GrRenderTargetProxy(std::move(surf)));
    }
}

sk_sp<GrTextureProxy> GrSurfaceProxy::MakeWrapped(sk_sp<GrTexture> tex) {
    if (!tex) {
        return nullptr;
    }

    if (tex->asRenderTarget()) {
        return sk_sp<GrTextureProxy>(new GrTextureRenderTargetProxy(std::move(tex)));
    } else {
        return sk_sp<GrTextureProxy>(new GrTextureProxy(std::move(tex)));
    }
}

#include "GrResourceProvider.h"

sk_sp<GrTextureProxy> GrSurfaceProxy::MakeDeferred(GrResourceProvider* resourceProvider,
                                                   const GrSurfaceDesc& desc,
                                                   SkBackingFit fit,
                                                   SkBudgeted budgeted,
                                                   uint32_t flags) {
    SkASSERT(0 == flags || GrResourceProvider::kNoPendingIO_Flag == flags);

    const GrCaps* caps = resourceProvider->caps();

    // TODO: move this logic into GrResourceProvider!
    // TODO: share this testing code with check_texture_creation_params
    if (GrPixelConfigIsCompressed(desc.fConfig)) {
        if (SkBackingFit::kApprox == fit || kBottomLeft_GrSurfaceOrigin == desc.fOrigin) {
            // We don't allow scratch compressed textures and, apparently can't Y-flip compressed
            // textures
            return nullptr;
        }

        if (!caps->npotTextureTileSupport() && (!SkIsPow2(desc.fWidth) || !SkIsPow2(desc.fHeight))) {
            return nullptr;
        }
    }

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

    if (desc.fWidth > maxSize || desc.fHeight > maxSize) {
        return nullptr;
    }

    GrSurfaceDesc copyDesc = desc;
    copyDesc.fSampleCnt = SkTMin(desc.fSampleCnt, caps->maxSampleCount());

#ifdef SK_DISABLE_DEFERRED_PROXIES
    sk_sp<GrTexture> tex;

    if (SkBackingFit::kApprox == fit) {
        tex.reset(resourceProvider->createApproxTexture(copyDesc, flags));
    } else {
        tex = resourceProvider->createTexture(copyDesc, budgeted, flags);
    }

    if (!tex) {
        return nullptr;
    }

    return GrSurfaceProxy::MakeWrapped(std::move(tex));
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

sk_sp<GrTextureProxy> GrSurfaceProxy::MakeDeferred(GrResourceProvider* resourceProvider,
                                                   const GrSurfaceDesc& desc,
                                                   SkBudgeted budgeted,
                                                   const void* srcData,
                                                   size_t rowBytes) {
    if (srcData) {
        GrMipLevel tempTexels;
        GrMipLevel* texels = nullptr;
        int levelCount = 0;
        if (srcData) {
            tempTexels.fPixels = srcData;
            tempTexels.fRowBytes = rowBytes;
            texels = &tempTexels;
            levelCount = 1;
        }
        return resourceProvider->createMipMappedTexture(desc, budgeted, texels, levelCount);
    }

    return GrSurfaceProxy::MakeDeferred(resourceProvider, desc, SkBackingFit::kExact, budgeted);
}

sk_sp<GrSurfaceProxy> GrSurfaceProxy::MakeWrappedBackend(GrContext* context,
                                                         GrBackendTextureDesc& desc) {
    sk_sp<GrTexture> tex(context->resourceProvider()->wrapBackendTexture(desc));
    return GrSurfaceProxy::MakeWrapped(std::move(tex));
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
                                           SkIRect srcRect,
                                           SkBudgeted budgeted) {
    if (!srcRect.intersect(SkIRect::MakeWH(src->width(), src->height()))) {
        return nullptr;
    }

    GrSurfaceDesc dstDesc = src->desc();
    dstDesc.fWidth = srcRect.width();
    dstDesc.fHeight = srcRect.height();

    sk_sp<GrSurfaceContext> dstContext(context->contextPriv().makeDeferredSurfaceContext(
                                                                            dstDesc,
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
                                           SkBudgeted budgeted) {
    return Copy(context, src, SkIRect::MakeWH(src->width(), src->height()), budgeted);
}

sk_sp<GrSurfaceContext> GrSurfaceProxy::TestCopy(GrContext* context, const GrSurfaceDesc& dstDesc,
                                                 GrSurfaceProxy* srcProxy) {

    sk_sp<GrSurfaceContext> dstContext(context->contextPriv().makeDeferredSurfaceContext(
                                                                            dstDesc,
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
        fProxy->fDesc.fWidth = fProxy->fTarget->width();
        fProxy->fDesc.fHeight = fProxy->fTarget->height();
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

