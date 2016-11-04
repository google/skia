/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrDiscardBatch_DEFINED
#define GrDiscardBatch_DEFINED

#include "GrBatch.h"
#include "GrBatchFlushState.h"
#include "GrGpu.h"
#include "GrRenderTarget.h"

class GrDiscardBatch final : public GrBatch {
public:
    DEFINE_BATCH_CLASS_ID

    GrDiscardBatch(GrRenderTarget* rt)
        : INHERITED(ClassID())
        , fRenderTarget(rt) {
        this->setBounds(SkRect::MakeIWH(rt->width(), rt->height()), HasAABloat::kNo,
                        IsZeroArea::kNo);
    }

    const char* name() const override { return "Discard"; }

    uint32_t renderTargetUniqueID() const override { return fRenderTarget.get()->uniqueID(); }
    GrRenderTarget* renderTarget() const override { return fRenderTarget.get(); }

    SkString dumpInfo() const override {
        SkString string;
        string.printf("RT: %d", fRenderTarget.get()->uniqueID());
        string.append(INHERITED::dumpInfo());
        return string;
    }

private:
    bool onCombineIfPossible(GrBatch* that, const GrCaps& caps) override {
        return this->renderTargetUniqueID() == that->renderTargetUniqueID();
    }

    void onPrepare(GrBatchFlushState*) override {}

    void onDraw(GrBatchFlushState* state) override {
        state->commandBuffer()->discard(fRenderTarget.get());
    }

    GrPendingIOResource<GrRenderTarget, kWrite_GrIOType> fRenderTarget;

    typedef GrBatch INHERITED;
};

#endif
