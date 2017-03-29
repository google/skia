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
#include "GrRenderTargetProxy.h"

class GrDiscardOp final : public GrOp {
public:
    DEFINE_OP_CLASS_ID
    static std::unique_ptr<GrOp> Make(GrRenderTargetProxy* rtp) {
        return std::unique_ptr<GrOp>(new GrDiscardOp(rtp));
    }

    const char* name() const override { return "Discard"; }

    SkString dumpInfo() const override {
        SkString string;
        string.printf("RT: %d", fRenderTargetProxy.get()->uniqueID().asUInt());
        string.append(INHERITED::dumpInfo());
        return string;
    }

private:
    GrDiscardOp(GrRenderTargetProxy* rtp) : INHERITED(ClassID()), fRenderTargetProxy(rtp) {
        this->setBounds(SkRect::MakeIWH(rtp->width(), rtp->height()), HasAABloat::kNo,
                        IsZeroArea::kNo);
    }

    bool onCombineIfPossible(GrOp* that, const GrCaps& caps) override {
        return fRenderTargetProxy.get()->uniqueID() ==
               that->cast<GrDiscardOp>()->fRenderTargetProxy.get()->uniqueID();
    }

    void onPrepare(GrOpFlushState*) override {}

    void onExecute(GrOpFlushState* state) override {
        GrRenderTarget* rt = fRenderTargetProxy.get()->instantiate(nullptr);
        if (!rt) {
            return;
        }

        state->commandBuffer()->discard(rt);
    }

    GrPendingIOResource<GrRenderTargetProxy, kWrite_GrIOType> fRenderTargetProxy;

    typedef GrOp INHERITED;
};

#endif
