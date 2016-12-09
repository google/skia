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

    GrDiscardBatch(GrRenderTargetProxy* rtp)
        : INHERITED(ClassID())
        , fRenderTargetProxy(rtp) {
        this->setBounds(SkRect::MakeIWH(rtp->width(), rtp->height()), HasAABloat::kNo,
                        IsZeroArea::kNo);
    }

    const char* name() const override { return "Discard"; }

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
    bool onCombineIfPossible(GrOp* that, const GrCaps& caps, GrTextureProvider* texProvider) override {
        return this->renderTargetProxyUniqueID() == that->renderTargetProxyUniqueID();
    }

    void onPrepare(GrOpFlushState*) override {}

    void onDraw(GrOpFlushState* state, const SkRect& /*bounds*/) override {
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
