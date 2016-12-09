/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrClearStencilClipBatch_DEFINED
#define GrClearStencilClipBatch_DEFINED

#include "GrFixedClip.h"
#include "GrGpu.h"
#include "GrGpuCommandBuffer.h"
#include "GrOp.h"
#include "GrOpFlushState.h"
#include "GrRenderTarget.h"

class GrClearStencilClipBatch final : public GrOp {
public:
    DEFINE_OP_CLASS_ID

    GrClearStencilClipBatch(const GrFixedClip& clip,
                            bool insideStencilMask,
                            GrRenderTargetProxy* rtp)
        : INHERITED(ClassID())
        , fClip(clip)
        , fInsideStencilMask(insideStencilMask)
        , fRenderTargetProxy(rtp) {
        const SkRect& bounds = fClip.scissorEnabled() ? SkRect::Make(fClip.scissorRect())
                                                      : SkRect::MakeIWH(rtp->width(),
                                                                        rtp->height());
        this->setBounds(bounds, HasAABloat::kNo, IsZeroArea::kNo);
    }

    const char* name() const override { return "ClearStencilClip"; }

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
    bool onCombineIfPossible(GrOp* t, const GrCaps& caps, GrTextureProvider* texProvider) override { return false; }

    void onPrepare(GrOpFlushState*) override {}

    void onDraw(GrOpFlushState* state, const SkRect& /*bounds*/) override {
        GrRenderTarget* rt = fRenderTargetProxy.get()->instantiate(nullptr);
        if (!rt) {
            return;
        }

        state->commandBuffer()->clearStencilClip(rt, fClip, fInsideStencilMask);
    }

    const GrFixedClip                                         fClip;
    const bool                                                fInsideStencilMask;
    GrPendingIOResource<GrRenderTargetProxy, kWrite_GrIOType> fRenderTargetProxy;

    typedef GrOp INHERITED;
};

#endif
