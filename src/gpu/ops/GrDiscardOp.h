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

    // MDB TODO: replace the renderTargetContext with just the renderTargetProxy.
    // For now, we need the renderTargetContext for its accessRenderTarget powers.
    static std::unique_ptr<GrOp> Make(GrRenderTargetContext* rtc) {

        // MDB TODO: remove this. In this hybrid state we need to be sure the RT is instantiable
        // so it can carry the IO refs. In the future we will just get the proxy and
        // it carry the IO refs.
        if (!rtc->accessRenderTarget()) {
            return nullptr;
        }

        return std::unique_ptr<GrOp>(new GrDiscardOp(rtc));
    }

    const char* name() const override { return "Discard"; }

    SkString dumpInfo() const override {
        SkString string;
        string.printf("rtID: %d proxyID: %d ", fRenderTarget1.get()->uniqueID().asUInt(),
                                              fProxyUniqueID.asUInt());
        string.append(INHERITED::dumpInfo());
        return string;
    }

private:
    GrDiscardOp(GrRenderTargetContext* rtc)
        : INHERITED(ClassID())
        , fProxyUniqueID(rtc->asSurfaceProxy()->uniqueID()) {
        this->setBounds(SkRect::MakeIWH(rtc->width(), rtc->height()), HasAABloat::kNo,
                        IsZeroArea::kNo);

        fRenderTarget1.reset(rtc->accessRenderTarget());
    }

    bool onCombineIfPossible(GrOp* that, const GrCaps& caps) override {
        SkASSERT((fRenderTarget1.get() == that->cast<GrDiscardOp>()->fRenderTarget1.get()) ==
                 (fProxyUniqueID == that->cast<GrDiscardOp>()->fProxyUniqueID));
        return fRenderTarget1.get() == that->cast<GrDiscardOp>()->fRenderTarget1.get();
    }

    void onPrepare(GrOpFlushState*) override {}

    void onExecute(GrOpFlushState* state) override {
        // MDB TODO: instantiate the renderTarget from the proxy in here
        state->commandBuffer()->discard(fRenderTarget1.get());
    }

    // MDB TODO: remove this. When the renderTargetProxy carries the refs this will be redundant.
    GrSurfaceProxy::UniqueID                             fProxyUniqueID;
    GrPendingIOResource<GrRenderTarget, kWrite_GrIOType> fRenderTarget1;

    typedef GrOp INHERITED;
};

#endif
