/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrMemoryPool.h"
#include "src/gpu/GrOpList.h"

GrOpList::GrOpList(sk_sp<GrOpMemoryPool> opMemoryPool,
                   sk_sp<GrSurfaceProxy> surfaceProxy,
                   GrAuditTrail* auditTrail)
        : GrRenderTask(std::move(surfaceProxy))
        , fOpMemoryPool(std::move(opMemoryPool))
        , fAuditTrail(auditTrail) {
    SkASSERT(fOpMemoryPool);
}

GrOpList::~GrOpList() {
}

void GrOpList::endFlush() {
    if (fTarget && this == fTarget->getLastRenderTask()) {
        fTarget->setLastRenderTask(nullptr);
    }

    fTarget.reset();
    fDeferredProxies.reset();
    fAuditTrail = nullptr;
}

#ifdef SK_DEBUG
static const char* op_to_name(GrLoadOp op) {
    return GrLoadOp::kLoad == op ? "load" : GrLoadOp::kClear == op ? "clear" : "discard";
}

void GrOpList::dump(bool printDependencies) const {
    GrRenderTask::dump(printDependencies);
    SkDebugf("ColorLoadOp: %s %x StencilLoadOp: %s\n",
             op_to_name(fColorLoadOp),
             GrLoadOp::kClear == fColorLoadOp ? fLoadClearColor.toBytes_RGBA() : 0x0,
             op_to_name(fStencilLoadOp));
}
#endif
