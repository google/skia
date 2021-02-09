/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrTextureResolveRenderTask.h"

#include "src/gpu/GrGpu.h"
#include "src/gpu/GrMemoryPool.h"
#include "src/gpu/GrOpFlushState.h"
#include "src/gpu/GrRenderTarget.h"
#include "src/gpu/GrResourceAllocator.h"
#include "src/gpu/GrTexture.h"

void GrTextureResolveRenderTask::addProxy(GrDrawingManager* drawingMgr,
                                          sk_sp<GrSurfaceProxy> proxyRef,
                                          GrSurfaceProxy::ResolveFlags flags,
                                          const GrCaps& caps) {
    Resolve& resolve = fResolves.emplace_back(flags);
    GrSurfaceProxy* proxy = proxyRef.get();

    // Ensure the last render task that operated on the proxy is closed. That's where msaa and
    // mipmaps should have been marked dirty.
    SkASSERT(!drawingMgr->getLastRenderTask(proxy)
             || drawingMgr->getLastRenderTask(proxy)->isClosed());
    SkASSERT(GrSurfaceProxy::ResolveFlags::kNone != flags);

    if (GrSurfaceProxy::ResolveFlags::kMSAA & flags) {
        GrRenderTargetProxy* renderTargetProxy = proxy->asRenderTargetProxy();
        SkASSERT(renderTargetProxy);
        SkASSERT(renderTargetProxy->isMSAADirty());
        resolve.fMSAAResolveRect = renderTargetProxy->msaaDirtyRect();
        renderTargetProxy->markMSAAResolved();
    }

    if (GrSurfaceProxy::ResolveFlags::kMipMaps & flags) {
        GrTextureProxy* textureProxy = proxy->asTextureProxy();
        SkASSERT(GrMipmapped::kYes == textureProxy->mipmapped());
        SkASSERT(textureProxy->mipmapsAreDirty());
        textureProxy->markMipmapsClean();
    }

    // Add the proxy as a dependency: We will read the existing contents of this texture while
    // generating mipmap levels and/or resolving MSAA.
    this->addDependency(drawingMgr, proxy, GrMipmapped::kNo,
                        GrTextureResolveManager(nullptr), caps);
    this->addTarget(drawingMgr, GrSurfaceProxyView(std::move(proxyRef)));
}

void GrTextureResolveRenderTask::gatherProxyIntervals(GrResourceAllocator* alloc) const {
    // This renderTask doesn't have "normal" ops, however we still need to add intervals so
    // fEndOfOpsTaskOpIndices will remain in sync. We create fake op#'s to capture the fact that we
    // manipulate the resolve proxies.
    auto fakeOp = alloc->curOp();
    SkASSERT(fResolves.count() == this->numTargets());
    for (const sk_sp<GrSurfaceProxy>& target : fTargets) {
        alloc->addInterval(target.get(), fakeOp, fakeOp, GrResourceAllocator::ActualUse::kYes);
    }
    alloc->incOps();
}

bool GrTextureResolveRenderTask::onExecute(GrOpFlushState* flushState) {
    // Resolve all msaa back-to-back, before regenerating mipmaps.
    SkASSERT(fResolves.count() == this->numTargets());
    for (int i = 0; i < fResolves.count(); ++i) {
        const Resolve& resolve = fResolves[i];
        if (GrSurfaceProxy::ResolveFlags::kMSAA & resolve.fFlags) {
            GrSurfaceProxy* proxy = this->target(i);
            // peekRenderTarget might be null if there was an instantiation error.
            if (GrRenderTarget* renderTarget = proxy->peekRenderTarget()) {
                flushState->gpu()->resolveRenderTarget(renderTarget, resolve.fMSAAResolveRect);
            }
        }
    }
    // Regenerate all mipmaps back-to-back.
    for (int i = 0; i < fResolves.count(); ++i) {
        const Resolve& resolve = fResolves[i];
        if (GrSurfaceProxy::ResolveFlags::kMipMaps & resolve.fFlags) {
            // peekTexture might be null if there was an instantiation error.
            GrTexture* texture = this->target(i)->peekTexture();
            if (texture && texture->mipmapsAreDirty()) {
                flushState->gpu()->regenerateMipMapLevels(texture);
                SkASSERT(!texture->mipmapsAreDirty());
            }
        }
    }

    return true;
}

#ifdef SK_DEBUG
void GrTextureResolveRenderTask::visitProxies_debugOnly(const GrOp::VisitProxyFunc& fn) const {}
#endif
