/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/d3d/GrD3DTexture.h"

#include "src/gpu/GrTexturePriv.h"
#include "src/gpu/d3d/GrD3DGpu.h"
#include "src/gpu/d3d/GrD3DUtil.h"

#include "include/gpu/d3d/GrD3DTypes.h"

// Because this class is virtually derived from GrSurface we must explicitly call its constructor.
GrD3DTexture::GrD3DTexture(GrD3DGpu* gpu,
                           SkBudgeted budgeted,
                           SkISize dimensions,
                           const GrD3DTextureResourceInfo& info,
                           sk_sp<GrD3DResourceState> state,
                           GrMipMapsStatus mipMapsStatus)
        : GrSurface(gpu, dimensions, info.fProtected)
        , GrD3DTextureResource(info, std::move(state))
        , INHERITED(gpu, dimensions, info.fProtected, GrTextureType::k2D, mipMapsStatus) {
    SkASSERT((GrMipMapsStatus::kNotAllocated == mipMapsStatus) == (1 == info.fLevelCount));
    this->registerWithCache(budgeted);
    if (GrDxgiFormatIsCompressed(info.fFormat)) {
        this->setReadOnly();
    }
}

GrD3DTexture::GrD3DTexture(GrD3DGpu* gpu, SkISize dimensions, const GrD3DTextureResourceInfo& info,
                           sk_sp<GrD3DResourceState> state, GrMipMapsStatus mipMapsStatus,
                           GrWrapCacheable cacheable, GrIOType ioType)
        : GrSurface(gpu, dimensions, info.fProtected)
        , GrD3DTextureResource(info, std::move(state))
        , INHERITED(gpu, dimensions, info.fProtected, GrTextureType::k2D, mipMapsStatus) {
    SkASSERT((GrMipMapsStatus::kNotAllocated == mipMapsStatus) == (1 == info.fLevelCount));
    if (ioType == kRead_GrIOType) {
        this->setReadOnly();
    }
    this->registerWithCacheWrapped(cacheable);
}

// Because this class is virtually derived from GrSurface we must explicitly call its constructor.
GrD3DTexture::GrD3DTexture(GrD3DGpu* gpu,
                           SkISize dimensions,
                           const GrD3DTextureResourceInfo& info,
                           sk_sp<GrD3DResourceState> state,
                           GrMipMapsStatus mipMapsStatus)
        : GrSurface(gpu, dimensions, info.fProtected)
        , GrD3DTextureResource(info, state)
        , INHERITED(gpu, dimensions, info.fProtected, GrTextureType::k2D, mipMapsStatus) {
    SkASSERT((GrMipMapsStatus::kNotAllocated == mipMapsStatus) == (1 == info.fLevelCount));
}

sk_sp<GrD3DTexture> GrD3DTexture::MakeNewTexture(GrD3DGpu* gpu, SkBudgeted budgeted,
                                                 SkISize dimensions,
                                                 const D3D12_RESOURCE_DESC& desc,
                                                 GrProtected isProtected,
                                                 GrMipMapsStatus mipMapsStatus) {
    GrD3DTextureResourceInfo info;
    if (!GrD3DTextureResource::InitTextureResourceInfo(gpu, desc, isProtected, &info)) {
        return nullptr;
    }

    sk_sp<GrD3DResourceState> state(
            new GrD3DResourceState(static_cast<D3D12_RESOURCE_STATES>(info.fResourceState)));

    GrD3DTexture* tex = new GrD3DTexture(gpu, budgeted, dimensions, info, std::move(state),
                                         mipMapsStatus);

    // The GrD3DTexture takes a ref on the texture so we need to release ours
    GrD3DTextureResource::ReleaseTextureResourceInfo(&info);

    return sk_sp<GrD3DTexture>(tex);
}

sk_sp<GrD3DTexture> GrD3DTexture::MakeWrappedTexture(GrD3DGpu* gpu,
                                                     SkISize dimensions,
                                                     GrWrapCacheable cacheable,
                                                     GrIOType ioType,
                                                     const GrD3DTextureResourceInfo& info,
                                                     sk_sp<GrD3DResourceState> state) {
    // TODO: If a client uses their own heap to allocate, how do we manage that?
    // Adopted textures require both image and allocation because we're responsible for freeing
    //SkASSERT(info.fTexture &&
    //         (kBorrow_GrWrapOwnership == wrapOwnership || VK_NULL_HANDLE != info.fAlloc.fMemory));

    GrMipMapsStatus mipMapsStatus = info.fLevelCount > 1 ? GrMipMapsStatus::kValid
                                                         : GrMipMapsStatus::kNotAllocated;

    return sk_sp<GrD3DTexture>(new GrD3DTexture(gpu, dimensions, info, std::move(state),
                                                mipMapsStatus, cacheable, ioType));
}

void GrD3DTexture::onRelease() {
    // We're about to be severed from our GrManagedResource. If there are "finish" idle procs we
    // have to decide who will handle them. If the resource is still tied to a command buffer we let
    // it handle them. Otherwise, we handle them.
    SkASSERT(this->resource());
    if (this->resource()->isQueuedForWorkOnGpu()) {
        this->removeFinishIdleProcs();
    }

    this->releaseResource(this->getD3DGpu());

    INHERITED::onRelease();
}

void GrD3DTexture::onAbandon() {
    // We're about to be severed from our GrManagedResource. If there are "finish" idle procs we
    // have to decide who will handle them. If the resource is still tied to a command buffer we let
    // it handle them. Otherwise, we handle them.
    SkASSERT(this->resource());
    if (this->resource()->isQueuedForWorkOnGpu()) {
        this->removeFinishIdleProcs();
    }

    this->releaseResource(this->getD3DGpu());
    INHERITED::onAbandon();
}

GrBackendTexture GrD3DTexture::getBackendTexture() const {
    return GrBackendTexture(this->width(), this->height(), fInfo, this->grD3DResourceState());
}

GrD3DGpu* GrD3DTexture::getD3DGpu() const {
    SkASSERT(!this->wasDestroyed());
    return static_cast<GrD3DGpu*>(this->getGpu());
}

void GrD3DTexture::addIdleProc(sk_sp<GrRefCntedCallback> idleProc, IdleState type) {
    INHERITED::addIdleProc(idleProc, type);
    if (type == IdleState::kFinished) {
        if (auto* resource = this->resource()) {
            resource->addIdleProc(this, std::move(idleProc));
        }
    }
}

void GrD3DTexture::callIdleProcsOnBehalfOfResource() {
    // If we got here then the resource is being removed from its last command buffer and the
    // texture is idle in the cache. Any kFlush idle procs should already have been called. So
    // the texture and resource should have the same set of procs.
    SkASSERT(this->resource());
    SkASSERT(this->resource()->idleProcCnt() == fIdleProcs.count());
#ifdef SK_DEBUG
    for (int i = 0; i < fIdleProcs.count(); ++i) {
        SkASSERT(fIdleProcs[i] == this->resource()->idleProc(i));
    }
#endif
    fIdleProcs.reset();
    this->resource()->resetIdleProcs();
}

void GrD3DTexture::willRemoveLastRef() {
    if (!fIdleProcs.count()) {
        return;
    }
    // This is called when the GrTexture is purgeable. However, we need to check whether the
    // Resource is still owned by any command buffers. If it is then it will call the proc.
    auto* resource = this->resource();
    SkASSERT(resource);
    if (!resource->isQueuedForWorkOnGpu()) {
        // Everything must go!
        fIdleProcs.reset();
        resource->resetIdleProcs();
    } else {
        // The procs that should be called on flush but not finish are those that are owned
        // by the GrD3DTexture and not the Resource. We do this by copying the resource's array
        // and thereby dropping refs to procs we own but the resource does not.
        fIdleProcs.reset(resource->idleProcCnt());
        for (int i = 0; i < fIdleProcs.count(); ++i) {
            fIdleProcs[i] = resource->idleProc(i);
        }
    }
}

void GrD3DTexture::removeFinishIdleProcs() {
    // This should only be called by onRelease/onAbandon when we have already checked for a
    // resource.
    const auto* resource = this->resource();
    SkASSERT(resource);
    SkSTArray<4, sk_sp<GrRefCntedCallback>> procsToKeep;
    int resourceIdx = 0;
    // The idle procs that are common between the GrD3DTexture and its Resource should be found in
    // the same order.
    for (int i = 0; i < fIdleProcs.count(); ++i) {
        if (fIdleProcs[i] == resource->idleProc(resourceIdx)) {
            ++resourceIdx;
        } else {
            procsToKeep.push_back(fIdleProcs[i]);
        }
    }
    SkASSERT(resourceIdx == resource->idleProcCnt());
    fIdleProcs = procsToKeep;
}
