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
#include "GrRenderTargetContext.h"

class GrDiscardOp final : public GrOp {
public:
    DEFINE_OP_CLASS_ID

    // MDB TODO: replace the renderTargetContext with just the renderTargetProxy.
    // For now, we need the renderTargetContext for its accessRenderTarget powers.
    static std::unique_ptr<GrOp> Make(GrRenderTargetContext* rtc) {
        return std::unique_ptr<GrOp>(new GrDiscardOp(rtc));
    }

    const char* name() const override { return "Discard"; }

    SkString dumpInfo() const override {
        SkString string;
        string.append(INHERITED::dumpInfo());
        string.printf("rtID: %d proxyID: %d ", fRenderTargetProxy.get()->uniqueID().asUInt(),
                                               fProxyUniqueID.asUInt());
        return string;
    }

private:
    GrDiscardOp(GrRenderTargetContext* rtc)
        : INHERITED(ClassID())
        , fProxyUniqueID(rtc->asSurfaceProxy()->uniqueID()) {
        this->setBounds(SkRect::MakeIWH(rtc->width(), rtc->height()), HasAABloat::kNo,
                        IsZeroArea::kNo);

        fRenderTargetProxy.reset(rtc->asRenderTargetProxy());
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

    // MDB TODO: remove this. When the renderTargetProxy carries the refs this will be redundant.
    GrSurfaceProxy::UniqueID                                  fProxyUniqueID;
    GrPendingIOResource<GrRenderTargetProxy, kWrite_GrIOType> fRenderTargetProxy;

    typedef GrOp INHERITED;
};

#endif
