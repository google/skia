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

    // MDB TODO: replace the renderTargetContext with just the renderTargetProxy.
    // For now, we need the renderTargetContext for its accessRenderTarget powers.
    static std::unique_ptr<GrOp> Make(const GrFixedClip& clip, bool insideStencilMask,
                                      GrRenderTargetContext* rtc) {

        // MDB TODO: remove this. In this hybrid state we need to be sure the RT is instantiable
        // so it can carry the IO refs. In the future we will just get the proxy and
        // it carry the IO refs.
        if (!rtc->accessRenderTarget()) {
            return nullptr;
        }

        return std::unique_ptr<GrOp>(new GrClearStencilClipOp(clip, insideStencilMask, rtc));
    }

    const char* name() const override { return "ClearStencilClip"; }

    SkString dumpInfo() const override {
        SkString string("Scissor [");
        if (fClip.scissorEnabled()) {
            const SkIRect& r = fClip.scissorRect();
            string.appendf("L: %d, T: %d, R: %d, B: %d", r.fLeft, r.fTop, r.fRight, r.fBottom);
        }
        string.appendf("], IC: %d, rtID: %d proxyID: %d",
                       fInsideStencilMask,
                       fRenderTarget.get()->uniqueID().asUInt(),
                       fProxyUniqueID.asUInt());
        string.append(INHERITED::dumpInfo());
        return string;
    }

private:
    GrClearStencilClipOp(const GrFixedClip& clip, bool insideStencilMask,
                         GrRenderTargetContext* rtc)
            : INHERITED(ClassID())
            , fClip(clip)
            , fInsideStencilMask(insideStencilMask)
            , fProxyUniqueID(rtc->asSurfaceProxy()->uniqueID()) {
        const SkRect& bounds = fClip.scissorEnabled()
                                            ? SkRect::Make(fClip.scissorRect())
                                            : SkRect::MakeIWH(rtc->width(), rtc->height());
        this->setBounds(bounds, HasAABloat::kNo, IsZeroArea::kNo);

        fRenderTarget.reset(rtc->accessRenderTarget());
    }

    bool onCombineIfPossible(GrOp* t, const GrCaps& caps) override { return false; }

    void onPrepare(GrOpFlushState*) override {}

    void onExecute(GrOpFlushState* state) override {
        // MDB TODO: instantiate the renderTarget from the proxy in here
        state->commandBuffer()->clearStencilClip(fRenderTarget.get(), fClip, fInsideStencilMask);
    }

    const GrFixedClip                                    fClip;
    const bool                                           fInsideStencilMask;
    // MDB TODO: remove this. When the renderTargetProxy carries the refs this will be redundant.
    GrSurfaceProxy::UniqueID                             fProxyUniqueID;
    GrPendingIOResource<GrRenderTarget, kWrite_GrIOType> fRenderTarget;

    typedef GrOp INHERITED;
};

#endif
