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
#include "src/gpu/GrTexturePriv.h"

GrTextureResolveRenderTask::~GrTextureResolveRenderTask() {
    for (const auto& resolve : fResolves) {
        // Ensure the proxy doesn't keep hold of a dangling back pointer.
        resolve.fProxyView.proxy()->setLastRenderTask(nullptr);
    }
}

void GrTextureResolveRenderTask::addProxy(
        GrSurfaceProxyView proxyView, GrSurfaceProxy::ResolveFlags flags, const GrCaps& caps) {
    fResolves.emplace_back(std::move(proxyView), flags);
    GrSurfaceProxy* proxy = fResolves.back().fProxyView.proxy();

    // Ensure the last render task that operated on the proxy is closed. That's where msaa and
    // mipmaps should have been marked dirty.
    SkASSERT(!proxy->getLastRenderTask() || proxy->getLastRenderTask()->isClosed());
    SkASSERT(GrSurfaceProxy::ResolveFlags::kNone != flags);

    if (GrSurfaceProxy::ResolveFlags::kMSAA & flags) {
        GrRenderTargetProxy* renderTargetProxy = proxy->asRenderTargetProxy();
        SkASSERT(renderTargetProxy);
        SkASSERT(renderTargetProxy->isMSAADirty());
        fResolves.back().fMSAAResolveRect = renderTargetProxy->msaaDirtyRect();
        renderTargetProxy->markMSAAResolved();
    }

    if (GrSurfaceProxy::ResolveFlags::kMipMaps & flags) {
        GrTextureProxy* textureProxy = proxy->asTextureProxy();
        SkASSERT(GrMipMapped::kYes == textureProxy->mipMapped());
        SkASSERT(textureProxy->mipMapsAreDirty());
        textureProxy->markMipMapsClean();
    }

    // Add the proxy as a dependency: We will read the existing contents of this texture while
    // generating mipmap levels and/or resolving MSAA.
    this->addDependency(proxy, GrMipMapped::kNo, GrTextureResolveManager(nullptr), caps);
    proxy->setLastRenderTask(this);
}

void GrTextureResolveRenderTask::gatherProxyIntervals(GrResourceAllocator* alloc) const {
    // This renderTask doesn't have "normal" ops, however we still need to add intervals so
    // fEndOfOpsTaskOpIndices will remain in sync. We create fake op#'s to capture the fact that we
    // manipulate the resolve proxies.
    auto fakeOp = alloc->curOp();
    for (const auto& resolve : fResolves) {
        alloc->addInterval(resolve.fProxyView.proxy(), fakeOp, fakeOp,
                           GrResourceAllocator::ActualUse::kYes);
    }
    alloc->incOps();
}

bool GrTextureResolveRenderTask::onExecute(GrOpFlushState* flushState) {
    // Resolve all msaa back-to-back, before regenerating mipmaps.
    for (const auto& resolve : fResolves) {
        if (GrSurfaceProxy::ResolveFlags::kMSAA & resolve.fFlags) {
            GrSurfaceProxy* proxy = resolve.fProxyView.proxy();
            // peekRenderTarget might be null if there was an instantiation error.
            if (GrRenderTarget* renderTarget = proxy->peekRenderTarget()) {
                flushState->gpu()->resolveRenderTarget(renderTarget, resolve.fMSAAResolveRect,
                                                       resolve.fProxyView.origin(),
                                                       GrGpu::ForExternalIO::kNo);
            }
        }
    }
    // Regenerate all mipmaps back-to-back.
    for (const auto& resolve : fResolves) {
        if (GrSurfaceProxy::ResolveFlags::kMipMaps & resolve.fFlags) {
            // peekTexture might be null if there was an instantiation error.
            GrTexture* texture = resolve.fProxyView.proxy()->peekTexture();
            if (texture && texture->texturePriv().mipMapsAreDirty()) {
                flushState->gpu()->regenerateMipMapLevels(texture);
                SkASSERT(!texture->texturePriv().mipMapsAreDirty());
            }
        }
    }

    return true;
}

#ifdef SK_DEBUG
void GrTextureResolveRenderTask::visitProxies_debugOnly(const GrOp::VisitProxyFunc& fn) const {
    for (const auto& resolve : fResolves) {
        fn(resolve.fProxyView.proxy(), GrMipMapped::kNo);
    }
}
#endif
