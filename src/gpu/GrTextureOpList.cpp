/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrTextureOpList.h"

#include "GrAuditTrail.h"
#include "GrGpu.h"
#include "GrResourceAllocator.h"
#include "GrTextureProxy.h"
#include "SkStringUtils.h"
#include "ops/GrCopySurfaceOp.h"

////////////////////////////////////////////////////////////////////////////////

GrTextureOpList::GrTextureOpList(GrResourceProvider* resourceProvider,
                                 GrTextureProxy* proxy,
                                 GrAuditTrail* auditTrail)
    : INHERITED(resourceProvider, proxy, auditTrail) {
}

GrTextureOpList::~GrTextureOpList() {
}

////////////////////////////////////////////////////////////////////////////////

#ifdef SK_DEBUG
void GrTextureOpList::dump() const {
    INHERITED::dump();

    SkDebugf("ops (%d):\n", fRecordedOps.count());
    for (int i = 0; i < fRecordedOps.count(); ++i) {
        if (!fRecordedOps[i]) {
            SkDebugf("%d: <failed instantiation>\n", i);
        } else {
            SkDebugf("*******************************\n");
            SkDebugf("%d: %s\n", i, fRecordedOps[i]->name());
            SkString str = fRecordedOps[i]->dumpInfo();
            SkDebugf("%s\n", str.c_str());
            const SkRect& clippedBounds = fRecordedOps[i]->bounds();
            SkDebugf("ClippedBounds: [L: %.2f, T: %.2f, R: %.2f, B: %.2f]\n",
                     clippedBounds.fLeft, clippedBounds.fTop, clippedBounds.fRight,
                     clippedBounds.fBottom);
        }
    }
}

#endif

void GrTextureOpList::onPrepare(GrOpFlushState* flushState) {
    SkASSERT(this->isClosed());

    // Loop over the ops that haven't yet generated their geometry
    for (int i = 0; i < fRecordedOps.count(); ++i) {
        if (fRecordedOps[i]) {
            GrOpFlushState::OpArgs opArgs = {
                fRecordedOps[i].get(),
                nullptr,
                nullptr,
                GrXferProcessor::DstProxy()
            };
            flushState->setOpArgs(&opArgs);
            fRecordedOps[i]->prepare(flushState);
            flushState->setOpArgs(nullptr);
        }
    }
}

bool GrTextureOpList::onExecute(GrOpFlushState* flushState) {
    if (0 == fRecordedOps.count()) {
        return false;
    }

    std::unique_ptr<GrGpuTextureCommandBuffer> commandBuffer(
                         flushState->gpu()->createCommandBuffer(fTarget.get()->priv().peekTexture(),
                                                                fTarget.get()->origin()));
    flushState->setCommandBuffer(commandBuffer.get());

    for (int i = 0; i < fRecordedOps.count(); ++i) {
        GrOpFlushState::OpArgs opArgs = {
            fRecordedOps[i].get(),
            nullptr,
            nullptr,
            GrXferProcessor::DstProxy()
        };
        flushState->setOpArgs(&opArgs);
        fRecordedOps[i]->execute(flushState);
        flushState->setOpArgs(nullptr);
    }

    commandBuffer->submit();
    flushState->setCommandBuffer(nullptr);

    return true;
}

void GrTextureOpList::endFlush() {
    fRecordedOps.reset();
    INHERITED::endFlush();
}

////////////////////////////////////////////////////////////////////////////////

// This closely parallels GrRenderTargetOpList::copySurface but renderTargetOpList
// stores extra data with the op
bool GrTextureOpList::copySurface(const GrCaps& caps,
                                  GrSurfaceProxy* dst,
                                  GrSurfaceProxy* src,
                                  const SkIRect& srcRect,
                                  const SkIPoint& dstPoint) {
    SkASSERT(dst == fTarget.get());

    std::unique_ptr<GrOp> op = GrCopySurfaceOp::Make(dst, src, srcRect, dstPoint);
    if (!op) {
        return false;
    }

    auto addDependency = [ &caps, this ] (GrSurfaceProxy* p) {
        this->addDependency(p, caps);
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
    for (int i = 0; i < fRecordedOps.count(); ++i) {
        const GrOp* op = fRecordedOps[i].get(); // only diff from the GrRenderTargetOpList version
        hasUninstantiatedProxy = false;
        if (op) {
            op->visitProxies(checkInstantiation);
        }
        if (hasUninstantiatedProxy) {
            // When instantiation of the proxy fails we drop the Op
            fRecordedOps[i] = nullptr;
        }
    }
}

void GrTextureOpList::gatherProxyIntervals(GrResourceAllocator* alloc) const {
    unsigned int cur = alloc->numOps();

    // Add the interval for all the writes to this opList's target
    if (fRecordedOps.count()) {
        alloc->addInterval(fTarget.get(), cur, cur+fRecordedOps.count()-1);
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
    for (int i = 0; i < fRecordedOps.count(); ++i) {
        const GrOp* op = fRecordedOps[i].get(); // only diff from the GrRenderTargetOpList version
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

    fRecordedOps.emplace_back(std::move(op));
}
