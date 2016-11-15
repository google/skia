/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrClearStencilClipBatch_DEFINED
#define GrClearStencilClipBatch_DEFINED

#include "GrBatch.h"
#include "GrBatchFlushState.h"
#include "GrFixedClip.h"
#include "GrGpu.h"
#include "GrGpuCommandBuffer.h"
#include "GrRenderTarget.h"

class GrClearStencilClipBatch final : public GrBatch {
public:
    DEFINE_BATCH_CLASS_ID

    GrClearStencilClipBatch(const GrFixedClip& clip,
                            bool insideStencilMask,
                            GrRenderTargetProxy* rtp)
        : INHERITED(ClassID())
        , fClip(clip)
        , fInsideStencilMask(insideStencilMask)
        , fRenderTargetProxy(rtp) {
        const SkRect& bounds = fClip.scissorEnabled() ? SkRect::Make(fClip.scissorRect())
                                                      : SkRect::MakeIWH(rtp->width(), rtp->height());
        this->setBounds(bounds, HasAABloat::kNo, IsZeroArea::kNo);
    }

    const char* name() const override { return "ClearStencilClip"; }

    // TODO: this needs to be updated to return GrSurfaceProxy::UniqueID
    GrSurfaceProxy::UniqueID renderTargetProxyUniqueID() const override {
        return fRenderTargetProxy.get()->uniqueID();
    }
    GrRenderTargetProxy* renderTargetProxy() const override { return fRenderTargetProxy.get(); }

    SkString dumpInfo() const override {
        SkString string("Scissor [");
        if (fClip.scissorEnabled()) {
            const SkIRect& r = fClip.scissorRect();
            string.appendf("L: %d, T: %d, R: %d, B: %d", r.fLeft, r.fTop, r.fRight, r.fBottom);
        }
        string.appendf("], IC: %d, RT: %d", fInsideStencilMask,
                                            fRenderTargetProxy.get()->uniqueID().asUInt());
        string.append(INHERITED::dumpInfo());
        return string;
    }

private:
    bool onCombineIfPossible(GrBatch* t, const GrCaps& caps) override { return false; }

    void onPrepare(GrBatchFlushState*) override {}

    void onDraw(GrBatchFlushState* state, const SkRect& /*bounds*/) override {
        state->commandBuffer()->clearStencilClip(fClip, fInsideStencilMask);
    }

    const GrFixedClip                                         fClip;
    const bool                                                fInsideStencilMask;
    GrPendingIOResource<GrRenderTargetProxy, kWrite_GrIOType> fRenderTargetProxy;

    typedef GrBatch INHERITED;
};

#endif
