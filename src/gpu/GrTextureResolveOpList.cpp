/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrTextureResolveOpList.h"
#include "src/gpu/GrGpu.h"
#include "src/gpu/GrMemoryPool.h"
#include "src/gpu/GrOpFlushState.h"
#include "src/gpu/GrResourceAllocator.h"
#include "src/gpu/GrTexturePriv.h"

GrTextureResolveOpList::GrTextureResolveOpList(
        sk_sp<GrOpMemoryPool> opMemoryPool, sk_sp<GrTextureProxy> textureProxy,
        ResolveFlags resolveFlags, GrAuditTrail* auditTrail,
 const GrCaps& caps)
        : GrOpList(std::move(opMemoryPool), textureProxy, auditTrail)
        , fResolveFlags(resolveFlags) {
    SkASSERT(ResolveFlags::kNone != fResolveFlags);

    // Add the target as a dependency: We will read the existing contents of this texture while
    // generating mipmap levels and/or resolving MSAA.
    //
    // NOTE: This must be called before makeClosed.
    this->addDependency(fTarget.get(), GrMipMapped::kNo, nullptr, caps);
    fTarget->setLastOpList(this);

    // We only resolve the texture; nobody should try to do anything else with this opList.
    this->makeClosed(caps);

    if (ResolveFlags::kMipMaps & fResolveFlags) {
        SkASSERT(GrMipMapped::kYes == textureProxy->mipMapped());
        SkASSERT(textureProxy->mipMapsAreDirty());
        textureProxy->markMipMapsClean();
    }
}

void GrTextureResolveOpList::gatherProxyIntervals(GrResourceAllocator* alloc) const {
    // Huh? This makes the following assert go away:
    //
    // https://cs.chromium.org/chromium/src/third_party/skia/src/gpu/GrResourceAllocator.cpp?l=61
    alloc->addInterval(fTarget.get(), alloc->curOp(), alloc->curOp(),
                       GrResourceAllocator::ActualUse::kYes);
    alloc->incOps();
}

bool GrTextureResolveOpList::onExecute(GrOpFlushState* flushState) {
    GrTexture* texture = fTarget->peekTexture();
    SkASSERT(texture);

    if (ResolveFlags::kMipMaps & fResolveFlags) {
        SkASSERT(texture->texturePriv().mipMapsAreDirty());
        flushState->gpu()->regenerateMipMapLevels(texture);
    }

    return true;
}
