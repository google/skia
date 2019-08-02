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
#include "src/gpu/GrResourceAllocator.h"
#include "src/gpu/GrTexturePriv.h"

GrTextureResolveRenderTask::GrTextureResolveRenderTask(
        sk_sp<GrTextureProxy> textureProxy, ResolveFlags resolveFlags, const GrCaps& caps)
        : GrRenderTask(textureProxy)
        , fResolveFlags(resolveFlags) {
    SkASSERT(ResolveFlags::kNone != fResolveFlags);

    // Add the target as a dependency: We will read the existing contents of this texture while
    // generating mipmap levels and/or resolving MSAA.
    //
    // NOTE: This must be called before makeClosed.
    this->addDependency(fTarget.get(), GrMipMapped::kNo, nullptr, caps);
    fTarget->setLastRenderTask(this);

    // We only resolve the texture; nobody should try to do anything else with this opList.
    this->makeClosed(caps);

    if (ResolveFlags::kMipMaps & fResolveFlags) {
        SkASSERT(GrMipMapped::kYes == textureProxy->mipMapped());
        SkASSERT(textureProxy->mipMapsAreDirty());
        textureProxy->markMipMapsClean();
    }
}

void GrTextureResolveRenderTask::gatherProxyIntervals(GrResourceAllocator* alloc) const {
    // Huh? This makes the following assert go away:
    //
    // https://cs.chromium.org/chromium/src/third_party/skia/src/gpu/GrResourceAllocator.cpp?l=61
    alloc->addInterval(fTarget.get(), alloc->curOp(), alloc->curOp(),
                       GrResourceAllocator::ActualUse::kYes);
    alloc->incOps();
}

bool GrTextureResolveRenderTask::onExecute(GrOpFlushState* flushState) {
    GrTexture* texture = fTarget->peekTexture();
    SkASSERT(texture);

    if (ResolveFlags::kMipMaps & fResolveFlags) {
        SkASSERT(texture->texturePriv().mipMapsAreDirty());
        flushState->gpu()->regenerateMipMapLevels(texture);
    }

    return true;
}
