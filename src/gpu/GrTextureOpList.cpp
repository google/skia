/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrTextureOpList.h"

#include "GrAuditTrail.h"
#include "GrContext.h"
#include "GrContextPriv.h"
#include "GrGpu.h"
#include "GrResourceAllocator.h"
#include "GrTextureProxy.h"
#include "SkStringUtils.h"
#include "ops/GrCopySurfaceOp.h"

////////////////////////////////////////////////////////////////////////////////

GrTextureOpList::GrTextureOpList(GrResourceProvider* resourceProvider,
                                 sk_sp<GrMemoryPool> opMemoryPool,
                                 GrTextureProxy* proxy,
                                 GrAuditTrail* auditTrail)
    : INHERITED(resourceProvider, std::move(opMemoryPool), proxy, auditTrail) {
}

void GrTextureOpList::deleteOp(int index) {
    SkASSERT(index < fRecordedOps1.count());
    fRecordedOps1[index]->~GrOp();
    fOpMemoryPool->release(fRecordedOps1[index]);
    fRecordedOps1[index] = nullptr;
}

void GrTextureOpList::deleteOps() {
    for (int i = 0; i < fRecordedOps1.count(); ++i) {
        fRecordedOps1[i]->~GrOp();
        fOpMemoryPool->release(fRecordedOps1[i]);
    }
    fRecordedOps1.reset();
    fOpMemoryPool = nullptr;
}

GrTextureOpList::~GrTextureOpList() {
    this->deleteOps();
}

////////////////////////////////////////////////////////////////////////////////

#ifdef SK_DEBUG
void GrTextureOpList::dump(bool printDependencies) const {
    INHERITED::dump(printDependencies);

    SkDebugf("ops (%d):\n", fRecordedOps1.count());
    for (int i = 0; i < fRecordedOps1.count(); ++i) {
        if (!fRecordedOps1[i]) {
            SkDebugf("%d: <failed instantiation>\n", i);
        } else {
            SkDebugf("*******************************\n");
            SkDebugf("%d: %s\n", i, fRecordedOps1[i]->name());
            SkString str = fRecordedOps1[i]->dumpInfo();
            SkDebugf("%s\n", str.c_str());
            const SkRect& clippedBounds = fRecordedOps1[i]->bounds();
            SkDebugf("ClippedBounds: [L: %.2f, T: %.2f, R: %.2f, B: %.2f]\n",
                     clippedBounds.fLeft, clippedBounds.fTop, clippedBounds.fRight,
                     clippedBounds.fBottom);
        }
    }
}

#endif

void GrTextureOpList::onPrepare(GrOpFlushState* flushState) {
    SkASSERT(fTarget.get()->priv().peekTexture());
    SkASSERT(this->isClosed());

    // Loop over the ops that haven't yet generated their geometry
    for (int i = 0; i < fRecordedOps1.count(); ++i) {
        if (fRecordedOps1[i]) {
            GrOpFlushState::OpArgs opArgs = {
                fRecordedOps1[i],
                nullptr,
                nullptr,
                GrXferProcessor::DstProxy()
            };
            flushState->setOpArgs(&opArgs);
            fRecordedOps1[i]->prepare(flushState);
            flushState->setOpArgs(nullptr);
        }
    }
}

bool GrTextureOpList::onExecute(GrOpFlushState* flushState) {
    if (0 == fRecordedOps1.count()) {
        return false;
    }

    SkASSERT(fTarget.get()->priv().peekTexture());

    std::unique_ptr<GrGpuTextureCommandBuffer> commandBuffer(
                         flushState->gpu()->createCommandBuffer(fTarget.get()->priv().peekTexture(),
                                                                fTarget.get()->origin()));
    flushState->setCommandBuffer(commandBuffer.get());

    for (int i = 0; i < fRecordedOps1.count(); ++i) {
        if (!fRecordedOps1[i]) {
            continue;
        }

        GrOpFlushState::OpArgs opArgs = {
            fRecordedOps1[i],
            nullptr,
            nullptr,
            GrXferProcessor::DstProxy()
        };
        flushState->setOpArgs(&opArgs);
        fRecordedOps1[i]->execute(flushState);
        flushState->setOpArgs(nullptr);
    }

    commandBuffer->submit();
    flushState->setCommandBuffer(nullptr);

    return true;
}

void GrTextureOpList::endFlush() {
    this->deleteOps();
    INHERITED::endFlush();
}

////////////////////////////////////////////////////////////////////////////////

// This closely parallels GrRenderTargetOpList::copySurface but renderTargetOpList
// stores extra data with the op
bool GrTextureOpList::copySurface(GrContext* context,
                                  GrSurfaceProxy* dst,
                                  GrSurfaceProxy* src,
                                  const SkIRect& srcRect,
                                  const SkIPoint& dstPoint) {
    SkASSERT(dst == fTarget.get());

    std::unique_ptr<GrOp> op = GrCopySurfaceOp::Make(context, dst, src, srcRect, dstPoint);
    if (!op) {
        return false;
    }

    const GrCaps* caps = context->contextPriv().caps();
    auto addDependency = [ caps, this ] (GrSurfaceProxy* p) {
        this->addDependency(p, *caps);
    };
    op->visitProxies(addDependency);

    this->recordOp(std::move(op));
    return true;
}

void GrTextureOpList::purgeOpsWithUninstantiatedProxies() {
    bool hasUninstantiatedProxy = false;
    auto checkInstantiation = [ &hasUninstantiatedProxy ] (GrSurfaceProxy* p) {
        if (!p->priv().isInstantiated()) {
            hasUninstantiatedProxy = true;
        }
    };
    for (int i = 0; i < fRecordedOps1.count(); ++i) {
        const GrOp* op = fRecordedOps1[i]; // only diff from the GrRenderTargetOpList version
        hasUninstantiatedProxy = false;
        if (op) {
            op->visitProxies(checkInstantiation);
        }
        if (hasUninstantiatedProxy) {
            // When instantiation of the proxy fails we drop the Op
            this->deleteOp(i);
        }
    }
}

void GrTextureOpList::gatherProxyIntervals(GrResourceAllocator* alloc) const {
    unsigned int cur = alloc->numOps();

    // Add the interval for all the writes to this opList's target
    if (fRecordedOps1.count()) {
        alloc->addInterval(fTarget.get(), cur, cur+fRecordedOps1.count()-1);
    } else {
        // This can happen if there is a loadOp (e.g., a clear) but no other draws. In this case we
        // still need to add an interval for the destination so we create a fake op# for
        // the missing clear op.
        alloc->addInterval(fTarget.get());
        alloc->incOps();
    }

    auto gather = [ alloc SkDEBUGCODE(, this) ] (GrSurfaceProxy* p) {
        alloc->addInterval(p SkDEBUGCODE(, p == fTarget.get()));
    };
    for (int i = 0; i < fRecordedOps1.count(); ++i) {
        const GrOp* op = fRecordedOps1[i]; // only diff from the GrRenderTargetOpList version
        if (op) {
            op->visitProxies(gather);
        }

        // Even though the op may have been moved we still need to increment the op count to
        // keep all the math consistent.
        alloc->incOps();
    }
}

void GrTextureOpList::recordOp(std::unique_ptr<GrOp> op) {
    SkASSERT(fTarget.get());
    // A closed GrOpList should never receive new/more ops
    SkASSERT(!this->isClosed());

    GR_AUDIT_TRAIL_ADD_OP(fAuditTrail, op.get(), fTarget.get()->uniqueID());
    GrOP_INFO("Re-Recording (%s, opID: %u)\n"
        "\tBounds LRTB (%f, %f, %f, %f)\n",
        op->name(),
        op->uniqueID(),
        op->bounds().fLeft, op->bounds().fRight,
        op->bounds().fTop, op->bounds().fBottom);
    GrOP_INFO(SkTabString(op->dumpInfo(), 1).c_str());
    GR_AUDIT_TRAIL_OP_RESULT_NEW(fAuditTrail, op.get());

    fRecordedOps1.emplace_back(op.release());
}
