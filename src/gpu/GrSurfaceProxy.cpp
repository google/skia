/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrSurfaceProxy.h"

#include "GrCaps.h"
#include "GrContext.h"
#include "GrContextPriv.h"
#include "GrGpuResourcePriv.h"
#include "GrOpList.h"
#include "GrSurfaceContext.h"
#include "GrTextureProvider.h"
#include "GrTextureRenderTargetProxy.h"

#include "SkMathPriv.h"

GrSurfaceProxy::GrSurfaceProxy(sk_sp<GrSurface> surface, SkBackingFit fit)
    : INHERITED(std::move(surface))
    , fDesc(fTarget->desc())
    , fFit(fit)
    , fBudgeted(fTarget->resourcePriv().isBudgeted())
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

GrSurface* GrSurfaceProxy::instantiate(GrTextureProvider* texProvider) {
    if (fTarget) {
        return fTarget;
    }

    if (SkBackingFit::kApprox == fFit) {
        fTarget = texProvider->createApproxTexture(fDesc);
    } else {
        fTarget = texProvider->createTexture(fDesc, fBudgeted);
    }
    if (!fTarget) {
        return nullptr;
    }

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
        return SkTMax(GrTextureProvider::kMinScratchTextureSize, GrNextPow2(fDesc.fWidth));
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
        return SkTMax(GrTextureProvider::kMinScratchTextureSize, GrNextPow2(fDesc.fHeight));
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

sk_sp<GrSurfaceProxy> GrSurfaceProxy::MakeDeferred(const GrCaps& caps,
                                                   const GrSurfaceDesc& desc,
                                                   SkBackingFit fit,
                                                   SkBudgeted budgeted) {
    if (desc.fWidth > caps.maxTextureSize() || desc.fHeight > caps.maxTextureSize()) {
        return nullptr;
    }

    if (kRenderTarget_GrSurfaceFlag & desc.fFlags) {
        // We know anything we instantiate later from this deferred path will be
        // both texturable and renderable
        return sk_sp<GrSurfaceProxy>(new GrTextureRenderTargetProxy(caps, desc, fit, budgeted));
    }

    return sk_sp<GrSurfaceProxy>(new GrTextureProxy(desc, fit, budgeted, nullptr, 0));
}

sk_sp<GrSurfaceProxy> GrSurfaceProxy::MakeDeferred(const GrCaps& caps,
                                                   GrTextureProvider* texProvider,
                                                   const GrSurfaceDesc& desc,
                                                   SkBudgeted budgeted,
                                                   const void* srcData,
                                                   size_t rowBytes) {
    if (srcData) {
        // If we have srcData, for now, we create a wrapped GrTextureProxy
        sk_sp<GrSurface> surf(texProvider->createTexture(desc, budgeted, srcData, rowBytes));
        return GrSurfaceProxy::MakeWrapped(std::move(surf));
    }

    return GrSurfaceProxy::MakeDeferred(caps, desc, SkBackingFit::kExact, budgeted);
}

sk_sp<GrSurfaceProxy> GrSurfaceProxy::MakeWrappedBackend(GrContext* context,
                                                         GrBackendTextureDesc& desc,
                                                         GrWrapOwnership ownership) {
    sk_sp<GrTexture> tex(context->textureProvider()->wrapBackendTexture(desc, ownership));
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

sk_sp<GrSurfaceProxy> GrSurfaceProxy::Copy(GrContext* context,
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

    return dstContext->asSurfaceProxyRef();
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
