/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrTextureOpList.h"

#include "GrAuditTrail.h"
#include "GrGpu.h"
#include "GrTextureProxy.h"
#include "SkStringUtils.h"
#include "ops/GrCopySurfaceOp.h"

////////////////////////////////////////////////////////////////////////////////

GrTextureOpList::GrTextureOpList(GrTextureProxy* tex, GrGpu* gpu, GrAuditTrail* auditTrail)
    : INHERITED(tex, auditTrail)
    , fGpu(SkRef(gpu)) {
}

GrTextureOpList::~GrTextureOpList() {
    fGpu->unref();
}

////////////////////////////////////////////////////////////////////////////////

#ifdef SK_DEBUG
void GrTextureOpList::dump() const {
    INHERITED::dump();

    SkDebugf("ops (%d):\n", fRecordedOps.count());
    for (int i = 0; i < fRecordedOps.count(); ++i) {
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
#endif

void GrTextureOpList::prepareOps(GrOpFlushState* flushState) {
    // MDB TODO: add SkASSERT(this->isClosed());

    // Loop over the ops that haven't yet generated their geometry
    for (int i = 0; i < fRecordedOps.count(); ++i) {
        if (fRecordedOps[i]) {
            // We do not call flushState->setDrawOpArgs as this op list does not support GrDrawOps.
            fRecordedOps[i]->prepare(flushState);
        }
    }
}

bool GrTextureOpList::executeOps(GrOpFlushState* flushState) {
    if (0 == fRecordedOps.count()) {
        return false;
    }

    for (int i = 0; i < fRecordedOps.count(); ++i) {
        // We do not call flushState->setDrawOpArgs as this op list does not support GrDrawOps.
        fRecordedOps[i]->execute(flushState);
    }

    fGpu->finishOpList();
    return true;
}

void GrTextureOpList::reset() {
    fRecordedOps.reset();
}

////////////////////////////////////////////////////////////////////////////////

bool GrTextureOpList::copySurface(GrResourceProvider* resourceProvider,
                                  GrSurfaceProxy* dst,
                                  GrSurfaceProxy* src,
                                  const SkIRect& srcRect,
                                  const SkIPoint& dstPoint) {
    std::unique_ptr<GrOp> op = GrCopySurfaceOp::Make(resourceProvider, dst, src, srcRect, dstPoint);
    if (!op) {
        return false;
    }
#ifdef ENABLE_MDB
    this->addDependency(src);
#endif

    // See the comment in GrRenderTargetOpList about why we pass the invalid ID here.
    this->recordOp(std::move(op),
                   GrGpuResource::UniqueID::InvalidID(),
                   GrSurfaceProxy::UniqueID::InvalidID());
    return true;
}

void GrTextureOpList::recordOp(std::unique_ptr<GrOp> op,
                               GrGpuResource::UniqueID resourceUniqueID,
                               GrSurfaceProxy::UniqueID proxyUniqueID) {
    // A closed GrOpList should never receive new/more ops
    SkASSERT(!this->isClosed());

    GR_AUDIT_TRAIL_ADD_OP(fAuditTrail, op.get(), resourceUniqueID, proxyUniqueID);
    GrOP_INFO("Re-Recording (%s, B%u)\n"
        "\tBounds LRTB (%f, %f, %f, %f)\n",
        op->name(),
        op->uniqueID(),
        op->bounds().fLeft, op->bounds().fRight,
        op->bounds().fTop, op->bounds().fBottom);
    GrOP_INFO(SkTabString(op->dumpInfo(), 1).c_str());
    GR_AUDIT_TRAIL_OP_RESULT_NEW(fAuditTrail, op.get());

    fRecordedOps.emplace_back(std::move(op));
}
