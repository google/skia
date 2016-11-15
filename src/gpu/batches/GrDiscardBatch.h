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

    GrDiscardBatch(GrRenderTargetProxy* rtp)
        : INHERITED(ClassID())
        , fRenderTargetProxy(rtp) {
        this->setBounds(SkRect::MakeIWH(rtp->width(), rtp->height()), HasAABloat::kNo,
                        IsZeroArea::kNo);
    }

    const char* name() const override { return "Discard"; }

    // TODO: this needs to be updated to return GrSurfaceProxy::UniqueID
    GrSurfaceProxy::UniqueID renderTargetProxyUniqueID() const override {
        return fRenderTargetProxy.get()->uniqueID();
    }
    GrRenderTargetProxy* renderTargetProxy() const override { return fRenderTargetProxy.get(); }

    SkString dumpInfo() const override {
        SkString string;
        string.printf("RT: %d", fRenderTargetProxy.get()->uniqueID().asUInt());
        string.append(INHERITED::dumpInfo());
        return string;
    }

private:
    bool onCombineIfPossible(GrBatch* that, const GrCaps& caps) override {
        return this->renderTargetProxyUniqueID() == that->renderTargetProxyUniqueID();
    }

    void onPrepare(GrBatchFlushState*) override {}

    void onDraw(GrBatchFlushState* state, const SkRect& /*bounds*/) override {
        state->commandBuffer()->discard();
    }

    GrPendingIOResource<GrRenderTargetProxy, kWrite_GrIOType> fRenderTargetProxy;

    typedef GrBatch INHERITED;
};

#endif
