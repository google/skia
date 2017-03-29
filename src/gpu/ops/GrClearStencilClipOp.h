/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrClearStencilClipOp_DEFINED
#define GrClearStencilClipOp_DEFINED

#include "GrFixedClip.h"
#include "GrGpu.h"
#include "GrGpuCommandBuffer.h"
#include "GrOp.h"
#include "GrOpFlushState.h"
#include "GrRenderTarget.h"

class GrClearStencilClipOp final : public GrOp {
public:
    DEFINE_OP_CLASS_ID

    static std::unique_ptr<GrOp> Make(const GrFixedClip& clip, bool insideStencilMask,
                                      GrRenderTargetProxy* rtp) {
        return std::unique_ptr<GrOp>(new GrClearStencilClipOp(clip, insideStencilMask, rtp));
    }

    const char* name() const override { return "ClearStencilClip"; }

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
    GrClearStencilClipOp(const GrFixedClip& clip, bool insideStencilMask, GrRenderTargetProxy* rtp)
            : INHERITED(ClassID())
            , fClip(clip)
            , fInsideStencilMask(insideStencilMask)
            , fRenderTargetProxy(rtp) {
        const SkRect& bounds = fClip.scissorEnabled() ? SkRect::Make(fClip.scissorRect())
                                                      : SkRect::MakeIWH(rtp->width(), rtp->height());
        this->setBounds(bounds, HasAABloat::kNo, IsZeroArea::kNo);
    }

    bool onCombineIfPossible(GrOp* t, const GrCaps& caps) override { return false; }

    void onPrepare(GrOpFlushState*) override {}

    void onExecute(GrOpFlushState* state) override {
        GrRenderTarget* rt = fRenderTargetProxy.get()->instantiate(nullptr);
        if (!rt) {
            return;
        }

        state->commandBuffer()->clearStencilClip(rt, fClip, fInsideStencilMask);
    }

    const GrFixedClip fClip;
    const bool fInsideStencilMask;
    GrPendingIOResource<GrRenderTargetProxy, kWrite_GrIOType> fRenderTargetProxy;

    typedef GrOp INHERITED;
};

#endif
