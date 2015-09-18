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
    GrDiscardBatch(GrRenderTarget* rt)
        : fRenderTarget(rt) {
        this->initClassID<GrDiscardBatch>();
        fBounds = SkRect::MakeWH(SkIntToScalar(rt->width()), SkIntToScalar(rt->height()));
    }

    const char* name() const override { return "Discard"; }

    uint32_t renderTargetUniqueID() const override { return fRenderTarget.get()->getUniqueID(); }

    SkString dumpInfo() const override {
        SkString string;
        string.printf("RT: 0x%p", fRenderTarget.get());
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
};

#endif
