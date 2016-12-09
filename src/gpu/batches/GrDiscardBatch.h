/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrDiscardBatch_DEFINED
#define GrDiscardBatch_DEFINED

#include "GrGpu.h"
#include "GrOp.h"
#include "GrOpFlushState.h"
#include "GrRenderTarget.h"

class GrDiscardBatch final : public GrOp {
public:
    DEFINE_OP_CLASS_ID
    static sk_sp<GrOp> Make(GrRenderTarget* rt) { return sk_sp<GrOp>(new GrDiscardBatch(rt)); }

    const char* name() const override { return "Discard"; }

    // TODO: this needs to be updated to return GrSurfaceProxy::UniqueID
    GrGpuResource::UniqueID renderTargetUniqueID() const override {
        return fRenderTarget.get()->uniqueID();
    }

    SkString dumpInfo() const override {
        SkString string;
        string.printf("RT: %d", fRenderTarget.get()->uniqueID().asUInt());
        string.append(INHERITED::dumpInfo());
        return string;
    }

private:
    GrDiscardBatch(GrRenderTarget* rt)
        : INHERITED(ClassID())
        , fRenderTarget(rt) {
        this->setBounds(SkRect::MakeIWH(rt->width(), rt->height()), HasAABloat::kNo,
                        IsZeroArea::kNo);
    }

    bool onCombineIfPossible(GrOp* that, const GrCaps& caps) override {
        return this->renderTargetUniqueID() == that->renderTargetUniqueID();
    }

    void onPrepare(GrOpFlushState*) override {}

    void onDraw(GrOpFlushState* state, const SkRect& /*bounds*/) override {
        state->commandBuffer()->discard(fRenderTarget.get());
    }

    GrPendingIOResource<GrRenderTarget, kWrite_GrIOType> fRenderTarget;

    typedef GrOp INHERITED;
};

#endif
