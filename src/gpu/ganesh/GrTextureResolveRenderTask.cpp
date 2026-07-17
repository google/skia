/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "src/gpu/ganesh/GrTextureResolveRenderTask.h"

#include "include/gpu/GpuTypes.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkMacros.h"
#include "src/gpu/ganesh/GrDrawingManager.h"
#include "src/gpu/ganesh/GrGpu.h"
#include "src/gpu/ganesh/GrOpFlushState.h"
#include "src/gpu/ganesh/GrRenderTargetProxy.h"
#include "src/gpu/ganesh/GrResourceAllocator.h"
#include "src/gpu/ganesh/GrSurfaceProxyView.h"
#include "src/gpu/ganesh/GrTexture.h"
#include "src/gpu/ganesh/GrTextureProxy.h"
#include "src/gpu/ganesh/GrTextureResolveManager.h"

#include <algorithm>
#include <cstddef>
#include <utility>

class GrRenderTarget;

void GrTextureResolveRenderTask::addProxy(GrDrawingManager* drawingMgr,
                                          sk_sp<GrSurfaceProxy> proxyRef,
                                          GrSurfaceProxy::ResolveFlags flags,
                                          const GrCaps& caps) {
    GrSurfaceProxy::ResolveFlags newFlags = flags;
    GrSurfaceProxy* proxy = proxyRef.get();
    Resolve* resolve;
    bool newProxy = false;

    // We might just need to update the flags for an existing dependency.
    if (auto found = std::find(fTargets.begin(), fTargets.end(), proxyRef);
        found != fTargets.end()) {
        size_t index = found - fTargets.begin();
        resolve = &fResolves[index];
        newFlags = ~resolve->fFlags & flags;
        resolve->fFlags |= flags;
    } else {
        // Ensure the last render task that operated on the proxy is closed. That's where msaa and
        // mipmaps should have been marked dirty.
        SkASSERT(!drawingMgr->getLastRenderTask(proxy)
                 || drawingMgr->getLastRenderTask(proxy)->isClosed());
        SkASSERT(GrSurfaceProxy::ResolveFlags::kNone != flags);
        resolve = &fResolves.emplace_back(flags);
        newProxy = true;
    }

    if (GrSurfaceProxy::ResolveFlags::kMSAA & newFlags) {
        GrRenderTargetProxy* renderTargetProxy = proxy->asRenderTargetProxy();
        SkASSERT(renderTargetProxy);
        SkASSERT(renderTargetProxy->isMSAADirty());
        resolve->fMSAAResolveRect = renderTargetProxy->msaaDirtyRect();
        renderTargetProxy->markMSAAResolved();
    }

    if (GrSurfaceProxy::ResolveFlags::kMipMaps & newFlags) {
        GrTextureProxy* textureProxy = proxy->asTextureProxy();
        SkASSERT(skgpu::Mipmapped::kYes == textureProxy->mipmapped());
        SkASSERT(textureProxy->mipmapsAreDirty());
        textureProxy->markMipmapsClean();
    }

    // We must do this after updating the proxy state because of assertions that the proxy isn't
    // dirty.
    if (newProxy) {
        // Add the proxy as a dependency: We will read the existing contents of this texture while
        // generating mipmap levels and/or resolving MSAA.
        this->addDependency(
                drawingMgr, proxy, skgpu::Mipmapped::kNo, GrTextureResolveManager(nullptr), caps);
        this->addTarget(drawingMgr, GrSurfaceProxyView(std::move(proxyRef)));
    }
}

void GrTextureResolveRenderTask::gatherProxyIntervals(GrResourceAllocator* alloc) const {
    // This renderTask doesn't have "normal" ops, however we still need to add intervals so
    // fEndOfOpsTaskOpIndices will remain in sync. We create fake op#'s to capture the fact that we
    // manipulate the resolve proxies.
    auto fakeOp = alloc->curOp();
    SkASSERT(fResolves.size() == this->numTargets());
    for (const sk_sp<GrSurfaceProxy>& target : fTargets) {
        alloc->addInterval(target.get(), fakeOp, fakeOp, GrResourceAllocator::ActualUse::kYes,
                           GrResourceAllocator::AllowRecycling::kYes);
    }
    alloc->incOps();
}

bool GrTextureResolveRenderTask::onExecute(GrOpFlushState* flushState) {
    // Resolve all msaa back-to-back, before regenerating mipmaps.
    SkASSERT(fResolves.size() == this->numTargets());
    for (int i = 0; i < fResolves.size(); ++i) {
        Resolve& resolve = fResolves[i];
        if (GrSurfaceProxy::ResolveFlags::kMSAA & resolve.fFlags) {
            GrSurfaceProxy* proxy = this->target(i);
            // peekRenderTarget might be null if there was an instantiation error.
            if (GrRenderTarget* renderTarget = proxy->peekRenderTarget()) {
                flushState->gpu()->resolveRenderTarget(renderTarget, resolve.fMSAAResolveRect);
                resolve.fFlags &= ~GrSurfaceProxy::ResolveFlags::kMSAA;
            }
        }
    }
    // Regenerate all mipmaps back-to-back.
    for (int i = 0; i < fResolves.size(); ++i) {
        Resolve& resolve = fResolves[i];
        if (GrSurfaceProxy::ResolveFlags::kMipMaps & resolve.fFlags) {
            // peekTexture might be null if there was an instantiation error.
            GrTexture* texture = this->target(i)->peekTexture();
            if (texture && (!texture->mipmapsAreDirty() ||
                            flushState->gpu()->regenerateMipMapLevels(texture))) {
                resolve.fFlags &= ~GrSurfaceProxy::ResolveFlags::kMipMaps;
            }
        }
    }

    return true;
}

void GrTextureResolveRenderTask::endFlush(GrDrawingManager* drawingMgr) {
    // Any flags still set here correspond to resolves that were recorded by addProxy() but never
    // executed (the flush was dropped before render-task execution, this task was skipped because
    // its targets failed to instantiate, or a per-target operation failed). Re-mark those proxies
    // dirty so a subsequent flush will re-record the resolve.
    SkASSERT(fResolves.size() == this->numTargets());
    for (int i = 0; i < fResolves.size(); ++i) {
        const Resolve& resolve = fResolves[i];
        GrSurfaceProxy* proxy = this->target(i);
        if (GrSurfaceProxy::ResolveFlags::kMSAA & resolve.fFlags) {
            if (GrRenderTargetProxy* rtProxy = proxy->asRenderTargetProxy()) {
                rtProxy->markMSAADirty(resolve.fMSAAResolveRect);
            }
        }
        if (GrSurfaceProxy::ResolveFlags::kMipMaps & resolve.fFlags) {
            if (GrTextureProxy* texProxy = proxy->asTextureProxy()) {
                texProxy->markMipmapsDirty();
            }
        }
    }
    this->GrRenderTask::endFlush(drawingMgr);
}

#ifdef SK_DEBUG
void GrTextureResolveRenderTask::visitProxies_debugOnly(const GrVisitProxyFunc&) const {}
#endif

#if defined(GPU_TEST_UTILS)
GrSurfaceProxy::ResolveFlags
GrTextureResolveRenderTask::flagsForProxy(sk_sp<GrSurfaceProxy> proxy) const {
    if (auto found = std::find(fTargets.begin(), fTargets.end(), proxy);
        found != fTargets.end()) {
        return fResolves[found - fTargets.begin()].fFlags;
    }
    return GrSurfaceProxy::ResolveFlags::kNone;
}
#endif
