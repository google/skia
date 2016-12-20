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
    // Semi-usually the GrOpLists are already closed at this point, but sometimes Ganesh
    // needs to flush mid-draw. In that case, the SkGpuDevice's GrOpLists won't be closed
    // but need to be flushed anyway. Closing such GrOpLists here will mean new
    // GrOpLists will be created to replace them if the SkGpuDevice(s) write to them again.
    this->makeClosed();

    // Loop over the ops that haven't yet generated their geometry
    for (int i = 0; i < fRecordedOps.count(); ++i) {
        if (fRecordedOps[i]) {
            fRecordedOps[i]->prepare(flushState);
        }
    }
}

bool GrTextureOpList::executeOps(GrOpFlushState* flushState) {
    if (0 == fRecordedOps.count()) {
        return false;
    }

    for (int i = 0; i < fRecordedOps.count(); ++i) {
        fRecordedOps[i]->draw(flushState, fRecordedOps[i]->bounds());
    }

    fGpu->finishOpList();
    return true;
}

void GrTextureOpList::reset() {
    fRecordedOps.reset();
}

////////////////////////////////////////////////////////////////////////////////

bool GrTextureOpList::copySurface(GrSurface* dst,
                                  GrSurface* src,
                                  const SkIRect& srcRect,
                                  const SkIPoint& dstPoint) {
    sk_sp<GrOp> op = GrCopySurfaceOp::Make(dst, src, srcRect, dstPoint);
    if (!op) {
        return false;
    }
#ifdef ENABLE_MDB
    this->addDependency(src);
#endif

    this->recordOp(std::move(op));
    return true;
}

void GrTextureOpList::recordOp(sk_sp<GrOp> op) {
    // A closed GrOpList should never receive new/more ops
    SkASSERT(!this->isClosed());

    GR_AUDIT_TRAIL_ADD_OP(fAuditTrail, op.get());
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
