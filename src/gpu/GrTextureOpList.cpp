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

#include "batches/GrCopySurfaceBatch.h"

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

    SkDebugf("batches (%d):\n", fRecordedBatches.count());
    for (int i = 0; i < fRecordedBatches.count(); ++i) {
        SkDebugf("*******************************\n");
        SkDebugf("%d: %s\n", i, fRecordedBatches[i]->name());
        SkString str = fRecordedBatches[i]->dumpInfo();
        SkDebugf("%s\n", str.c_str());
        const SkRect& clippedBounds = fRecordedBatches[i]->bounds();
        SkDebugf("ClippedBounds: [L: %.2f, T: %.2f, R: %.2f, B: %.2f]\n",
                    clippedBounds.fLeft, clippedBounds.fTop, clippedBounds.fRight,
                    clippedBounds.fBottom);
    }
}
#endif

void GrTextureOpList::prepareBatches(GrBatchFlushState* flushState) {
    // Semi-usually the GrOpLists are already closed at this point, but sometimes Ganesh
    // needs to flush mid-draw. In that case, the SkGpuDevice's GrOpLists won't be closed
    // but need to be flushed anyway. Closing such GrOpLists here will mean new
    // GrOpLists will be created to replace them if the SkGpuDevice(s) write to them again.
    this->makeClosed();

    // Loop over the batches that haven't yet generated their geometry
    for (int i = 0; i < fRecordedBatches.count(); ++i) {
        if (fRecordedBatches[i]) {
            fRecordedBatches[i]->prepare(flushState);
        }
    }
}

bool GrTextureOpList::drawBatches(GrBatchFlushState* flushState) {
    if (0 == fRecordedBatches.count()) {
        return false;
    }

    for (int i = 0; i < fRecordedBatches.count(); ++i) {
        fRecordedBatches[i]->draw(flushState, fRecordedBatches[i]->bounds());
    }

    fGpu->finishOpList();
    return true;
}

void GrTextureOpList::reset() {
    fRecordedBatches.reset();
}

////////////////////////////////////////////////////////////////////////////////

bool GrTextureOpList::copySurface(GrSurface* dst,
                                  GrSurface* src,
                                  const SkIRect& srcRect,
                                  const SkIPoint& dstPoint) {
    GrOp* batch = GrCopySurfaceBatch::Create(dst, src, srcRect, dstPoint);
    if (!batch) {
        return false;
    }
#ifdef ENABLE_MDB
    this->addDependency(src);
#endif

    this->recordBatch(batch);
    batch->unref();
    return true;
}

void GrTextureOpList::recordBatch(GrOp* batch) {
    // A closed GrOpList should never receive new/more batches
    SkASSERT(!this->isClosed());

    GR_AUDIT_TRAIL_ADDBATCH(fAuditTrail, batch);
    GrOP_INFO("Re-Recording (%s, B%u)\n"
        "\tBounds LRTB (%f, %f, %f, %f)\n",
        batch->name(),
        batch->uniqueID(),
        batch->bounds().fLeft, batch->bounds().fRight,
        batch->bounds().fTop, batch->bounds().fBottom);
    GrOP_INFO(SkTabString(batch->dumpInfo(), 1).c_str());
    GR_AUDIT_TRAIL_BATCHING_RESULT_NEW(fAuditTrail, batch);

    fRecordedBatches.emplace_back(sk_ref_sp(batch));
}
