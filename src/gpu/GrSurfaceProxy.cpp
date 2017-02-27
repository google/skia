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

GrSurface* GrSurfaceProxy::instantiate(GrTextureProvider* texProvider) {
    if (fTarget) {
        return fTarget;
    }

    if (SkBackingFit::kApprox == fFit) {
        fTarget = texProvider->createApproxTexture(fDesc, fFlags);
    } else {
        fTarget = texProvider->createTexture(fDesc, fBudgeted, fFlags);
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

sk_sp<GrSurfaceProxy> GrSurfaceProxy::MakeDeferred(const GrCaps& caps,
                                                   const GrSurfaceDesc& desc,
                                                   SkBackingFit fit,
                                                   SkBudgeted budgeted,
                                                   uint32_t flags) {
    SkASSERT(0 == flags || GrResourceProvider::kNoPendingIO_Flag == flags);

    // TODO: share this testing code with check_texture_creation_params
    if (GrPixelConfigIsCompressed(desc.fConfig)) {
        if (SkBackingFit::kApprox == fit || kBottomLeft_GrSurfaceOrigin == desc.fOrigin) {
            // We don't allow scratch compressed textures and, apparently can't Y-flip compressed
            // textures
            return nullptr;
        }

        if (!caps.npotTextureTileSupport() && (!SkIsPow2(desc.fWidth) || !SkIsPow2(desc.fHeight))) {
            return nullptr;
        }
    }

    if (!caps.isConfigTexturable(desc.fConfig)) {
        return nullptr;
    }

    bool willBeRT = SkToBool(desc.fFlags & kRenderTarget_GrSurfaceFlag);
    if (willBeRT && !caps.isConfigRenderable(desc.fConfig, desc.fSampleCnt > 0)) {
        return nullptr;
    }

    // We currently do not support multisampled textures
    if (!willBeRT && desc.fSampleCnt > 0) {
        return nullptr;
    }

    int maxSize;
    if (willBeRT) {
        maxSize = caps.maxRenderTargetSize();
    } else {
        maxSize = caps.maxTextureSize();
    }

    if (desc.fWidth > maxSize || desc.fHeight > maxSize) {
        return nullptr;
    }

    GrSurfaceDesc copyDesc = desc;
    copyDesc.fSampleCnt = SkTMin(desc.fSampleCnt, caps.maxSampleCount());

    if (willBeRT) {
        // We know anything we instantiate later from this deferred path will be
        // both texturable and renderable
        return sk_sp<GrSurfaceProxy>(new GrTextureRenderTargetProxy(caps, copyDesc, fit,
                                                                    budgeted, flags));
    }

    return sk_sp<GrSurfaceProxy>(new GrTextureProxy(copyDesc, fit, budgeted, nullptr, 0, flags));
}

sk_sp<GrSurfaceProxy> GrSurfaceProxy::MakeDeferred(const GrCaps& caps,
                                                   GrTextureProvider* texProvider,
                                                   const GrSurfaceDesc& desc,
                                                   SkBudgeted budgeted,
                                                   const void* srcData,
                                                   size_t rowBytes) {
    if (srcData) {
        // If we have srcData, for now, we create a wrapped GrTextureProxy
        sk_sp<GrTexture> tex(texProvider->createTexture(desc, budgeted, srcData, rowBytes));
        return GrSurfaceProxy::MakeWrapped(std::move(tex));
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
