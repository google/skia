/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrTextureOpList.h"

#include "include/gpu/GrContext.h"
#include "include/private/GrRecordingContext.h"
#include "src/core/SkStringUtils.h"
#include "src/gpu/GrAuditTrail.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrGpu.h"
#include "src/gpu/GrMemoryPool.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/GrResourceAllocator.h"
#include "src/gpu/GrTextureProxy.h"
#include "src/gpu/ops/GrCopySurfaceOp.h"

////////////////////////////////////////////////////////////////////////////////

GrTextureOpList::GrTextureOpList(sk_sp<GrOpMemoryPool> opMemoryPool,
                                 sk_sp<GrTextureProxy> proxy,
                                 GrAuditTrail* auditTrail)
        : INHERITED(std::move(opMemoryPool), proxy, auditTrail) {
    SkASSERT(fOpMemoryPool);
    SkASSERT(!proxy->readOnly());
}

void GrTextureOpList::deleteOp(int index) {
    SkASSERT(index >= 0 && index < fRecordedOps.count());
    fOpMemoryPool->release(std::move(fRecordedOps[index]));
}

void GrTextureOpList::deleteOps() {
    for (int i = 0; i < fRecordedOps.count(); ++i) {
        if (fRecordedOps[i]) {
            fOpMemoryPool->release(std::move(fRecordedOps[i]));
        }
    }
    fRecordedOps.reset();
    fOpMemoryPool = nullptr;
}

GrTextureOpList::~GrTextureOpList() {
    this->deleteOps();
}

////////////////////////////////////////////////////////////////////////////////

#ifdef SK_DEBUG
void GrTextureOpList::dump(bool printDependencies) const {
    INHERITED::dump(printDependencies);

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
    SkASSERT(fTarget.get()->peekTexture());
    SkASSERT(this->isClosed());

    // Loop over the ops that haven't yet generated their geometry
    for (int i = 0; i < fRecordedOps.count(); ++i) {
        if (fRecordedOps[i]) {
            SkASSERT(fRecordedOps[i]->isChainHead());
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

    SkASSERT(fTarget.get()->peekTexture());

    GrGpuTextureCommandBuffer* commandBuffer(
                         flushState->gpu()->getCommandBuffer(fTarget.get()->peekTexture(),
                                                             fTarget.get()->origin()));
    flushState->setCommandBuffer(commandBuffer);

    for (int i = 0; i < fRecordedOps.count(); ++i) {
        if (!fRecordedOps[i]) {
            continue;
        }
        SkASSERT(fRecordedOps[i]->isChainHead());
        GrOpFlushState::OpArgs opArgs = {
            fRecordedOps[i].get(),
            nullptr,
            nullptr,
            GrXferProcessor::DstProxy()
        };
        flushState->setOpArgs(&opArgs);
        fRecordedOps[i]->execute(flushState, fRecordedOps[i].get()->bounds());
        flushState->setOpArgs(nullptr);
    }

    flushState->gpu()->submit(commandBuffer);
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
bool GrTextureOpList::copySurface(GrRecordingContext* context,
                                  GrSurfaceProxy* dst,
                                  GrSurfaceProxy* src,
                                  const SkIRect& srcRect,
                                  const SkIPoint& dstPoint) {
    SkASSERT(dst == fTarget.get());

    std::unique_ptr<GrOp> op = GrCopySurfaceOp::Make(context, dst, src, srcRect, dstPoint);
    if (!op) {
        return false;
    }

    const GrCaps* caps = context->priv().caps();
    auto addDependency = [ caps, this ] (GrSurfaceProxy* p, GrMipMapped) {
        this->addDependency(p, *caps);
    };
    op->visitProxies(addDependency);

    this->recordOp(std::move(op));
    return true;
}

void GrTextureOpList::purgeOpsWithUninstantiatedProxies() {
    bool hasUninstantiatedProxy = false;
    auto checkInstantiation = [&hasUninstantiatedProxy](GrSurfaceProxy* p, GrMipMapped) {
        if (!p->isInstantiated()) {
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
            this->deleteOp(i);
        }
    }
}

bool GrTextureOpList::onIsUsed(GrSurfaceProxy* proxyToCheck) const {
    bool used = false;

    auto visit = [ proxyToCheck, &used ] (GrSurfaceProxy* p, GrMipMapped) {
        if (p == proxyToCheck) {
            used = true;
        }
    };
    for (int i = 0; i < fRecordedOps.count(); ++i) {
        const GrOp* op = fRecordedOps[i].get();
        if (op) {
            op->visitProxies(visit);
        }
    }

    return used;
}

void GrTextureOpList::gatherProxyIntervals(GrResourceAllocator* alloc) const {

    // Add the interval for all the writes to this opList's target
    if (fRecordedOps.count()) {
        unsigned int cur = alloc->curOp();

        alloc->addInterval(fTarget.get(), cur, cur+fRecordedOps.count()-1,
                           GrResourceAllocator::ActualUse::kYes);
    } else {
        // This can happen if there is a loadOp (e.g., a clear) but no other draws. In this case we
        // still need to add an interval for the destination so we create a fake op# for
        // the missing clear op.
        alloc->addInterval(fTarget.get(), alloc->curOp(), alloc->curOp(),
                           GrResourceAllocator::ActualUse::kYes);
        alloc->incOps();
    }

    auto gather = [ alloc SkDEBUGCODE(, this) ] (GrSurfaceProxy* p, GrMipMapped) {
        alloc->addInterval(p, alloc->curOp(), alloc->curOp(), GrResourceAllocator::ActualUse::kYes
                           SkDEBUGCODE(, p == fTarget.get()));
    };
    for (int i = 0; i < fRecordedOps.count(); ++i) {
        const GrOp* op = fRecordedOps[i].get(); // only diff from the GrRenderTargetOpList version
        if (op) {
            op->visitProxies(gather);
        }

        // Even though the op may have been (re)moved we still need to increment the op count to
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

    fRecordedOps.emplace_back(std::move(op));
}
