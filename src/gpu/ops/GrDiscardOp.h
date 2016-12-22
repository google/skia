/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrDiscardOp_DEFINED
#define GrDiscardOp_DEFINED

#include "GrGpu.h"
#include "GrOp.h"
#include "GrOpFlushState.h"
#include "GrRenderTarget.h"

class GrDiscardOp final : public GrOp {
public:
    DEFINE_OP_CLASS_ID
    static sk_sp<GrOp> Make(GrRenderTarget* rt) { return sk_sp<GrOp>(new GrDiscardOp(rt)); }

    const char* name() const override { return "Discard"; }

    SkString dumpInfo() const override {
        SkString string;
        string.printf("RT: %d", fRenderTarget.get()->uniqueID().asUInt());
        string.append(INHERITED::dumpInfo());
        return string;
    }

private:
    GrDiscardOp(GrRenderTarget* rt) : INHERITED(ClassID()), fRenderTarget(rt) {
        this->setBounds(SkRect::MakeIWH(rt->width(), rt->height()), HasAABloat::kNo,
                        IsZeroArea::kNo);
    }

    bool onCombineIfPossible(GrOp* that, const GrCaps& caps) override {
        return fRenderTarget.get() == that->cast<GrDiscardOp>()->fRenderTarget.get();
    }

    void onPrepare(GrOpFlushState*) override {}

    void onExecute(GrOpFlushState* state, const SkRect& /*bounds*/) override {
        state->commandBuffer()->discard(fRenderTarget.get());
    }

    GrPendingIOResource<GrRenderTarget, kWrite_GrIOType> fRenderTarget;

    typedef GrOp INHERITED;
};

#endif
