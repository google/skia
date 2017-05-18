/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrDiscardOp_DEFINED
#define GrDiscardOp_DEFINED

#include "GrOp.h"
#include "GrOpFlushState.h"
#include "GrRenderTarget.h"
#include "GrRenderTargetProxy.h"

class GrDiscardOp final : public GrOp {
public:
    DEFINE_OP_CLASS_ID

    static std::unique_ptr<GrOp> Make(GrRenderTargetProxy* proxy) {
        return std::unique_ptr<GrOp>(new GrDiscardOp(proxy));
    }

    const char* name() const override { return "Discard"; }

    SkString dumpInfo() const override {
        SkString string;
        string.append(INHERITED::dumpInfo());
        string.printf("proxyID: %d\n", fRenderTargetProxy.get()->uniqueID().asUInt());
        return string;
    }

private:
    GrDiscardOp(GrRenderTargetProxy* proxy)
        : INHERITED(ClassID())
        , fRenderTargetProxy(proxy) {
        this->setBounds(SkRect::MakeIWH(proxy->width(), proxy->height()),
                        HasAABloat::kNo, IsZeroArea::kNo);
    }

    bool onCombineIfPossible(GrOp* that, const GrCaps& caps) override { return false; }

    void onPrepare(GrOpFlushState*) override {}

    void onExecute(GrOpFlushState* state) override {
        GrRenderTarget* rt = fRenderTargetProxy.get()->instantiateRenderTarget(
                                                                    state->resourceProvider());
        if (!rt) {
            return;
        }

        state->commandBuffer()->discard(rt);
    }

    GrPendingIOResource<GrRenderTargetProxy, kWrite_GrIOType> fRenderTargetProxy;

    typedef GrOp INHERITED;
};

#endif
