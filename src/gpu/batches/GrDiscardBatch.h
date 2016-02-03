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
        fBounds = SkRect::MakeWH(SkIntToScalar(rt->width()), SkIntToScalar(rt->height()));
    }

    const char* name() const override { return "Discard"; }

    uint32_t renderTargetUniqueID() const override { return fRenderTarget.get()->getUniqueID(); }
    GrRenderTarget* renderTarget() const override { return fRenderTarget.get(); }

    SkString dumpInfo() const override {
        SkString string;
        string.printf("RT: %d", fRenderTarget.get()->getUniqueID());
        return string;
    }

private:
    bool onCombineIfPossible(GrBatch* that, const GrCaps& caps) override {
        return fRenderTarget == that->cast<GrDiscardBatch>()->fRenderTarget;
    }

    void onPrepare(GrBatchFlushState*) override {}

    void onDraw(GrBatchFlushState* state) override {
        state->gpu()->discard(fRenderTarget.get());
    }

    GrPendingIOResource<GrRenderTarget, kWrite_GrIOType> fRenderTarget;

    typedef GrBatch INHERITED;
};

#endif
